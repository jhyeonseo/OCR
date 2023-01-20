#define _CRT_SECURE_NO_WARNINGS
#include "ocr.h"

const char letter_lookup[52] = {
'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z'
,'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z' };


int main()
{
	
	Mat alphabet = imread("pictures/alphabet.bmp", 1);
	Mat alphabet2 = imread("pictures/alphabet2.bmp", 1);
	Mat number = imread("pictures/number.bmp", 1);
	Mat example = imread("pictures/example6.bmp", 1);
	/*
	std::vector<Mat>cap = letters(alphabet, 0, 15);
	std::vector<Mat>lit = letters(alphabet2, 0, 10);
	double* ref[52];
	for (int i = 0; i < 26; i++)
	{
		ref[i * 2] = LBP(cap[i]);
		ref[i * 2 + 1] = LBP(lit[i]);
	}
	*/

	std::vector<Mat>words = letters(example, 0, 15);

	/*
	double* temp;
	for (int i = 0; i < words.size(); i++)
	{
		temp = LBP(words[i]);
		double max = 0;
		int maxindex = 0;
		for (int j = 0; j < 52; j++)
		{
			if (max < COMPARE(ref[j], temp, 828495))
			{
				max = COMPARE(ref[j], temp, 828495);
				maxindex = j;
			}
		}

		if (max > 0.5)
		{
			printf("감지 성공!\n%c : %f %\n", letter_lookup[maxindex / 2], max);
			imshow("compare", words[i]);
			if (maxindex % 2 == 0)
				imshow("ref", cap[maxindex / 2]);
			else
				imshow("ref", lit[maxindex / 2]);
			waitKey(0);
		}


	}
	*/

	return 0;
}