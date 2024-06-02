#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include "ReadQR.h"

using namespace cv;
using namespace std;

#ifdef __cplusplus
extern "C"{
#endif

char* readQR() {
    VideoCapture cap(0);
    if (!cap.isOpened()) {
        cerr << "Error1." << endl;
        return NULL;
    }

    QRCodeDetector qrDecoder;

    Mat frame, bbox, rectifiedImage;
    while (true) {
        cap >> frame;
        if (frame.empty()) {
            cerr << "Error2." << endl;
            return NULL ;
        }

        vector<Point> points;
        string data = qrDecoder.detectAndDecode(frame, points);
		static char result[2];
        if (!data.empty()) {
            cout << "Decoded Data : " << data << endl; ///////////////////////////////////////////
			strcpy(result, data.c_str());
            return result;

            //for (int i = 0; i < points.size(); i++) {
            //    line(frame, points[i], points[(i + 1) % points.size()], Scalar(255, 0, 0), 4);
            //}
        }

        imshow("QR Code Detector", frame);
        if (waitKey(1) == 27) {
            break;
        }
    }
    
    return NULL;
}

#ifdef __cplusplus
}
#endif


