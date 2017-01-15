/*
 *
 * This file is part of the open-source SeetaFace engine, which includes three modules:
 * SeetaFace Detection, SeetaFace Alignment, and SeetaFace Identification.
 *
 * This file is part of the SeetaFace Identification module, containing codes implementing the
 * face identification method described in the following paper:
 *
 *   
 *   VIPLFaceNet: An Open Source Deep Face Recognition SDK,
 *   Xin Liu, Meina Kan, Wanglong Wu, Shiguang Shan, Xilin Chen.
 *   In Frontiers of Computer Science.
 *
 *
 * Copyright (C) 2016, Visual Information Processing and Learning (VIPL) group,
 * Institute of Computing Technology, Chinese Academy of Sciences, Beijing, China.
 *
 * The codes are mainly developped by Wanglong Wu(a Ph.D supervised by Prof. Shiguang Shan)
 *
 * As an open-source face recognition engine: you can redistribute SeetaFace source codes
 * and/or modify it under the terms of the BSD 2-Clause License.
 *
 * You should have received a copy of the BSD 2-Clause License along with the software.
 * If not, see < https://opensource.org/licenses/BSD-2-Clause>.
 *
 * Contact Info: you can send an email to SeetaFace@vipl.ict.ac.cn for any problems. 
 *
 * Note: the above information must be kept whenever or wherever the codes are used.
 *
 */

#include<iostream>
using namespace std;

#if defined(__unix__) || defined(__APPLE__)

#ifndef fopen_s

#define fopen_s(pFile,filename,mode) ((*(pFile))=fopen((filename),(mode)))==NULL

#endif //fopen_s

#endif //__unix
 
#include "mropencv.h"
#include "face_identification.h"
#include "face_alignment.h" 
#include "face_detection.h" 
#include "common.h"
    
#include "math.h"
#include "time.h" 
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <stdlib.h>
#include <stdio.h>

#include "ctime"

using namespace seeta;

#define TEST(major, minor) major##_##minor##_Tester()
#define EXPECT_NE(a, b) if ((a) == (b)) std::cout << "ERROR: "
#define EXPECT_EQ(a, b) if ((a) != (b)) std::cout << "ERROR: "

#ifdef _WIN32
std::string DATA_DIR = "../../data/";
std::string MODEL_DIR = "../../model/";
#else
std::string DATA_DIR = "./data/";
std::string MODEL_DIR = "./model/";
#endif

void TEST(FaceRecognizerTest, CropFace) {
  FaceIdentification face_recognizer((MODEL_DIR + "seeta_fr_v1.0.bin").c_str());
  std::string test_dir = DATA_DIR + "test_face_recognizer/";
  /* data initialize */
  std::ifstream ifs;
  std::string img_name;
  FacialLandmark pt5[5];
  ifs.open(test_dir + "test_file_list.txt", std::ifstream::in);
  clock_t start, count = 0;
  int img_num = 0;
  while (ifs >> img_name) {
    img_num ++ ;
    // read image
    cv::Mat src_img = cv::imread(test_dir + img_name, 1);
    EXPECT_NE(src_img.data, nullptr) << "Load image error!";

    // ImageData store data of an image without memory alignment.
    ImageData src_img_data(src_img.cols, src_img.rows, src_img.channels());
    src_img_data.data = src_img.data;
	 
    // 5 located landmark points (left eye, right eye, nose, left and right 
    // corner of mouse).
    for (int i = 0; i < 5; ++ i) {
      ifs >> pt5[i].x >> pt5[i].y;
    }

    // Create a image to store crop face.
    cv::Mat dst_img(face_recognizer.crop_height(),
     face_recognizer.crop_width(),
     CV_8UC(face_recognizer.crop_channels()));
	
    ImageData dst_img_data(dst_img.cols, dst_img.rows, dst_img.channels());
    dst_img_data.data = dst_img.data;
    /* Crop Face */
    start = clock();
    face_recognizer.CropFace(src_img_data, pt5, dst_img_data);
    count += clock() - start;

    // Show crop face
    cv::imshow("Crop Face", dst_img);
	cv::waitKey(0);
    cv::destroyWindow("Crop Face");
  }
  ifs.close();
  std::cout << "Test successful! \nAverage crop face time: "
    << 1000.0 * count / CLOCKS_PER_SEC / img_num << "ms" << std::endl;
}

