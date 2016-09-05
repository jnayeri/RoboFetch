//
//  main.cpp
//  RoboFetch_ImageProc
//
//  Created by Joubin Nayeri on 2016-08-27.
//  Copyright (c) 2016 Joubin Nayeri. All rights reserved.
//

#include "main.h"

#define ROW_MAX             40
#define COL_MAX             30
#define CIRCLE_RADIUS       20
#define CIRCLE_THICKNESS    1

using namespace std;
using namespace cv;

int main(int, char**)
{
    cv::VideoCapture vcap;
    cv::Mat image;
    bool bSuccess = false;
    const std::string videoStreamAddress = "http://dragino-jn.local:8080/?action=stream.mjpg";
    
    // Open the video stream and make sure it's opened
    if(!vcap.open(videoStreamAddress, cv::CAP_FFMPEG)) {
        std::cout << "Error opening video stream or file" << std::endl;
        return -1;
    }
    
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
    
    /* Capture a temporary image from the camera */
    cv::Mat imgTmp;
    vcap.read(imgTmp);
    
    /* Create a black image with the size as the camera output */
    cv::Mat imgLines = Mat::zeros( imgTmp.size(), CV_8UC3 );
    
    while (true)
    {
        // Delete previous circle
        cv::Mat imgLines = Mat::zeros( imgTmp.size(), CV_8UC3 );
        cv::Mat imgOriginal;
        
        bSuccess = vcap.read(imgOriginal); // Read a new frame from video
        
        if (!bSuccess) // If not successful, break loop
        {
            cout << "Cannot read a frame from video stream" << endl;
            break;
        }
        
        Mat imgHSV;
        cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV); // Convert the captured frame from BGR to HSV
        
        Mat imgThresholded;
        // Threshold the image
        inRange(imgHSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded);
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
        if (dArea > 156)
        {
            // Calculate the position of the ball
            int posX = dM10 / dArea;
            int posY = dM01 / dArea;
            
            circle(imgLines, cv::Point(posX, posY), CIRCLE_RADIUS, cv::Scalar(0,0,255), CIRCLE_THICKNESS);
            
            iLastX = posX;
            iLastY = posY;
        }
        
        imshow("Thresholded Image", imgThresholded); //show the thresholded image
        
        imgOriginal = imgOriginal + imgLines;
        imshow("Original", imgOriginal); // Show the original image
        
        if (waitKey(30) == 27) // Wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
        {
            cout << "esc key is pressed by user" << endl;
            break;
        }
    }
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    /*while (true)
    {
        if(!vcap.read(image)) {
            std::cout << "No frame" << std::endl;
            cv::waitKey();
        }
        cv::imshow("Output Window", image);
        if(cv::waitKey(1) >= 0) break;
    }*/
    
    return 0;
}