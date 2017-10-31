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


using namespace std;
using namespace cv;


void signDetect(VideoCapture cap , CascadeClassifier classifier, int id);
void setLabel(Mat& im, const std::string label, const cv::Point & pt);
Mat colorSegment(Mat *imBGR, int colorId);
inline std::string narrow(std::wstring const& text)
{
    std::locale const loc("");
    wchar_t const* from = text.c_str();
    std::size_t const len = text.size();
    std::vector<char> buffer(len + 1);
    std::use_facet<std::ctype<wchar_t> >(loc).narrow(from, from + len, '_', &buffer[0]);
    return std::string(&buffer[0], &buffer[len]);
}
int SIGNS_NUM = 0;
std::vector<CascadeClassifier> classifiers;
Mat MASTER_FRAME;
std::vector<std::vector<Rect>> rawRois;
bool isRun = true;
double interval;
int main(int, char**)
{
    vector<String> signName;
    vector<String> xmlFilesName;
    cout << "Loading files...\n";
    //get list of files in detectors dir
    WIN32_FIND_DATA search_data;

       memset(&search_data, 0, sizeof(WIN32_FIND_DATA));
       std::wstring path = L".\\detectors\\*.xml";
       HANDLE handle = FindFirstFile(path.c_str(), &search_data);

       while(handle != INVALID_HANDLE_VALUE)
       {
          xmlFilesName.push_back(".\\detectors\\" + narrow(search_data.cFileName));
          signName.push_back(narrow(search_data.cFileName).erase(narrow(search_data.cFileName).find_last_of("."), string::npos));

          rawRois.push_back(std::vector<Rect>());
          classifiers.push_back(CascadeClassifier());
          if(FindNextFile(handle, &search_data) == FALSE)
            break;
       }

       //Close the handle after use or memory/resource leak
       FindClose(handle);
     //end file finder

    for(int i = 0; i < xmlFilesName.size(); i++){
        if(!classifiers[i].load( xmlFilesName[i] )){
            cout << "Can't load \"" << xmlFilesName[i] << "\".\n";
        }
        else{
            cout << "Loaded \"" << xmlFilesName[i] << "\" as \"" << signName[i] << "\".\n";
            SIGNS_NUM++;
        }
    }
    cout << SIGNS_NUM << " files loaded.\n";
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
        detection[i] = std::thread(signDetect, cap, classifiers[i], i);}

    LARGE_INTEGER frequency;
    LARGE_INTEGER start;
    LARGE_INTEGER end;
    QueryPerformanceFrequency(&frequency);
    cout << "Started! Press 'C' to stop.\n";

   for(;;){
       QueryPerformanceCounter(&start);
       cap.read(MASTER_FRAME);
       for(int id = 0; id < SIGNS_NUM; id++){
           for( size_t i = 0; i < rawRois[id].size(); i++ )
           {
                 Point p1( rawRois[id][i].x, rawRois[id][i].y);
                 Point p2( rawRois[id][i].x + rawRois[id][i].width, rawRois[id][i].y + rawRois[id][i].height);

                 rectangle(MASTER_FRAME, p1, p2, Scalar (0,255,0), 2);
                 setLabel(MASTER_FRAME, signName[id], p1);
           }

       }
       QueryPerformanceCounter(&end);
       interval = static_cast<double>(end.QuadPart- start.QuadPart) /
                                     frequency.QuadPart;
       if(interval<0.067){
            Sleep((0.067-interval)*1000);
            interval = 0.067;
       }
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
void signDetect( VideoCapture cap , CascadeClassifier classifier, int id)
{
    Mat frame_gray;
    Mat frame;
  //-- Detect
  for(;;){
      if(interval<0.067){
           Sleep((0.067-interval)*1000);
           interval = 0.067;
      }
      cap.read(frame);
      //frame = colorSegment(&frame, signColor); <- will use if color segment function is done!
      cvtColor( frame, frame_gray, CV_BGR2GRAY );
      equalizeHist( frame_gray, frame_gray );
      classifier.detectMultiScale( frame_gray, rawRois[id], 1.1, 2, 0, Size(12, 12), Size(300, 300));
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




