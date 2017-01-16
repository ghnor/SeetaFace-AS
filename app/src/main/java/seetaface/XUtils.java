/**
 * 采用中科院山世光开源的SeetaFaceEngine实现android上的人脸检测与对齐、识别
 * 遵照BSD license
 * 广州炒米信息科技有限公司
 * www.cume.cc
 * 吴祖玉
 * wuzuyu365@163.com
 * 2016.11.9
 *
 */

package seetaface;

import android.app.ActivityManager;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Bitmap.Config;
import android.graphics.BitmapFactory;
import android.graphics.BitmapFactory.Options;
import android.graphics.Matrix;
import android.media.ExifInterface;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.text.TextUtils;
import android.util.Log;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.nio.MappedByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.channels.FileChannel.MapMode;


public class XUtils {

  // 照片文件是否存在，注意，与数据库是否存在是不同的
  public static Boolean fileExists(String vPath) {
    if (null == vPath || "".equals(vPath)) {
      return false;
    }

    try {
      File file = new File(vPath);
      if (!file.exists()) {
        return false;
      }
    } catch (Exception e) {
      return false;
    }

    return true;
  }

  public static int calculateInSampleSize(Options options, int maxWidth, int maxHeight) {
/*	  
    final int height = options.outHeight;
    final int width = options.outWidth;
    int inSampleSize = 1;

    if (width > maxWidth || height > maxHeight) {
      if (width > height) {
        inSampleSize = Math.round((float) height / (float) maxHeight);
      } else {
        inSampleSize = Math.round((float) width / (float) maxWidth);
      }

      final float totalPixels = width * height;

      final float maxTotalPixels = maxWidth * maxHeight * 2;

      while (totalPixels / (inSampleSize * inSampleSize) > maxTotalPixels) {
        inSampleSize *= 2;
      }
    }
    return inSampleSize;
    
*/   
	  	int inSampleSize;
	  	
	    final int height = options.outHeight;
	    final int width = options.outWidth;
	    int minhw;
	    if(height < width)
	    	minhw = height;
	    else
	    	minhw = width;
	    
	    if(minhw < maxWidth)
	    	inSampleSize = 1;
	    else
	    {
	    	inSampleSize = Math.round((float) width / (float) maxWidth);
	        while (minhw / inSampleSize > maxWidth) {
	            inSampleSize *= 2;
	          }	    	
	    }
	    return inSampleSize;
  }

  // 获取图片的缩略图， 确保宽度和高度最小的都能覆盖到，比如图片是3000*2000，要缩放到 200*200, 那么缩放后是300*200;
  public static Bitmap getThumbImg(String vPath, int vWidth, int vHeight) {
    Log.v("getThumbImg", "1==================================");
    if (null == vPath) {
      Log.v("getThumbImg", "路径为null");
      return null;
    }

    if (vPath.trim().equals("")) {
      Log.v("getThumbImg", "路径为空");
      return null;
    }

    Log.v("getThumbImg", "path=" + vPath + ", 期望：width=" + vWidth + ",vheight=" + vHeight);

    File file = new File(vPath);
    // 如果不存在了，直接返回
    if (!file.exists()) {
      Log.v("getThumbImg", "文件不存在：path=" + vPath);
      return null;
    }

    // 先获取图片的宽和高
    Options options = new Options();
    options.inJustDecodeBounds = true;
    options.inDither = false; /* 不进行图片抖动处理 */
    options.inPreferredConfig = null; /* 设置让解码器以最佳方式解码 */

    /* 下面两个字段需要组合使用 */
//    options.inPurgeable = true;
//    options.inInputShareable = true;
    BitmapFactory.decodeFile(vPath, options);
    if (options.outWidth <= 0 || options.outHeight <= 0) {
      Log.v("getThumbImg", "解析图片失败");
      return null;
    }
    Log.v("getThumbImg", "wid1:" + options.outWidth + ",height:" + options.outHeight + ",path=" + vPath);
    int height0 = options.outHeight;

    // 压缩图片，注意inSampleSize只能是2的整数次幂，如果不是的，话，向下取最近的2的整数次幂，例如3实际上会是2，7实际上会是4
    options.inSampleSize = calculateInSampleSize(options, vWidth, vHeight);

    Log.v("getThumbImg", "options.inSampleSize=" + options.inSampleSize);

    // 不能用Config.RGB_565

    options.inJustDecodeBounds = false;
    Bitmap thumbImgNow = null;
    try {
      thumbImgNow = BitmapFactory.decodeFile(vPath, options);
    } catch (OutOfMemoryError e) {
      Log.v("getThumbImg", "OutOfMemoryError, decodeFile失败 XXXXXXXXXXXXXXXXXXXXXXXXXXX ");
      return null;
    }

    if (null == thumbImgNow) {
      Log.v("getThumbImg", "decodeFile失败 XXXXXXXXXXXXXXXXXXXXXXXXXXX ");
      return null;
    }

    int wid = thumbImgNow.getWidth();
    int hgt = thumbImgNow.getHeight();
    Log.v("getThumbImg", "1, wid=" + wid + ",hgt=" + hgt);

    int degree = readPictureDegree(vPath);
    if (degree != 0) {
      Log.v("getThumbImg", "degree=" + degree);

      // 把图片旋转为正的方向
      thumbImgNow = rotateImage(degree, thumbImgNow);
    }
    return thumbImgNow;
  }

