#include "opencv2/core/core.hpp"        // Mat!
#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"  // imread, imWindow and so
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/ml/ml.hpp"

#include "capillaroscope.h"

#include <iostream>
#include <string>

using namespace cv;
using namespace cv::ml;
using namespace std;

size_t  GOOD_NUM_OF_POINTS = 5;

int main(int argc, char* argv[])
{

    Capillaroscope* capillaroscope = new Capillaroscope(true);

    VideoCapture cap(0); // open the video camera no. 0
    if (!cap.isOpened())  // if not success, exit program
    {
        cout << "Cannot open the video cam" << endl;
        return -1;
    }

    double dWidth = cap.get(CV_CAP_PROP_FRAME_WIDTH); //get the width of frames of the video
    double dHeight = cap.get(CV_CAP_PROP_FRAME_HEIGHT); //get the height of frames of the video

    cout << "Frame size : " << dWidth << " x " << dHeight << endl;

    while (1){

        if (waitKey(1) == 27) break;

        Mat image; // = imread("/home/lysuhin/Coding/OpenCV/examples/test_image_4.jpg");

        #if 1
        bool bSuccess = cap.read(image);
        if (!bSuccess){
             cout << "Cannot read a frame from video stream" << endl;
             break;
        }
        #endif // 0

        imshow("rgb", image);

        capillaroscope->detect(image, true);
        imshow("filtered", capillaroscope->image_detected);

        if (capillaroscope->number_of_detected_points > GOOD_NUM_OF_POINTS){

            cout << "Looks like a good picture. Use it? (y/n)" << endl;
            char c = waitKey(3000);

            if (c == 'y')
            {
                cout << "Okay! Processing the picture..." << endl;

                capillaroscope->detect(image, true, true, true, true);
                imshow("detected capillars", capillaroscope->image_detected);

                cout << "And to perivascular space width..." << endl;
                imshow("perivascular space", capillaroscope->image_perivascular);

                waitKey();
                break;
            }
            else continue;
        }
    }
    return 0;
}
