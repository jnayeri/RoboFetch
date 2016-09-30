//
//  main.cpp
//  RoboFetch_ImageProc
//
//  Created by Joubin Nayeri on 2016-08-27.
//  Copyright (c) 2016 Joubin Nayeri. All rights reserved.
//

#include "main.h"

#define CIRCLE_RADIUS           8
#define CIRCLE_THICKNESS        2
#define ESCAPE_KEY              27
#define MAX_SERIAL_ATTEMPTS     5

using namespace std;
using namespace cv;

int posX = 0;
int posY = 0;

bool target_detected = false;

uint8_t tx_buf[] = {'j', 0, 0, 0};

int main(int, char**)
{
    
    /* Open the Serial Port for Data Transfer to HC-05 */
    int count = -1;
    int fd;
    do
    {
        count++;
        fd = serialport_init("/dev/cu.HC-05-DevB", 9600);
        if (count < MAX_SERIAL_ATTEMPTS) continue;
        std::cout << "Error opening HC-05 Serial Port on attempt " << count << std::endl;
    } while (fd == -1 && count < MAX_SERIAL_ATTEMPTS);
    
    /* Open the video stream and make sure it's opened */
    cv::VideoCapture vcap;
    if(!vcap.open("http://dragino-jn.local:8080/?action=stream.mjpg", cv::CAP_FFMPEG)) {
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
    
    while (waitKey(30) != ESCAPE_KEY)
    {
        // Delete previous circle
        cv::Mat imgLines = Mat::zeros( imgTmp.size(), CV_8UC3 );
        cv::Mat imgOriginal;
        
        if (!vcap.read(imgOriginal)) // Read a new frame from video. If not successful, break loop
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
        if (dArea > 500)
        {
            target_detected = true;
            // Calculate the position of the ball
            posX = dM10 / dArea;
            posY = dM01 / dArea;
            
            circle(imgLines, cv::Point(posX, posY), CIRCLE_RADIUS, cv::Scalar(0,0,255), CIRCLE_THICKNESS);
            
            iLastX = posX;
            iLastY = posY;
        }
        
        else
        {
            target_detected = false;
        }
        
        imshow("Thresholded Image", imgThresholded); //show the thresholded image
        
        imgOriginal = imgOriginal + imgLines;
        imshow("Original", imgOriginal); // Show the original image
        
        // Size of the Image //
        //std::cout << "Width : " << imgOriginal.cols << std::endl;
        //std::cout << "Height: " << imgOriginal.rows << std::endl << std::endl;
        
        /*  Transfer of Center Position to Arduino through Serial Port  */
        
        if (target_detected == true)
        {
            tx_buf[1] = 1;
            tx_buf[2] = posX;
            tx_buf[3] = posY;
            
            for (int i = 0; i < sizeof(tx_buf); i++)
            {
                if (serialport_writebyte(fd, (uint8_t)tx_buf[i]) != 0)
                    std::cout << "Error sending " << (uint8_t)tx_buf[i] << "to HC-05 serial port." << std::endl;
            }
        }
        else
        {
            tx_buf[1] = 0;
        }
        
    }
    
    serialport_close(fd);
    return 0;
}








