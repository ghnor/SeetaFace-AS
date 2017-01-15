#include "CMImgProc.h"

#define CV_DESCALE(x,n) (((x) + (1 << ((n) - 1))) >> (n))

// 双线性插值图像缩放,注意:缩放只改变宽高,不改变通道数
// src			源图像指针
// sw,sh,sc		分别表示源图像的宽,高,及通道数
// dst			缩放后图像指针
// dw,dh		缩放后图像宽,高
void CMImgProc::ResizeLinear(unsigned char *src, int sw, int sh, int sc, unsigned char *dst, int dw, int dh)
{
	//LOGD("native ResizeLinear 1,sw=%d, sh=%d, dw=%d, dh=%d", sw, sh, dw, dh);
	unsigned char *dataDst = dst;  
	int stepDst = dw * sc;
	unsigned char *dataSrc = src;  
	int stepSrc = sw * sc;  
	int iWidthSrc = sw;  
	int iHiehgtSrc = sh; 

	double scale_x = (double)sw / dw;  
	double scale_y = (double)sh / dh;

	//LOGD("native ResizeLinear 2, scale_x=%f, scale_y=%f", scale_x, scale_y);

	for (int j = 0; j < dh; ++j)  
	{  
		float fy = (float)((j + 0.5) * scale_y - 0.5);  
		int sy = (int)floor(fy);// 向下取整
		fy -= sy;  
		sy = std::min(sy, iHiehgtSrc - 2);  
		sy = std::max(0, sy);  

		int cbufy[2];  
		cbufy[0] = (int)((1.f - fy) * 2048 + 0.5);  
		cbufy[1] = 2048 - cbufy[0];  

		for (int i = 0; i < dw; ++i)  
		{  
			float fx = (float)((i + 0.5) * scale_x - 0.5);  
			int sx = (int)floor(fx);  
			fx -= sx;  

			if (sx < 0) {  
				fx = 0, sx = 0;  
			}  
			if (sx >= iWidthSrc - 1) {  
				fx = 0, sx = iWidthSrc - 2;  
			}  

			short cbufx[2];  
			cbufx[0] = (int)((1.f - fx) * 2048 + 0.5);
			cbufx[1] = 2048 - cbufx[0];  

			// 对每个通道进行计算
			for (int k = 0; k < sc; k++)  
			{  
				*(dataDst+ j*stepDst + sc*i + k) = (*(dataSrc + sy*stepSrc + sc*sx + k) * cbufx[0] * cbufy[0] +   
					*(dataSrc + (sy+1)*stepSrc + sc*sx + k) * cbufx[0] * cbufy[1] +   
					*(dataSrc + sy*stepSrc + sc*(sx+1) + k) * cbufx[1] * cbufy[0] +   
					*(dataSrc + (sy+1)*stepSrc + sc*(sx+1) + k) * cbufx[1] * cbufy[1]) >> 22;  
			} 


		}  
	}
}

/**
 * RGBA的图像转为RGB图像
 */
void CMImgProc::RGBA2RGB(const unsigned char *src, unsigned char *dst, int w, int h)
{
	//LOGD("native RGBA2GRAY 1......");

	const unsigned char *pSrc;
	unsigned char *pDst;
	for(int j = 0; j < h; j++)
	{
		pSrc = src + j * w * 4;
		pDst = dst + j * w * 3;
		for(int i = 0; i < w; i++)
		{
			pDst[2] = pSrc[2];
			pDst[1] = pSrc[1];
			pDst[0] = pSrc[0];

			pSrc += 4;
			pDst += 3;
		}
	}
}



// RGBA格式转GRAY,采用公式如下
// Y = 0.299 * R + 0.587 * G + 0.114 * B
// Cr = (R - Y) * 0.713 + delta
// Cb = (B - Y) * 0.564 + delta
// src	源数据,4通道,RGBA
// dst	结果数据,3通道,YCrCb
// w	图像的宽度
// h	图像的高度
// sc	源图像的通道数,一般为4
void CMImgProc::RGBA2GRAY(const unsigned char *src, unsigned char *dst, int w, int h, int sc)
{
	//LOGD("native RGBA2GRAY 1......");
	static int yuv_shift = 14;
	static int c0 = 4899,c1 = 9617,c2 = 1868;//R,G,B

	size_t len = strlen((char*)src);
	size_t t_sc = len/(w*h);
	//LOGD("native RGBA2GRAY 1......,len=%d, w=%d, h=%d, sc=%d, t_sc=%d", len, w, h, sc, t_sc);

	const unsigned char *pSrc;
	unsigned char *pDst;
	for(int j = 0; j < h; j++)
	{
		pSrc = src + j * w * sc;
		pDst = dst + j * w;
		for(int i = 0; i < w; i++)
		{
			//int Y = CV_DESCALE(pSrc[2]*c2 + pSrc[1]*c1 + pSrc[0]*c0, yuv_shift);
			int Y = (pSrc[2] + pSrc[1] + pSrc[0])/3;
			*pDst = (unsigned char)(Y);

			pSrc += sc;
			pDst++;
		}
	}
}

