/*
*  FILE stdafx.h
*  AUTHOR Sarah
*  DATE 2015/08/14 22:27
*  TODO: 鏍囧噯绯荤粺鍖呭惈鏂囦欢鐨勫寘鍚枃浠讹紝鎴栨槸缁忓父浣跨敤浣嗕笉甯告洿鏀圭殑鐗瑰畾浜庨」鐩殑鍖呭惈鏂囦欢
*/
#pragma once

#include <stdio.h>

/*
*  ROWS: 鏁扮粍鐢宠鍐呭瓨绌洪棿澶у皬锛屽ぇ浜庣瓑浜嶩EIGHT.
*  COLS: 鏁扮粍鐢宠鍐呭瓨绌洪棿澶у皬锛屽ぇ浜庣瓑浜嶹IDTH.
*  HEIGHT: 閲嶇疆鍥惧儚鍚庣殑楂?
*  WIDTH: 閲嶇疆鍥惧儚鍚庣殑瀹?
*  MAX_CORNERS: PyrLK璁＄畻鐗瑰緛鐐规暟鐩?
*  WINSIZE: 绐楀彛澶у皬.
*  DRAWGAP: 鐢诲厜娴佺殑闂撮殧.
*  UNKNOWN_FLOW_THRESH: 澶т簬姝ゅ?寰楀厜娴佽涓烘槸error鍏夋祦. 
*/
#define ROWS 350
#define COLS 350
#define HEIGHT 190  
#define WIDTH 288
#define MAX_CORNERS  300
#define WINSIZE 5
#define DRAWGAP 15
#define UNKNOWN_FLOW_THRESH 1e8
#define LK_K 6
#define HS_K 4.3
#define BM_K 1.4
#define FB_K 7
#define SF_K 3
#define INT_FLOAT 100.0 //matlab中int向上float转型有问题，所以结果乘100，处理时再除100.
#define K_FLAG true  //在避撞偏移量中，false表示k取1，而非上述*_K变量
#define EDGE 0 //0.1429
#define EDGE_OBS 0.33
#define COLOR_SCALE 20
#define THRESHOLD_TIMER 0.75 //同色所占比例大于整幅图像的75%，则认为是墙，停止。
#define THRESHOLD_ZERO 0.8