  /**
   * 旋转图片
   * 
   * @param angle
   * @param bitmap
   * @return Bitmap
   */
  public static Bitmap rotateImage(int angle, Bitmap bitmap) {
    if (null == bitmap) {
      return bitmap;
    }
    // 图片旋转矩阵
    Matrix matrix = new Matrix();
    matrix.postRotate(angle);
    if (null == bitmap) {
      Log.v("rotateImageError", "bitmap is null");
      return bitmap;
    }

    // 得到旋转后的图片
    try {
      Bitmap resizedBitmap = Bitmap.createBitmap(bitmap, 0, 0, bitmap.getWidth(), bitmap.getHeight(), matrix, true);

      return resizedBitmap;
    } catch (OutOfMemoryError e) {
      return bitmap;
    }
  }

  /**
   * 读取图片属性：旋转的角度
   * 
   * @param path
   *          图片绝对路径
   * @return degree旋转的角度
   */
  public static int readPictureDegree(String path) {
    int degree = 0;
    try {
      ExifInterface exifInterface = new ExifInterface(path);

      int orientation = exifInterface.getAttributeInt(ExifInterface.TAG_ORIENTATION, ExifInterface.ORIENTATION_NORMAL);
      switch (orientation) {
        case ExifInterface.ORIENTATION_ROTATE_90:
          degree = 90;
          break;
        case ExifInterface.ORIENTATION_ROTATE_180:
          degree = 180;
          break;
        case ExifInterface.ORIENTATION_ROTATE_270:
          degree = 270;
          break;
      }
    } catch (IOException e) {
      e.printStackTrace();
    }
    return degree;
  }

  public static Boolean saveByteToFile(byte[] vData, String vFilePath) {
    Log.v("saveByteToFile", "1");

    if (null == vData || TextUtils.isEmpty(vFilePath)) {
      Log.v("saveByteToFile", "vData or filepath is error");
      return false;
    }

    Log.v("saveByteToFile", "vData.len=" + vData.length);
    Log.v("saveByteToFile", "vFilePath=" + vFilePath);

    File tFile = new File(vFilePath);
    if (null == tFile) {
      Log.v("saveByteToFile", "new file failed ");
      return false;
    }

    try {
      FileOutputStream fos = new FileOutputStream(tFile);
      try {
        fos.write(vData);
      } catch (IOException e) {
        e.printStackTrace();
      }
    } catch (Exception e) {

    }
    Log.v("saveByteToFile", "success");

    return true;
  }

  public static byte[] loadByteFromFile(String vFilePath, int vLen) {
    Log.v("loadByteFromFile", "1");

    if (TextUtils.isEmpty(vFilePath) || vLen < 1) {
      return null;
    }

    File tFile = new File(vFilePath);
    if (null == tFile) {
      Log.v("loadByteFromFile", "new file failed ");
      return null;
    }

    byte tByte[] = new byte[vLen];
    FileInputStream fin = null;

    try {
      fin = new FileInputStream(tFile);
      int r = fin.read(tByte);
      if (r != vLen) {
        throw new IOException("Can't read all, " + r + " != " + vLen);
      }

    } catch (Exception e) {

    }

    return tByte;
  }

