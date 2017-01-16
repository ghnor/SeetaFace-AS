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

import android.content.Context;
import android.graphics.Bitmap;
import android.os.Environment;
import android.text.TextUtils;
import android.util.Log;

import java.io.BufferedOutputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.nio.channels.FileChannel;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

public class FileUtils {

  private String TAG = FileUtils.class.getSimpleName();
  public static String SDPATH = Environment.getExternalStorageDirectory() + "/.photocomb/";
  public static String DBName = "privalbum.db";
  public static String DBPATH = "/data/data/com.cm.photocomb/databases/" + DBName;

  /**
   * 操作成功返回值
   */
  public static final int SUCCESS = 0;

  /**
   * 操作失败返回值
   */
  public static final int FAILED = -1;

  private static final int BUF_SIZE = 32 * 1024; // 32KB

  /**
   * 
   * 方法名: </br>
   * 详述: <返回录音文件路径/br> 开发人员：王太顺</br>
   * 创建时间：2015-4-10</br>
   * 
   * @return
   */
  public static String getFileName() {
    String path = SDPATH;
    File file = new File(path);
    file.mkdirs();
    path += System.currentTimeMillis();
    return path;
  }

  public static byte[] Bitmap2Bytes(Bitmap bm) {
    ByteArrayOutputStream baos = new ByteArrayOutputStream();
    bm.compress(Bitmap.CompressFormat.PNG, 100, baos);
    return baos.toByteArray();
  }

  private String generate(String key) {
    if (TextUtils.isEmpty(key)) {
      return "";
    }
    String cacheKey;
    try {
      final MessageDigest mDigest = MessageDigest.getInstance("MD5");
      mDigest.update(key.getBytes());
      cacheKey = bytesToHexString(mDigest.digest());
    } catch (NoSuchAlgorithmException e) {
      cacheKey = String.valueOf(key.hashCode());
    }
    return cacheKey;
  }

  /**
   * bytes转换成十六进制字符串
   * 
   * @param byte[]
   *          b byte数组
   * @return String 每个Byte值之间空格分隔
   */
  public static String byte2HexStr(byte[] b) {
    String stmp = "";
    StringBuilder sb = new StringBuilder("");
    for (int n = 0; n < b.length; n++) {
      stmp = Integer.toHexString(b[n] & 0xFF);
      sb.append((stmp.length() == 1) ? "0" + stmp : stmp);
      sb.append(" ");
    }
    return sb.toString().toUpperCase().trim();
  }

  /**
   * bytes字符串转换为Byte值
   * 
   * @param String
   *          src Byte字符串，每个Byte之间没有分隔符
   * @return byte[]
   */
  public static byte[] hexStr2Bytes(String src) {
    int m = 0, n = 0;
    int l = src.length() / 2;
    System.out.println(l);
    byte[] ret = new byte[l];
    for (int i = 0; i < l; i++) {
      m = i * 2 + 1;
      n = m + 1;
      ret[i] = Byte.decode("0x" + src.substring(i * 2, m) + src.substring(m, n));
    }
    return ret;
  }

  public static String bytesToHexString(byte[] bytes) {
    StringBuilder sb = new StringBuilder();
    for (int i = 0; i < bytes.length; i++) {
      String hex = Integer.toHexString(0xFF & bytes[i]);
      if (hex.length() == 1) {
        sb.append('0');
      }
      sb.append(hex);
    }
    return sb.toString();
  }

  private static final char HEX_DIGITS[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E',
      'F' };

  public static String toHexString(byte[] b) {
    StringBuilder sb = new StringBuilder(b.length * 2);
    for (int i = 0; i < b.length; i++) {
      sb.append(HEX_DIGITS[(b[i] & 0xf0) >>> 4]);
      sb.append(HEX_DIGITS[b[i] & 0x0f]);
    }
    return sb.toString();
  }

  public static String md5sum(String filename) {
    if (TextUtils.isEmpty(filename)) {
      return null;
    }
    InputStream fis;
    byte[] buffer = new byte[1024];
    int numRead = 0;
    MessageDigest md5;
    try {
      fis = new FileInputStream(filename);
      md5 = MessageDigest.getInstance("MD5");
      while ((numRead = fis.read(buffer)) > 0) {
        md5.update(buffer, 0, numRead);
      }
      fis.close();
      return toHexString(md5.digest());
    } catch (Exception e) {
      return null;
    }
  }

