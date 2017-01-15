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
#include "seetaface_JniClient.h"
#include "CMImgProc.h"
#include <android/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <android/bitmap.h>

//#include <opencv2/opencv.hpp>
#include "FaceDetection/include/common.h"

#include "FaceDetection/include/face_detection.h"
#include "FaceAlignment/include/face_alignment.h"
#include "FaceIdentification/include/Face_identification.h"

#define  LOG_TAG    "libseetafaceso"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

typedef struct
{
	uint8_t red;
	uint8_t green;
	uint8_t blue;
	uint8_t alpha;
} rgba;

typedef struct
{
	uint8_t red;
	uint8_t green;
	uint8_t blue;
} rgb;


//using namespace cv;
using namespace seeta;

#ifdef __cplusplus
extern "C" {
#endif

/*
	gamma变换调节亮度,
	用法 jni.CMimGamma(Bitmap vBmpSrc, Bitmap vBmpDst, float vGamma)
 */
JNIEXPORT void JNICALL Java_seetaface_JniClient_CMimGamma(JNIEnv
		* env, jobject  obj, jobject bitmapcolor1, jobject bitmapcolor2, float vGamma)
{
	AndroidBitmapInfo  infocolor1;
	void*              pixelscolor1;
	AndroidBitmapInfo  infocolor2;
	void*              pixelscolor2;
	int                ret;
	int             y;
	int             x;

	LOGI("CMimGamma");
	if ((ret = AndroidBitmap_getInfo(env, bitmapcolor1, &infocolor1)) < 0) {
		LOGE("AndroidBitmap_getInfo() failed ! error=%d", ret);
		return;
	}

	if ((ret = AndroidBitmap_getInfo(env, bitmapcolor2, &infocolor2)) < 0) {
		LOGE("AndroidBitmap_getInfo() failed ! error=%d", ret);
		return;
	}

	LOGI("color image 1 :: width is %d; height is %d; stride is %d; format is %d;flags is %d",
			infocolor1.width,infocolor1.height,infocolor1.stride,infocolor1.format,infocolor1.flags);
	if (infocolor1.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
		LOGE("infocolor1 format is not RGBA_8888 !");
		return;
	}

	LOGI("color image 2:: width is %d; height is %d; stride is %d; format is %d;flags is %d",
			infocolor2.width,infocolor2.height,infocolor2.stride,infocolor2.format,infocolor2.flags);
	if (infocolor2.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
		LOGE("infocolor2 format is not RGBA_8888 !");
		return;
	}


	if ((ret = AndroidBitmap_lockPixels(env, bitmapcolor1, &pixelscolor1)) < 0) {
		LOGE("AndroidBitmap_lockPixels() failed ! error=%d", ret);
	}

	if ((ret = AndroidBitmap_lockPixels(env, bitmapcolor2, &pixelscolor2)) < 0) {
		LOGE("AndroidBitmap_lockPixels() failed ! error=%d", ret);
	}

	// modify pixels with image processing algorithm

	float red,green,blue;
	for (y=0;y<infocolor1.height;y++) {
		rgba * line1 = (rgba *) pixelscolor1;
		rgba * line2 = (rgba *) pixelscolor2;
		//uint8_t * line2 = (uint8_t *) pixelscolor2;
		for (x=0; x<infocolor1.width; x++) {

			line2[x].alpha = line1[x].alpha;

			red = (line1[x].red + 0.5F)/256;//归一化
			red = (float)pow(red, vGamma);
			line2[x].red = (uint8_t)(red*256-0.5F);//反归一化

			green = (line1[x].green + 0.5F)/256;//归一化
			green = (float)pow(green, vGamma);
			line2[x].green = (uint8_t)(green*256-0.5F);//反归一化

			blue = (line1[x].blue + 0.5F)/256;//归一化
			blue = (float)pow(blue, vGamma);
			line2[x].blue = (uint8_t)(blue*256-0.5F);//反归一化

		}

		pixelscolor1 = (char *)pixelscolor1 + infocolor1.stride;
		pixelscolor2 = (char *)pixelscolor2 + infocolor2.stride;
	}

	LOGI("unlocking pixels");
	AndroidBitmap_unlockPixels(env, bitmapcolor1);
	AndroidBitmap_unlockPixels(env, bitmapcolor2);
}

/*
彩色转灰度图
 */
JNIEXPORT void JNICALL Java_seetaface_JniClient_CMim2gray(JNIEnv
		* env, jobject  obj, jobject bitmapcolor,jobject bitmapgray)
{
	AndroidBitmapInfo  infocolor;
	void*              pixelscolor;
	AndroidBitmapInfo  infogray;
	void*              pixelsgray;
	int                ret;
	int             y;
	int             x;

	LOGI("convertToGray");
	if ((ret = AndroidBitmap_getInfo(env, bitmapcolor, &infocolor)) < 0) {
		LOGE("AndroidBitmap_getInfo() failed ! error=%d", ret);
		return;
	}

	if ((ret = AndroidBitmap_getInfo(env, bitmapgray, &infogray)) < 0) {
		LOGE("AndroidBitmap_getInfo() failed ! error=%d", ret);
		return;
	}

	LOGI("color image :: width is %d; height is %d; stride is %d; format is %d;flags is %d",
			infocolor.width,infocolor.height,infocolor.stride,infocolor.format,infocolor.flags);
	if (infocolor.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
		LOGE("Bitmap format is not RGBA_8888 !");
		return;
	}


	LOGI("gray image :: width is %d; height is %d; stride is %d; format is %d;flags is %d",
			infogray.width,infogray.height,infogray.stride,infogray.format,infogray.flags);
	if (infogray.format != ANDROID_BITMAP_FORMAT_A_8) {
		LOGE("Bitmap format is not A_8 !");
		return;
	}


	if ((ret = AndroidBitmap_lockPixels(env, bitmapcolor, &pixelscolor)) < 0) {
		LOGE("AndroidBitmap_lockPixels() failed ! error=%d", ret);
	}

	if ((ret = AndroidBitmap_lockPixels(env, bitmapgray, &pixelsgray)) < 0) {
		LOGE("AndroidBitmap_lockPixels() failed ! error=%d", ret);
	}

	// modify pixels with image processing algorithm

	for (y=0;y<infocolor.height;y++) {
		rgba * line = (rgba *) pixelscolor;
		uint8_t * grayline = (uint8_t *) pixelsgray;
		for (x=0;x<infocolor.width;x++) {
			grayline[x] = 256 - (0.3 * line[x].red + 0.59 * line[x].green + 0.11*line[x].blue);
		}

		pixelscolor = (char *)pixelscolor + infocolor.stride;
		pixelsgray = (char *) pixelsgray + infogray.stride;
	}

	LOGI("unlocking pixels");
	AndroidBitmap_unlockPixels(env, bitmapcolor);
	AndroidBitmap_unlockPixels(env, bitmapgray);
}

/*
 *计算2个人脸的相似度， 根据人脸编号，找到他们的特征文件来比对
 */
JNIEXPORT jfloat JNICALL Java_seetaface_JniClient_CMCalcFaceSim
(JNIEnv *env, jobject obj, jint vFaceNo1, jint vFaceNo2, jstring vFaceModelPath)
{
	jstring tRtnNullStr = env->NewStringUTF("");
	float tSim = -1;
	//获取正面人脸检测模型的绝对路径
	const char* tFaceModelPath;
	tFaceModelPath = env->GetStringUTFChars(vFaceModelPath, 0);
	if(tFaceModelPath == NULL) {
		return tSim;
	}

//	LOGD("tFaceModelPath = %s", tFaceModelPath);
//	LOGD("CMCalcFaceSim,   vFaceNo1=%d, vFaceNo2=%d",   vFaceNo1, vFaceNo2);

	string tFaceModelPathStr = tFaceModelPath;

	//识别模型路径
	string tRecoModelPath = tFaceModelPathStr + "seeta_fr_v1.0.bin";
//	LOGD("CMCalcFaceSim,  tRecoModelPath=%s", tRecoModelPath.c_str());
	FaceIdentification face_recognizer(tRecoModelPath.c_str());
//	LOGD("CMCompFace, face_recognizer ok");

	char fname[100];
	sprintf(fname, "/sdcard/seeta_fea%d.txt", vFaceNo1);

	float feat1[2048];
	float feat2[2048];

	FILE* fp = fopen(fname, "r");
	if(!fp){
		return -1;
	}
	for (int i = 0; i < 2048; i++){
		fscanf(fp, "%f,", &feat1[i]);
	}

	fclose(fp);
	fname[100];
	sprintf(fname, "/sdcard/seeta_fea%d.txt", vFaceNo2);
	fp = fopen(fname, "r");
	if(!fp){
		return -1;
	}

	for (int i = 0; i < 2048; i++){
		fscanf(fp, "%f,", &feat2[i]);
	}

	fclose(fp);

	// Caculate similarity of two faces
	tSim = face_recognizer.CalcSimilarity(feat1, feat2);
	//保留2位小数
	tSim = int(100*tSim)/100.0;
	//LOGD("CMCalcFaceSim, tSim=%.2f",tSim);
	env->ReleaseStringUTFChars(vFaceModelPath, tFaceModelPath);

	return tSim;
}

/*
 *检测人脸，获取人脸范围及5个点的坐标
 */
JNIEXPORT jstring JNICALL Java_seetaface_JniClient_CMDetectFace
(JNIEnv *env, jobject obj, jbyteArray v_img_data,
		jint cols, jint rows, jint ch, jstring vFaceModelPath, jint vFaceNo, jobject vFaceBmp)
{
	LOGD("CMDetectFace, 1111");
	jstring tRtnNullStr = env->NewStringUTF("");

	jbyte *tImgData = env->GetByteArrayElements(v_img_data,0);

	//获取正面人脸检测模型的绝对路径
	const char* tFaceModelPath;
	tFaceModelPath = env->GetStringUTFChars(vFaceModelPath, 0);
	if(tFaceModelPath == NULL) {
		return tRtnNullStr;
	}

	//LOGD("tFaceModelPath = %s", tFaceModelPath);
	//LOGD("CMDetectFace,   rows=%d, cols=%d, ch=%d",   rows, cols, ch);

	string tFaceModelPathStr = tFaceModelPath;

	//检测模型路径
	string tDetectModelPath = tFaceModelPathStr + "seeta_fd_frontal_v1.0.bin";
	//LOGD("CMDetectFace,  tDetectModelPath=%s", tDetectModelPath.c_str());

	//对齐模型路径
	string tAlignModelPath = tFaceModelPathStr + "seeta_fa_v1.1.bin";
	//LOGD("CMDetectFace,  tAlignModelPath=%s", tAlignModelPath.c_str());

	//识别模型路径
	string tRecoModelPath = tFaceModelPathStr + "seeta_fr_v1.0.bin";
	//LOGD("CMDetectFace,  tRecoModelPath=%s", tRecoModelPath.c_str());

	//初始化人脸检测器
	seeta::FaceDetection detector(tDetectModelPath.c_str());
	//LOGD("CMDetectFace, detector ok");

	//4通道转3通道
	unsigned char *rgb_bmp = new unsigned char[rows*cols*3];
	CMImgProc::RGBA2RGB((unsigned char*)tImgData, rgb_bmp, cols, rows);

	unsigned char *gray = new unsigned char[rows*cols];
	CMImgProc::RGBA2GRAY((unsigned char*)tImgData, gray, cols, rows, ch);
	//LOGD("灰度化ok===");

	//LOGD("tImg, rows=%d, cols=%d",   tImg.rows, tImg.cols);

	ImageData img_color;
	img_color.data = rgb_bmp;
	img_color.width = cols;
	img_color.height = rows;
	img_color.num_channels = 3; //因为recognizer要求的是3通道

	ImageData img_gray;
	img_gray.data = gray;
	img_gray.width = cols;
	img_gray.height = rows;
	img_gray.num_channels = 1;


	time_t t0;
	int tTime0;
	tTime0 = time(&t0);
	LOGD("CMDetectFace, 当前time:%d", tTime0);

	std::vector<seeta::FaceInfo> faces = detector.Detect(img_gray);
	//LOGD("CMDetectFace, detector.Detect");
	time_t t1;
	int tTime1 = time(&t1);
	LOGD("CMDetectFace, 检测耗时:%d秒", tTime1 - tTime0);

	int32_t num_face = static_cast<int32_t>(faces.size());
	if(0 == num_face ){
		//没有人脸
		LOGD("CMDetectFace, 没有人脸");
		jstring rtstr = env->NewStringUTF("");
		return rtstr;
	}else{
		LOGD("CMDetectFace, 人脸数:%d", num_face);
	}

	//初始化人脸对齐器
	seeta::FaceAlignment point_detector(tAlignModelPath.c_str());
	//LOGD("CMDetectFace, point_detector ok");

	std::string tRetFacePosStr = "";
	char tPosStr[200] = {0};

	for(int i=0; i<num_face; i++){
		//每个人脸位置数据用分号分隔
		if(i > 0){
			tRetFacePosStr += ";";
		}

		//组装脸部矩形数据
		seeta::FaceInfo tFace = faces[i];
		sprintf(tPosStr, "%d,%d,%d,%d",
				tFace.bbox.x, tFace.bbox.y, tFace.bbox.width, tFace.bbox.height);

		//LOGD("face_tchar=%s", tPosStr);

		//首先记录人脸位置坐标，4个int
		tRetFacePosStr += tPosStr;

		//然后检测特征点
		seeta::FacialLandmark face_points[5];
		point_detector.PointDetectLandmarks(img_gray, faces[i], face_points);
		//组装特征点字符串
		for(int i=0; i<5; i++){
			char tPntStr[100] = {0};
			sprintf(tPntStr, ",%d,%d", (int) face_points[i].x, (int)face_points[i].y);
			tRetFacePosStr += tPntStr;
			//strcat(pnts_str, x);
		}
		//
	}
//	LOGD("tRetFacePosStr=%s", tRetFacePosStr.c_str());

	FaceIdentification face_recognizer(tRecoModelPath.c_str());
//	LOGD("CMDetectFace, face_recognizer ok");

	//准备裁剪人脸
	if(1){
		AndroidBitmapInfo  infocolor;
		void*              pixelscolor;
		int	ret;

		if ((ret = AndroidBitmap_getInfo(env, vFaceBmp, &infocolor)) < 0) {
			LOGE("AndroidBitmap_getInfo() failed ! error=%d", ret);
		}

//		LOGI("vFaceBmp color image :: width is %d; height is %d; stride is %d; format is %d;flags is %d",
//				infocolor.width,infocolor.height,infocolor.stride,infocolor.format,infocolor.flags);
		if (infocolor.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
			LOGE("Face Bitmap format is not RGBA_8888 !");
		}

		if ((ret = AndroidBitmap_lockPixels(env, vFaceBmp, &pixelscolor)) < 0) {
			LOGE("AndroidBitmap_lockPixels() failed ! error=%d", ret);
		}

		float feat1[2048];
		seeta::FacialLandmark face_points_x[5];
		point_detector.PointDetectLandmarks(img_gray, faces[0], face_points_x);
//		LOGD("CMDetectFace, before ExtractFeatureWithCrop 1");

		int tCropWidth = face_recognizer.crop_width();
		int tCropHeight = face_recognizer.crop_height();
		int tCropChannels = face_recognizer.crop_channels();

//		LOGD("CMDetectFace, tCropWidth=%d, tCropHeight=%d, tCropChannels=%d" ,
//				tCropWidth, tCropHeight, tCropChannels);


		long tSize = tCropWidth * tCropHeight * tCropChannels;
		unsigned char *tFaceBytes = new unsigned char[tSize];
		//剪切出的人脸
		ImageData img_face;
		img_face.data = tFaceBytes;
		img_face.width = face_recognizer.crop_width();
		img_face.height = face_recognizer.crop_height();
		img_face.num_channels = face_recognizer.crop_channels();

		//裁剪出人脸
		int tCropRet = face_recognizer.CropFace(img_color, face_points_x, img_face);


//		LOGD("CMDetectFace, tCropRet=%d", tCropRet);
		int tFaceByteStride = 256*3;
		int x,y;
		for (y=0;y<256;y++) {
			rgb * line1 = (rgb *) tFaceBytes;
			rgba * line2 = (rgba *) pixelscolor;
			//uint8_t * line2 = (uint8_t *) pixelscolor2;
			for (x=0; x<256; x++) {
				line2[x].alpha = 255;
				line2[x].red = line1[x].red;
				line2[x].green = line1[x].green;
				line2[x].blue = line1[x].blue;
			}
			tFaceBytes = (unsigned char *)tFaceBytes + tFaceByteStride;
			pixelscolor = (unsigned char *)pixelscolor + infocolor.stride;
		}

		//提取识别用的特征
		face_recognizer.ExtractFeatureWithCrop(img_color, face_points_x, feat1);
		//LOGD("CMDetectFace, ExtractFeatureWithCrop 1 ok");

		char fname[100];
		sprintf(fname, "/sdcard/seeta_fea%d.txt", vFaceNo);
		FILE* fp = fopen(fname, "w");
		for (int i = 0; i < 2048; i++){
			fprintf(fp, "%.5f,", feat1[i]);
		}
		fclose(fp);
	}

	AndroidBitmap_unlockPixels(env, vFaceBmp);

	time_t t2;
	int tTime2 = time(&t2);
	LOGD("CMDetectFace, 总耗时:%d秒", tTime2 - tTime0);

	env->ReleaseStringUTFChars(vFaceModelPath, tFaceModelPath);

	jstring rtstr = env->NewStringUTF(tRetFacePosStr.c_str());
	return rtstr;
}


/*
 *剪切出人脸
 */
JNIEXPORT jbyteArray JNICALL Java_seetaface_JniClient_CMCropFace
(JNIEnv *env, jobject obj, jbyteArray v_img_data,
		jint cols, jint rows, jint ch, jstring vFaceModelPath, jobject vFaceBmp)
{
//	LOGD("CMCropFace, 1");

	AndroidBitmapInfo  infoFaceBmp;

	jbyte *tImgData = env->GetByteArrayElements(v_img_data,0);

	return v_img_data;

	jbyteArray tRetFace = env->NewByteArray(256*256*3);

	//获取正面人脸检测模型的绝对路径
	const char* tFaceModelPath;
	tFaceModelPath = env->GetStringUTFChars(vFaceModelPath, 0);
	if(tFaceModelPath == NULL) {
		return tRetFace;
	}

	//LOGD("tFaceModelPath = %s", tFaceModelPath);
	//LOGD("CMDetectFace,   rows=%d, cols=%d, ch=%d",   rows, cols, ch);

	string tFaceModelPathStr = tFaceModelPath;

	//检测模型路径
	string tDetectModelPath = tFaceModelPathStr + "seeta_fd_frontal_v1.0.bin";
	//LOGD("CMDetectFace,  tDetectModelPath=%s", tDetectModelPath.c_str());

	//对齐模型路径
	string tAlignModelPath = tFaceModelPathStr + "seeta_fa_v1.1.bin";
	//LOGD("CMDetectFace,  tAlignModelPath=%s", tAlignModelPath.c_str());

	//识别模型路径
	string tRecoModelPath = tFaceModelPathStr + "seeta_fr_v1.0.bin";
	//LOGD("CMDetectFace,  tRecoModelPath=%s", tRecoModelPath.c_str());

	//初始化人脸检测器
	seeta::FaceDetection detector(tDetectModelPath.c_str());
	//LOGD("CMDetectFace, detector ok");
	//
	unsigned char *gray = new unsigned char[rows*cols];
	CMImgProc::RGBA2GRAY((unsigned char*)tImgData, gray, cols, rows, ch);
	//LOGD("灰度化ok===");

	//LOGD("tImg, rows=%d, cols=%d",   tImg.rows, tImg.cols);

	ImageData img_data;
	img_data.data =  gray;
	img_data.width = cols;
	img_data.height = rows;
	img_data.num_channels = 1;

	time_t timep;
	time (&timep);
	LOGD("CMDetectFace, 时间1： %s ", ctime(&timep));

	std::vector<seeta::FaceInfo> faces = detector.Detect(img_data);
	//LOGD("CMDetectFace, detector.Detect");
	LOGD("CMDetectFace, 时间2： %s ", ctime(&timep));
	LOGD("CMDetectFace, 时间2");

	int32_t num_face = static_cast<int32_t>(faces.size());
	if(0 == num_face ){
		//没有人脸
		LOGD("CMDetectFace, 没有人脸");
		jstring rtstr = env->NewStringUTF("");
		return tRetFace;
	}else{
		LOGD("CMDetectFace, 人脸数:%d", num_face);
	}

	//初始化人脸对齐器
	seeta::FaceAlignment point_detector(tAlignModelPath.c_str());
	//LOGD("CMDetectFace, point_detector ok");

	std::string tRetFacePosStr = "";
	char tPosStr[200] = {0};

	for(int i=0; i<num_face; i++){
		//每个人脸位置数据用分号分隔
		if(i > 0){
			tRetFacePosStr += ";";
		}

		//组装脸部矩形数据
		seeta::FaceInfo tFace = faces[i];
		sprintf(tPosStr, "%d,%d,%d,%d",
				tFace.bbox.x, tFace.bbox.y, tFace.bbox.width, tFace.bbox.height);

		//LOGD("face_tchar=%s", tPosStr);

		//首先记录人脸位置坐标，4个int
		tRetFacePosStr += tPosStr;

		//然后检测特征点
		seeta::FacialLandmark face_points[5];
		point_detector.PointDetectLandmarks(img_data, faces[i], face_points);
		//组装特征点字符串
		for(int i=0; i<5; i++){
			char tPntStr[100] = {0};
			sprintf(tPntStr, ",%d,%d", (int) face_points[i].x, (int)face_points[i].y);
			tRetFacePosStr += tPntStr;
			//strcat(pnts_str, x);
		}
		//
	}
	LOGD("tRetFacePosStr=%s", tRetFacePosStr.c_str());

	FaceIdentification face_recognizer(tRecoModelPath.c_str());
	LOGD("CMDetectFace, face_recognizer ok");

	float feat1[2048];
	seeta::FacialLandmark face_points_x[5];
	point_detector.PointDetectLandmarks(img_data, faces[0], face_points_x);
	LOGD("CMDetectFace, before ExtractFeatureWithCrop 1");

	// Create a image to store crop face.
	//剪切出来的头像

	ImageData face_img(face_recognizer.crop_width(), face_recognizer.crop_height(), face_recognizer.crop_channels());

	face_recognizer.CropFace(img_data, face_points_x, face_img);

	LOGD("CMDetectFace, CropFace 1 ok");

	int ret;
	if ((ret = AndroidBitmap_getInfo(env, vFaceBmp, &infoFaceBmp)) < 0) {
		LOGE("AndroidBitmap_getInfo() failed ! error=%d", ret);
		return tRetFace;
	}
	//LOGD("CMDetectFace, tRetFacePosStr.c_str()=%s", tRetFacePosStr.c_str());

	//sprintf(tRetStr, "face_num=%d", num_face);

	env->ReleaseStringUTFChars(vFaceModelPath, tFaceModelPath);

	return tRetFace;
}


JNIEXPORT jstring JNICALL Java_seetaface_JniClient_CMimread(JNIEnv *env, jobject obj, jstring impath)
{
	const char* tPath;
	tPath = env->GetStringUTFChars(impath, 0);
	if(tPath == NULL) {
		return NULL; /* OutOfMemoryError already thrown */
	}
	int32_t num_face = 0;
	/*

	 Mat im;
	im = imread(tPath);
	LOGD("CMimread, impath=%s, rows=%d, cols=%d", tPath, im.rows, im.cols);

	// Initialize face detection model
	seeta::FaceDetection detector("/mnt/sdcard/seeta_fd_frontal_v1.0.bin");


	//char* impath = "/mnt/sdcard/DCIM/lxr6.jpg";
	//char* impath = "/sdcard/temp.jpg";
	Mat tImg;
	tImg = imread(tPath);
	LOGD("tImg, tPath = %s, rows=%d, cols=%d", tPath, tImg.rows, tImg.cols);

	cv::Mat img_gray;

	if (tImg.channels() != 1)
		cv::cvtColor(tImg, img_gray, cv::COLOR_BGR2GRAY);
	else
		img_gray = tImg;

	ImageData img_data;
	img_data.data = img_gray.data;
	img_data.width = img_gray.cols;
	img_data.height = img_gray.rows;
	img_data.num_channels = 1;
	cv::TickMeter tm;
	tm.start();
	std::vector<seeta::FaceInfo> faces = detector.Detect(img_data);
	tm.stop();
	//std::cout << "Detect cost " << tm.getTimeMilli() << "ms" << std::endl;
	cv::Rect face_rect;
	  num_face = static_cast<int32_t>(faces.size());
	LOGD("face_num=%d", num_face);


	env->ReleaseStringUTFChars(impath, tPath);
	char* tmpstr = "return string succeeded";
	 */
	char tRetStr[260] = {0};
	sprintf(tRetStr, "face_num=%d", num_face);

	jstring rtstr = env->NewStringUTF(tRetStr);
	return rtstr;
}



#ifdef __cplusplus
}
#endif
