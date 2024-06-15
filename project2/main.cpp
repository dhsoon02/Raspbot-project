#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <opencv2/opencv.hpp>
#include <zbar.h>
#include <iostream>
#include <string>
#include <wiringPi.h>
#include <signal.h>
#include <thread>
#include <atomic>
#include "client.h"
#include "tracking.h"

using namespace cv;
using namespace std;
using namespace zbar;

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 12345

std::atomic<bool> running(true);
Mat currentFrame;
struct QRCodeScanner* qrScanner = nullptr;
int server_socket;
int sock;
ClientAction clientAction;
pthread_t updateThread;

DGIST dgist_data; // 전역 변수~

// Function prototypes
void signalHandler(int sig);
void QRCodeScanningThread();
void TrackingThread();
void* receive_updates(void* arg);
void setup();
void Car_Stop();
void tracking_function();
int find_best_move(DGIST* dgist, int start_row, int start_col);

// 추가 함수
int find_best_move(DGIST* dgist, int start_row, int start_col) {
    int directions[4][2] = {
        {0, -1}, {0, 1}, {-1, 0}, {1, 0} // 좌, 우, 상, 하
    };
    int max_score = -1;
    int best_direction = -1;

    for (int i = 0; i < 4; i++) {
        int new_row = start_row + directions[i][0];
        int new_col = start_col + directions[i][1];

        if (new_row >= 0 && new_row < MAP_ROW && new_col >= 0 && new_col < MAP_COL) {
            int score = dgist->map[new_row][new_col].item.score;
            if (score >= 0 && score > max_score) {
                max_score = score;
                best_direction = i;
                printf("new row, new col and max_score: %d %d and %d", new_row, new_col, max_score);
            }
        }
    }
    printf("The way we go is %d\n", best_direction);
    // Return values: 좌(0,-1) -> 1, 우(0,1) -> 0, 상(-1,0) -> 2, 하(1,0) -> 3
    switch (best_direction) {
        case 0: return 1; // 좌
        case 1: return 0; // 우
        case 2: return 2; // 상
        case 3: return 3; // 하
        default: return -1; // Invalid
    }
}

struct QRCodeResult {
    bool success;
    int digit1;
    int digit2;
};

struct QRCodeScanner {
    ImageScanner scanner;
};

QRCodeScanner* createQRCodeScanner() {
    QRCodeScanner* scanner = new QRCodeScanner();
    scanner->scanner.set_config(ZBAR_QRCODE, ZBAR_CFG_ENABLE, 1);
    return scanner;
}

bool initializeQRCodeScanner(QRCodeScanner* scanner) {
    return scanner != nullptr;
}

QRCodeResult scanQRCode(QRCodeScanner* scanner, const Mat& frame) {
    QRCodeResult result;
    result.success = false;

    // Convert image to grayscale
    Mat gray;
    cvtColor(frame, gray, COLOR_BGR2GRAY);

    // Wrap image data in a ZBar image
    Image zbarImage(frame.cols, frame.rows, "Y800", gray.data, frame.cols * frame.rows);

    // Scan the image for barcodes
    int n = scanner->scanner.scan(zbarImage);

    // Extract results
    for (Image::SymbolIterator symbol = zbarImage.symbol_begin(); symbol != zbarImage.symbol_end(); ++symbol) {
        if (symbol->get_type() == ZBAR_QRCODE) {
            string data = symbol->get_data();
            if (data.length() >= 2) {
                result.digit1 = data[0] - '0';
                result.digit2 = data[1] - '0';
                result.success = true;
                break;
            }
        }
    }
    return result;
}
int fd;

void setup() {
    if (wiringPiSetup() == -1) {
        printf("WiringPi setup failed!\n");
    }
    
    pinMode(Tracking_Right1, INPUT);
    pinMode(Tracking_Right2, INPUT);
    pinMode(Tracking_Left1, INPUT);
    pinMode(Tracking_Left2, INPUT);
    
    fd = open("/dev/i2c-1", O_RDWR);
    if (fd < 0) {
        printf("I2C setup error\n");
        exit(1);
    }
    if (ioctl(fd, I2C_SLAVE, I2C_ADDR) < 0) {
        printf("I2C setup failed!\n");
    }
}

