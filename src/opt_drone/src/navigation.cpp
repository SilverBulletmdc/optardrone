#include "stdafx.h"
#include <cv.h>
#include "optutil.h"
#include "opticalflow.h"
#include "navigation.h"
#include "optcvmatutil.h"
#include "optfeatureutil.h"
#include "optmatutil.h"
#include "motioncolor.h"

using namespace cv;
using namespace std;

//利用左右光流平衡返回左1右2前3停止4，为金字塔光流算法
int imgFeatureBalance(ImgFeatureFunType funtype,IplImage* imgprev, IplImage* imgcurr, IplImage* imgdst){
	//1.计算光流
	IplImage* imgprev_1 = imgResize(imgprev);
	IplImage* imgcurr_1 = imgResize(imgcurr);
	IplImage* eig_image=cvCreateImage(cvSize(WIDTH,HEIGHT),IPL_DEPTH_32F,1);
	IplImage* tmp_image=cvCreateImage(cvSize(WIDTH,HEIGHT),IPL_DEPTH_32F,1);
	CvPoint2D32f* cornersprev = new CvPoint2D32f[ MAX_CORNERS ];
	CvPoint2D32f* cornerscurr = new CvPoint2D32f[ MAX_CORNERS ];
	int corner_count = MAX_CORNERS;
	cvGoodFeaturesToTrack(
		imgprev_1,
		eig_image,
		tmp_image,
		cornersprev,
		&corner_count,
		0.01,
		5.0,
		0,
		3,
		0,
		0.04
		);
	cvFindCornerSubPix(
		imgprev_1,
		cornersprev,
		corner_count,
		cvSize(WINSIZE,WINSIZE),
		cvSize(-1,-1),
		cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03)
		);
	funtype(imgprev_1, imgcurr_1, cornersprev, cornerscurr);

	drawFlowForFeatureCvPoint(cornersprev, cornerscurr, imgdst);

	cvReleaseImage(&imgprev_1);
	cvReleaseImage(&imgcurr_1);
	cvReleaseImage(&eig_image);
	cvReleaseImage(&tmp_image);
	free(cornerscurr);
	free(cornersprev);

	return 0;
}

float imgStrategic(ImgFunType funtype, IplImage* imgprev, IplImage* imgcurr, IplImage* imgdst, Mat &color, int strategic){
	//计算光流初始化变量
	IplImage* imgprev_1 = imgResize(imgprev);
	IplImage* imgcurr_1 = imgResize(imgcurr);
	CvMat *velx, *vely;
	velx = cvCreateMat(HEIGHT, WIDTH, CV_32FC1);
	vely = cvCreateMat(HEIGHT, WIDTH, CV_32FC1);
	cvSetZero(velx);
	cvSetZero(vely);
	//计算光流
	float k = funtype(imgprev_1,imgcurr_1,velx,vely);

	float result  = 0;
	if ((strategic >> 0 & 1) == 1) //balance
	{
		result = balanceForDenseCvMat(velx, vely, imgdst, k);
		printf("balance result : %d\n", result);
	}    
	if ((strategic >> 1 & 1) == 1) //draw optflow
	{
		drawFlowForDenseCvMat(velx, vely, imgdst);
	}
	if ((strategic >> 2 & 1) == 1) //mation to color
	{
		motionCvMatToColor(velx, vely, color);
	}
	
	cvReleaseImage(&imgprev_1);
	cvReleaseImage(&imgcurr_1);
	cvReleaseMat(&velx);
	cvReleaseMat(&vely);

	return result;
}

float matStrategic(MatFunType funtype, Mat frameprev, Mat framecurr, Mat framedst, Mat &color, int strategic, bool issf){
	Mat frameprev_1, framecurr_1, flow;
	if (issf)
	{
		frameprev_1 = matColorResize(frameprev);
		framecurr_1 = matColorResize(framecurr);
	}else{
		frameprev_1 = matResize(frameprev);
		framecurr_1 = matResize(framecurr);
	}

	flow = funtype(frameprev_1, framecurr_1, flow);
    float k = FB_K;
    if(issf){
        k = SF_K;
    }
    float result = 0;
	if ((strategic >> 0 & 1) == 1) //balance
	{
		result = balanceForDenseMat(flow, framedst, k);
	}
	if ((strategic >> 1 & 1) == 1) //draw optflow
	{
		drawFlowForDenseMat(flow, framedst);
	}
	if ((strategic >> 2 & 1) == 1) //mation to color
	{
		motionMatToColor(flow, color);
	}

	framecurr_1.release();
	frameprev_1.release();
	flow.release();
	return result;
}

