//
//  main.cpp
//  RoboFetch_ImageProc
//
//  Created by Joubin Nayeri on 2016-08-27.
//  Copyright (c) 2016 Joubin Nayeri. All rights reserved.
//

#include "main.h"

#define ESCAPE_KEY          27

#define ROW_MAX             40
#define COL_MAX             30
#define CIRCLE_RADIUS       20
#define CIRCLE_THICKNESS    1

using namespace std;
using namespace cv;

namespace
{
    // windows and trackbars name
    const std::string windowName = "Hough Circle Detection Demo";
    const std::string cannyThresholdTrackbarName = "Canny threshold";
    const std::string accumulatorThresholdTrackbarName = "Accumulator Threshold";
    
    // initial and max values of the parameters of interests.
    const int cannyThresholdInitialValue = 30;
    const int accumulatorThresholdInitialValue = 30;
    const int maxAccumulatorThreshold = 200;
    const int maxCannyThreshold = 255;
    
    void HoughDetection(const Mat& src_gray, const Mat& src_display, int cannyThreshold, int accumulatorThreshold)
    {
        // will hold the results of the detection
        std::vector<Vec3f> circles;
        // runs the actual detection
        HoughCircles( src_gray, circles, HOUGH_GRADIENT, 1, src_gray.rows/8, cannyThreshold, accumulatorThreshold, 0, 0 );
        
        // clone the colour, input image for displaying purposes
        Mat display = src_display.clone();
        for( size_t i = 0; i < circles.size(); i++ )
        {
            cv::Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
            int radius = cvRound(circles[i][2]);
            // circle center
            circle( display, center, 1, Scalar(0,255,0), -1, 8, 0 );
            // circle outline
            circle( display, center, radius, Scalar(0,0,255), 3, 8, 0 );
        }
        
        // shows the results
        imshow( windowName, display);
    }
}



int main(int, char**)
{
    cv::VideoCapture vcap;
    const std::string videoStreamAddress = "http://dragino-jn.local:8080/?action=stream.mjpg";
    
    // Open the video stream and make sure it's opened
    if(!vcap.open(videoStreamAddress, cv::CAP_FFMPEG)) {
        std::cout << "Error opening video stream or file" << std::endl;
        return -1;
    }
    
    cv::Mat src, src_gray;
    vcap.read(src);
    cvtColor( src, src_gray, COLOR_BGR2GRAY ); // convert it to gray-scale
    GaussianBlur( src_gray, src_gray, cv::Size(9, 9), 2, 2); // reduces the noise so we avoid false circle detection
    
    // Declare and initialize both parameters that are subject to change
    int cannyThreshold = cannyThresholdInitialValue;
    int accumulatorThreshold = accumulatorThresholdInitialValue;
    
    // Create the main window, and attach the trackbars
    namedWindow( windowName, WINDOW_AUTOSIZE );
    createTrackbar(cannyThresholdTrackbarName, windowName, &cannyThreshold,maxCannyThreshold);
    createTrackbar(accumulatorThresholdTrackbarName, windowName, &accumulatorThreshold, maxAccumulatorThreshold);

    while(true)
    {
        // read new image
        vcap.read(src);
        cvtColor( src, src_gray, COLOR_BGR2GRAY ); // convert it to gray-scale
        GaussianBlur( src_gray, src_gray, cv::Size(9, 9), 2, 2); // reduces the noise so we avoid false circle detection
        
        cannyThreshold = std::max(cannyThreshold, 1);
        accumulatorThreshold = std::max(accumulatorThreshold, 1);
        
        //runs the detection, and update the display
        HoughDetection(src_gray, src, cannyThreshold, accumulatorThreshold);
        
        // get user key
        if (waitKey(30) == ESCAPE_KEY) // Wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
        {
            cout << "esc key is pressed by user" << endl;
            break;
        }
    }
    
    return 0;
}



