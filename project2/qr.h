#ifndef QR_H
#define QR_H

#include <opencv2/opencv.hpp>
#include <zbar.h>

using namespace cv;
using namespace std;
using namespace zbar;

// Function to decode and display barcode/QR code from an image
int decodeDisplay(Mat& im);

// Function to detect barcode/QR code from video stream
int detect();

#endif // QR_H