void TEST(FaceRecognizerTest, ExtractFeature) {
  FaceIdentification face_recognizer((MODEL_DIR + "seeta_fr_v1.0.bin").c_str());
  std::string test_dir = DATA_DIR + "test_face_recognizer/";

  int feat_size = face_recognizer.feature_size();
  EXPECT_EQ(feat_size, 2048);

  FILE* feat_file = NULL;

  // Load features extract from caffe
  fopen_s(&feat_file, (test_dir + "feats.dat").c_str(), "rb");
  int n, c, h, w;
  EXPECT_EQ(fread(&n, sizeof(int), 1, feat_file), (unsigned int)1);
  EXPECT_EQ(fread(&c, sizeof(int), 1, feat_file), (unsigned int)1);
  EXPECT_EQ(fread(&h, sizeof(int), 1, feat_file), (unsigned int)1);
  EXPECT_EQ(fread(&w, sizeof(int), 1, feat_file), (unsigned int)1);
  n *= 2; 
  float* feat_caffe = new float[n * c * h * w];
  float* feat_sdk = new float[n * c * h * w];
  EXPECT_EQ(fread(feat_caffe, sizeof(float), n * c * h * w, feat_file),
    n * c * h * w);
  EXPECT_EQ(feat_size, c * h * w);

  int cnt = 0;

  /* Data initialize */
  std::ifstream ifs(test_dir + "crop_file_list.txt");   
  std::string img_name;

  clock_t start, count = 0;
  int img_num = 0, lb;
  double average_sim = 0.0;
  while (ifs >> img_name >> lb) {
    // read image
    cv::Mat src_img = cv::imread(test_dir + img_name, 1);
    EXPECT_NE(src_img.data, nullptr) << "Load image error!";
    cv::resize(src_img, src_img, cv::Size(face_recognizer.crop_height(),
      face_recognizer.crop_width()));

    // ImageData store data of an image without memory alignment.
    ImageData src_img_data(src_img.cols, src_img.rows, src_img.channels());
    src_img_data.data = src_img.data;

    /* Extract feature */
    start = clock();
    face_recognizer.ExtractFeature(src_img_data,
      feat_sdk + img_num * feat_size);
    count += clock() - start;

    /* Caculate similarity */
    float* feat1 = feat_caffe + img_num * feat_size;
    float* feat2 = feat_sdk + img_num * feat_size;
    float sim = face_recognizer.CalcSimilarity(feat1, feat2);
    average_sim += sim;
    img_num ++ ;
  }
  ifs.close();
  average_sim /= img_num;
  if (1.0 - average_sim >  0.01) {
    std::cout<< "average similarity: " << average_sim << std::endl;
  }
  else {
    std::cout << "Test successful!\nAverage extract feature time: "
      << 1000.0 * count / CLOCKS_PER_SEC / img_num << "ms" << std::endl;
  }
  delete []feat_caffe;
  delete []feat_sdk;
}

void TEST(FaceRecognizerTest, ExtractFeatureWithCrop) {
	std::cout << "ExtractFeatureWithCrop 111" << endl; 

  FaceIdentification face_recognizer((MODEL_DIR + "seeta_fr_v1.0.bin").c_str());
  std::string test_dir = DATA_DIR + "test_face_recognizer/";

  int feat_size = face_recognizer.feature_size();
  EXPECT_EQ(feat_size, 2048);

  FILE* feat_file = NULL;

  // Load features extract from caffe
  fopen_s(&feat_file, (test_dir + "feats.dat").c_str(), "rb");
  int n, c, h, w;
  EXPECT_EQ(fread(&n, sizeof(int), 1, feat_file), (unsigned int)1);
  EXPECT_EQ(fread(&c, sizeof(int), 1, feat_file), (unsigned int)1);
  EXPECT_EQ(fread(&h, sizeof(int), 1, feat_file), (unsigned int)1);
  EXPECT_EQ(fread(&w, sizeof(int), 1, feat_file), (unsigned int)1);
  
  std::cout << "ExtractFeatureWithCrop n="<<n<<",c="<<c<<",h="<<h<<",w="<<w << endl;

  float* feat_caffe = new float[n * c * h * w];
  float* feat_sdk = new float[n * c * h * w];
  EXPECT_EQ(fread(feat_caffe, sizeof(float), n * c * h * w, feat_file),
    n * c * h * w);
  EXPECT_EQ(feat_size, c * h * w);

  int cnt = 0;

  /* Data initialize */
  std::ifstream ifs(test_dir + "test_file_list.txt");
  std::string img_name;
  FacialLandmark pt5[5];

  clock_t start, count = 0;
  int img_num = 0;
  double average_sim = 0.0;
  while (ifs >> img_name) {
    // read image
    cv::Mat src_img = cv::imread(test_dir + img_name, 1);

    EXPECT_NE(src_img.data, nullptr) << "Load image error!";

    // ImageData store data of an image without memory alignment.
    ImageData src_img_data(src_img.cols, src_img.rows, src_img.channels());
    src_img_data.data = src_img.data;

    // 5 located landmark points (left eye, right eye, nose, left and right 
    // corner of mouse).
    for (int i = 0; i < 5; ++ i) {
		ifs >> pt5[i].x >> pt5[i].y;
    }
	
    /* Extract feature: ExtractFeatureWithCrop */
    start = clock();
    face_recognizer.ExtractFeatureWithCrop(src_img_data, pt5,   feat_sdk + img_num * feat_size);
	 
    count += clock() - start;

    /* Caculate similarity*/
    float* feat1 = feat_caffe + img_num * feat_size;
    float* feat2 = feat_sdk + img_num * feat_size;
    float sim = face_recognizer.CalcSimilarity(feat1, feat2);
	std::cout << "img_name=" << img_name << ",sim="<<sim<<std::endl;

    average_sim += sim;
    img_num ++ ;
  }
  
  ifs.close();
  average_sim /= img_num;
  std::cout << "average similarity: " << average_sim << std::endl;

  if (1.0 - average_sim >  0.02) {
    std::cout<< "small average similarity: " << average_sim << std::endl;
  }

  else {
    std::cout << "Test successful!\nAverage extract feature time: "
      << 1000.0 * count / CLOCKS_PER_SEC / img_num << "ms" << std::endl;
  }
  delete []feat_caffe;
  delete []feat_sdk;
}