void tracking_function() {
    int Tracking_Left1Value = digitalRead(Tracking_Left1);
    int Tracking_Left2Value = digitalRead(Tracking_Left2);
    int Tracking_Right1Value = digitalRead(Tracking_Right1);
    int Tracking_Right2Value = digitalRead(Tracking_Right2);
    if (Tracking_Left1Value == LOW && Tracking_Left2Value == HIGH && Tracking_Right1Value == HIGH && Tracking_Right2Value == LOW){
        Control_Car(-60,-60);
        delay(100);
        printf("1\n");
    } else if ((Tracking_Left1Value == LOW && Tracking_Left2Value == LOW && Tracking_Right1Value == LOW && Tracking_Right2Value == LOW) ||
        (Tracking_Left1Value == HIGH && Tracking_Left2Value == LOW && Tracking_Right1Value == LOW && Tracking_Right2Value == LOW) ||
        (Tracking_Left1Value == LOW && Tracking_Left2Value == LOW && Tracking_Right1Value == LOW && Tracking_Right2Value == HIGH)) {
        printf("교점에 진입: %d %d %d %d\n", Tracking_Left1Value, Tracking_Left2Value, Tracking_Right1Value, Tracking_Right2Value);
        //새로운 노드 진입
        //정지
        Control_Car(0,0);
        delay(500);
        printf("정지 후 출발\n");
        //외부 함수로 노드주변 템 점수 확인 후, 방향 설정
        int res = find_best_move(&dgist_data, dgist_data.players[0].row, dgist_data.players[0].col);
        printf("res: %d", res);
        if (res == 0) { // 우
            Control_Car(90, -20); // 우회전
            delay(750);
            printf("2\n");
        } else if (res == 1) { // 좌
            Control_Car(-20, 90); // 좌회전
            delay(750);
            printf("3\n");
        } else if (res == 2) { // 상
            Control_Car(50, 50); 
            delay(300);
            printf("4\n");
        } else if (res == 3) { // 하
            Control_Car(-50, -50); 
            delay(300);
            printf("5\n");
        }
    } else if ((Tracking_Left1Value == LOW || Tracking_Left2Value == LOW) && Tracking_Right2Value == LOW) { //0000 1000
        Control_Car(0,0);
        delay(200);
        Control_Car(90, -60);
        delay(200); // 0.2 seconds
        printf("6\n");
    } else if (Tracking_Left1Value == LOW && (Tracking_Right1Value == LOW || Tracking_Right2Value == LOW)) {
        Control_Car(0,0);
        delay(200);
        Control_Car(-60, 90);
        delay(200);
        printf("7\n");
    } else if (Tracking_Left1Value == LOW) {
        Control_Car(0,0);
        delay(20);
        Control_Car(-70, 70);
        delay(50);
        printf("8\n");
    } else if (Tracking_Right2Value == LOW) {
        Control_Car(0,0);
        delay(20);
        Control_Car(70, -70);
        delay(50);
        printf("9\n");
    } else if (Tracking_Left2Value == LOW && Tracking_Right1Value == HIGH) {
        Control_Car(0,0);
        delay(20);
        Control_Car(-60, 60);
        delay(20);
        printf("10\n");
    } else if (Tracking_Left2Value == HIGH && Tracking_Right1Value == LOW) {
        Control_Car(0,0);
        delay(20);
        Control_Car(60, -60);
        delay(20);
        printf("11\n");
    } else if (Tracking_Left2Value == LOW && Tracking_Right1Value == LOW) {
        Control_Car(50, 50);
    }    
}

void destroyQRCodeScanner(QRCodeScanner* scanner) {
    delete scanner;
}

void signalHandler(int sig) {
    running = false;
    Car_Stop();
    printf("Program terminated\n");
    close(server_socket);
    pthread_cancel(updateThread);
    exit(0);
}

void Ctrl_Car(int l_dir, int l_speed, int r_dir, int r_speed) {
    int reg = 0x01;
    int data[4] = {l_dir, l_speed, r_dir, r_speed};
    write_array(reg, data, 4);
}

void Control_Car(int speed1, int speed2) {
    int dir1 = (speed1 < 0) ? 0 : 1;
    int dir2 = (speed2 < 0) ? 0 : 1;
    Ctrl_Car(dir1, abs(speed1), dir2, abs(speed2));
}

void Car_Stop() {
    int reg = 0x02;
    write_u8(reg, 0x00);
}

int decodeDisplay(Mat& im) {
    ImageScanner scanner;
    scanner.set_config(ZBAR_NONE, ZBAR_CFG_ENABLE, 1);

    // 영상을 그레이스케일로 변환
    Mat imGray;
    cvtColor(im, imGray, COLOR_BGR2GRAY);

    int width = imGray.cols;
    int height = imGray.rows;
    uchar* raw = (uchar*)(imGray.data);

    // ZBar 이미지 객체 생성 및 스캔
    Image image(width, height, "Y800", raw, width * height);
    scanner.scan(image);

    // 인식된 QR 코드의 데이터를 처리
    for (Image::SymbolIterator symbol = image.symbol_begin(); symbol != image.symbol_end(); ++symbol) {
        string data = symbol->get_data();

        // 인식된 문자열이 두 자리 숫자인 경우 처리
        if (data.length() == 2 && isdigit(data[0]) && isdigit(data[1])) {
            int result = (data[0] - '0') * 10 + (data[1] - '0'); // 두 자리 숫자
            return result;
        }
    }
    // 유효한 두 자리 숫자를 인식하지 못한 경우 -1 반환
    return -1;
}

