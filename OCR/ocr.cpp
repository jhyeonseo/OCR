#define _CRT_SECURE_NO_WARNINGS
#include "ocr.h"

int stack[Max];
int top;

std::vector<Mat> letters(Mat input, int color, int pontsize)
{
	Mat copy;
	int count;
	std::vector<Mat> outputs;
	pontsize = pontsize * 50;
	resize(input, input, Size(800, 800));
	cvtColor(input, copy, COLOR_BGR2GRAY);
	Mat line = LINE(copy, color, pontsize);
	if (color)
		copy = threshold(copy, color, 200);
	else
		copy = threshold(copy, color, 100);
	
	Mat indexmap = paint(copy, line, count, pontsize);
	Mat lettermap = input.clone();
	for (int c = 1; c <= count; c++)
	{
		int minx, miny;
		minx = miny = 10000;
		int maxx, maxy;
		maxx = maxy = -1;
		int trigger = 1;
		for (int y = 2; y < indexmap.rows - 2; y++)
		{
			for (int x = 2; x < indexmap.cols - 2; x++)
			{
				if (indexmap.at<uchar>(y, x) == c)
				{
					if (maxx < x)
						maxx = x;
					if (minx > x)
						minx = x;
					if (maxy < y)
						maxy = y;
					if (miny > y)
						miny = y;

					trigger = 0;
				}
			}
		}
		if (trigger || maxx == minx || maxy == miny)
		{
			printf("triggered: %d\n", c);
			continue;
		}
		
		Mat temp = copy(Range(miny - 2, maxy + 2), Range(minx - 0, maxx + 0));
		resize(temp, temp, Size(256, 256));
		temp = threshold(temp, 1, 200);
		outputs.push_back(temp);
		rectangle(lettermap, Rect(Point(minx - 0, miny - 2), Point(maxx + 0, maxy + 2)), Scalar(255, 0, 255), 1, 8, 0);
	}

	imshow("input", input);
	imshow("threshold", copy);
	imshow("refined", line);
	//imshow("indexmap", indexmap);
	imshow("Letter map", lettermap);
	waitKey(0);
	//printf("\nTotal predicted = %d\n", outputs.size());

	return outputs;
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
/*
*/
Mat threshold(Mat input, int color, int threshold)
{
	Mat output(input.rows, input.cols, CV_8UC1);
	for (int y = 0; y < input.rows; y++)
	{
		for (int x = 0; x < input.cols; x++)
		{
			if (color)
				if (input.at<uchar>(y, x) > threshold)
					output.at<uchar>(y, x) = 255;
				else
					output.at<uchar>(y, x) = 0;
			else
				if (input.at<uchar>(y, x) < threshold)
					output.at<uchar>(y, x) = 255;
				else
					output.at<uchar>(y, x) = 0;
		}
	}

	return output;
}
Mat LINE(Mat input,int color, int pontsize)
{
	pontsize /= 50;
	Mat grad = GRADIENT(input);
	Mat output = Mat::zeros(input.rows, input.cols, CV_8UC1);
	Mat edge = EDGE(grad);
	threshold(edge, edge, 100, 255, THRESH_BINARY);
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

				for (double i = 0; i <= pontsize; i += 0.25)
				{
					int xx = x + (int)(round(dir[0] * i));
					int yy = y + (int)(round(dir[1] * i));
					if (xx < 0 || xx >= input.cols || yy < 0 || yy >= input.rows)
						break;

					if ((edge.at<uchar>(yy, xx) == 255) && ((xx != x) || (yy != y)))
					{
						flag = i;
						break;
					}
				}
				if (flag)
				{
					double oflag = 0;
					Vec2d odir;
					odir[0] = -dir[1];
					odir[1] = dir[0];
					int cx = (int)(round(dir[0] * (flag - 0.25) / 2));
					int cy = (int)(round(dir[1] * (flag - 0.25) / 2));

					///*
					int trigger = 0;
					while (1)
					{
						int tx = x + (int)(round(odir[0] * oflag)) + cx;
						int ty = y + (int)(round(odir[1] * oflag)) + cy;

						if (tx < 0 || tx >= input.cols || ty < 0 || ty >= input.rows)
							break;
						
						if (edge.at<uchar>(ty, tx) == 255)
							trigger = 1;
						if (trigger == 1 && edge.at<uchar>(ty, tx) == 0)
							trigger = 2;
						
						if (trigger == 2)
						{
							if (oflag / flag > 1.5)
							{
								for (double i = 0; i < flag; i += 0.25)
								{
									int xx = x + (int)(round(dir[0] * i));
									int yy = y + (int)(round(dir[1] * i));

									output.at<uchar>(yy, xx) = 255;
								}
							}
							break;
						}

						oflag += 0.25;
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
Mat paint(Mat original, Mat line, int& count, int pontsize)
{
	Mat output = original.clone();
	count = 0;

	for (int y = 0; y < output.rows; y++)
	{
		int trigger = 0;
		for (int x = 0; x < output.cols; x++)
		{
			if (line.at<uchar>(y, x) == 255)
			{
				if (trigger)
				{
					count++;
					int check = fill(line, output, count, x, y, pontsize);
					if (check == -1)
					{
						remove(output, count, x, y);
						count--;
					}
				}
				else
				{
					int check = fill(line, output.clone(), count, x, y, pontsize);
					if (check != -1)
					{
						trigger = 1;
						y = check;
						x = 0;
					}
				}
				
				//imshow("!", output);
				//waitKey(1);
			}
			if (count == 254)
				return output;
		}
	}
	return output;
}
/*
*/
int fill(Mat ref, Mat output, int fill, int x, int y, int pontsize)
{
	int line = 0;
	int notline = 0;

	int miny = 1000;
	int maxy = 0;

	top = -1;
	push(x); push(y);
	while (top > 0)
	{
		y = pop();
		x = pop();
		if (line >= pontsize * 4)
			break;

		if ((x >= 0) && (x < output.cols) && (y >= 0) && (y < output.rows))
		{
			if (output.at<uchar>(y, x) == 255)
			{
				if (ref.at<uchar>(y, x) == 255)
					line++;
				else notline++;

				if (miny > y) miny = y;
				else if (maxy < y) maxy = y;

				output.at<uchar>(y, x) = fill;
				push(x - 1); push(y);
				push(x + 1); push(y);
				push(x); push(y - 1);
				push(x); push(y + 1);
			}
		}
	}

//	printf("%d\n", line);
	if (line >= pontsize * 4 || line <= 30 || line < notline)//|| line <= pontsize / 4
		return -1;
	else
		return (miny + maxy) / 2;
}
void remove(Mat tar, int remove, int x, int y)
{
	top = -1;
	push(x); push(y);
	while (top > 0)
	{
		y = pop();
		x = pop();
		if ((x >= 0) && (x < tar.cols) && (y >= 0) && (y < tar.rows))
		{
			if ((tar.at<uchar>(y, x) == remove) || (tar.at<uchar>(y, x) == 255))
			{
				tar.at<uchar>(y, x) = 0;

				push(x - 1); push(y);
				push(x + 1); push(y);
				push(x); push(y - 1);
				push(x); push(y + 1);
			}
		}
	}

	return;
}
/*
*/
double COMPARE(double* input1, double* input2, int size)
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
	int WIN = 256;

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
	int features = 254;
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