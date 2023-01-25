#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <cmath>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/photo.hpp>

#define Max 1000000

using namespace cv;

std::vector<Mat> letters(Mat input, int color, int pontwidth);
Mat threshold(Mat input, int color, int threshold);
Mat CONV(Mat input, Mat filter);
Mat GRADIENT(Mat input);
Mat threshold(Mat input, int color, int threshold);
Mat LINE(Mat input,int color, int pontwidth);
Mat paint(Mat ref, Mat tar, int& count, int pontsize);
int fill(Mat ref, Mat tar, int fill, int x, int y, int pontsize);
void remove(Mat tar, int remove, int x, int y);
double COMPARE(double* input1, double* input2, int size);
void NORMALIZE(double* input, double inputsize);
double* LBP(Mat img);
Mat EDGE(Mat input);
int push(int n);
int pop();