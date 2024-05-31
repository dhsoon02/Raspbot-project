#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "server.h"

#define SERVER_IP "127.0.0.1" // 서버 IP 주소
#define SERVER_PORT 87301 // 서버 포트 번호

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    ClientAction clientAction;

    // 소켓 생성
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n 소켓 생성 실패 \n");
        return -1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT);

    // 서버 주소 변환
    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        printf("\n 서버 주소 변환 실패 \n");
        return -1;
    }

    // 서버에 연결
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\n 서버 연결 실패 \n");
        return -1;
    }

    // 예시: (1, 2) 위치로 이동하고 함정 설치
    clientAction.row = 1;
    clientAction.col = 2;
    clientAction.action = setBomb;

    // 서버에 데이터 전송
    send(sock, &clientAction, sizeof(ClientAction), 0);
    printf("서버에 데이터 전송 완료: row=%d, col=%d, action=%d\n", clientAction.row, clientAction.col, clientAction.action);

    // 서버로부터 게임 상태 정보 수신
    DGIST dgist;
    int valread = read(sock, &dgist, sizeof(DGIST));
    if (valread > 0) {
        printf("서버로부터 게임 상태 정보 수신 완료\n");
        // 수신한 게임 상태 정보를 활용한 로직 작성
        // 예시: 첫 번째 플레이어의 점수 출력
        printf("Player 1 score: %d\n", dgist.players[0].score);
    }

    // 소켓 닫기
    close(sock);

    return 0;
}
