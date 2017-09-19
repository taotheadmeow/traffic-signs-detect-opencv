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


Mat signDetect( Mat frame , CascadeClassifier classifier, const Scalar& color, String signName);
//void startDetect(VideoCapture stream, CascadeClassifier classifier);
//gpu::GpuMat drawRoi( gpu::GpuMat frame, vector<Rect>* rawRois);


CascadeClassifier stopSignCascade;
CascadeClassifier leftSignCascade;
CascadeClassifier rightSignCascade;
CascadeClassifier leftorrightSignCascade;
CascadeClassifier xJunctionSignCascade;

int main(int, char**)
{
    VideoCapture cap(0);
    Mat f;
    // open the default camera, use something different from 0 otherwise;
    // Check VideoCapture documentation.
    if(!stopSignCascade.load( "r-stop-SignDetector2.xml" )){
        printf("Error loading file1!\n");
        return 0;
    };
    if(!leftSignCascade.load( "w-curve-left-SignDetector2.xml" )){
        printf("Error loading file2!\n");
        return 0;
    };
    if(!xJunctionSignCascade.load( "w-junction-xshape-SignDetector2.xml" )){
        printf("Error loading file3!\n");
        return 0;
    };
    if(!rightSignCascade.load( "w-curve-right-SignDetector2.xml" )){
        printf("Error loading file4!\n");
        return 0;
    };
    if(!leftorrightSignCascade.load( "r-leftorright-SignDetector2.xml" )){
        printf("Error loading file5!\n");
        return 0;
    };
    //start detect
    LARGE_INTEGER frequency;
    LARGE_INTEGER start;
    LARGE_INTEGER end;
    for(;;){

        QueryPerformanceFrequency(&frequency);

            QueryPerformanceCounter(&start);
        cap.read(f);
        std::future<Mat> stopSignsRoi = std::async(signDetect, f, stopSignCascade, Scalar (0,0,255), "Stop");
        std::future<Mat> leftSignsRoi = std::async(signDetect, f, leftSignCascade, Scalar (0,255,255), "Left Curve");
        std::future<Mat> xJunctionSignRoi = std::async(signDetect, f, xJunctionSignCascade, Scalar (0,255,255), "4-way Intersection");
        std::future<Mat> rightSignRoi = std::async(signDetect, f, rightSignCascade, Scalar (0,255,255), "Right Curve");
        std::future<Mat> leftorrightSignRoi = std::async(signDetect, f, leftorrightSignCascade, Scalar (0,255,0), "Left or Right Only");
        xJunctionSignRoi.get();
        rightSignRoi.get();
        stopSignsRoi.get();
        leftorrightSignRoi.get();
        f = leftSignsRoi.get();
        QueryPerformanceCounter(&end);
        double interval = static_cast<double>(end.QuadPart- start.QuadPart) /
                              frequency.QuadPart;

        rectangle(f, cvPoint(20,20), cvPoint(50,10), cvScalar(0,0,0,128), CV_FILLED);
        putText(f, to_string((int) (1.0/interval)), cvPoint(20,20),
            CV_FONT_HERSHEY_PLAIN, 0.8, cvScalar(255,255,250), 1, CV_AA);
        imshow("test2", (f));
        int c = waitKey(10);
        if( (char)c == 'c' ) { break; }
    }

    // the camera will be closed autogpu::GpuMatically upon exit
    // cap.close();
    return 0;
}
Mat signDetect( Mat frame , CascadeClassifier classifier, const Scalar& color, String signName)
{
  std::vector<Rect> rawRois;
Mat frame_gray;

  //gpu::GpuMat m = gpu::GpuMat(frame_gray.size(), CV_8UC3); //3-channel
  //m = Scalar(0,0,0);  //Scalar.val[0-2] used
  cvtColor( frame, frame_gray, CV_BGR2GRAY );
  equalizeHist( frame_gray, frame_gray );

  //-- Detect
  classifier.detectMultiScale( frame_gray, rawRois, 1.1, 2, 0, Size(35, 35), Size(200, 200));
  for( size_t i = 0; i < rawRois.size(); i++ )
  {
    Point p1( rawRois[i].x, rawRois[i].y);
    Point p2( rawRois[i].x + rawRois[i].width, rawRois[i].y + rawRois[i].height);

    rectangle(frame, p1, p2, color, 2);
    putText(frame, signName, cvPoint(p1.x, p1.y+10),
        CV_FONT_HERSHEY_PLAIN, 0.8, cvScalar(150,150,200), 1, CV_AA);
  }
//  Mat image_cpu;
//  Mat obj_host;
//  // download only detected number of rectangles
//  objBuff.colRange(0, detections_number).download(obj_host);

//  Rect* signs = obj_host.ptr<Rect>();
//  for(int i = 0; i < detections_number; ++i)
//     cv::rectangle(image_cpu, signs[i], Scalar(255));
  return frame;
  //-- Show what you got
 }

