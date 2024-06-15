#include "qr.h"

using namespace cv;
using namespace std;
using namespace zbar;

// QR 코드 인식 및 문자열 출력 함수
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

// main 함수
int main() {
    int result = detect();
    if (result != -1) {
        cout << "Detected two-digit number: " << result << endl;
    } else {
        cout << "No valid QR code detected or 'q' key pressed to exit." << endl;
    }
    return 0;
}
