/*
Created on Sat Feb 19 06:20:53 2022
@author: Swapnil Vivek Kulkarni

Problem: Server for key-value store
description: Implement a server that handles get and put requests to an
in-memory key-value store. Each request is a single-line message of the
following format: put <key> <value> get <key> The key cannot contain a space.
For put, the server must store the value under the specified key.
For get, the server must respond with the value corresponding to that key.

*/

#ifndef __Server_H__
#define __Server_H__

#include <string.h>

#include <map>

using namespace std;

// Macros
#define SERVER_PORT 9000
#define LISTEN_BACKLOG 5
#define NO_OF_CLIENTS 10
#define MAX_BUFFER_SIZE 1024
#define CMD_PUT "put"
#define CMD_GET "get"

struct client {
  int file_des;
};

class Server_data {
  std::map<string, string> mapOfKeyVal;

 public:
  int total_client;
  struct client client_list[NO_OF_CLIENTS];

  void update_server_data(string key, string value) {
    auto iterFind = mapOfKeyVal.find(key);
    if (iterFind != mapOfKeyVal.end()) {
      mapOfKeyVal[iterFind->first] = value;
    } else {
      mapOfKeyVal.insert(std::pair<string, string>(key, value));
    }
  }

  string get_server_data(const string &key) {
    auto iterFind = mapOfKeyVal.find(key);
    if (iterFind != mapOfKeyVal.end()) {
      return iterFind->second + "\n";
    } else {
      return "ERROR: Not Found \n";
    }
  }
};

class Server {
 private:
  Server_data server_data;
  int m_listenfd = 0;

 public:
  Server();
  ~Server();

  // Function declarations
  int server_create_socket();
  int server_build_fdsets(fd_set *readfds, fd_set *writefds, fd_set *exceptfds);
  int server_select(int max_fd, fd_set *readfds, fd_set *writefds);
  int server_new_client_handle(int listen_fd, int *new_socket_fd);
  int server_recv_from_client(int socket_client, char *recv_msg);
  int server_send_to_client(int socket_client, char *send_msg);
  void server_delete_client(int socket_fd_del);
  int process_recv_data(int socket, char *buffer);
  void parse_server_command(char *cmd, string *pCmd, string *pKey,
                            string *pValue);
  void server_add_new_client(struct sockaddr_in client_info, int new_socket_fd);
  void cleanup();
};

#endif  // __Server_H__
