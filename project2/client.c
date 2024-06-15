#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "client.h"

void* receive_updates(void* arg) {
    int sock = *(int*)arg;
    DGIST dgist;

    while (1) {
        if (receive_update(sock, &dgist) <= 0) {
            printf("Server closed the connection.\n");
            close(sock);
            pthread_exit(NULL);
        }

        // Print the received update
        printf("==========MAP==========\n");

        for (int i = 0; i < MAP_ROW; i++) {
            for (int j = 0; j < MAP_COL; j++) {
                int player_found = 0;
                
                // Check if any player is at this position
                for (int k = 0; k < MAX_CLIENTS; k++) {
                    if (dgist.players[k].row == i && dgist.players[k].col == j) {
                        printf("O ");
                        player_found = 1;
                        break;
                    }
                }

                if (!player_found) {
                    switch (dgist.map[i][j].item.status) {
                        case nothing:
                            printf("- ");
                            break;
                        case item:
                            printf("%d ", dgist.map[i][j].item.score);
                            break;
                        case trap:
                            printf("x ");
                            break;
                    }
                }
            }
            printf("\n");
        }

        printf("========PLAYERS========\n");
        for (int i = 0; i < MAX_CLIENTS; i++) {
            printf("Player %d: Position (%d, %d), Score: %d, Bombs: %d\n",
                i + 1, dgist.players[i].row, dgist.players[i].col, dgist.players[i].score, dgist.players[i].bomb);
        }
        printf("=======================\n");
    }

    return NULL;
}

int connect_to_server(const char *server_ip, int server_port) {
    int sock;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(server_port);

    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    printf("Connected to the server.\n");
    return sock;
}

int send_action(int sock, ClientAction *cAction) {
    return send(sock, cAction, sizeof(ClientAction), 0);
}

int receive_update(int sock, DGIST *dgist) {
    return recv(sock, dgist, sizeof(DGIST), 0);
}
