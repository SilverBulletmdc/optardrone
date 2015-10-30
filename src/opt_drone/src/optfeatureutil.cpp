#include "stdafx.h"
#include <cv.h>

#include "optutil.h"
#include "optfeatureutil.h"

using namespace cv;
using namespace std;

int balanceForFeatureCvPoint(CvPoint2D32f* cornersprev, CvPoint2D32f* cornerscurr, IplImage* imgdst, int threshold, float k, int px, int py){
	return 0;
}

void drawFlowForFeatureCvPoint(CvPoint2D32f* cornersprev, CvPoint2D32f* cornerscurr, IplImage* imgdst){
	for (int i =0; i < MAX_CORNERS; i++)
	{
		CvPoint p,q;
		p.x = (int) cornersprev[i].x;
		p.y = (int) cornersprev[i].y;
		q.x = (int) cornerscurr[i].x;
		q.y = (int) cornerscurr[i].y;
		drawFlow(p, q, imgdst);
	}
}

Vec2i foeForFeatureCvPoint(CvPoint2D32f* cornersprev, CvPoint2D32f* cornerscurr){
	return Vec2i(0,0);
}

float ttcForFeatureCvPoint(CvPoint2D32f* cornersprev, CvPoint2D32f* cornerscurr){
	return 1;
}