  public static Boolean saveBitmap(Bitmap vBmp, String vPath) {
    Log.v("saveBitmap", "1");
    if (null == vBmp) {
      Log.v("saveBitmap", "(null == vBmp)");
      return false;
    }

    if (vBmp.isRecycled()) {
      Log.e("saveBitmap", "已被回收");
      return false;
    }

    if (vBmp.getWidth() < 1) {
      Log.e("saveBitmap", "图像宽度<1");
      return false;
    }

    if (null == vPath || vPath.trim().equals("")) {
      Log.e("saveBitmap", "(null == vPath)");
      return false;
    }

    int tPos = vPath.lastIndexOf("/");
    String tDir = tPos >= 0 ? vPath.substring(0, tPos) : vPath;

    Log.v("saveBitmap", "父目录，tDir = " + tDir);

    File file = new File(tDir);
    // 如果文件夹不存在则创建
    if (!file.exists() && !file.isDirectory()) {
      file.mkdirs();
    }

    Log.v("saveBitmap", "vPath = " + vPath);
    try {
      FileOutputStream fout = new FileOutputStream(vPath);
      BufferedOutputStream bos = new BufferedOutputStream(fout);
      vBmp.compress(Bitmap.CompressFormat.JPEG, 100, bos);
      bos.flush();
      bos.close();
      Log.v("saveBitmap", "保存成功");
      return true;
    } catch (IOException e) {
      // TODO Auto-generated catch block
      Log.e("saveBitmap", "保存失败," + e.toString());
      e.printStackTrace();
      return false;
    }
  }

  // 整数数组转字符串
  public static String intArrayToString(int[] vArr) {
    String tRetString = "";
    if (null == vArr || vArr.length < 1) {
      return tRetString;
    }
    int num = vArr.length;
    for (int i = 0; i < num; i++) {
      tRetString += vArr[i] + ",";
    }

    if (!TextUtils.isEmpty(tRetString)) {
      tRetString = tRetString.substring(0, tRetString.length() - 1);
    }
    return tRetString;
  }

  /**
   * 根据图像完整路径判断是否图像文件
   * 
   * @param
   * @return
   */
  private boolean isImageFile(String vPath) {
    if (null == vPath || vPath.trim().length() < 1) {
      return false;
    }

    String tPath = vPath.trim();

    // 检查后缀名
    String end = tPath.substring(tPath.lastIndexOf(".") + 1, tPath.length()).toLowerCase();

    if ("".equals(end.trim())) {
      return false;
    }

    // 依据文件扩展名判断是否为图像文件
    if (end.equals("jpg") || end.equals("gif") || end.equals("png") || end.equals("jpeg") || end.equals("bmp")
        || end.equals("mov")) {
    } else {
      return false;
    }

    // 判断文件是否存在
    File file = new File(tPath);
    if (!file.exists()) {
      return false;
    }

    return true;
  }

  // 判断网络是否连接
  public static Boolean isNetworkConnected(Context context) {
    if (context != null) {
      ConnectivityManager mConnectivityManager = (ConnectivityManager) context
          .getSystemService(Context.CONNECTIVITY_SERVICE);
      NetworkInfo mNetworkInfo = mConnectivityManager.getActiveNetworkInfo();
      if (mNetworkInfo != null) {
        return mNetworkInfo.isAvailable();
      }
    }
    return false;
  }

  // 把逗号、分号等符号分隔的字符串转换为double数组
  public static String[] stringStringValueSplit(String vStr) {
    Log.v("stringStringValueSplit", "1");
    if (TextUtils.isEmpty(vStr)) {
      return null;
    }

    Log.v("stringStringValueSplit", vStr);

    String oldString = vStr;
    vStr = vStr.replaceAll("\\r", "");
    vStr = vStr.replaceAll("\\n", "");
    vStr = vStr.replaceAll("\\[", "");
    vStr = vStr.replaceAll("\\]", "");
    vStr = vStr.replaceAll(";", ",");
    vStr = vStr.replaceAll(" ", "");

    Log.v("stringStringValueSplit", "str after replace =" + vStr + ", old=" + oldString);

    return vStr.split(",|;|\\[|\\]");
  }

  // 把逗号、分号等符号分隔的字符串转换为double数组
  public static int[] stringIntValueSplit(String vStr) {
    Log.v("stringIntValueSplit", "1");
    if (TextUtils.isEmpty(vStr)) {
      return null;
    }

    Log.v("stringIntValueSplit", vStr);

    String oldString = vStr;
    vStr = vStr.replaceAll("\\r", "");
    vStr = vStr.replaceAll("\\n", "");
    vStr = vStr.replaceAll("\\[", "");
    vStr = vStr.replaceAll("\\]", "");
    vStr = vStr.replaceAll(";", ",");
    vStr = vStr.replaceAll(" ", "");

    Log.v("stringIntValueSplit", "str after replace =" + vStr + ", old=" + oldString);

    String[] pStr = vStr.split(",|;|\\[|\\]");

    if (null == pStr || 0 == pStr.length) {
      Log.v("stringIntValueSplit", "pStr.length=0");
      return null;
    }
    int[] dRet = new int[pStr.length];

    Log.v("stringIntValueSplit", "pStr.length=" + pStr.length);

    for (int i = 0; i < pStr.length; i++) {
      String tString = pStr[i];
      if (null == tString || tString.trim().equals("")) {
        Log.v("stringIntValueSplit", "i=" + i + ", null, or '' ");
        continue;
      }

      try {
        dRet[i] = Integer.parseInt(tString);
      } catch (NumberFormatException ex) {
        // System.out.println("The String does not contain a parsable integer");
        Log.v("stringIntValueSplit", "NumberFormatException," + ex.toString());
      }
    }

    return dRet;

  }