void testDetect(){
	cout << "testDetect"<<endl;
	seeta::FaceDetection detector("D:\\SeetaFaceEngine-windows\\FaceDetection\\model\\seeta_fd_frontal_v1.0.bin");

	detector.SetMinFaceSize(40);
	detector.SetScoreThresh(2.f);
	detector.SetImagePyramidScaleFactor(0.8f);
	detector.SetWindowStep(4, 4);

	// Initialize face alignment model  
	cv::Mat img_color = cv::imread("e:\\img\\me1.jpg");
	int pts_num = 5; 
	cv::Mat img_gray;
	if (img_color.channels() != 1)
		cv::cvtColor(img_color, img_gray, cv::COLOR_BGR2GRAY);
	else
		img_gray = img_color;
	seeta::ImageData image_data;
	image_data.data = img_gray.data;
	image_data.width = img_gray.cols;
	image_data.height = img_gray.rows;
	image_data.num_channels = 1;

	std::vector<seeta::FaceInfo> faces = detector.Detect(image_data);
	
	int32_t face_num = static_cast<int32_t>(faces.size());
	cout << "face_num =" << face_num;
		
	// Initialize face alignment model 
	seeta::FaceAlignment point_detector("D:\\SeetaFaceEngine-windows\\FaceAlignment\\seeta_fa_v1.0.bin");
	
	FaceIdentification face_recognizer((MODEL_DIR + "seeta_fr_v1.0.bin").c_str());

	int feat_size = face_recognizer.feature_size();
	EXPECT_EQ(feat_size, 2048);
	
	int n = 51, c = feat_size, h = 1, w = 1;
	float* feat_caffe = new float[n * c * h * w];
	//float* feat_sdk = new float[n * c * h * w];
	
	// Detect 5 facial landmarks
	for (int f = 0; f < face_num; f++){
		FacialLandmark points[5]; 
		point_detector.PointDetectLandmarks(image_data, faces[f], points);
		face_recognizer.ExtractFeatureWithCrop(image_data, points, feat_caffe );

		cv::rectangle(img_color, cv::Point(faces[f].bbox.x, faces[f].bbox.y),
			cv::Point(faces[f].bbox.x + faces[f].bbox.width - 1, faces[f].bbox.y + faces[f].bbox.height - 1), CV_RGB(255, 0, 0));
		for (int i = 0; i < pts_num; i++)
		{
			cv::circle(img_color, cv::Point(points[i].x, points[i].y), 2, CV_RGB(0, 255, 0), -CV_FILLED);
		}		 
	}
		
	cv::imwrite("result.jpg", img_color);
	cv::imshow("face_alignment", img_color);
	cv::waitKey();
}

