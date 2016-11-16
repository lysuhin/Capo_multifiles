#include "opencv2/core/core.hpp"        // Mat!
#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"  // imread, imWindow and so
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/ml/ml.hpp"
#include "detection.h"
#include <iostream>
#include <string>

using namespace cv;
using namespace cv::ml;
using namespace std;


void applyCLAHEToRGB(Mat source, Mat output){
    Mat lab_image;
    cvtColor(source, lab_image, CV_BGR2Lab);

    // Extract the L channel
    std::vector<cv::Mat> lab_planes(3);
    cv::split(lab_image, lab_planes);  // now we have the L image in lab_planes[0]

    // apply the CLAHE algorithm to the L channel
    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE();
    clahe->setClipLimit(4);
    cv::Mat dst;
    clahe->apply(lab_planes[0], dst);

    // Merge the the color planes back into an Lab image
    dst.copyTo(lab_planes[0]);
    cv::merge(lab_planes, lab_image);

    // convert back to RGB
    cv::Mat image_clahe;
    cv::cvtColor(lab_image, image_clahe, CV_Lab2BGR);

    output = image_clahe;
}

int main(int argc, char* argv[])
{

    Detection* detector = new Detection(1, 1);
    detector->train();

    vector <Point2d> points;

    VideoCapture cap(1); // open the video camera no. 0
    if (!cap.isOpened())  // if not success, exit program
    {
        cout << "Cannot open the video cam" << endl;
        return -1;
    }

    double dWidth = cap.get(CV_CAP_PROP_FRAME_WIDTH); //get the width of frames of the video
    double dHeight = cap.get(CV_CAP_PROP_FRAME_HEIGHT); //get the height of frames of the video

    cout << "Frame size : " << dWidth << " x " << dHeight << endl;

    while (1){
        Mat LoadedImage;// = imread("/home/lysuhin/Coding/OpenCV/examples/test_image_4.jpg");

        #if 1
        bool bSuccess = cap.read(LoadedImage);
        if (!bSuccess){
             cout << "Cannot read a frame from video stream" << endl;
             break;
        }
        #endif // 0

        //flip(LoadedImage, LoadedImage, 1);

        detector->detect(LoadedImage, points);
        cout << "points number = " << points.size() << endl;

        for (size_t i = 0; i < points.size(); i++){
            Point2d point1 = points.at(i);
            Point2d point2 = Point2d(point1.x + 64, point1.y + 64);
            rectangle(LoadedImage, point1, point2, Scalar(0, 255, 0), 1, 8, 0);
            rectangle(detector->image, point1, point2, Scalar(0, 255, 0), 1, 8, 0);
        }

        imshow("raw", LoadedImage);
        imshow("gra", detector->image);

        if (waitKey(50) == 27){
            break;
        };
        points.clear();
    }
    return 0;
}