// RGBA格式转YCrCb
// Y = 0.299 * R + 0.587 * G + 0.114 * B
// Cr = (R - Y) * 0.713 + delta
// Cb = (B - Y) * 0.564 + delta
// src		源数据,4通道,RGBA
// dst		结果数据,3通道,YCrCb
// w		图像的宽度
// h		图像的高度
// sc		源图像的通道数,一般为4
// dc		结果图像的通道数,为3通道
void RGBA2YCrCb(const unsigned char *src, unsigned char *dst, int w, int h, int sc, int dc)
{

	static int yuv_shift = 14;
	static int c0 = 4899,c1 = 9617,c2 = 1868,c3 = 11682,c4 = 9241;//R,G,B
	static int value = 128*(1<<14);

	const unsigned char *pSrc;
	unsigned char *pDst;
	for(int j = 0; j < h; j++)
	{
		pSrc = src + j * w * sc;
		pDst = dst + j * w * dc;
		for(int i = 0; i < w; i++)
		{
			int Y = CV_DESCALE(pSrc[2]*c2 + pSrc[1]*c1 + pSrc[0]*c0, yuv_shift);
			int Cr = CV_DESCALE((pSrc[0] - Y)*c3 + value, yuv_shift);  
			int Cb = CV_DESCALE((pSrc[2] - Y)*c4 + value, yuv_shift);

			pDst[0] = (unsigned char)(Y);
			pDst[1] = (unsigned char)(Cr);
			pDst[2] = (unsigned char)(Cb);
			
			pSrc += sc;
			pDst += dc;
		}
	}
}

// 直方图统计
// 归一化
// 均值平移
// gray		传入图像的头指针,注:只针对灰度图,即通道数为1
// w,h		表示图像的宽和高
// hist		直方图统计的头指针
// len		直方图数组的长度,默认为256
void HistStat(unsigned char *gray, int w, int h, float *hist, int len)
{
	int count = w * h;

	unsigned char *pix = gray;
	for(int i = 0; i < count; i++)
	{
		hist[*(pix++)]++;
	}
	// 对整个直方图进行归一化，同时计算均值
	float ave = 0.0f;
	float *ph = hist;
	for(int m = 0; m < len; m++)
	{
		// 归一化
		*ph /= count;
		ave += *ph;
		ph++;
	}
	ave /= len;

	// 对数据进行平移处理
	ph = hist;
	for(int m = 0; m < len; m++)
	{
		*ph -= ave;
		ph++;
	}
}

// 传入YCrCb的数据,主要关注Y分量
// src		源图像数据
// lap		数据单通道的结果数据
// w		图像的宽度
// h		图像的高度
// sc		源图像的通道数
// 默认采用核为 {2, 0, 2, 0, -8, 0, 2, 0, 2}
// 返回去除四周的其他像素的均值
double Laplace(const unsigned char *src, unsigned char *lap, int w, int h, int sc)
{
	int val = 0;
	double ave = 0;
	memset(lap,0,w*h*sizeof(unsigned char));

	const unsigned char *pcur,*plt,*prt,*plb,*prb;
	unsigned char lv;
	// 边缘不做处理
	for(int j = 1; j < h - 1; j++)
	{
		pcur = src + (j * w + 1)* sc;// 当前
		plt = pcur - (w + 1) * sc;// 左上
		prt = pcur - (w - 1) * sc;// 右上
		plb = pcur + (w - 1) * sc;// 左下
		prb = pcur + (w + 1) * sc;// 右下
		for(int i = 1; i < w - 1; i++)
		{
			// laplace变换
			val = (int)(*plt) * 2 + (int)(*prt) * 2 + 
				(int)(*plb) * 2 + (int)(*prb) * 2 - (int)(*pcur) * 8;
			// 溢出判断
			if(abs(val) > 255)
				lv = (unsigned char)(255);
			else
				lv = (unsigned char)(abs(val));

			lap[j * w + i] = lv;

			// 求和
			ave += lv;

			pcur += sc;
			plt += sc;
			prt += sc;
			plb += sc;
			prb += sc;
		}
	}

	// 示范案例写的有问题,
	// 该问题就按照错误的来写,以免阈值发生错误
	// 实际应写(ave/((h - 2)*(w - 2)))
	return (ave/((h - 1)*(w - 1)));
}