void testReco(){
	cout << "testReco1" << endl;
	seeta::FaceDetection detector("D:\\SeetaFaceEngine-windows\\FaceDetection\\model\\seeta_fd_frontal_v1.0.bin");

	detector.SetMinFaceSize(40); 
	detector.SetScoreThresh(2.f);
	detector.SetImagePyramidScaleFactor(0.8f);
	detector.SetWindowStep(4, 4);
	//截图时已经旋转正了
	// Initialize face alignment model  1  
	cv::Mat img_color = cv::imread("e:\\img\\lxr8.jpg");
	if (img_color.rows < 10 || img_color.cols < 10){
		cout << "error, img_color size = 0" << endl;
		return;
	}

	//std::cout << "testReco2 rows=" << img_color.rows << ",cols=" << img_color.cols << endl;
	//缩放后的图像大小
	int tDestSize = 1200; 
	float scale = float(tDestSize) / (img_color.rows + img_color.cols);
	  
	int tRows = img_color.rows * scale;
	int tCols = img_color.cols * scale;
	std::cout << "testReco3 tRows=" << tRows << ",tCols=" << tCols << ", scale=" << scale<<endl;
	 
	if (tRows < 40 || tCols < 40){
		cout << "error, img_color resize size two small" << endl;
		return;
	}
	 
	Size tSize = cv::Size(tCols, tRows);
	 
	cv::resize(img_color, img_color, tSize);

	std::cout << "testReco rows=" << img_color.rows << ",cols=" << img_color.cols << endl;
  
	/* 旋转
	cv::Point center(img_color.cols/2, img_color.rows / 2);
	double  angle = -15;	 
	Mat R = getRotationMatrix2D(center, angle, 1.0);// 逆时针旋转angle  
	Mat imgR;    
	warpAffine(img_color, img_color, R, Size(img_color.cols, img_color.rows));
	//imshow("Image", img_color);
	imshow("Rotate image", img_color);
	waitKey(0);
	*/ 
	//cv::warpAffine(img_color, img_color, )
	
	int pts_num = 5; 
	cv::Mat img_gray;
	if (img_color.channels() != 1)
		cv::cvtColor(img_color, img_gray, cv::COLOR_BGR2GRAY);
	else
		img_gray = img_color;
	seeta::ImageData image_data;
	image_data.data = img_gray.data;
	image_data.width = img_gray.cols;
	image_data.height = img_gray.rows;
	image_data.num_channels = 1;

	//彩色的ImageData
	seeta::ImageData image_data_color;
	image_data_color.data = img_color.data;
	image_data_color.width = img_color.cols;
	image_data_color.height = img_color.rows;
	image_data_color.num_channels = 3;


	std::vector<seeta::FaceInfo> faces = detector.Detect(image_data);

	int32_t face_num = static_cast<int32_t>(faces.size());
	cout << "img_color face_num =" << face_num <<endl;

	// Initialize face alignment model 2
	cv::Mat img_color2 = cv::imread("e:\\img\\lxr1.jpg");
	if (img_color2.rows < 10 || img_color2.cols < 10){
		cout << "error, img_color2 size = 0" << endl;
		return;
	}

	scale = float(tDestSize) / (img_color2.rows + img_color2.cols);
	tRows = img_color2.rows * scale;
	tCols = img_color2.cols * scale;
	//std::cout << "testReco3 tRows=" << tRows << ",tCols=" << tCols << ", scale=" << scale<<endl;
	if (tRows < 40 || tCols < 40){
		cout << "error, img_color2 resize size two small" << endl;
		return;
	}

	tSize = cv::Size(tCols, tRows);

	cv::resize(img_color2, img_color2, tSize);
	if (img_color2.rows < 10 || img_color2.cols < 10){
		cout << "error, img_color2 resized size = 0" << endl;
		return;
	}

	
	cv::Mat img_gray2;
	if (img_color2.channels() != 1)
		cv::cvtColor(img_color2, img_gray2, cv::COLOR_BGR2GRAY);
	else
		img_gray2 = img_color2;
	seeta::ImageData image_data2;
	image_data2.data = img_gray2.data;
	image_data2.width = img_gray2.cols;
	image_data2.height = img_gray2.rows;
	image_data2.num_channels = 1;

	std::vector<seeta::FaceInfo> faces2 = detector.Detect(image_data2);
	
	int32_t face_num2 = static_cast<int32_t>(faces2.size());
	cout << "img_color2 face_num2 =" << face_num2<<endl;
	if (face_num2 < 1){
		cout << "face_num2=" << face_num2 << endl; 
		return; 
	}

	// Initialize face alignment model 
	seeta::FaceAlignment point_detector("D:\\SeetaFaceEngine-windows\\FaceAlignment\\seeta_fa_v1.0.bin");
	
	FaceIdentification face_recognizer((MODEL_DIR + "seeta_fr_v1.0.bin").c_str());
	//FaceIdentification face_recognizer2((MODEL_DIR + "seeta_fr_v1.0.bin").c_str());

	int feat_size = face_recognizer.feature_size();
	EXPECT_EQ(feat_size, 2048);
	
	int n = 43, c = feat_size, h = 1, w = 1;
	float* feat_caffe = new float[n * c * h * w];
	
	float* feat_sdk = new float[n * c * h * w];

	float* feat0 = new float[2048]; 
	

	// Detect 5 facial landmarks
	FacialLandmark points[5];
	for (int f = 0; f < face_num; f++){	
		point_detector.PointDetectLandmarks(image_data, faces[f], points);
		
		// Create a image to store crop face.
		cv::Mat dst_img(face_recognizer.crop_height(),
			face_recognizer.crop_width(),
			CV_8UC(face_recognizer.crop_channels()));

		ImageData dst_img_data(dst_img.cols, dst_img.rows, dst_img.channels());
		dst_img_data.data = dst_img.data;
		/* Crop Face */
		face_recognizer.ExtractFeatureWithCrop(image_data, points, feat_caffe  );
		
		//face_recognizer.ExtractFeatureWithCrop(image_data, points, feat0);
		face_recognizer.CropFace(image_data_color, points, dst_img_data);
	 
		char winname[50] = { 0 };
		sprintf(winname, "faceno%d", f); 
		//std::string win_name = "facewin" + f; 
		//cout << "win_name=" << win_name << endl; 
		cv::imshow(winname, dst_img);
		
		cv::rectangle(img_color, cv::Point(faces[f].bbox.x, faces[f].bbox.y),
			cv::Point(faces[f].bbox.x + faces[f].bbox.width - 1, faces[f].bbox.y + faces[f].bbox.height - 1), CV_RGB(255, 0, 0));
		  
		for (int i = 0; i < pts_num; i++){
			cv::circle(img_color, cv::Point(points[i].x, points[i].y), 2, CV_RGB(0, 255, 0), -CV_FILLED);
		}
	}
	
	//cv::destroyWindow("Crop Face");
	 
	/*
	FILE* feat_file = NULL;
	fopen_s(&feat_file, "d:\\face1.txt", "wb"); 
	fwrite(feat_caffe, sizeof(float), 1 * c * h * w, feat_file);
		
	FILE* feat_file2 = NULL;
	fopen_s(&feat_file2, "d:\\face1.txt", "rb");
	fread(feat_sdk, sizeof(float), 1 * c * h * w, feat_file2);
	*/
	
	float *feat2 = new float[2048]; 
	//// Detect 5 facial landmarks 2
	FacialLandmark pnts[5];
	for (int j = 0; j < face_num2; j++){		
		point_detector.PointDetectLandmarks(image_data2, faces2[j], pnts);	 
		//float* featf = new float[2048];
		//face_recognizer.ExtractFeatureWithCrop(image_data2, pnts, feat2); 	 
		face_recognizer.ExtractFeatureWithCrop(image_data2, pnts, feat_sdk); 	 
		cv::rectangle(img_color2, cv::Point(faces2[j].bbox.x, faces2[j].bbox.y),
			cv::Point(faces2[j].bbox.x + faces2[j].bbox.width - 1, faces2[j].bbox.y + faces2[j].bbox.height - 1), CV_RGB(255, 0, 0));
		for (int i = 0; i < pts_num; i++)
		{
			cv::circle(img_color2, cv::Point(pnts[i].x, pnts[i].y), 2, CV_RGB(0, 255, 0), -CV_FILLED);
		}
	} 
		   
	float sim = -1; 
	sim = face_recognizer.CalcSimilarity(feat_caffe, feat_sdk);
 
	std::cout << "sim=" << sim << std::endl;

	cv::imwrite("result.jpg", img_color);
	cv::imshow("face_alignment1", img_color);
	cv::imshow("face_alignment2", img_color2);

	cv::waitKey();
}

