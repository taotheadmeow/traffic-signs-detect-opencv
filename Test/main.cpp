#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <future>
#include <thread>
#include <chrono>
#include <Windows.h>
#define SIGNS_NUM 5

using namespace std;
using namespace cv;


void signDetect(VideoCapture cap , CascadeClassifier classifier, int signColor, int id);
void setLabel(Mat& im, const std::string label, const cv::Point & pt);
Mat colorSegment(Mat *imBGR, int colorId);
CascadeClassifier classifiers[SIGNS_NUM];
Mat MASTER_FRAME;
std::vector<Rect> rowRois[SIGNS_NUM];
bool isRun = true;
int main(int, char**)
{
    cout << "Loading files....\n";
    String signName[SIGNS_NUM] = {
        "Stop",
        "Left Curve",
        "Right Curve",
        "4-way Intersection",
        "Left or Right way only"
    };
    String xmlFilesName[SIGNS_NUM] = {
        "r-stop-SignDetector2.xml",
        "w-curve-left-SignDetector2.xml",
        "w-curve-right-SignDetector2.xml",
        "w-junction-xshape-SignDetector2.xml",
        "r-leftorright-SignDetector2.xml"
    };
    int signColor[SIGNS_NUM] = {
        0, 1, 1, 1, 2
    };
    for(int i = 0; i < SIGNS_NUM; i++){
        if(!classifiers[i].load( xmlFilesName[i] )){
            cout << "Can't load\"" << xmlFilesName[i] << "\".\n";
            return 0;
        }
        else{
            cout << "Loaded \"" << xmlFilesName[i] << "\" as \"" << signName[i] << "\".\n";
        }
    }

    cout << "Looking for default camera..." << "\n";
    VideoCapture cap(0);

    if(!cap.isOpened()){
        cout << "Camera not founded!\n";
        return -1;
    }
    else{
        cout << "Camera founded!\n";
    }
    cap.set(CV_CAP_PROP_FPS, 12);
    cap.read(MASTER_FRAME);

    //start detect
    cout << "Starting..." <<  "\n";
    std::thread detection[SIGNS_NUM];
    for(int i = 0; i < SIGNS_NUM; i++){
        detection[i] = std::thread(signDetect, cap, classifiers[i], signColor[i], i);}

    LARGE_INTEGER frequency;
    LARGE_INTEGER start;
    LARGE_INTEGER end;
    QueryPerformanceFrequency(&frequency);
    cout << "Started! Press 'C' to stop.\n";
   for(;;){
       QueryPerformanceCounter(&start);
       cap.read(MASTER_FRAME);
       for(int id = 0; id < SIGNS_NUM; id++){
           for( size_t i = 0; i < rowRois[id].size(); i++ )
           {
                 Point p1( rowRois[id][i].x, rowRois[id][i].y);
                 Point p2( rowRois[id][i].x + rowRois[id][i].width, rowRois[id][i].y + rowRois[id][i].height);

                 rectangle(MASTER_FRAME, p1, p2, Scalar (0,255,0), 2);
                 setLabel(MASTER_FRAME, signName[id], p1);
           }

       }
       QueryPerformanceCounter(&end);
       double interval = static_cast<double>(end.QuadPart- start.QuadPart) /
                                     frequency.QuadPart;

       rectangle(MASTER_FRAME, cvPoint(20,20), cvPoint(50,10), cvScalar(0,0,0), CV_FILLED);
       putText(MASTER_FRAME, to_string((int) (1.0/interval)), cvPoint(20,20),
                   CV_FONT_HERSHEY_PLAIN, 0.8, cvScalar(255,255,250), 1, CV_AA);
       imshow("Traffic Signs Recognition", MASTER_FRAME);

       int c = waitKey(10);
       if( (char)c == 'c' ) { isRun = false; break; }


  }
   for(int i = 0; i < SIGNS_NUM; i++){
      detection[i].join();
    }

    cout << "Exiting..." << "\n";
    // the camera will be closed automatically upon exit
    // cap.close();
    return 0;
}
void signDetect( VideoCapture cap , CascadeClassifier classifier, int signColor, int id)
{
    Mat frame_gray;
    Mat frame;
  //-- Detect
  for(;;){
      cap.read(frame);
      //frame = colorSegment(&frame, signColor); <- will use if color segment function is done!
      cvtColor( frame, frame_gray, CV_BGR2GRAY );
      equalizeHist( frame_gray, frame_gray );
      classifier.detectMultiScale( frame_gray, rowRois[id], 1.1, 2, 0, Size(12, 12), Size(300, 300));
      if(!isRun){
          break;
      }
  }
}
Mat colorSegment(Mat *imBGR, int colorId){
    //0 for red; 1 for yellow; 2 for blue
    Mat labIm;
    Mat mask;
    Mat result;
    cv::cvtColor(*imBGR, labIm, cv::COLOR_BGR2Lab);
    switch(colorId) {
        case 0:
            inRange(labIm, Scalar(10, 8, -22), Scalar(75, 35, 45), mask);
        case 1:
            inRange(labIm, Scalar(13, -17, 12), Scalar(75, 18, 58), mask);
        case 2:
            inRange(labIm, Scalar(10, -33, 46), Scalar(75, -46, -13), mask);
    }
    //cv::floodFill(mask, Point(0,0), Scalar(255));
    cv::bitwise_and(*imBGR, *imBGR, result, mask);
    return result;
}
void setLabel(cv::Mat& im, const std::string label, const cv::Point & pt)
{
    int fontface = CV_FONT_HERSHEY_PLAIN;
    double scale = 1;
    int thickness = 1;
    int baseline = 0;

    cv::Size text = cv::getTextSize(label, fontface, scale, thickness, &baseline);
    cv::rectangle(im, pt + cv::Point(0, baseline), pt + cv::Point(text.width, -text.height), CV_RGB(0,0,0), CV_FILLED);
    cv::putText(im, label, pt, CV_FONT_HERSHEY_PLAIN, 1, cvScalar(255,255,250), 1, CV_AA);
}




