#ifndef SIGNCOLORIMAGE_H
#define SIGNCOLORIMAGE_H
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

using namespace std;
using namespace cv;

class SignColorImage
{
private:
    Mat img;
public:
    SignColorImage(Mat imgRgb){
        printf("Created!");
        img = imgRgb;
    }
    Mat getRedSigns(){
        return img;
    }
    Mat getBlueSigns(){
        return img;
    }
    Mat getYellowSigns(){
        return img;
    }
    ~SignColorImage(){
        delete[] Mat;
    }

};



#endif // SIGNCOLORIMAGE_H
