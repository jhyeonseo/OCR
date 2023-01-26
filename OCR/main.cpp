#define _CRT_SECURE_NO_WARNINGS
#include "ocr.h"

const char letter_lookup[62] = {
'0','1','2','3','4','5','6','7','8','9',
'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z'
,'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z' };


int main()
{
	Mat alphabet = imread("pictures/alphabet.bmp", 1);
	Mat alphabet2 = imread("pictures/alphabet2.bmp", 1);
	Mat number = imread("pictures/number.bmp", 1);
	Mat example = imread("pictures/sign2.bmp", 1);
	/*
	std::vector<Mat>cap = letters(alphabet, 0, 50);
	std::vector<Mat>lit = letters(alphabet2, 0, 50);
	std::vector<Mat>num = letters(number, 0, 50);
	double* ref[62];
	for (int i = 0; i < 10; i++)
		ref[i] = LBP(num[i]);
	for (int i = 0; i < 26; i++)
		ref[i + 10] = LBP(cap[i]);
	for (int i = 0; i < 26; i++)
		ref[i + 36] = LBP(lit[i]);
	
	*/
//	std::vector<Mat>word2s = letters(alphabet, 0, 50);
	std::vector<Mat>words = letters(example, 1, 5);

	/*
	double* temp;
	for (int i = 0; i < words.size(); i++)
	{
		temp = LBP(words[i]);
		double max = 0;
		int maxindex = 0;
		for (int j = 0; j < 62; j++)
		{
			double score = COMPARE(ref[j], temp, 42926);
			if (max < score)
			{
				max = score;
				maxindex = j;
			}
		}

		if (max > 0.6)
		{
			printf("감지 성공!\n%c : %f %\n", letter_lookup[maxindex], max);
			imshow("compare", words[i]);

			waitKey(0);
		}
		else
		{
			printf("감지 실패!\n%c : %f %\n", letter_lookup[maxindex], max);
			imshow("compare", words[i]);
			waitKey(0);
		}

	}
	*/

	return 0;
}