// 카메라에서 실시간 영상 캡처 및 QR 코드 인식 함수
int detect() {
    // 비디오 캡처 객체 생성 (카메라 열기)
    VideoCapture cap(0);
    if (!cap.isOpened()) {
        cerr << "Error opening video stream" << endl;
        return -1;
    }

    // 실시간 영상 처리 루프
    while (true) {
        Mat frame;
        cap >> frame;
        if (frame.empty())
            break;

        // QR 코드 인식 및 문자열 출력
        int decodedValue = decodeDisplay(frame);

        // 유효한 두 자리 숫자를 인식하면 반환
        if (decodedValue != -1) {
            cap.release(); // 자원 해제
            return decodedValue;
        }

        // 'q' 키 입력 시 종료
        if (waitKey(1) == 'q') {
            cap.release(); // 자원 해제
            return -1;
        }
    }

    // 자원 해제
    cap.release();
    return -1;
}

// 필요한 함수 정의 추가
void write_array(int reg, int* data, int length) {
    unsigned char buffer[5];
    buffer[0] = static_cast<unsigned char>(reg);
    for (int i = 0; i < length; ++i) {
        buffer[i + 1] = static_cast<unsigned char>(data[i]);
    }
    write(fd, buffer, length + 1);
}

void write_u8(int reg, int data) {
    unsigned char buffer[2] = {static_cast<unsigned char>(reg), static_cast<unsigned char>(data)};
    write(fd, buffer, 2);
}

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

void TrackingThread() {
    
    //Control_Car(90, -20); // 우회전
    //Control_Car(-20, 90); //좌회전
    //delay(750);

    //Control_Car(50,50); 직진
    //delay(300);
    
    while (running) {
        //tracking_function();
        
        tracking_function();
    }
}

void QRCodeScanningThread() {
    extern struct QRCodeScanner* qrScanner; // 전역 변수 참조
    extern ClientAction clientAction;\
    int count = 0;
    while (running) {
        count += 1;
        if (!currentFrame.empty()) {
            QRCodeResult qr_result = scanQRCode(qrScanner, currentFrame);
            if (qr_result.success) {
                int qr_row = qr_result.digit1;
                int qr_col = qr_result.digit2;
                cout << "QR Code Scan Succeed: {" << qr_row << ", " << qr_col << "}" << endl;
                clientAction.row = qr_row;
                clientAction.col = qr_col;
                if (count == 6) {
                    clientAction.action = Action::move;
                }
                else {
                    clientAction.action = Action::move;  // or setbomb
                }
                if (send_action(sock, &clientAction) <= 0) { // 전역 변수 sock 사용
                    printf("Failed to send action to the server.\n");
                    close(sock);
                }
            } 
            /*
            else {
                cout << "Failed to scan QR code." << endl;
            }
            */
            this_thread::sleep_for(chrono::milliseconds(200)); // Adding a delay to reduce CPU usage
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_ip> <server_port>\n", argv[0]);
        return 1;
    }

    const char* server_ip = argv[1];
    int server_port = atoi(argv[2]);

    sock = connect_to_server(server_ip, server_port); // 전역 변수 sock 사용
    if (sock < 0) {
        return 1;
    }

    // 초기 설정 함수 호출
    setup();
    signal(SIGINT, signalHandler);

    // 비디오 캡처 객체 생성 (카메라 열기)
    VideoCapture cap(0);
    if (!cap.isOpened()) {
        cerr << "Error opening video stream" << endl;
        return -1;
    }

    // 픽셀 포맷 설정
    cap.set(CAP_PROP_FOURCC, VideoWriter::fourcc('M', 'J', 'P', 'G'));

    // QR 코드 스캐너 객체 생성
    qrScanner = createQRCodeScanner();
    if (!initializeQRCodeScanner(qrScanner)) {
        cerr << "Failed to initialize QR code scanner" << endl;
        return -1;
    }

    // Start the QR code scanning and tracking, server comm threads
    thread qrThread(QRCodeScanningThread);
    thread trackingThread(TrackingThread);

    // Capture frames from the camera
    while (running) {
        cap >> currentFrame;
        if (currentFrame.empty()) {
            cerr << "Failed to capture image" << endl;
            break;
        }

        // 'q' 키 입력 시 종료
        if (waitKey(1) == 'q') {
            running = false;
            Car_Stop();
            break;
        }
    }

    // Wait for threads to finish
    qrThread.join();
    trackingThread.join();

    // 자원 해제
    destroyQRCodeScanner(qrScanner);
    Car_Stop();
    cap.release();
    close(sock); // 전역 변수 sock 사용

    return 0;
}


//g++ -o main nalgo_main.cpp -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_videoio -lzbar -lpthread -lwiringPi
