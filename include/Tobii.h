#ifndef TOBII_H
#define TOBII_H


#pragma comment(lib, "ws2_32.lib")

// zeromq lib
#include <zmq.h>
#include <zhelpers.h>

#include <conio.h>
#include <Windows.h>
#include <atltypes.h>
#include <assert.h>
#include <EyeX.h>
#include <time.h>
#include <iostream>
//opencv ¿â
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>
//tobii³õÊ¼»¯
BOOL tobii_init();

//tobii×¢Ïú
void tobii_uninit();

void DrawAttentionPicture(cv::Mat &eyeImage, const int x, const int y);



#endif