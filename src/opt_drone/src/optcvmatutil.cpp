#include "stdafx.h"
#include <cv.h>

#include "optutil.h"
#include "optcvmatutil.h"

using namespace cv;
using namespace std;

/************************************************************************/
/* 稠密光流，光流参数类型为CvMat的FOE（x，y）的计算
   velx：x方向光流；vely：y方向光流                                     */
/************************************************************************/
Vec2i foeForDenseCvMat(CvMat* velx, CvMat* vely){
	//calculate x of foe
	float cols[COLS];
	for (int col = 0; col < velx->width; col++)
	{
		float tmp = 0;
		for (int row = 0; row < velx->height; row++)
		{
			float* pData = velx->data.fl + row*velx->step/4;
			tmp += *(pData + col); 
		}
		cols[col] = tmp;
	}
	float colsLeft[COLS];//以col左侧的光流之和
	for (int i = 0; i < velx->width; i++)
	{
		colsLeft[i] = cols[i];
		if (i > 0)
		{
			colsLeft[i] += colsLeft[i-1];
		}
	}
	float colsRight[COLS];//以col右侧的光流之和
	for (int j = velx->width - 1; j >= 0; j--)
	{
		colsRight[j] = cols[j];
		if (j < velx->width -1)
		{
			colsRight[j] += colsRight[j+1];
		};
	}
	//找出x使其左右两边之差最小的为foe点的x
	float colsMin = abs(abs(colsLeft[0]) - abs(colsRight[0]));
	int foeX = 0;
	for (int i = 1; i < velx->width; i++)
	{
		float tmp = abs(abs(colsLeft[i]) - abs(colsRight[i]));
		if (tmp < colsMin)
		{
			colsMin = tmp;
			foeX = i;
		}
	}

	//calculate y of foe
	float rows[ROWS];
	for (int row = 0 ; row < vely->height; row++ ){
		float tmp = 0;
		float* pData = vely->data.fl + row*vely->step/4;
		for(int col = 0 ; col < vely->width; col++){
			tmp += *(pData + col);
		}
		rows[row] = tmp;
	}
	float rowsUp[ROWS];
	for (int i = 0; i < vely->height; i++)
	{
		rowsUp[i] = rows[i];
		if (i > 0)
		{
			rowsUp[i] += rowsUp[i-1];
		}
	}
	float rowsDown[ROWS];
	for (int j = vely->height - 1; j >= 0; j--)
	{
		rowsDown[j] = rows[j];
		if (j < vely->height-1)
		{
			rowsDown[j] += rowsDown[j+1];
		}
	}
	float rowsMin = abs(abs(rowsUp[0]) - abs(rowsDown[0]));
	int foeY = 0;
	for (int i = 1; i < vely->height; i++)
	{
		float tmp = abs(abs(rowsUp[i]) - abs(rowsDown[i]));
		if (tmp < rowsMin)
		{
			rowsMin = tmp;
			foeY = i;
		}
	}
	printf("FOE : [cols(x) = %d, rows(y) = %d]\n", foeX, foeY);

	return Vec2i(foeX, foeY);
}

/************************************************************************/
/* 稠密光流且光流参数类型为CvMat的TTC计算，返回平均TTC值
   vely：y方向光流； foeY：foe点y的坐标；ttc：计算获得的每列的ttc       */
/************************************************************************/
float ttcForDenseCvMat(CvMat* vely, int foeY, float *ttc){
	float sum  = 0;
	for (int col = 0; col < vely->width; col++)
	{
		float tmp = 0;
		for (int row = 0; row < vely->height; row++)
		{
			float* pData = vely->data.fl + row*vely->step/4 + col;
			if (*pData == 0)
			{
				tmp += abs((row - foeY)*10); //(row - foeY)/0.1,如果为0，则认为速度为0.1
			}else{
				tmp += abs((row - foeY)/(*pData));
			}
		}
		ttc[col] = tmp/(vely->height);
		sum += ttc[col];
	}
	return sum/(vely->width);
}

