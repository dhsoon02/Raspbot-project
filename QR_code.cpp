#include <opencv2/opencv.hpp>
#include <zbar.h>
#include <iostream>

using namespace cv;
using namespace std;
using namespace zbar;

void decodeDisplay(Mat& im) {
    ImageScanner scanner;
    scanner.set_config(ZBAR_NONE, ZBAR_CFG_ENABLE, 1);

    Mat imGray;
    cvtColor(im, imGray, COLOR_BGR2GRAY);

    int width = imGray.cols;
    int height = imGray.rows;
    uchar* raw = (uchar*)(imGray.data);

    Image image(width, height, "Y800", raw, width * height);
    scanner.scan(image);

    for (Image::SymbolIterator symbol = image.symbol_begin(); symbol != image.symbol_end(); ++symbol) {
        vector<Point> vp;
        for (int i = 0; i < symbol->get_location_size(); i++) {
            vp.push_back(Point(symbol->get_location_x(i), symbol->get_location_y(i)));
        }
        RotatedRect r = minAreaRect(vp);
        Point2f pts[4];
        r.points(pts);
        for (int i = 0; i < 4; i++) {
            line(im, pts[i], pts[(i + 1) % 4], Scalar(255, 0, 0), 3);
        }

        string type = symbol->get_type_name();
        string data = symbol->get_data();
        cout << "[INFO] Found " << type << " barcode: " << data << endl;

        putText(im, data + " (" + type + ")", Point(pts[1].x, pts[1].y), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 0, 0), 2);
    }
}

void detect() {
    //video14 or video15
    VideoCapture cap(0);
    if (!cap.isOpened()) {
        cerr << "Error opening video stream" << endl;
        return;
    }

    cap.set(CAP_PROP_FRAME_WIDTH, 320);
    cap.set(CAP_PROP_FRAME_HEIGHT, 240);
    cap.set(CAP_PROP_FPS, 30);
    cap.set(CAP_PROP_FOURCC, VideoWriter::fourcc('M', 'J', 'P', 'G'));
    cap.set(CAP_PROP_BRIGHTNESS, 40);
    cap.set(CAP_PROP_CONTRAST, 50);
    cap.set(CAP_PROP_EXPOSURE, 156);


    while (true) {
        Mat frame;
        cap >> frame;
        if (frame.empty())
            break;

        decodeDisplay(frame);

        imshow("Barcode/QR code reader", frame);

        if (waitKey(1) == 'q')
            break;
    }

    cap.release();
    destroyAllWindows();
}

int main() {
    detect();
    return 0;
}

//실행 명령어
/*
cd "/Users/donghyeonsoon/Documents/컴퓨터 비전 개론/시프/C_ver/"
g++ QR_code.cpp -o QR_code `pkg-config --cflags --libs opencv4 zbar` -std=c++11
./QR_code
*/
