#include "stdafx.h"
#include "optutil.h"
#include <cv.h>

using namespace std;
using namespace cv;

/***************************************************************************************/
/*    根据TTC，标记是否为安全区域，暂定大于ttcAvg为安全1，反之，不安全0，然后再调整        
      cols 列数；ttc：计算的ttc数组；ttcAvg：ttc平均值；k：比例系数
      tagSafe : 通过比较规则获得是否安全，标记之                                       */
/***************************************************************************************/
void tagSafeAreaByTTC(int cols, float* ttc, float ttcAvg , float k,int *tagSafe){
	for (int i = 0; i < cols; i++)
	{
		if (ttc[i] > ttcAvg*k)
		{
			tagSafe[i] = 1;
		}
	}
}

/***************************************************************************************/
/*  判定标记是否准确。安全判定安全1，安全非安全2，非安全判定安全3，非安全判定非安全4    
  tagOrigin: 准确的tag标记； tagSafe：根据算法估计的tag标记；tags：根据上述原则判定结果
  函数返回：根据1-4结果，返回此光流方法避撞的评价指数,值越大性能越差                   */
/***************************************************************************************/
float compareTag(int *tagOrigin, int *tagSafe, int cols, int *tags){
	int tag_1 = 0, tag_2 = 0, tag_3 = 0, tag_4 = 0;
	for (int i = 0; i < cols; i++)
	{
		if (tagOrigin[i] == 1 && tagSafe[i] == 1)
		{
			tags[i] = 1;
			tag_1++;
		}else if (tagOrigin[i] == 1 && tagSafe[i] == 0)
		{
			tags[i] = 2;
			tag_2++;
		}else if (tagOrigin[i] == 0 && tagSafe[i] == 1)
		{
			tags[i] = 3;
			tag_3++;
		} 
		else if (tagOrigin[0] == 0 && tagSafe[i] == 0)
		{
			tags[i] = 4;
			tag_4++;
		}
	}
	return tag_1 + tag_2*1.5 + tag_3*2 + tag_4;//各比例项暂定1-1.5-2-1；
}

float balanceControlLR(bool isBig, int leftSumFlow, int rightSumFlow, float k){
    if(isBig){ //前方遇到同一色巨型障碍物，返回-2
        return -2*INT_FLOAT;
    }
    if(leftSumFlow == 0 || rightSumFlow == 0){
        return turnLRScale(leftSumFlow, rightSumFlow, k);
    }
    float gain = (rightSumFlow*INT_FLOAT)/(leftSumFlow*1.0);
    //return gain;
    if(gain < INT_FLOAT*k && gain > INT_FLOAT/k){
        return 0;
    }else{
        return turnLRScale(leftSumFlow, rightSumFlow, k);
    }
}
float turnLRScale(float leftSumFlow, float rightSumFlow, float k){
    if(leftSumFlow == rightSumFlow){
        return 0;
    }else{
        if(K_FLAG){
            k = 1;
        }
        if(leftSumFlow > rightSumFlow){
            return ((leftSumFlow - k*rightSumFlow)*INT_FLOAT)/(leftSumFlow + rightSumFlow);
        }else{
            return ((k*leftSumFlow - rightSumFlow)*INT_FLOAT)/(leftSumFlow + rightSumFlow);
        }
    }
}

Mat calibrate(Mat img)
{
	double mtx[3][3]={562.89836209,0,           314.41994795,0		   ,552.27825968,   160.37443571,0           ,0           ,1           };
	Mat matrix(Size(3,3),CV_64F,mtx);
	Mat img2;
	Vec<float,5> dist(-5.25438307e-01,3.76324874e-01,-4.78114662e-04,3.51717002e-04,-1.37709825e-01); 
	Mat newcameramtx; 
	newcameramtx = getOptimalNewCameraMatrix(matrix , dist ,Size(img.rows,img.cols), 0,Size(img.rows,img.cols));
	undistort(img,img2,matrix,dist,newcameramtx);
    matrix.release();
    img2.release();
    newcameramtx.release();
	return img2;
}

IplImage* calibrate(IplImage* iplimg)
{
	Mat img(iplimg,false);
	Mat img2 = calibrate(img);
	IplImage iplimg2 = img2;
    img.release();
    img2.release();
	return &iplimg2;
}







