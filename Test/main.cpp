#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect/objdetect.hpp>

using namespace std;
using namespace cv;

Mat signDetect( Mat frame , CascadeClassifier classifier);
String stopSignCascadeName = "r-stop-SignDetector.xml";


CascadeClassifier stopSignCascade;
CascadeClassifier leftSignCascade;
int main(int, char**)
{
    VideoCapture cap(0);
    Mat frame;
    // open the default camera, use something different from 0 otherwise;
    // Check VideoCapture documentation.
    if(!stopSignCascade.load( stopSignCascadeName )){
        printf("Error loading file!\n");
        return 0;
    };
    if(!leftSignCascade.load( "w-curve-left-SignDetector.xml" )){
        printf("Error loading file2!\n");
        return 0;
    };

    for(;;)
    {
          cap.read(frame);
          if( frame.empty() ) break; // end of video stream
          frame = signDetect(frame, stopSignCascade);
          frame = signDetect(frame, leftSignCascade);
          imshow("Traffic Sign Recognition Demo", frame);
          if( waitKey(10) == 27 ) break; // stop capturing by pressing ESC
    }
    // the camera will be closed automatically upon exit
    // cap.close();
    printf("Ended!");
    return 0;
}
Mat signDetect( Mat frame , CascadeClassifier classifier)
{
  std::vector<Rect> rawRois;
  Mat frame_gray;

  cvtColor( frame, frame_gray, CV_BGR2GRAY );
  equalizeHist( frame_gray, frame_gray );

  //-- Detect
  classifier.detectMultiScale( frame_gray, rawRois, 1.1, 2, 0, Size (20,20), Size (500, 500));

  for( size_t i = 0; i < rawRois.size(); i++ )
  {
    Point p1( rawRois[i].x, rawRois[i].y);
    Point p2( rawRois[i].x + rawRois[i].width, rawRois[i].y + rawRois[i].height);
    rectangle( frame, p1, p2, Scalar (0,255,0));
    Mat detectedROI = frame_gray( rawRois[i] );

  }
  //-- Show what you got
  return frame;
 }