  public static void saveBitmap(Bitmap bm, String picName) {
    Log.e("", "保存图片");
    try {
      File f = new File(picName);
      if (f.exists()) {
        f.delete();
      }
      FileOutputStream out = new FileOutputStream(f);
      if (bm != null) {
        bm.compress(Bitmap.CompressFormat.JPEG, 90, out);
      }
      out.flush();
      out.close();
    } catch (FileNotFoundException e) {
      e.printStackTrace();
    } catch (IOException e) {
      e.printStackTrace();
    }
  }

  public static boolean saveBitmap(Bitmap bitmap, File file) {
    if (!file.getParentFile().exists()) {
      file.getParentFile().mkdirs();
    }
    if (file.exists()) {
      file.delete();
    }
    FileOutputStream out = null;
    try {
      out = new FileOutputStream(file);
      bitmap.compress(Bitmap.CompressFormat.JPEG, 90, out);
      out.flush();
      out.close();
    } catch (FileNotFoundException e) {
      e.printStackTrace();
      return false;
    } catch (IOException e) {
      e.printStackTrace();
      return false;
    }
    return true;
  }

  public static File createSDDir(String dirName) throws IOException {
    File dir = new File(SDPATH + dirName);
    if (Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED)) {
      dir.mkdir();
      System.out.println("createSDDir:" + dir.getAbsolutePath());
      System.out.println("createSDDir:" + dir.mkdir());
    }
    return dir;
  }

  public static boolean isFileExist(String fileName) {
    File file = new File(SDPATH + fileName);
    file.isFile();
    return file.exists();
  }

  public static void delFile(String fileName) {
    File file = new File(SDPATH + fileName);
    if (file.isFile()) {
      file.delete();
    }
    file.exists();
  }

  public static void deleteDir() {
    File dir = new File(SDPATH);
    if (dir == null || !dir.exists() || !dir.isDirectory())
      return;

    for (File file : dir.listFiles()) {
      if (file.isFile())
        file.delete(); //
      else if (file.isDirectory())
        deleteDir(); // 递规的方式删除文件夹
    }
    dir.delete();// 删除目录本身
  }

  /**
   * 
   * 方法名: </br>
   * 详述: 判断文件是否存在</br>
   * 开发人员：王太顺</br>
   * 创建时间：2015年11月13日</br>
   * 
   * @param path
   * @return
   */
  public static boolean fileIsExists(String path) {
    try {
      File f = new File(path);
      if (!f.exists()) {
        return false;
      }
    } catch (Exception e) {
      return false;
    }
    return true;
  }

  /**
   * 删除单个文件
   * 
   * @param sPath
   *          被删除文件的文件名
   * @return 单个文件删除成功返回true，否则返回false
   */
  public static boolean deleteFile(String sPath) {
    File file = new File(sPath);
    // 路径为文件且不为空则进行删除
    if (file.isFile() && file.exists()) {
      file.delete();
      return true;
    }
    return false;
  }

  /**
   * 删除目录（文件夹）以及目录下的文件
   * 
   * @param sPath
   *          被删除目录的文件路径
   * @return 目录删除成功返回true，否则返回false
   */
  public static boolean deleteDirectory(String sPath) {
    if (TextUtils.isEmpty(sPath)) {
      return false;
    }
    boolean flag;
    // 如果sPath不以文件分隔符结尾，自动添加文件分隔符
    if (!sPath.endsWith(File.separator)) {
      sPath = sPath + File.separator;
    }
    File dirFile = new File(sPath);
    // 如果dir对应的文件不存在，或者不是一个目录，则退出
    if (!dirFile.exists() || !dirFile.isDirectory()) {
      return false;
    }
    flag = true;
    // 删除文件夹下的所有文件(包括子目录)
    File[] files = dirFile.listFiles();
    for (int i = 0; i < files.length; i++) {
      // 删除子文件
      if (files[i].isFile()) {
        flag = deleteFile(files[i].getAbsolutePath());
        if (!flag)
          break;
      } // 删除子目录
      else {
        flag = deleteDirectory(files[i].getAbsolutePath());
        if (!flag)
          break;
      }
    }
    if (!flag)
      return false;
    // 删除当前目录
    if (dirFile.delete()) {
      return true;
    } else {
      return false;
    }
  }

  /**
   * 文件转化为字节数组
   * 
   * @EditTime 2007-8-13 上午11:45:28
   */
  public static byte[] getBytesFromFile(File f) {
    if (f == null) {
      return null;
    }
    try {
      FileInputStream stream = new FileInputStream(f);
      ByteArrayOutputStream out = new ByteArrayOutputStream(1000);
      byte[] b = new byte[1000];
      int n;
      while ((n = stream.read(b)) != -1) {
        out.write(b, 0, n);
      }
      stream.close();
      out.close();
      return out.toByteArray();
    } catch (IOException e) {
    }
    return null;
  }

  /**
   * 把字节数组保存为一个文件
   * 
   * @EditTime 2007-8-13 上午11:45:56
   */
  public static File getFileFromBytes(byte[] b, String outputFile) {
    BufferedOutputStream stream = null;
    File file = null;
    try {
      file = new File(outputFile);
      FileOutputStream fstream = new FileOutputStream(file);
      stream = new BufferedOutputStream(fstream);
      stream.write(b);
    } catch (Exception e) {
      e.printStackTrace();
    } finally {
      if (stream != null) {
        try {
          stream.close();
        } catch (IOException e1) {
          e1.printStackTrace();
        }
      }
    }
    return file;
  }

  /**
   * 
   * 方法名: </br>
   * 详述: <备份数据库/br> 开发人员：王太顺</br>
   * 创建时间：2015-10-26</br>
   */
  public static void backupDB(Context mContext) {
    try {
      File backup = new File(SDPATH + DBName);
      File dbFile = mContext.getDatabasePath(DBPATH);
      backup.createNewFile();
      fileCopy(dbFile, backup);
    } catch (Exception e) {
      e.printStackTrace();
    }
  }


  /**
   * 
   * 方法名: </br>
   * 详述: </br>
   * 开发人员：王太顺</br>
   * 创建时间：2015-10-26</br>
   * 
   * @param dbFile
   * @param backup
   * @throws IOException
   */
  public static void fileCopy(File dbFile, File backup) throws IOException {
    FileChannel inChannel = new FileInputStream(dbFile).getChannel();
    FileChannel outChannel = new FileOutputStream(backup).getChannel();
    try {
      inChannel.transferTo(0, inChannel.size(), outChannel);
    } catch (IOException e) {
      e.printStackTrace();
    } finally {
      if (inChannel != null) {
        inChannel.close();
      }
      if (outChannel != null) {
        outChannel.close();
      }
    }
  }

  /**
   * @see {@link #assetToFile(Context context, String assetName, File file)}
   */
  public static int assetToFile(Context context, String assetName, String path) {
    return assetToFile(context, assetName, new File(path));
  }

  /**
   * assets 目录下的文件保存到本地文件
   * 
   * @param context
   * @param assetName
   *          assets下名字，非根目录需包含路径 a/b.xxx
   * @param file
   *          目标文件
   * 
   * @return 成功 {@link #SUCCESS}； 失败 {@link #FAILED}
   */
  public static int assetToFile(Context context, String assetName, File file) {
    InputStream is = null;

    try {
      is = context.getAssets().open(assetName);
      return streamToFile(file, is, false);
    } catch (Exception e) {
    } finally {
      try {
        is.close();
      } catch (Exception e) {
      }
    }

    return FAILED;
  }

  /**
   * 将一缓冲流写入文件
   * 
   * @param path
   *          目标文件路径
   * @param is
   *          输入流
   * @param isAppend
   *          是否追加
   * 
   * @return 成功 {@link #SUCCESS}； 失败 {@link #FAILED}
   */
  public static int streamToFile(String path, InputStream is, boolean isAppend) {
    return streamToFile(new File(path), is, isAppend);
  }

  public static int streamToFile(File file, InputStream is, boolean isAppend) {
    checkParentPath(file);
    FileOutputStream fos = null;
    try {
      fos = new FileOutputStream(file, isAppend);
      byte[] buf = new byte[BUF_SIZE];
      int readSize = 0;

      while ((readSize = is.read(buf)) != -1)
        fos.write(buf, 0, readSize);
      fos.flush();

      return SUCCESS;
    } catch (Exception e) {
    } finally {
      try {
        fos.close();
      } catch (Exception e) {
      }
    }

    return FAILED;
  }

  /**
   * @see {@link #checkParentPath(File)}
   */
  public static void checkParentPath(String path) {
    checkParentPath(new File(path));
  }

  /**
   * 在打开一个文件写数据之前，先检测该文件路径的父目录是否已创建，保证能创建文件
   * 
   * @param file
   */
  public static void checkParentPath(File file) {
    File parent = file.getParentFile();
    if (parent != null && !parent.isDirectory())
      createDir(parent);
  }

  /**
   * 创建文件夹
   * 
   * @param path
   * @return
   */
  public static int createDir(String path) {
    return createDir(new File(path));
  }

  public static int createDir(File file) {
    if (file.exists()) {
      if (file.isDirectory())
        return SUCCESS;
      file.delete(); // 避免他是一个文件存在
    }

    if (file.mkdirs())
      return SUCCESS;

    return FAILED;
  }

}
