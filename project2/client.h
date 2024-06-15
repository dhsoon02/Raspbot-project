#ifndef CLIENT_H
#define CLIENT_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include "server.h" // server.h를 포함하여 중복 정의 방지

// 클라이언트 관련 정의
int connect_to_server(const char *server_ip, int server_port);
int send_action(int sock, ClientAction *cAction);
int receive_update(int sock, DGIST *dgist);
void* receive_updates(void* arg);

#endif // CLIENT_H