  public static long getmem_UNUSED(Context mContext) {
    long MEM_UNUSED;
    // 得到ActivityManager
    ActivityManager am = (ActivityManager) mContext.getSystemService(Context.ACTIVITY_SERVICE);
    // 创建ActivityManager.MemoryInfo对象

    ActivityManager.MemoryInfo mi = new ActivityManager.MemoryInfo();
    am.getMemoryInfo(mi);

    // 取得剩余的内存空间

    MEM_UNUSED = mi.availMem / 1024;
    return MEM_UNUSED;
  }


  // android sdk 中的Bitmap类提供了一个实例方法copy用来复制位图，该方法在复制较大图像时
  //
  // 容易造成内存溢出；原因：该方法在复制图像时将在内存中保存两份图像数据。
  //
  // 为了解决这个问题，可以将大图像写入SD卡中的一个临时文件中，然后再从文件中取出图像。
  //
  // 根据以上思路用代码如下：
  /**
   * 根据原位图生成一个新的位图，并将原位图所占空间释放
   * 
   * @param srcBmp
   *          原位图
   * @return 新位图
   */
  public static Bitmap bmpcopy(Bitmap srcBmp) {

    Bitmap destBmp = null;

    try {

      // 创建一个临时文件
      File file = new File("/mnt/sdcard/tempbmp/tmp.txt");
      file.getParentFile().mkdirs();

      RandomAccessFile randomAccessFile = new RandomAccessFile(file, "rw");

      int width = srcBmp.getWidth();
      int height = srcBmp.getHeight();

      FileChannel channel = randomAccessFile.getChannel();
      MappedByteBuffer map = channel.map(MapMode.READ_WRITE, 0, width * height * 4);
      // 将位图信息写进buffer
      srcBmp.copyPixelsToBuffer(map);

      // 释放原位图占用的空间
      srcBmp.recycle();

      // 创建一个新的位图
      destBmp = Bitmap.createBitmap(width, height, Config.ARGB_8888);
      map.position(0);
      // 从临时缓冲中拷贝位图信息
      destBmp.copyPixelsFromBuffer(map);

      channel.close();
      randomAccessFile.close();
    } catch (Exception ex) {
      destBmp = null;
    }

    return destBmp;
  }

  /**
   * 
   * 方法名: </br>
   * 详述:取识别人脸截图 </br>
   * 开发人员：王太顺</br>
   * 创建时间：2015年12月10日</br>
   * 
   * @param
   * @param
   * @return
   */
  
