/**
 * 采用中科院山世光开源的SeetaFaceEngine实现android上的人脸检测与对齐、识别
 * BSD license
 * 广州炒米信息科技有限公司
 * www.cume.cc
 * 吴祖玉
 * wuzuyu365@163.com
 * 2016.11.9
 *
 */

#include <iostream>
#include <cmath>
using namespace std;

#include <string.h>
#include <android/log.h>
//#include <opencv2/opencv.hpp>

#define TAG "libSeetafaceSo"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG,__VA_ARGS__)
//using namespace cv;

namespace CMImgProc
{
	// 双线性插值图像缩放,注意:缩放只改变宽高,不改变通道数
	// src			源图像指针
	// sw,sh,sc		分别表示源图像的宽,高,及通道数
	// dst			缩放后图像指针
	// dw,dh		缩放后图像宽,高
	void ResizeLinear(unsigned char *src, int sw, int sh, int sc, unsigned char *dst, int dw, int dh);

	/**
	 * RGBA的图像转为RGB图像
	 */
	void RGBA2RGB(const unsigned char *src, unsigned char *dst, int w, int h);

	/**
	 * 彩色图像转灰度
	 * src:源图像
	 * dst:目标图像
	 * w:图像宽度
	 * h:图像高度
	 * sc:图像通道数
	 */
	void RGBA2GRAY(const unsigned char *src, unsigned char *dst, int w, int h, int sc);
};
