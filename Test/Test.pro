TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt


SOURCES += main.cpp
INCLUDEPATH += D:\opencv\build\include

LIBS += D:\opencv\build\qt\bin\libopencv_core2413.dll
LIBS += D:\opencv\build\qt\bin\libopencv_highgui2413.dll
LIBS += D:\opencv\build\qt\bin\libopencv_imgproc2413.dll
LIBS += D:\opencv\build\qt\bin\libopencv_features2d2413.dll
LIBS += D:\opencv\build\qt\bin\libopencv_calib3d2413.dll
LIBS += D:\opencv\build\qt\bin\libopencv_objdetect2413.dll
LIBS += D:\opencv\build\qt\bin\libopencv_gpu2413.dll

