#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fstream>
#include <iostream>
#include <cmath>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/photo.hpp>

#define Max 10000000

using namespace cv;

const char letter_lookup[62] = {
'0','1','2','3','4','5','6','7','8','9',
'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z'
,'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z' };

void REFERENCE(double** ref);
std::vector<Mat> LETTERS(Mat input, int color, int pontsize);
void COMPARE(double** ref, std::vector<Mat> words);


Mat CONV(Mat input, Mat filter);
Mat GRADIENT(Mat input);
Mat threshold(Mat input, int color, int threshold);
Mat LINE(Mat input,int color, int pontwidth);
Mat PAINT(Mat ref, Mat tar, int color, int pontsize, int index[256][4]);
std::vector<Mat> CROP(Mat input, Mat indexmap, Mat lettermap, int index[256][4]);
int fill(Mat ref_original, Mat ref_line, Mat output, int fill, int pontsize, int x, int y, int index[4]);
void remove(Mat tar, int remove, int x, int y);
double SIMILARITY(double* input1, double* input2, int size);
void NORMALIZE(double* input, double inputsize);
double* LBP(Mat img);
Mat EDGE(Mat input);
int push(int n);
int pop();

