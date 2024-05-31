#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "ReadQR.h"

#define MAX_CLIENTS 2
#define _MAP_ROW 4
#define _MAP_COL 4
#define MAP_ROW _MAP_ROW + 1
#define MAP_COL _MAP_COL + 1
#define MAP_SIZE = MAP_COL*MAP_ROW

const int MAX_SCORE = 4; // Item max score
const int SETTING_PERIOD = 20; //Boradcast & Item generation period
const int INITIAL_ITEM = 10; //Initial number of item
const int INITIAL_BOMB = 4; //The number of bomb for each user
const int SCORE_DEDUCTION = 2; //The amount of score deduction due to bomb

//섹션1 서버가 여러분에게 주는 구조체에요.

//여기서 row, col을 통해 상대방의 위치 정보를 알 수 있어요.
//만약 전략을 설정하는데 상대방의 점수와 trap 개수가 필요하다면 score, bomb을 통해 알 수 있어요.
typedef struct{
    int socket;
    struct sockaddr_in address;
	int row;
	int col;
	int score;
	int bomb;
} client_info;

//nothing은 아무것도 없는 상태에요.
//item은 item이 있는 상태에요.
//trap은 trap이 있는 상태에요.
enum Status{
	nothing, //0
	item, //1
	trap //2
};

typedef struct{
	enum Status status;
	int score;
} Item;

//이 구조체의 row, col을 통해서 위치를, item의 status와 score를 통해 아이템이 있는지, trap이 있는지 판별하세요.
typedef struct {
	int row;
	int col;
	Item item; 
} Node;

//여러분은 서버에서 이 구조체를 받아올거에요. 
//players는 여러분과 상대방의 정보가 들어있는데 이 중에서 상대방의 정보만 잘 골라서 얻어야 해요.
//map은 전체 게임 map이 들어가있어요. Node는 intersection(교차점을 의미해요)
typedef struct{
	client_info players[MAX_CLIENTS];
	Node map[MAP_ROW][MAP_COL];
} DGIST;

//섹션2 여러분이 서버에게 주어야 하는 구조체에 대한 설명이에요.

//방문한 교차점에 함정을 설치하고 싶으면 1, 그렇지 않으면 0으로 설정하면 돼요.
enum Action{
	move, //0
	setBomb, //1
};

//서버에게 소켓을 통해 전달하는 구조체에요.
//QR에서 읽어온 숫자 2개를 row, col에 넣고 위의 enum Action을 참고해서 action 값을 설정하세요.
typedef struct{
	int row;
	int col;
	enum Action action;
} ClientAction;


////////////////////////////////////////////////////////////////////////
void print_map(DGIST dgist, int posX, int posY) {
    for (int i=4; i>-1; i--) {
        for (int j=0; j<5; j++) {
            if (i == posY && j == posX) {
                printf("  M");
            }
            else if (dgist.map[i][j].item.status == nothing) {
                printf("  -");
            }
            else if (dgist.map[i][j].item.status == item) {
                printf("%3d", dgist.map[i][j].item.score);
            }
            else if (dgist.map[i][j].item.status == trap) {
                printf("  B");
            }
            
        }
        printf("\n");
    }
}

int abs(int num) {
    if (num < 0) {
        return -num;
    }
    return num;
}


