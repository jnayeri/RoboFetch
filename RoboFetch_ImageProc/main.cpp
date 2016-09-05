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

bool isBall = false;    // HSV boolean flag
int posX = 0;
int posY = 0;

namespace
{
    // windows and trackbars name
    const std::string windowName = "Hough Circle Detection Demo";
    const std::string cannyThresholdTrackbarName = "Canny threshold";
    const std::string accumulatorThresholdTrackbarName = "Accumulator Threshold";
    
    // initial and max values of the parameters of interests.
    const int cannyThresholdInitialValue = 20;
    const int accumulatorThresholdInitialValue = 34;
    const int maxAccumulatorThreshold = 200;
    const int maxCannyThreshold = 255;
    
    void HoughDetection(const Mat& src_gray, const Mat& src_display, int cannyThreshold, int accumulatorThreshold)
    {
        // clone the colour, input image for displaying purposes
        cv::Mat display = src_display.clone();
        
        if (isBall == true)
        {
            // will hold the results of the detection
            std::vector<Vec3f> circles;
            // runs the actual detection
            HoughCircles( src_gray, circles, HOUGH_GRADIENT, 1, src_gray.rows, cannyThreshold, accumulatorThreshold, 0, 1000 );

            for( size_t i = 0; i < circles.size(); i++ )
            {
                int diffX = abs(cvRound(circles[i][0]) - posX);
                int diffY = abs(cvRound(circles[i][1]) - posY);
                
                if ((diffX < 15) && (diffY < 15))
                {
                    std::cout << "Hough Center: (" << cvRound(circles[i][0]) << ", " << cvRound(circles[i][1]) << ")" << std::endl;
                    std::cout << "HSV Center: (" << posX << ", " << posY << ")" << std::endl << std::endl;
 
                    cv::Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
                    int radius = cvRound(circles[i][2]);
                    // circle center
                    //circle( display, center, 3, Scalar(0,255,0), -1, 8, 0 );
                    // circle outline
                    circle( display, center, radius, Scalar(0,0,255), 3, 8, 0 );
                }
            }
            
            isBall = false;
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
    
    
    /////////////////  HSV Implementation  /////////////////////
    
    cv::namedWindow("Control", CV_WINDOW_AUTOSIZE); // create a window called "Control"
    
    int iLowH = 32;
    int iHighH = 94;
    
    int iLowS = 25;
    int iHighS = 185;
    
    int iLowV = 85;
    int iHighV = 235;
    
    /* Create trackbars in "Control" window */
    // Hue (0 - 179)
    cv::createTrackbar("LowH", "Control", &iLowH, 179);
    cv::createTrackbar("HighH", "Control", &iHighH, 179);
    
    // Saturation (0 - 255)
    cv::createTrackbar("LowS", "Control", &iLowS, 255);
    cv::createTrackbar("HighS", "Control", &iHighS, 255);
    
    // Value (0 - 255)
    cv::createTrackbar("LowV", "Control", &iLowV, 255);
    cv::createTrackbar("HighV", "Control", &iHighV, 255);
    
    int iLastX = -1;
    int iLastY = -1;
    
    
    cv::Mat src, src_gray, src_HSV;
    
    // Read a new frame //
    vcap.read(src);
    cvtColor( src, src_gray, COLOR_BGR2GRAY ); // convert it to gray-scale
    GaussianBlur( src_gray, src_gray, cv::Size(9, 9), 2, 2); // reduces the noise so we avoid false circle detection
    
    cvtColor(src, src_HSV, COLOR_BGR2HSV); // Convert the captured frame from BGR to HSV
    
    
    /////////////////  Hough Transform and Canny Edge Detection Implementation  ///////////////
    
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
        cvtColor(src, src_HSV, COLOR_BGR2HSV); // Convert the captured frame from BGR to HSV
        
        Mat imgThresholded;
        // Threshold the image
        inRange(src_HSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded);
        // Morphological opening (removes small objects from the foreground)
        erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, cv::Size(5, 5)) );
        dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, cv::Size(5, 5)) );
        // Morphological closing (removes small holes from the foreground)
        dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, cv::Size(5, 5)) );
        erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, cv::Size(5, 5)) );
        
        // Calculate the moments of the thresholded image
        Moments oMoments = moments(imgThresholded);
        
        double dM01 = oMoments.m01;
        double dM10 = oMoments.m10;
        double dArea = oMoments.m00;
        
        // If the area <= 10000, I consider that the there are no object in the image and it's because of the noise, the area is not zero
        //std::cout << "Area: " << dArea << std::endl;
        isBall = false;
        if (dArea > 10000 && dArea < 500000)
        {
            // Calculate the position of the ball
            posX = dM10 / dArea;
            posY = dM01 / dArea;
            
            isBall = true;
            
            iLastX = posX;
            iLastY = posY;
        }
        
        
        imshow("Thresholded Image", imgThresholded);
        
        
        cannyThreshold = std::max(cannyThreshold, 1);   // 20
        accumulatorThreshold = std::max(accumulatorThreshold, 1);   // 34
        
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



