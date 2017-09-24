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
#include <mutex>
#define SIGNS_NUM 5

using namespace std;
using namespace cv;


void signDetect(VideoCapture cap , CascadeClassifier classifier, int id);
CascadeClassifier classifiers[SIGNS_NUM];
Mat MASTER_FRAME;
std::vector<Rect> rowRois[SIGNS_NUM];
bool isRun = true;
int main(int, char**)
{
    cout << "Loading files....\n";
    String signs_name[SIGNS_NUM] = {
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
    for(int i = 0; i < SIGNS_NUM; i++){
        if(!classifiers[i].load( xmlFilesName[i] )){
            cout << "Can't load" << xmlFilesName[i] << "\n";
            return 0;
        }
        else{
            cout << "Loaded " << xmlFilesName[i] << " as " << signs_name[i] << "\n";
        }
    }

    cout << "Looking for default camera...." << "\n";
    VideoCapture cap(0);
    if(!cap.isOpened()){
        cout << "Camera not founded!";
        return -1;
    }


    cap.read(MASTER_FRAME);

    //start detect
    cout << "Running..." << "\n";
    std::thread stopSign(signDetect, cap, classifiers[0], 0);
    std::thread leftSign(signDetect, cap, classifiers[1], 1);
    std::thread xJunctionSign(signDetect, cap, classifiers[2], 2);
    std::thread rightSign(signDetect, cap, classifiers[3], 3);
    std::thread leftorrightSign(signDetect, cap, classifiers[4], 4);
    LARGE_INTEGER frequency;
    LARGE_INTEGER start;
    LARGE_INTEGER end;
    QueryPerformanceFrequency(&frequency);
   for(;;){
       QueryPerformanceCounter(&start);
       cap.read(MASTER_FRAME);
       for(int id = 0; id < SIGNS_NUM; id++){
           for( size_t i = 0; i < rowRois[id].size(); i++ )
           {
                 Point p1( rowRois[id][i].x, rowRois[id][i].y);
                 Point p2( rowRois[id][i].x + rowRois[id][i].width, rowRois[id][i].y + rowRois[id][i].height);
                 rectangle(MASTER_FRAME, p1, p2, Scalar (0,255,0), 2);
                 putText(MASTER_FRAME, signs_name[id], cvPoint(p1.x, p1.y+10),
                     CV_FONT_HERSHEY_PLAIN, 0.8, cvScalar(150,150,200), 1, CV_AA);
           }

       }
       QueryPerformanceCounter(&end);
       double interval = static_cast<double>(end.QuadPart- start.QuadPart) /
                                     frequency.QuadPart;

       rectangle(MASTER_FRAME, cvPoint(20,20), cvPoint(50,10), cvScalar(0,0,0,128), CV_FILLED);
       putText(MASTER_FRAME, to_string((int) (1.0/interval)), cvPoint(20,20),
                   CV_FONT_HERSHEY_PLAIN, 0.8, cvScalar(255,255,250), 1, CV_AA);
       imshow("Traffic Signs Recognition", (MASTER_FRAME));

       int c = waitKey(10);
       if( (char)c == 'c' ) { isRun = false; break; }


  }
       stopSign.join();
       leftSign.join();
       xJunctionSign.join();
       rightSign.join();
       leftorrightSign.join();

    // the camera will be closed automatically upon exit
    // cap.close();
    return 0;
}
void signDetect( VideoCapture cap , CascadeClassifier classifier, int id)
{
    Mat frame_gray;
    Mat frame;
  //-- Detect
  for(;;){
      cap.read(frame);
      cvtColor( frame, frame_gray, CV_BGR2GRAY );
      equalizeHist( frame_gray, frame_gray );
      classifier.detectMultiScale( frame_gray, rowRois[id], 1.1, 2, 0, Size(35, 35), Size(300, 300));
      if(!isRun){
          break;
      }
  }

 }