  /*
  public static Bitmap getScaledBitmap(String vPath, int vWidth, int vHeight) {
    Log.v("getThumbImg", "1==================================");
    if (null == vPath) {
      Log.v("getThumbImg", "路径为null");
      return null;
    }

    if (vPath.trim().equals("")) {
      Log.v("getThumbImg", "路径为空");
      return null;
    }

    Log.v("getThumbImg", "path=" + vPath + ", 期望：width=" + vWidth + ",vheight=" + vHeight);

    File file = new File(vPath);
    // 如果不存在了，直接返回
    if (!file.exists()) {
      Log.v("getThumbImg", "文件不存在：path=" + vPath);
      return null;
    }

    // 先获取图片的宽和高
    Options options = new Options();
    options.inJustDecodeBounds = true;
    BitmapFactory.decodeFile(vPath, options);
    if (options.outWidth <= 0 || options.outHeight <= 0) {
      Log.v("getThumbImg", "解析图片失败");
      return null;
    }
    Log.v("getThumbImg", "wid1:" + options.outWidth + ",height:" + options.outHeight + ",path=" + vPath);
    int height0 = options.outHeight;

    // 压缩图片，注意inSampleSize只能是2的整数次幂，如果不是的，话，向下取最近的2的整数次幂，例如3实际上会是2，7实际上会是4
    options.inSampleSize = calculateInSampleSize(options, vWidth, vHeight);

    Log.v("getThumbImg", "options.inSampleSize=" + options.inSampleSize);

    // 不能用Config.RGB_565

    options.inJustDecodeBounds = false;
    Bitmap thumbImgNow = null;
    try {
      thumbImgNow = BitmapFactory.decodeFile(vPath, options);
    } catch (OutOfMemoryError e) {
      Log.v("getThumbImg", "OutOfMemoryError, decodeFile失败 XXXXXXXXXXXXXXXXXXXXXXXXXXX ");
      return null;
    }
    
    //Bitmap.createScaledBitmap(src, dstWidth, dstHeight, filter)
    
    

    if (null == thumbImgNow) {
      Log.v("getThumbImg", "decodeFile失败 XXXXXXXXXXXXXXXXXXXXXXXXXXX ");
      return null;
    }

    int wid = thumbImgNow.getWidth();
    int hgt = thumbImgNow.getHeight();
    Log.v("getThumbImg", "1, wid=" + wid + ",hgt=" + hgt);

    int degree = readPictureDegree(vPath);
    if (degree != 0) {
      Log.v("getThumbImg", "degree=" + degree);

      // 把图片旋转为正的方向
      thumbImgNow = rotateImage(degree, thumbImgNow);
    }
    return thumbImgNow;
  }
  
 */
  
//获取图片的缩略图,宽度和高度中较小的缩放到vMinWidth. 确保宽度和高度最小的都能覆盖到，
 	//比如图片是3000*2000，要缩放到 150*100, 那么vMinWidth=100;	
	public static Bitmap getScaledBitmap(String vPath, int vMinWidth) {
		String tag = "getScaledBitmap";
		Log.v(tag, "1==");
		if(null == vPath){
			Log.v(tag, "路径为null");
			return null; 
		}
		
		if(vPath.trim().equals("")){
			Log.v(tag, "路径为空");
			return null; 
		}
		
		Log.v(tag, "path="+vPath+", 期望：vMinWidth="+vMinWidth);
		
		File file = new File(vPath);
		//如果不存在了，直接返回
		if(!file.exists()){
			Log.v(tag, "文件不存在：path="+vPath);
			return null; 
		}
		 		
		// 先获取图片的宽和高
		Options options = new Options();
		options.inJustDecodeBounds = true;
		BitmapFactory.decodeFile(vPath, options);
		if (options.outWidth <= 0 || options.outHeight <= 0) {
			Log.v(tag, "解析图片失败");
			return null;
		}
		Log.v(tag, "原图大小：width:" + options.outWidth + ",height:"
				+ options.outHeight + ",path=" + vPath);
		int height0 = options.outHeight;

		int tMinWidth = Math.min(options.outWidth, options.outHeight);
				
		// 压缩图片，注意inSampleSize只能是2的整数次幂，如果不是的，话，向下取最近的2的整数次幂，例如3实际上会是2，7实际上会是4
		options.inSampleSize = Math.max(1, tMinWidth/vMinWidth);
		//Log.v(tag, "options.inSampleSize="+options.inSampleSize);
			 
		//不能用Config.RGB_565
		//options.inPreferredConfig = Config.RGB_565;  
//		options.inDither = false;
//		options.inPurgeable = true;
//		options.inInputShareable = true;
		options.inJustDecodeBounds = false;
		Bitmap thumbImgNow = null;
		try{
			 thumbImgNow = BitmapFactory.decodeFile(vPath, options);
		}catch(OutOfMemoryError e){
			Log.v(tag, "OutOfMemoryError, decodeFile失败   ");
			return null; 
		}
		 
		//Log.v(tag, "thumbImgNow.size="+thumbImgNow.getWidth()+","+thumbImgNow.getHeight()); 
		
		if(null == thumbImgNow){
			Log.v(tag, "decodeFile失败   ");
			return null;
		}
		
		int wid = thumbImgNow.getWidth();
		int hgt = thumbImgNow.getHeight();
		
		int degree = readPictureDegree(vPath);
		if (degree != 0) {
			//Log.v(tag, "degree="+degree);			
			// 把图片旋转为正的方向 
			thumbImgNow = rotateImage(degree, thumbImgNow);
		}
				 
		 wid = thumbImgNow.getWidth();
		 hgt = thumbImgNow.getHeight();
				
		tMinWidth = Math.min(wid, hgt);
		if (tMinWidth > vMinWidth) {
			//如果原图片最小宽度比预期最小高度大才进行缩小
			float ratio = ((float) vMinWidth) / tMinWidth; 
			Matrix matrix = new Matrix();
			matrix.postScale(ratio, ratio);    
			thumbImgNow  = Bitmap.createBitmap(thumbImgNow, 0, 0, wid, hgt, matrix, true);
		}
		 
	    Log.v(tag, "处理后, size, width="+thumbImgNow.getWidth()+",height="+thumbImgNow.getHeight());
	
		return thumbImgNow;
	}

}




