#define _CRT_SECURE_NO_WARNINGS
#include "ocr.h"

int stack[Max];
int top;

void REFERENCE(double **ref)
{
	Mat alphabet = imread("pictures/alphabet.bmp", 1);
	Mat alphabet2 = imread("pictures/alphabet2.bmp", 1);
	Mat number = imread("pictures/number.bmp", 1);

	std::vector<Mat>cap = LETTERS(alphabet, 0, 15);
	std::vector<Mat>lit = LETTERS(alphabet2, 0, 8);
	std::vector<Mat>num = LETTERS(number, 0, 15);

	for (int i = 0; i < 10; i++)
		ref[i] = LBP(num[i]);
	for (int i = 0; i < 26; i++)
		ref[i + 10] = LBP(cap[i]);
	for (int i = 0; i < 26; i++)
		ref[i + 36] = LBP(lit[i]);

}
std::vector<Mat> LETTERS(Mat input, int color, int pontsize)
{
	std::vector<Mat> outputs;
	Mat copy, line, indexmap, lettermap;
	int index[256][4] = { 0, };

	resize(input, input, Size(800, 800));
	cvtColor(input, copy, COLOR_BGR2GRAY);
	line = LINE(copy, color, pontsize);
	index[256][4] = { 0, };
	indexmap = PAINT(copy, line, color, pontsize, index);
	lettermap = input.clone();
	outputs = CROP(copy, indexmap, lettermap, index);

	//imshow("input", input);
	//imshow("threshold", copy);
	//imshow("Line", line);
	//imshow("Indexmap", indexmap);
	imshow("Letter map", lettermap);
	waitKey(0);
	//printf("\nTotal predicted = %d\n", outputs.size());

	return outputs;
}
void COMPARE(double **ref, std::vector<Mat> words)
{
	double* temp;
	for (int i = 0; i < words.size(); i++)
	{
		temp = LBP(words[i]);
		double max = 0;
		int maxindex = 0;
		for (int j = 0; j < 62; j++)
		{
			double score = SIMILARITY(ref[j], temp, 43095);
			if (max < score)
			{
				max = score;
				maxindex = j;
			}
		}

		if (max > 0.5)
		{
			printf("[%c]", letter_lookup[maxindex]);
			//printf("감지 성공!\n%c : %f %\n", letter_lookup[maxindex], max);
			//imshow("compare", words[i]);
			//waitKey(0);
		}
		else
		{
			//printf("감지 실패!\n%c : %f %\n", letter_lookup[maxindex], max);
			//imshow("compare", words[i]);
			//waitKey(0);
		}

	}
}
/*
*/
Mat LINE(Mat input, int color, int pontsize)
{
	Mat grad = GRADIENT(input);
	Mat edge = EDGE(grad);

	int average = 0;
	int count = 0;
	for (int y = 0; y < edge.rows; y++)
	{
		for (int x = 0; x < edge.cols; x++)
		{
			if (edge.at<uchar>(y, x) > 10)
			{
				average += edge.at<uchar>(y, x);
				count++;
			}
		}
	}
	threshold(edge, edge, (int)((average / count)), 255, THRESH_BINARY);

	Mat output = Mat::zeros(input.rows, input.cols, CV_8UC1);
	//imshow("!", edge);
	//waitKey();

	for (int y = 0; y < input.rows; y++)
	{
		for (int x = 0; x < input.cols; x++)
		{
			if (edge.at<uchar>(y, x) == 255)
			{
				Vec2d dir = grad.at<Vec2d>(y, x);
				double normalize = sqrt(dir[0] * dir[0] + dir[1] * dir[1]);
				double flag = 0;
				if (color)
				{
					dir[0] = dir[0] / normalize;
					dir[1] = dir[1] / normalize;
				}
				else
				{
					dir[0] = -dir[0] / normalize;
					dir[1] = -dir[1] / normalize;
				}

				for (double i = 0.25; i <= pontsize; i += 0.25)
				{
					int xx = x + (int)(round(dir[0] * i));
					int yy = y + (int)(round(dir[1] * i));
					if (xx < 0 || xx >= input.cols || yy < 0 || yy >= input.rows)
						break;

					if ((edge.at<uchar>(yy, xx) == 255) && ((xx != x) || (yy != y)))
					{
						Vec2d dir2 = grad.at<Vec2d>(yy, xx);
						double angle = (dir[0] * dir2[0] + dir[1] * dir2[1]) / (sqrt(dir2[0] * dir2[0] + dir2[1] * dir2[1]));
						angle = acos(angle) * 57.3;
						
						if (angle >= 150 || angle <= 30)
							flag = i;
						break;
					}
				}
				if (flag)
				{
					double oflag1 = 0.25;
					double oflag2 = 0.25;
					Vec2d odir;
					odir[0] = -dir[1];
					odir[1] = dir[0];
					int cx = (int)(round(dir[0] * flag / 2));
					int cy = (int)(round(dir[1] * flag / 2));

					///*
					int trigger = 0;
					int tx, ty;
					while (1)
					{
						tx = x + (int)(round(odir[0] * oflag1)) + cx;
						ty = y + (int)(round(odir[1] * oflag1)) + cy;

						if (tx < 0 || tx >= input.cols || ty < 0 || ty >= input.rows)
							break;

						if (edge.at<uchar>(ty, tx) == 255 && ((tx != x) || (ty != y)))
							trigger = 1;
						if (trigger == 1 && edge.at<uchar>(ty, tx) == 0)
							break;

						oflag1 += 0.25;
					}
					trigger = 0;
					while (1)
					{
						tx = x + (int)(round(-odir[0] * oflag2)) + cx;
						ty = y + (int)(round(-odir[1] * oflag2)) + cy;

						if (tx < 0 || tx >= input.cols || ty < 0 || ty >= input.rows)
							break;

						if (edge.at<uchar>(ty, tx) == 255 && ((tx != x) || (ty != y)))
							trigger = 1;
						if (trigger == 1 && edge.at<uchar>(ty, tx) == 0)
							break;

						oflag2 += 0.25;
					}

					if ((oflag1 + oflag2) > flag * 2.5)
					{
						for (double i = 0.25; i < flag; i += 0.25)
						{
							int xx = x + (int)(round(dir[0] * i));
							int yy = y + (int)(round(dir[1] * i));

							output.at<uchar>(yy, xx) = 255;
						}
					}
					//*/
					//imshow("a", output);
					//waitKey(1);
				}
			}
		}
	}
	return output;
}
Mat PAINT(Mat original, Mat line, int color, int pontsize, int index[256][4])
{
	int average = 0;
	int count = 0;
	int BLK = 7;
	for (int y = 0; y < line.rows - BLK; y++)
	{
		for (int x = 0; x < line.cols - BLK; x++)
		{
			int temp_average = 0;
			int trigger = 1;
			for (int yy = y; yy < y + BLK; yy++)
			{
				for (int xx = x; xx < x + BLK; xx++)
				{
					if (line.at<uchar>(yy, xx) == 255)
						temp_average += original.at<uchar>(yy, xx);

					else
					{
						trigger = 0;
						break;
					}
				}
				if (trigger == 0)
					break;
			}
			if (trigger)
			{
				average += temp_average;
				count += BLK * BLK;
			}
		}
	}

	//imshow("original", original);
	if (color)
		threshold(original, original, (int)((average / count) * 0.95), 255, THRESH_BINARY);
	else
		threshold(original, original, (int)((average / count) * 1.05), 255, THRESH_BINARY_INV);

	//imshow("thresholding", original);
	//imshow("line", line);
	//waitKey();
	Mat output = original.clone();

	count = 0;
	for (int y = 0; y < output.rows; y++)
	{
		for (int x = 0; x < output.cols; x++)
		{
			if ((line.at<uchar>(y, x) == 255) && (output.at<uchar>(y, x) == 255))
			{
				count++;
				int check = fill(original, line, output, count, pontsize, x, y, index[count]);
				//printf("*****%d*******\n", check);
				if (check == -1)
					count--;

			}
			if (count == 254)
				return output;

		}
	}

	return output;
}
std::vector<Mat> CROP(Mat input, Mat indexmap, Mat lettermap, int index[256][4])
{
	std::vector<Mat> outputs;
	for (int y = 0; y < indexmap.rows; y++)
	{
		int trigger = 0;
		for (int x = 0; x < indexmap.cols; x++)
		{
			if (indexmap.at<uchar>(y, x) != 255 && index[indexmap.at<uchar>(y, x)][0] != 0)
			{
				if (trigger)
				{
					int key = indexmap.at<uchar>(y, x);
					Mat temp = input(Range(index[key][1] - 2, index[key][3] + 2), Range(index[key][0] - 1, index[key][2] + 1));
					resize(temp, temp, Size(128, 128));
					threshold(temp, temp, 200, 255, THRESH_BINARY);
					outputs.push_back(temp);
					rectangle(lettermap, Rect(Point(index[key][0] - 1, index[key][1] - 2), Point(index[key][2] + 1, index[key][3] + 2)), Scalar(255, 0, 255), 1, 8, 0);

					x = index[key][2];
					y = (index[trigger][1] + index[trigger][3]) / 2;
					index[key][0] = 0;
					//imshow("%", temp);
					//waitKey();
					//imshow("a", lettermap);
					//waitKey();
				}
				else
				{
					trigger = indexmap.at<uchar>(y, x);
					//printf("%d, %d: %d %d\n", trigger, index[indexmap.at<uchar>(y, x)][2], x, y);
					x = 0;
					y = (index[trigger][1] + index[trigger][3]) / 2;
				}

			}
		}
	}

	return outputs;
}
/*
*/
int fill(Mat ref, Mat ref_line, Mat output, int filling, int pontsize, int x, int y, int index[4])
{
	//printf("fill\n");
	//filling = 200 - filling*2;
	int miny, minx, maxy, maxx;
	miny = minx = 10000;
	maxy = maxx = 0;
	int line = 0;
	int notline = 0;
	top = -1;
	push(x); push(y);
	int ox = x;
	int oy = y;
	while (top > 0)
	{
		y = pop();
		x = pop();
		//printf("%d %d %d\n", x, y, ref_original.at<uchar>(y, x));
		if ((x >= 2) && (x < output.cols - 2) && (y >= 2) && (y < output.rows - 2))
		{
			if (output.at<uchar>(y, x) == 255 && ref.at<uchar>(y, x) == 255)
			{
				//	printf("%d %d %d\n", x, y, ref_original.at<uchar>(y, x));
				if (ref_line.at<uchar>(y, x) == 255)
					line++;
				else notline++;

				if (miny > y) miny = y;
				else if (maxy < y) maxy = y;
				if (minx > x) minx = x;
				else if (maxx < x) maxx = x;

				output.at<uchar>(y, x) = filling;
				push(x - 1); push(y);
				push(x + 1); push(y);
				push(x); push(y - 1);
				push(x); push(y + 1);

				if (line + notline >= pontsize * 300) // 5000 -> pontsize 대체
				{
					//printf("**********%d %d*********\n", line, notline);
					remove(output, filling, ox, oy);
					return -1;
				}
			}
		}
	}

	if (((line + notline) <= 70 || line < notline)) //|| line <= pontsize / 4
	{
		//printf("%d %d %d\n", line, notline, filling);L
		remove(output, filling, ox, oy);
		return -1;
	}
	else
	{
		//printf("~~~~~~~~~%d~~~~~~~~\n", filling);
		//imshow("?", ref_original);
		//imshow("!", output);
		//imshow(",", ref_line);
		//waitKey(0);
		index[0] = minx;
		index[1] = miny;
		index[2] = maxx;
		index[3] = maxy;
		return 1;
	}
}
void remove(Mat tar, int remove, int x, int y)
{
	//printf("remove\n");
	top = -1;
	push(x); push(y);
	while (top > 0)
	{
		y = pop();
		x = pop();
		if ((x >= 2) && (x < tar.cols - 2) && (y >= 2) && (y < tar.rows - 2))
		{
			if ((tar.at<uchar>(y, x) == remove) || (tar.at<uchar>(y, x) == 255))  // 배경으로 확정된 관심영역은 삭제해도 된다
			{
				tar.at<uchar>(y, x) = 0;
				push(x - 1); push(y);
				push(x + 1); push(y);
				push(x); push(y - 1);
				push(x); push(y + 1);
			}
		}
	}
	//printf("^^^^^^^^^^%d^^^^^^^^^^\n", count);
	//imshow("&", tar);
	//waitKey(0);
	return;
}
/*
*/
Mat CONV(Mat input, Mat filter)
{
	Mat output = Mat::zeros(input.rows, input.cols, CV_64FC1);
	int fcx = filter.cols / 2;
	int fcy = filter.rows / 2;

	for (int cy = 0; cy < input.rows; cy++)
	{
		for (int cx = 0; cx < input.cols; cx++)
		{
			double value = 0;
			for (int i = -fcy; i <= fcy; i++)
			{
				for (int j = -fcx; j <= fcx; j++)
				{
					// fcx, fcy = 필터의 중심
					// i, j = input, filter의 중심으로부터 떨어진 칸수
					if (cx + j < 0 || cx + j >= input.cols || cy + i < 0 || cy + i >= input.rows)
						continue;
					if (fcx + j < 0 || fcx + j >= filter.cols || fcy + i < 0 || fcy + i >= filter.rows)
						continue;

					value += input.at<uchar>(cy + i, cx + j) * filter.at<double>(fcy + i, fcx + j);
				}
				output.at<double>(cy, cx) = value;
			}
		}
	}

	return output;
}
Mat GRADIENT(Mat input)
{
	if (input.channels() == 3)
		cvtColor(input, input, COLOR_BGR2GRAY);
	Mat output(input.rows, input.cols, CV_64FC2);
	double difference[] = { -1,0,1 };
	Mat edge_x(1, 3, CV_64FC1, difference);
	Mat edge_y(3, 1, CV_64FC1, difference);
	Mat grad_x = CONV(input, edge_x);
	Mat grad_y = CONV(input, edge_y);
	for (int y = 0; y < input.rows; y++)
	{
		for (int x = 0; x < input.cols; x++)
		{
			output.at<Vec2d>(y, x) = Vec2d(grad_x.at<double>(y, x), grad_y.at<double>(y, x));
		}
	}

	return output;
}
Mat EDGE(Mat input)
{
	Mat result(input.rows, input.cols, CV_64FC1);

	for (int y = 0; y < input.rows; y++)
	{
		for (int x = 0; x < input.cols; x++)
		{
			double fx = input.at<Vec2d>(y, x)[0];
			double fy = input.at<Vec2d>(y, x)[1];

			result.at<double>(y, x) = std::sqrt(fx * fx + fy * fy);
		}
	}


	double max = 0;
	double min = 0;
	double ratio = 255;  // normalize factor
	for (int y = 0; y < result.rows; y++)
	{
		for (int x = 0; x < result.cols; x++)
		{
			double value = result.at<double>(y, x);
			if (value > max)
				max = value;
			else if (value < min)
				min = value;
		}
	}
	if (max != min)
		ratio = 255 / (max - min);

	Mat output(result.rows, result.cols, CV_8UC1);
	for (int y = 0; y < result.rows; y++)
	{
		for (int x = 0; x < result.cols; x++)
		{
			output.at<uchar>(y, x) = (int)(result.at<double>(y, x) * ratio);
		}
	}

	return output;
}
double SIMILARITY(double* input1, double* input2, int size)
{
	double similarity = 0;
	double inner = 0;
	double anorm = 0;
	double bnorm = 0;
	//printf("%f\n", similarity);
	for (int i = 0; i < size; i++)
	{
		inner += input1[i] * input2[i];
		anorm += input1[i] * input1[i];
		bnorm += input2[i] * input2[i];
	}

	if (anorm == 0 || bnorm == 0)
		similarity = 0;
	else
	{
		similarity = inner / (std::sqrt(anorm) * sqrt(bnorm));
	}

	return similarity;
}
void NORMALIZE(double* input, double inputsize)
{
	double value = 0;
	for (int i = 0; i < inputsize; i++)
		value += input[i] * input[i];
	if (value != 0)
	{
		value = std::sqrt(value);
		for (int i = 0; i < inputsize; i++)
			input[i] = input[i] / value;
	}
}
double* LBP(Mat img)
{
	int WIN = 128;

	Mat LBP(WIN, WIN, CV_8UC1);
	for (int y = 1; y < WIN - 1; y++)
	{
		for (int x = 1; x < WIN - 1; x++)
		{
			uchar pixel = img.at<uchar>(y, x);
			int lbp = 0;

			if (pixel > img.at<uchar>(y, x + 1)) lbp += 1;
			if (pixel > img.at<uchar>(y + 1, x + 1)) lbp += 2;
			if (pixel > img.at<uchar>(y + 1, x)) lbp += 4;
			if (pixel > img.at<uchar>(y + 1, x - 1)) lbp += 8;
			if (pixel > img.at<uchar>(y, x - 1)) lbp += 16;
			if (pixel > img.at<uchar>(y - 1, x - 1)) lbp += 32;
			if (pixel > img.at<uchar>(y - 1, x)) lbp += 64;
			if (pixel > img.at<uchar>(y - 1, x + 1)) lbp += 128;

			LBP.at<uchar>(y, x) = lbp;
		}
	}
	int BLK = WIN / 4;
	int interval = BLK / 4;
	int block = (WIN - BLK) / interval + 1;
	int features = 255;
	double* output = (double*)calloc(block * block * features, sizeof(double));
	double* temp = (double*)calloc(features, sizeof(double));
	//printf("%d\n", block * block * features);
	for (int y = 0; y <= WIN - BLK; y += interval)
	{
		for (int x = 0; x <= WIN - BLK; x += interval)
		{
			int by = y / interval;
			int bx = x / interval;

			for (int i = 0; i < features; i++)
				temp[i] = 0;
			for (int yy = y; yy < y + BLK; yy++)
				for (int xx = x; xx < x + BLK; xx++)
					if (LBP.at<uchar>(yy, xx) != 0)
						temp[LBP.at<uchar>(yy, xx) - 1] += 1;
			
			NORMALIZE(temp, features);

			for (int i = 0; i < features; i++)
				output[by * block * features + bx * features + i] = temp[i];
			
		}
	}

	free(temp);
	return output;
}
/*
*/
int push(int n)
{
	if (top >= Max - 1)
	{
		printf("Stack Overflow!\n");
		return -1;
	}


	stack[++top] = n;
	return n;
};
int pop()
{
	if (top < 0)
	{
		printf("Stack Underflow\n");
		return -1;
	}
	else
		return stack[top--];
}