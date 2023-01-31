#define _CRT_SECURE_NO_WARNINGS
#include "ocr.h"


int main()
{
	double* ref[62];
	REFERENCE(ref);
	Mat example = imread("pictures/movie2.bmp", 1);

	std::vector<Mat>words = LETTERS(example, 1, 10);
	COMPARE(ref, words);


	return 0;
}