//利用左右光流平衡返回左1右2前3停止4
float balanceForDenseCvMat(CvMat* velx, CvMat* vely, IplImage* imgdst,  float k, int px, int py){
	Vec2i leftSumFlow = Vec2i(0, 0);
    float up = EDGE*HEIGHT;
    float down = (1-EDGE)*HEIGHT;
	for (int i = 0; i < px; i++)
	{
		for (int j = up; j < down; j++)
		{
			leftSumFlow[0] += (int) cvGetReal2D(velx, j, i);
			leftSumFlow[1] += (int) cvGetReal2D(vely, j, i);
		}
	}
	Vec2i rightSumFlow = Vec2i(0, 0);
	for (int i = px; i < WIDTH; i++)
	{
		for(int j = up; j < down; j++){
			rightSumFlow[0] += (int) cvGetReal2D(velx, j, i);
			rightSumFlow[1] += (int) cvGetReal2D(vely, j, i);
		}
	}
	//printf("pre left :%d,%d  right:%d,%d\n", leftSumFlow[0], leftSumFlow[1], rightSumFlow[0], rightSumFlow[1]);
	leftSumFlow[0] = abs(leftSumFlow[0] / px);
	leftSumFlow[1] = abs(leftSumFlow[1] / px);
	rightSumFlow[0] = abs(rightSumFlow[0] / (WIDTH - px));
	rightSumFlow[1] = abs(rightSumFlow[1] / (WIDTH - px));

    bool isBig = isBigObstacle(imgdst, velx);
    float result  = balanceControlLR(isBig, leftSumFlow[0], rightSumFlow[0], k);
    
    Vec2i diffFlow = Vec2i(leftSumFlow[0] - rightSumFlow[0], rightSumFlow[1] - leftSumFlow[1]);
	//printf("diffFlow: %d , %d \n", diffFlow[0], diffFlow[1]);
	//printf("left :%d  right:%d\n", leftSumFlow[0], rightSumFlow[0]);
	cvLine(imgdst, cvPoint(px, py), cvPoint(px+diffFlow[0], py), CV_RGB (0, 255, 0), 3, CV_AA, 0);
	
	CvFont font;
	cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX|CV_FONT_ITALIC, 1, 1, 0, 2);

	if (result == -2*INT_FLOAT)
	{
		cvPutText(imgdst, "S", cvPoint(20, 20), &font, CV_RGB(255, 0, 0));
	}else if (result  == 0)
	{
		cvPutText(imgdst, "F", cvPoint(20, 20), &font, CV_RGB(255, 0, 0));
 	}else if (result < 0)
	{
		cvPutText(imgdst, "L", cvPoint(20, 20), &font, CV_RGB(255, 0, 0));
	}else if (result > 0)
	{
		cvPutText(imgdst, "R", cvPoint(20, 20), &font, CV_RGB(255, 0, 0));
	}
    return result;
}

void drawFlowForDenseCvMat(CvMat* velx, CvMat* vely, IplImage* imgdst){
	 for (int i = DRAWGAP; i < HEIGHT; i += DRAWGAP)
	 {
		 for(int j = DRAWGAP; j < WIDTH; j += DRAWGAP){
			 CvPoint p, q;
			 p.x = (int) cvGetReal2D(velx, i, j) + j;
			 p.y = (int) cvGetReal2D(vely, i, j) + i;
			 q.x = j;
			 q.y = i;
			 drawFlow(p, q, imgdst);
		 }
	 }
}
void drawFlow(CvPoint p, CvPoint q, IplImage* imgdst){
	double angle; 
	angle = atan2((double) p.y - q.y, (double) p.x - q.x);
	double hypotenuse; 
	hypotenuse = sqrt(((p.y - q.y)*(p.y - q.y) +(p.x - q.x)*(p.x - q.x))*1.0);
	
	q.x = (int) (p.x - 3 * hypotenuse * cos(angle));
	q.y = (int) (p.y - 3 * hypotenuse * sin(angle));
	cvLine(imgdst, p, q, CV_RGB(0,0,255),1);

	p.x = (int) (q.x + 2 * hypotenuse * cos(angle + CV_PI / 4));
	p.y = (int) (q.y + 2 * hypotenuse * sin(angle + CV_PI / 4));
	cvLine(imgdst, p, q, CV_RGB(0,0,255),1 );
	p.x = (int) (q.x + 2 * hypotenuse * cos(angle - CV_PI / 4));
	p.y = (int) (q.y + 2 * hypotenuse * sin(angle - CV_PI / 4));
	cvLine(imgdst, p, q, CV_RGB(0,0,255),1 );
}
bool isBigObstacle(IplImage* imgdst, CvMat* velx){
   uchar *data = (uchar*)imgdst->imageData;
	int step = imgdst->widthStep / sizeof(uchar);
	int channels = imgdst->nChannels;
	int sumR = 0, sumG = 0, sumB = 0;
	int count = 0;
	for(int i = EDGE_OBS*HEIGHT; i < (1-EDGE_OBS)*HEIGHT; i++){
		for(int j = EDGE_OBS*WIDTH; j < (1-EDGE_OBS)*WIDTH; j++ ){
			sumB += (int)data[i*step+j*channels+0];
			sumG += (int)data[i*step+j*channels+1];
			sumR += (int)data[i*step+j*channels+2];
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
			if(abs((int)data[i*step+j*channels+0] - avgB) < COLOR_SCALE){
				timers += 1;
			}
			if(abs((int)data[i*step+j*channels+1] - avgG) < COLOR_SCALE){
				timers += 1;
			}
			if(abs((int)data[i*step+j*channels+2] - avgR) < COLOR_SCALE){
				timers += 1;
			}
			if (timers == 3)
			{
				timerCount ++;
				if (((int) cvGetReal2D(velx, i, j)) == 0)
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


