/*
 *
 * This file is part of the open-source SeetaFace engine, which includes three modules:
 * SeetaFace Detection, SeetaFace Alignment, and SeetaFace Identification.
 *
 * This file is an example of how to use SeetaFace engine for face detection, the
 * face detection method described in the following paper:
 *
 *
 *   Funnel-structured cascade for multi-view face detection with alignment awareness,
 *   Shuzhe Wu, Meina Kan, Zhenliang He, Shiguang Shan, Xilin Chen.
 *   In Neurocomputing (under review)
 *
 *
 * Copyright (C) 2016, Visual Information Processing and Learning (VIPL) group,
 * Institute of Computing Technology, Chinese Academy of Sciences, Beijing, China.
 *
 * The codes are mainly developed by Shuzhe Wu (a Ph.D supervised by Prof. Shiguang Shan)
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

#include <cstdint>
#include <fstream> 
#include <iostream>
#include <string>
#include "mropencv.h"
#include "face_detection.h"

#define DETECT_FROM_VIDEO


int testfromvideo(int argc, char** argv)
{
	seeta::FaceDetection detector("seeta_fd_frontal_v1.0.bin");
	detector.SetMinFaceSize(80);
	detector.SetScoreThresh(2.f);
	detector.SetImagePyramidScaleFactor(0.8f);
	detector.SetWindowStep(2, 2);

	cv::VideoCapture capture(0);
	cv::Mat img;
	while (1)
	{
		capture >> img;
		if (!img.data)
			return -1;

		cv::Mat img_gray;

		if (img.channels() != 1)
			cv::cvtColor(img, img_gray, cv::COLOR_BGR2GRAY);
		else
			img_gray = img;

		seeta::ImageData img_data;
		img_data.data = img_gray.data;
		img_data.width = img_gray.cols;
		img_data.height = img_gray.rows;
		img_data.num_channels = 1;
		cv::TickMeter tm;
		tm.start();
		std::vector<seeta::FaceInfo> faces = detector.Detect(img_data);
		tm.stop();
		std::cout << "Detect cost " << tm.getTimeMilli() << "ms" << std::endl;
		cv::Rect face_rect;
		int32_t num_face = static_cast<int32_t>(faces.size());

		for (int32_t i = 0; i < num_face; i++) {
			face_rect.x = faces[i].bbox.x;
			face_rect.y = faces[i].bbox.y;
			face_rect.width = faces[i].bbox.width;
			face_rect.height = faces[i].bbox.height;
			cv::rectangle(img, face_rect, CV_RGB(0, 0, 255), 1, 1, 0);
		}

		cv::namedWindow("Test", cv::WINDOW_AUTOSIZE);
		cv::imshow("Test", img);
		cv::waitKey(1);
	}

	return 0;
}

int testfromimg(int argc, char** argv) {
	std::cout << "testfromimg xx" << std::endl; 
	//return 0; 
	////getchar(); 
  //const char* img_path = "e:\\img\\p.jpg";
  const char* img_path = "test_image.jpg";
  seeta::FaceDetection detector("seeta_fd_frontal_v1.0.bin");
 
  detector.SetMinFaceSize(20);
  detector.SetScoreThresh(2.f);
  detector.SetImagePyramidScaleFactor(0.8f);
  detector.SetWindowStep(2, 2);

  cv::Mat img = cv::imread(img_path, cv::IMREAD_UNCHANGED);
  cv::Mat img_gray;

  if (img.channels() != 1)
    cv::cvtColor(img, img_gray, cv::COLOR_BGR2GRAY);
  else
    img_gray = img;

  //std::cout << "testfromimg xx2" << std::endl;
    
  seeta::ImageData img_data;
  img_data.data = img_gray.data;
  img_data.width = img_gray.cols;
  img_data.height = img_gray.rows;
  img_data.num_channels = 1;  

  std::vector<seeta::FaceInfo> faces = detector.Detect(img_data);
  //std::cout << "testfromimg xx3" << std::endl;
  cv::Rect face_rect;
  int32_t num_face = static_cast<int32_t>(faces.size());

  //std::cout << "num_face=" << num_face << std::endl;

  for (int32_t i = 0; i < num_face; i++) {
    face_rect.x = faces[i].bbox.x;
    face_rect.y = faces[i].bbox.y;
    face_rect.width = faces[i].bbox.width;
    face_rect.height = faces[i].bbox.height;
    cv::rectangle(img, face_rect, CV_RGB(0, 0, 255), 1, 1, 0);
  }

  cv::namedWindow("test", cv::WINDOW_AUTOSIZE);
  cv::imshow("test", img);
  cv::waitKey(0);
  cv::destroyAllWindows(); 
  
  return 0;
}


int main(int argc, char** argv)
{
	/*
	const char* img_path = "e:\\img\\p.jpg";
	 
	cv::Mat img = cv::imread(img_path, cv::IMREAD_UNCHANGED);

	cv::namedWindow("test", cv::WINDOW_AUTOSIZE);
	cv::imshow("test", img);
	cv::waitKey(0);
	cv::destroyAllWindows();
	*/
	    
//	testfromvideo(argc,argv);
 	testfromimg(argc, argv);

	return 0;
}