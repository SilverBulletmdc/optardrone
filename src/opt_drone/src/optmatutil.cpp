#include "stdafx.h"
#include <cv.h>

#include "optutil.h"
#include "optmatutil.h"

using namespace cv;
using namespace std;

Vec2i foeForDenseMat(Mat flow){
	return Vec2i(0, 0);
}

float ttcForDenseMat(Mat flow, int foeY, float *ttc){
	return 0;
}

float balanceForDenseMat(Mat flow, Mat framedst, float k, int px, int py){
	Vec2i leftSumFlow = Vec2i(0, 0);
	float up = EDGE*HEIGHT;
	float down = (1-EDGE)*HEIGHT;
	for (int i = 0; i < px; i++)
	{
		for (int j = up; j < down; j++)
		{
			leftSumFlow[0] += flow.at<Vec2i>(j, i)[0];
			leftSumFlow[1] += flow.at<Vec2i>(j, i)[1];
		}
	}
	Vec2i rightSumFlow = Vec2i(0, 0);
	for (int i = px; i < WIDTH; i++)
	{
		for(int j = up; j < down; j++){
			rightSumFlow[0] += flow.at<Vec2i>(j, i)[0];
			rightSumFlow[1] += flow.at<Vec2i>(j, i)[1];
		}
	}

	leftSumFlow[0] = abs(leftSumFlow[0] / px);
	leftSumFlow[1] = abs(leftSumFlow[1] / px);
	rightSumFlow[0] = abs(rightSumFlow[0] / (WIDTH - px));
	rightSumFlow[1] = abs(rightSumFlow[1] / (WIDTH - px));

	bool isBig = isBigObstacleMat(framedst, flow);
	float result = balanceControlLR(isBig, leftSumFlow[0], rightSumFlow[0], k);

	Vec2i diffFlow = Vec2i(leftSumFlow[0] - rightSumFlow[0], rightSumFlow[1] - leftSumFlow[1]);
	line(framedst, cvPoint(px, py), cvPoint(px+diffFlow[0], py), CV_RGB(0,255,0), 1);

	if (result == -2*INT_FLOAT)
	{
		putText(framedst, "S", cvPoint(20, 20),CV_FONT_HERSHEY_DUPLEX, 1.0f, CV_RGB(255, 0, 0));
	}else if (result  == 0)
	{
		putText(framedst, "F", cvPoint(20, 20), CV_FONT_HERSHEY_DUPLEX, 1.0f, CV_RGB(255, 0, 0));
	}else if (result < 0)
	{
		putText(framedst, "L", cvPoint(20, 20), CV_FONT_HERSHEY_DUPLEX, 1.0f, CV_RGB(255, 0, 0));
	}else if (result > 0)
	{
		putText(framedst, "R", cvPoint(20, 20), CV_FONT_HERSHEY_DUPLEX, 1.0f, CV_RGB(255, 0, 0));
	}
	return result;
}

bool isBigObstacleMat(Mat framedst, Mat flow){
	int sumR = 0, sumG = 0, sumB = 0;
	int count = 0;
	for(int i = EDGE_OBS*HEIGHT; i < (1-EDGE_OBS)*HEIGHT; i++){
		for(int j = EDGE_OBS*WIDTH; j < (1-EDGE_OBS)*WIDTH; j++ ){
			sumB += (int)framedst.row(i).col(j).data[0];
			sumG += (int)framedst.row(i).col(j).data[1];
			sumR += (int)framedst.row(i).col(j).data[2];
			count ++ ;
		}
	}
	int avgB = sumB / count;
	int avgG = sumG / count;
	int avgR = sumR / count;
	int timers;
	int timerCount = 0;
	int flowZeroCount = 0;
	for(int i = EDGE_OBS*HEIGHT; i < (1-EDGE_OBS)*HEIGHT; i++){
		for(int j = EDGE_OBS*WIDTH; j < (1-EDGE_OBS)*WIDTH; j++ ){
			timers = 0;
			if(abs((int)framedst.row(i).col(j).data[0] - avgB) < COLOR_SCALE){
				timers += 1;
			}
			if(abs((int)framedst.row(i).col(j).data[1] - avgG) < COLOR_SCALE){
				timers += 1;
			}
			if(abs((int)framedst.row(i).col(j).data[2] - avgR) < COLOR_SCALE){
				timers += 1;
			}
			if (timers == 3)
			{
				timerCount ++;
				if ((flow.at<Vec2i>(i, j)[0]) == 0)
				{
					flowZeroCount ++;
				}
			}
		}
	}
	float timerPro = (timerCount*1.0)/count;
	float flowZeroPro = (flowZeroCount*1.0)/timerCount;
	if (timerPro > THRESHOLD_TIMER && flowZeroPro > THRESHOLD_ZERO)
	{
		return true;
	}else{
		return false;
	}
}

void drawFlowForDenseMat(Mat flow, Mat framedst){
	int gap = 10;
	for (int i = gap; i < HEIGHT; i += gap)
	{
		for(int j = gap; j < WIDTH; j += gap){
			Vec2i flow_at_point = flow.at<Vec2i>(i, j);
			float fx = flow_at_point[0];
			float fy = flow_at_point[1];
			if (fabs(fx) > UNKNOWN_FLOW_THRESH || fabs(fy) > UNKNOWN_FLOW_THRESH)
			{
				continue;
			}
			CvPoint p, q;
			p.x = j;
			p.y = i;
			q.x = fx + j;
			q.y = fy + i;
			//printf("p: (%d,%d). q :(%d, %d)\n", p.x, p.y, flow.at<Vec2i>(i, j)[0], flow.at<Vec2i>(i, j)[1]);
			drawMatFlow(p, q, framedst);
		}
	}
}

void drawMatFlow(CvPoint p, CvPoint q, Mat framedst){
	double angle; 
	angle = atan2((double) p.y - q.y, (double) p.x - q.x);
	double hypotenuse; 
	hypotenuse = sqrt(((p.y - q.y)*(p.y - q.y) +(p.x - q.x)*(p.x - q.x))*1.0);

	q.x = (int) (p.x - 3 * hypotenuse * cos(angle));
	q.y = (int) (p.y - 3 * hypotenuse * sin(angle));
	line(framedst, p, q, CV_RGB(0,0,255),1);

	p.x = (int) (q.x + 3 * cos(angle + CV_PI / 4));
	p.y = (int) (q.y + 3  * sin(angle + CV_PI / 4));
	line(framedst, p, q,CV_RGB(0,0,255),1 );
	p.x = (int) (q.x + 3 * cos(angle - CV_PI / 4));
	p.y = (int) (q.y + 3 * sin(angle - CV_PI / 4));
	line(framedst, p, q, CV_RGB(0,0,255),1 );
}