int main(int argc, char *argv[]) {

    printf("Client started\n");

    int clientfd;
    struct sockaddr_in address;

    const int PORT = atoi(argv[1]);


    if ( (clientfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        printf("Socket creation failed\n");
        return 1;
    }
    printf("Socket created successfully.\n");

    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);


	// IP 주소 변환
    if (inet_pton(AF_INET, argv[2], &address.sin_addr) <= 0) {
        printf("Invalid address/ Address not supiported\n");
        return -1;
    }
    printf("Address converted successfully.\n");

    if ( connect(clientfd, (struct sockaddr *)&address, sizeof(address)) < 0 ) {
        printf("Connect failed\n");
        return 1;
    }
    printf("Client connection success\n");

    ClientAction cAction;
    cAction.row = 0;
    cAction.col = 0;
    cAction.action = move;

    DGIST dgist;

    int valSend;
    int valRead;

	char* data;
	
	int loop = 0;

	// 알고리즘에서 사용되는 변수
    //
    // 다음에 향할 방향을 정하기 위한 변수
    // [0][0]:+x , [0][1]:-x , [1][0]:+y , [1][1]:-y
    double plan[2][2];
    int ind = 0;

	int posX = 0, posY = 0;
	int direction = 0;      // 0:+x , 1:-x , 2:+y , 3:-y

    double value;
    double dist;

    while (1) {
		if (loop % 6 == 0) {
            cAction.action = setBomb;
        }else {
			cAction.action = move;
		}
		//QRcode recognition & parsing

		if ((data = readQR()) != 0 ){
			cAction.row = data[0] - '0';
			cAction.col = data[1] - '0';
			if ((valSend = send(clientfd, &cAction, sizeof(ClientAction), 0)) == 0) {
            	printf("Sending failed");
            	break;
        	} else {
				printf("Client sending success\n");
			}

		}
        if ((valRead = read(clientfd, &dgist, sizeof(DGIST))) == 0) {
            printf("Reading failed");
            break;
        } else {
			printf("Client Receive success\n");
		}


		// 알고리즘
        for (int i=0; i<2; i++) {
            for (int j=0; j<2; j++) {
                plan[i][j] = 0.0;
            }
        }


        for (int i=0; i<5; i++) {
            for (int j=0; j<5; j++) {
                value = 0;

                if (dgist.map[i][j].item.status == nothing) { value = 0; continue; }
                else if (dgist.map[i][j].item.status == item) { value = dgist.map[i][j].item.score; }
                else if (dgist.map[i][j].item.status == trap) { value = 0; continue; }

                dist = (posX - j)*(posX - j) + (posY - i)*(posY - i);

                if (dist == 0) {
                    continue;
                }

                dist *= dist;
                value /= dist;


                if (posX < j) {
                    plan[0][0] += value;
                }
                else if (posX > j) {
                    plan[0][1] += value;
                }
                else {
                    if (posY < i) {
                        plan[1][0] += value;
                    }
                    else if (posY > i) {
                        plan[1][1] += value;
                    }
                    plan[0][0] += value / 1000;
                    plan[0][1] += value / 1000;
                }



                if (posY < i) {
                    plan[1][0] += value;
                }
                else if (posY > i) {
                    plan[1][1] += value;
                }
                else {
                    if (posX < j) {
                        plan[0][0] += value;
                    }
                    else if (posX > j) {
                        plan[0][1] += value;
                    }
                    plan[1][0] += value / 1000;
                    plan[1][1] += value / 1000;
                }
            }
        }


        value = -1000;

        if (posX == 0 || dgist.map[posY][posX-1].item.status == trap) {
            plan[0][1] = -1000;
        }
        if (posX == 4 || dgist.map[posY][posX+1].item.status == trap) {
            plan[0][0] = -1000;
        }

        if (posY == 0 || dgist.map[posY-1][posX].item.status == trap) {
            plan[1][1] = -1000;
        }
        if (posY == 4 || dgist.map[posY+1][posX].item.status == trap) {
            plan[1][0] = -1000;
        }

        ind = -1;

        for (int i=0; i<2; i++) {
            for (int j=0; j<2; j++) {
                printf("plan[%d][%d] : %f\n", i, j, plan[i][j]);
                if (plan[i][j] > value) {
                    value = plan[i][j];
                    ind = i*2 + j;
                }
            }
        }

        /////////////////////////////////////////////////////////////////////////
        printf("value : %f\n", value);
        printf("ind : %d\n", ind);
        printf("posX : %d, posY : %d\n", posX, posY);
        printf("direction : %d\n", direction);

        printf("Present map:\n");
        print_map(dgist, posX, posY);

        ///////////////////////////////////////////////////////////////////////////////
        if (ind == 0) {
            // +x 방향으로... 그냥 시뮬레이션 해보려고 그냥 움직이게 한다.
            posX += 1;
        }
        else if (ind == 1) {
            // -x 방향으로
            posX -= 1;
        }
        else if (ind == 2) {
            // +y 방향으로
            posY += 1;
        }
        else if (ind == 3) {
            // -y 방향으로
            posY -= 1;
        }
        
		printf("\n");
    }

    

    return 0;
}