void TEST(FaceRecognizerTest, compare) {
	std::cout << "FaceRecognizerTest compare 111" << endl;
	/*
	seeta::FaceDetection detector("seeta_fd_frontal_v1.0.bin");

	detector.SetMinFaceSize(40);
	detector.SetScoreThresh(2.f);
	detector.SetImagePyramidScaleFactor(0.8f);
	detector.SetWindowStep(4, 4);

	// Initialize face alignment model 
	//seeta::FaceAlignment point_detector("seeta_fa_v1.0.bin");
	cv::Mat img_color = cv::imread("e:\\img\\3.jpg");
	int pts_num = 5;
	cv::Mat img_gray;
	if (img_color.channels() != 1)
		cv::cvtColor(img_color, img_gray, cv::COLOR_BGR2GRAY);
	else
		img_gray = img_color;
	seeta::ImageData image_data;
	image_data.data = img_gray.data;
	image_data.width = img_gray.cols;
	image_data.height = img_gray.rows;
	image_data.num_channels = 1;

	std::vector<seeta::FaceInfo> faces = detector.Detect(image_data);
	return ;

	// Detect faces
	
	int32_t face_num = static_cast<int32_t>(faces.size());
	*/
	FaceIdentification face_recognizer((MODEL_DIR + "seeta_fr_v1.0.bin").c_str());
	std::string test_dir =  "E:\\img";

	seeta::FaceAlignment point_detector("D:\\SeetaFaceEngine-windows\\FaceAlignment\\seeta_fa_v1.0.bin");

	int feat_size = face_recognizer.feature_size();
	EXPECT_EQ(feat_size, 2048);

	FILE* feat_file = NULL;

	// Load features extract from caffe
	//fopen_s(&feat_file, (test_dir + "feats.dat").c_str(), "rb");
	int n=50, c=2048, h=1, w=1;
	/*EXPECT_EQ(fread(&n, sizeof(int), 1, feat_file), (unsigned int)1);
	EXPECT_EQ(fread(&c, sizeof(int), 1, feat_file), (unsigned int)1);
	EXPECT_EQ(fread(&h, sizeof(int), 1, feat_file), (unsigned int)1);
	EXPECT_EQ(fread(&w, sizeof(int), 1, feat_file), (unsigned int)1);
	*/
	std::cout << "ExtractFeatureWithCrop n=" << n << ",c=" << c << ",h=" << h << ",w=" << w << endl;

	float* feat_caffe = new float[n * c * h * w];
	float* feat_sdk = new float[n * c * h * w];
	//EXPECT_EQ(fread(feat_caffe, sizeof(float), n * c * h * w, feat_file),
	//	n * c * h * w);

	//EXPECT_EQ(feat_size, c * h * w);

	int cnt = 0;

	/* Data initialize */
	
	std::string img_name;
	FacialLandmark pt5[5];

	clock_t start, count = 0;
 
	double average_sim = 0.0;

	//anchorͼƬ
	cv::Mat src_img = cv::imread("e:\\img\\ldh1.jpg", 1);

	EXPECT_NE(src_img.data, nullptr) << "Load image error!";

	// ImageData store data of an image without memory alignment.
	ImageData src_img_data(src_img.cols, src_img.rows, src_img.channels());
	src_img_data.data = src_img.data;

	// 5 located landmark points (left eye, right eye, nose, left and right 
	// corner of mouse).
 
	//seeta::FaceAlignment point_detector("seeta_fa_v1.0.bin");
	seeta::FacialLandmark points[5];

	// Detect 5 facial landmarks
	//point_detector.PointDetectLandmarks(src_img_data, faces[0], points);


	/* Extract feature: ExtractFeatureWithCrop */
	start = clock();
	//face_recognizer.ExtractFeatureWithCrop(src_img_data, pt5, feat_caffe);
	 

	/* Caculate similarity*/
	float* feat1 = feat_caffe;
	 
	int img_num = 0;
	while (img_num ++ < 1) {
		// read image
		img_name = "e:\\img\\ldh2.jpg";
		cv::Mat src_img = cv::imread(img_name, 1);

		EXPECT_NE(src_img.data, nullptr) << "Load image error!";

		// ImageData store data of an image without memory alignment.
		ImageData src_img_data(src_img.cols, src_img.rows, src_img.channels());
		src_img_data.data = src_img.data;

		// 5 located landmark points (left eye, right eye, nose, left and right 
		// corner of mouse).
		/*for (int i = 0; i < 5; ++i) {
			ifs >> pt5[i].x >> pt5[i].y;
		}*/

		/* Extract feature: ExtractFeatureWithCrop */
		start = clock();
		face_recognizer.ExtractFeatureWithCrop(src_img_data, pt5,
			feat_sdk + img_num * feat_size);
		count += clock() - start;

		/* Caculate similarity*/
	  
		float* feat2 = feat_sdk + img_num * feat_size;
		float sim = face_recognizer.CalcSimilarity(feat1, feat2);
		std::cout << "img_name=" << img_name << ",sim=" << sim << std::endl;

		average_sim += sim;
		img_num++;
	}

	//ifs.close();
	average_sim /= img_num;
	std::cout << "average similarity: " << average_sim << std::endl;

	if (1.0 - average_sim >  0.02) {
		std::cout << "small average similarity: " << average_sim << std::endl;
	}

	else {
		std::cout << "Test successful!\nAverage extract feature time: "
			<< 1000.0 * count / CLOCKS_PER_SEC / img_num << "ms" << std::endl;
	}
	delete[]feat_caffe;
	delete[]feat_sdk;
}

