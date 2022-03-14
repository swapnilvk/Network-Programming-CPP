/*Created on Sat Feb 19 06:20:53 2022
@author: Swapnil Vivek Kulkarni

Problem: Server for key-value store
description: Implement a server that handles get and put requests to an in-memory key-value store. Each request is a single-line message of the following format:
put <key> <value>
get <key>
The key cannot contain a space.
For put, the server must store the value under the specified key.
For get, the server must respond with the value corresponding to that key.
*/
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <bits/stdc++.h>
#include "server.h"

int main() {
    fd_set      readfds;
    fd_set      writefds;
    fd_set      exceptfds;
    int         max_fd = 0;
    Server      obj_server;

    while (1) {
        max_fd = obj_server.server_build_fdsets(&readfds, &writefds, &exceptfds);
        obj_server.server_select(max_fd, &readfds, &writefds);
    }
    cout << "Bye From server" << endl;

    return 0;
}

int Server::server_build_fdsets(fd_set *readfds, fd_set *writefds, fd_set *exceptfds) {
    int max_fd = m_listenfd;

    FD_ZERO(readfds);
    FD_SET(m_listenfd, readfds);
    FD_SET(STDIN_FILENO,readfds);
    fcntl(STDIN_FILENO,F_SETFL,O_NONBLOCK);

    for(int i = 0; i<server_data.total_client; i++) {
        FD_SET(server_data.client_list[i].file_des,readfds);
        max_fd++;
    }
    return max_fd;
}

//Constructor
Server::Server() {   
    cout<<"Server Started !!!"<<endl;
    
    if(server_create_socket() != 0) {
        perror("ERROR : creation socket failed");
        exit(0);
    }
}

//Destructor
Server::~Server() {
    cleanup();
}

//create the server socket
int Server::server_create_socket() {
    struct sockaddr_in server_addr;

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT); 
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if((m_listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("ERROR : socket creation failed");
        return -1;
    }

    if( 0!=bind(m_listenfd,(struct sockaddr*)&server_addr,sizeof(struct sockaddr))) {
        perror("ERROR : socket bind failed");
        return -1;
    }

    if(0!=listen(m_listenfd, LISTEN_BACKLOG)) {
        perror("ERROR : socket listen failed");
        return -1;
    }

    return 0;
}

//select based processing 
int Server::server_select(int max_fd, fd_set *readfds, fd_set *writefds) {
    int     new_socket_fd = 0;
    char    recv_msgg[MAX_BUFFER_SIZE] ;
    char    send_buff[MAX_BUFFER_SIZE] ;
    int     listen_fd = m_listenfd;

    memset(recv_msgg, 0 ,sizeof(recv_msgg));
    memset(send_buff, 0 ,sizeof(send_buff));

    int action = select(max_fd+1,readfds,writefds,NULL,NULL);

    if(action == -1 || action == 0) {
        perror("ERROR: select");
        exit(0);
    }

    //check the server listenfd
     if(FD_ISSET(listen_fd,readfds)) {
        server_new_client_handle(listen_fd,&new_socket_fd);            
    }           

    //check the data from stdin and send message to all connected clients
    if(FD_ISSET(STDIN_FILENO,readfds)) {
         if(read(0,send_buff,sizeof(send_buff))>0) {
            for(int i = 0;i<server_data.total_client;i++)
                server_send_to_client(server_data.client_list[i].file_des,send_buff);
        }             
    }
     
    for(int i = 0; i<server_data.total_client; i++) {    
           if(FD_ISSET(server_data.client_list[i].file_des,readfds)) {
               server_recv_from_client(server_data.client_list[i].file_des, recv_msgg); 
        } 
    } 
    return 0;
}

//Accept the connection to server
int Server::server_new_client_handle(int listen_fd, int *new_socket_fd) {
    struct sockaddr_in client_addr;
    int len = sizeof(struct sockaddr);

    bzero(&client_addr,sizeof(client_addr));

    if((*new_socket_fd = accept(listen_fd, (struct sockaddr*)&client_addr,(socklen_t *) &len)) < 0) {
        perror("ERROR :accept failed");
        return -1;
    }

    server_add_new_client(client_addr ,*new_socket_fd);

    return 0;
}

//Adding a new client to the server
void Server::server_add_new_client(struct sockaddr_in client_info, int new_socket_fd) {
    char buffer[MAX_BUFFER_SIZE] = {0};
    
    server_recv_from_client(new_socket_fd, buffer);
   
    if(server_data.total_client >=NO_OF_CLIENTS) {
        perror("ERROR : no more space for client to save");
    }

    //populate the new client data 
    server_data.client_list[server_data.total_client].file_des=new_socket_fd;
    server_data.total_client++;
}

//Receiving socket data from clients
int Server::server_recv_from_client(int client_socket, char *recv_msg) {
    int read_bytes = 0;
    memset(recv_msg,0,strlen(recv_msg));
    
    if((read_bytes = recv(client_socket, recv_msg, MAX_BUFFER_SIZE, 0)) > 0) {
        process_recv_data(client_socket, recv_msg);
    } else if(read_bytes == 0) {
           cout<<"Client Disconnected"<<endl;
           server_delete_client(client_socket);  
    } else {
            perror("ERROR: recv failed");
    }
    return 0;
}

//processing the received data from clients
int Server::process_recv_data(int socket,char*buffer) {
    string command;
    string key;
    string value;

    cout<<"Request: "<<buffer;
    parse_server_command(buffer, &command, &key, &value);

    if(command.compare(CMD_PUT) == 0) {
        memset(buffer,0,sizeof(buffer));
        server_data.update_server_data(key, value);
    } else if(command.compare(CMD_GET) == 0) {
        memset(buffer,0,sizeof(buffer));
        string msg;
        msg = server_data.get_server_data(key);
        strcpy(buffer, msg.c_str());
        server_send_to_client(socket,buffer);
    } else {
        strcpy(buffer, "ERROR: Invalid Command\n");
        server_send_to_client(socket,buffer);
    }
    return 0;
}

int Server::server_send_to_client(int client_socket, char *send_msg) {
    int write_bytes = 0;
    int len  =strlen(send_msg);
    if((write_bytes = send(client_socket, send_msg, len, 0)) > 0) {
        cout<<"Response:"<<send_msg;
    } else {
        perror("Error : send failed");
        return -1;
    }

    return write_bytes;
}

//Detete the client data on client exit
void Server::server_delete_client(int socket_fd_del) {
    cout<<"Socket deleted  = "<<socket_fd_del<<endl;
    close(socket_fd_del);
}

void Server::parse_server_command(char * cmd, string *pCmd, string *pKey, string *pValue ) {
    string key;
    string value;
    int pos = 0;
    // Used to split string around spaces.
    istringstream ss(cmd);
  
    string word; // for storing each word
    while (ss >> word) 
    {   
        if(pCmd && pos == 0) {
            *pCmd = word;
        } else if (pKey && pos == 1) {
            *pKey = word;
        } else {
            if(pValue) {
                *pValue+= word;
                *pValue +=" ";
            }
        }
        pos++;
    }
}

void  Server::cleanup() {
    close(m_listenfd);
    for(int i = 0; i<server_data.total_client; i++) {
           close(server_data.client_list[i].file_des);
    }
}
