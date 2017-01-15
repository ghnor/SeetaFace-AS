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

package seetaface;

import android.graphics.Bitmap;

public class JniClient {
	static {
		System.loadLibrary("SeetafaceSo");
	}

	/**
	 * 计算2个人脸的相似度，是根据保存在文件里的特征进行比对的，文件名格式为 /sdcard/seeta_fea%d.txt
	 * @param vFaceNo1
	 * @param vFaceNo2
	 * @param vDetectModelPath
	 * @return
	 */
	public native float CMCalcFaceSim(int vFaceNo1, int vFaceNo2, String vDetectModelPath);
	
	/**
	 * 图像的gamma校正
	 * @param vColorBmp:原图
	 * @param vGammaBmp:处理后的图
	 * @param vGamma:gamma值
	 */
	public native void CMimGamma(Bitmap vColorBmp, Bitmap vGammaBmp, float vGamma);
	
	public native void CMim2gray(Bitmap vColorBmp, Bitmap vGrayBmp);
	
	/**
	 * 检测人脸，返回各人脸位置，每个人的以;分隔，坐标以分号分隔
	 * @param vImgData:图像的char*数据
	 * @param vColNum:图像列数
	 * @param vRowNum:图像行数
	 * @param vCh:图像通道数，3或4
	 * @param vDetectModelPath:正面人脸检测模型的绝对路径
	 * @param vFaceNo:人脸编号，用于保存特征数据生成文件名用
	 * @param vFaceBmp:人脸抠图
	 * @return
	 */	
	public native String CMDetectFace(byte[] vImgData, int vColNum, int vRowNum, int vCh, String vDetectModelPath, int vFaceNo, Bitmap vFaceBmp);
	
	/**
	 * 剪切出人脸
	 * @param vImgData
	 * @param vColNum
	 * @param vRowNum
	 * @param vCh
	 * @param vDetectModelPath
	 * @return
	 */
	public native byte[] CMCropFace(byte[] vImgData, int vColNum, int vRowNum, int vCh, String vDetectModelPath, Bitmap vFaceBmp);
	
	/**
	 * 人脸比对
	 * @param vImgData1
	 * @param vColNum1
	 * @param vRowNum1
	 * @param vCh1
	 * @param vImgData2
	 * @param vColNum2
	 * @param vRowNum2
	 * @param vCh2
	 * @param vDetectModelPath
	 * @return
	 */
	public native float CMCompFace(byte[] vImgData1, int vColNum1, int vRowNum1, int vCh1,
			byte[] vImgData2, int vColNum2, int vRowNum2, int vCh2,
			String vDetectModelPath);
	
} 