int main(int argc, char* argv[]) {
	
	//testDetect();
	//testReco(); 
  TEST(FaceRecognizerTest, CropFace);
  
  //TEST(FaceRecognizerTest, ExtractFeature);
   
  //TEST(FaceRecognizerTest, ExtractFeatureWithCrop);
  //TEST(FaceRecognizerTest, compare);
  getchar();
  return 0;  
}
 

//
//
///*
//*
//* This file is part of the open-source SeetaFace engine, which includes three modules:
//* SeetaFace Detection, SeetaFace Alignment, and SeetaFace Identification.
//*
//* This file is part of the SeetaFace Identification module, containing codes implementing the
//* face identification method described in the following paper:
//*
//*
//*   VIPLFaceNet: An Open Source Deep Face Recognition SDK,
//*   Xin Liu, Meina Kan, Wanglong Wu, Shiguang Shan, Xilin Chen.
//*   In Frontiers of Computer Science.
//*
//*
//* Copyright (C) 2016, Visual Information Processing and Learning (VIPL) group,
//* Institute of Computing Technology, Chinese Academy of Sciences, Beijing, China.
//*
//* The codes are mainly developed by Jie Zhang(a Ph.D supervised by Prof. Shiguang Shan)
//*
//* As an open-source face recognition engine: you can redistribute SeetaFace source codes
//* and/or modify it under the terms of the BSD 2-Clause License.
//*
//* You should have received a copy of the BSD 2-Clause License along with the software.
//* If not, see < https://opensource.org/licenses/BSD-2-Clause>.
//*
//* Contact Info: you can send an email to SeetaFace@vipl.ict.ac.cn for any problems.
//*
//* Note: the above information must be kept whenever or wherever the codes are used.
//*
//*/
//
//#include<iostream>
//using namespace std;
//
//#ifdef _WIN32
//#pragma once
//#include <opencv2/core/version.hpp>
//
//#define CV_VERSION_ID CVAUX_STR(CV_MAJOR_VERSION) CVAUX_STR(CV_MINOR_VERSION) \
//  CVAUX_STR(CV_SUBMINOR_VERSION)
//
//#ifdef _DEBUG
//#define cvLIB(name) "opencv_" name CV_VERSION_ID "d"
//#else
//#define cvLIB(name) "opencv_" name CV_VERSION_ID
//#endif //_DEBUG
//
//#pragma comment( lib, cvLIB("core") )
//#pragma comment( lib, cvLIB("imgproc") )
//#pragma comment( lib, cvLIB("highgui") )
//
//#endif //_WIN32
//
//#if defined(__unix__) || defined(__APPLE__)
//
//#ifndef fopen_s
//
//#define fopen_s(pFile,filename,mode) ((*(pFile))=fopen((filename),(mode)))==NULL
//
//#endif //fopen_s
//
//#endif //__unix
//
//#include <opencv/cv.h>
//#include <opencv/highgui.h>
//#include "face_identification.h"
//#include "recognizer.h"
//#include "face_detection.h"
//#include "face_alignment.h"
//
//#include "math_functions.h"
//
//#include <vector>
//#include <string>
//#include <iostream>
//#include <algorithm>
//using namespace std;
//using namespace seeta;
//
//#define TEST(major, minor) major##_##minor##_Tester()
//#define EXPECT_NE(a, b) if ((a) == (b)) std::cout << "ERROR: "
//#define EXPECT_EQ(a, b) if ((a) != (b)) std::cout << "ERROR: "
//
//#ifdef _WIN32
//std::string DATA_DIR = "../../data/";
//std::string MODEL_DIR = "../../model/";
//#else
//std::string DATA_DIR = "./data/";
//std::string MODEL_DIR = "./model/";
//#endif
//
//
//int main(int argc, char* argv[]) {
//
//	// Initialize face detection model
//	seeta::FaceDetection detector("D:\\SeetaFaceEngine-windows\\x64\\Release\\seeta_fd_frontal_v1.0.bin");
//	
//	detector.SetMinFaceSize(40);
//	detector.SetScoreThresh(2.f);
//	detector.SetImagePyramidScaleFactor(0.8f);
//	detector.SetWindowStep(4, 4);
//
//	// Initialize face alignment model 
//	seeta::FaceAlignment point_detector("D:\\SeetaFaceEngine-windows\\x64\\Release\\seeta_fa_v1.1.bin");
//	 
//	// Initialize face Identification model 
//	FaceIdentification face_recognizer("D:\\SeetaFaceEngine-windows\\x64\\Release\\seeta_fr_v1.0.bin");
//	//std::string test_dir = DATA_DIR + "test_face_recognizer/";
//	std::string test_dir = "e:\\img\\";
//	cout << "main wzy" << endl;
//	
//	//load image
//	cv::Mat gallery_img_color = cv::imread(test_dir + "ldh2.jpg", 1);
//	cv::Mat gallery_img_gray;
//	cv::cvtColor(gallery_img_color, gallery_img_gray, CV_BGR2GRAY);
//
//
//	cv::Mat probe_img_color = cv::imread(test_dir + "ldh8.jpg", 1);
//	cv::Mat probe_img_gray;
//	cv::cvtColor(probe_img_color, probe_img_gray, CV_BGR2GRAY);
//	
//	//cv::imshow("gallery_img_gray", gallery_img_gray);
//	//cv::imshow("probe_img_gray", probe_img_gray);
//
//	ImageData gallery_img_data_color(gallery_img_color.cols, gallery_img_color.rows, gallery_img_color.channels());
//	gallery_img_data_color.data = gallery_img_color.data;
//
//	ImageData gallery_img_data_gray(gallery_img_gray.cols, gallery_img_gray.rows, gallery_img_gray.channels());
//	gallery_img_data_gray.data = gallery_img_gray.data;
//
//	ImageData probe_img_data_color(probe_img_color.cols, probe_img_color.rows, probe_img_color.channels());
//	probe_img_data_color.data = probe_img_color.data;
//
//	ImageData probe_img_data_gray(probe_img_gray.cols, probe_img_gray.rows, probe_img_gray.channels());
//	probe_img_data_gray.data = probe_img_gray.data;
//	
//	// Detect faces
//	std::vector<seeta::FaceInfo> gallery_faces = detector.Detect(gallery_img_data_gray);
//	int32_t gallery_face_num = static_cast<int32_t>(gallery_faces.size());
//	 
//	std::vector<seeta::FaceInfo> probe_faces = detector.Detect(probe_img_data_gray);
//	int32_t probe_face_num = static_cast<int32_t>(probe_faces.size());
//
//	cout << "gallery_face_num = " << gallery_face_num << ", probe_face_num=" << probe_face_num << endl; 
//	if (gallery_face_num == 0 || probe_face_num == 0)
//	{
//		std::cout << "Faces are not detected.";
//		return 0;
//	}
//	
//	// Detect 5 facial landmarks
//	seeta::FacialLandmark gallery_points[5];
//	point_detector.PointDetectLandmarks(gallery_img_data_gray, gallery_faces[0], gallery_points);
//
//	seeta::FacialLandmark probe_points[5];
//	point_detector.PointDetectLandmarks(probe_img_data_gray, probe_faces[0], probe_points);
//
//	for (int i = 0; i<5; i++)
//	{
//		cv::circle(gallery_img_color, cv::Point(gallery_points[i].x, gallery_points[i].y), 2,
//			CV_RGB(0, 255, 0));
//
//		cv::circle(probe_img_color, cv::Point(probe_points[i].x, probe_points[i].y), 2,
//			CV_RGB(0, 255, 0));
//	}
//	cv::imwrite("gallery_point_result.jpg", gallery_img_color);
//	cv::imwrite("probe_point_result.jpg", probe_img_color);
//	cv::imshow("gallery_img_color", gallery_img_color);
//	cv::imshow("probe_img_color", probe_img_color);
//	// Extract face identity feature
//	float gallery_fea[2048];
//	float probe_fea[2048];
//	 
//
//	face_recognizer.ExtractFeatureWithCrop(gallery_img_data_color, gallery_points, gallery_fea);
//	face_recognizer.ExtractFeatureWithCrop(probe_img_data_color, probe_points, probe_fea);
//
//	// Caculate similarity of two faces
//	float sim = face_recognizer.CalcSimilarity(gallery_fea, probe_fea);
//	std::cout << "sim="<< sim << endl;
//	
//	cv::waitKey();
//	getchar();
//	return 0;
//}

