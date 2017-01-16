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


import android.app.Activity;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.Bitmap.Config;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.net.Uri;
import android.os.Bundle;
import android.text.TextUtils;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;

import com.ghnor.seetaface_as.R;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.nio.ByteBuffer;

public class MainActivity extends Activity {
	ImageView imv1,imv2, imvFace1, imvFace2;
	TextView textView1,textView2;
	Button btn1,btn2;
	Context mContext;
	String mFaceModelDir; //人脸正面检测模型
	int mCurIndex = 1; //当前是选中了第几个图，1或2
	Boolean mModelFileExist = false; //人脸检测模型文件存在否，如果不存在，则不能继续下去
	byte[] mFaceByte1, mFaceByte2; 
	//Bitmap mBitmap1, mBitmap2; 
	int mWidth1, mHeight1, mCh1; //照片1的宽度高度、通道
	int mWidth2, mHeight2, mCh2; //照片2的宽度高度、通道
	int mFaceNum1, mFaceNum2; //人脸数
	JniClient jni;
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		jni = new JniClient();
		
		setContentView(R.layout.activity_main);
		btn1 = (Button)findViewById(R.id.button1);
		btn2 = (Button)findViewById(R.id.button2);
		
		mContext = MainActivity.this;
		imv1 = (ImageView)findViewById(R.id.imageView1);
		imv2 = (ImageView)findViewById(R.id.imageView2);
		textView1 = (TextView)findViewById(R.id.textView1);
		
		imvFace1 = (ImageView)findViewById(R.id.ImageView01);
		imvFace2 = (ImageView)findViewById(R.id.ImageView02);
		
		//先显示空白图片
		imv1.setImageBitmap(null);
		imv2.setImageBitmap(null);
		imvFace1.setImageBitmap(null);
		imvFace2.setImageBitmap(null);
		
		mFaceNum1 = 0; 
		mFaceNum2 = 0; 
		//获取正面人脸检测模型文件目录，这里因为有3个文件，都在同一个目录下，就只穿目录，不传文件路径
		//mFaceDetectModelPath = getAssetFilePath("seeta_fd_frontal_v1.0.bin");
		//mFAModelPath = getAssetFilePath("seeta_fa_v1.1.bin");
		//Log.i("loadimg", "mFAModelPath="+mFAModelPath); 
 
		mFaceModelDir = "/sdcard/";
		    
		//检查人脸检测模型文件是否存在
		String tPath = mFaceModelDir + "seeta_fd_frontal_v1.0.bin";
		mModelFileExist = FileUtils.fileIsExists(tPath);
		//Log.i("loadimg", " model mModelFileExist="+mModelFileExist+", tPath="+tPath); 
		if(!mModelFileExist){
			textView1.setText("模型不存在，无法检测人脸:"+tPath);
		}else{ 
			//检查人脸识别模型文件是否存在
			tPath = mFaceModelDir + "seeta_fr_v1.0.bin";
			mModelFileExist = FileUtils.fileIsExists(tPath);
			Log.i("loadimg", " model mModelFileExist="+mModelFileExist+", tPath="+tPath);
			if(!mModelFileExist){ 
				textView1.setText("模型不存在，无法检测人脸:"+tPath);
			}
		}  
		 
		btn1.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				mCurIndex = 1;
				Intent getAlbum = new Intent(Intent.ACTION_GET_CONTENT);
				getAlbum.setType("image/*");
				startActivityForResult(getAlbum, 0);
			}
		});
		
		btn2.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				mCurIndex = 2;
				Intent getAlbum = new Intent(Intent.ACTION_GET_CONTENT);
				getAlbum.setType("image/*");
				startActivityForResult(getAlbum, 0);
			}
		});
	}
	
	public void showCompare(){
		if(mFaceNum1 < 1 || mFaceNum2 < 1){
			textView1.setText("要有2个人脸才能比对相似度");
			return; 
		}
		float tSim = jni.CMCalcFaceSim(1, 2, mFaceModelDir);
		if(tSim > 0.5){
			textView1.setText("相似度:"+tSim+", 应该是一个人");	
		}else{
			textView1.setText("相似度:"+tSim+", 应该不是一个人");
		}				 
	}
	public byte[] getPixelsRGBA(Bitmap image) {
		// calculate how many bytes our image consists of
		int bytes = image.getByteCount();

		ByteBuffer buffer = ByteBuffer.allocate(bytes); // Create a new buffer
		image.copyPixelsToBuffer(buffer); // Move the byte data to the buffer

		byte[] temp = buffer.array(); // Get the underlying array containing the
	 
		return temp;
	}

	byte[] Bitmap2Bytes(Bitmap bm) {
		ByteArrayOutputStream baos = new ByteArrayOutputStream();
		bm.compress(Bitmap.CompressFormat.JPEG, 70, baos);
		return baos.toByteArray();
	}

	public void onClick(View v) {
		Intent getAlbum = new Intent(Intent.ACTION_GET_CONTENT);
		getAlbum.setType("image/*");
		startActivityForResult(getAlbum, 0);
	}

	/**
	 * byte转bitmap 
	 * @param b
	 * @return
	 */
	Bitmap Bytes2Bimap(byte[] b) {
        if (b.length != 0) {  
            return BitmapFactory.decodeByteArray(b, 0, b.length);
        } else {  
            return null;  
        }  
    }  
	 
	/**
	 * 
	 * 方法名: </br>
	 * 详述: </br>
	 * 开发人员：吴祖玉</br>
	 * 创建时间：2015年12月16日</br>
	 * 
	 * @param vFileName:assets下的文件名
	 */
	public String getAssetFilePath(String vFileName) {
		File dir = mContext.getDir("file", Context.MODE_PRIVATE);
		File soFile = new File(dir, vFileName);
		FileUtils.assetToFile(mContext, vFileName, soFile);
		
		try {
			return soFile.getAbsolutePath();
		} catch (Exception e) {
		}
		return null;
	}
	
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
	
		if (resultCode != RESULT_OK) { // 此处的 RESULT_OK 是系统自定义得一个常量
			Log.e("TAG->onresult", "ActivityResult resultCode error");
			return;
		}

		Bitmap bm = null;

		//清空提示信息
		textView1.setText("");
		
		// 外界的程序访问ContentProvider所提供数据 可以通过ContentResolver接口
		ContentResolver resolver = getContentResolver();
		
		// 此处的用于判断接收的Activity是不是你想要的那个
		if (requestCode == 0) {
			// try {
			Uri originalUri = data.getData(); // 获得图片的uri
			//String path0 = originalUri.getPath();
			String imgPath = GetPathFromUri4kitkat.getPath(mContext, originalUri);
			Log.i("loadimg", "path="+imgPath);
		 
			//ProgressDialogUtil.startProgress(GrayProcess.this, "正在识别图片");
			long te = System.currentTimeMillis();
			Bitmap bitmap = XUtils.getScaledBitmap(imgPath, 600);
			//Log.i("loadimg", "getScaledBitmap"); 
			if(null == bitmap){
				Log.i("loadimg", "图片无法加载");
				return;
			} 
			
			if(!mModelFileExist){
				if(1 == mCurIndex){
		 			imv1.setImageBitmap(bitmap);	
		 		}else{
		 			imv2.setImageBitmap(bitmap);
		 		}
				return; 
			}
			
			int width = bitmap.getWidth(); 
			int height = bitmap.getHeight();
	 		
			byte[] bs = getPixelsRGBA(bitmap);
//			Log.i("loadimg", "width="+width+", height="+height+
//					",bs.len="+bs.length); 
//			Log.i("loadimg", "bp1, width="+bp1.getWidth()+", height="+bp1.getHeight()); 
			
			//计算图像通道数ch
			int ch = bs.length / (width*height); 
			if (ch < 1){
				ch = 1; 
			} 
			
			if(1 == mCurIndex){
				mFaceByte1 = getPixelsRGBA(bitmap);
				mWidth1 = width; 
				mHeight1 = height;
				mCh1 = mFaceByte1.length / (width*height);
				//计算图像通道数ch
				if (mCh1 < 1){
					mCh1 = 1; 
				} 					 
			}else{
				mFaceByte2 = getPixelsRGBA(bitmap); 
				mWidth2 = width; 
				mHeight2 = height; 
				mCh2 = mFaceByte2.length / (width*height);
				//计算图像通道数ch
				if (mCh2 < 1){
					mCh2 = 1; 
				}
			}
			
			//剪切的人脸图像
			Bitmap tFaceBmp = Bitmap.createBitmap(256,256, Config.ARGB_8888);
			long t = System.currentTimeMillis();
			
		 	String tRetStr = jni.CMDetectFace(bs, width, height, ch, mFaceModelDir, mCurIndex,tFaceBmp);
		 	t = System.currentTimeMillis() - t;
		 	Log.i("loadimg", "t="+t);
		 	
		 	if (TextUtils.isEmpty(tRetStr) || tRetStr.equals("")) {
		 		if(1 == mCurIndex){
		 			imv1.setImageBitmap(bitmap);
		 			imvFace1.setImageBitmap(null);
		 			mFaceNum1 = 0; 
		 		}else{
		 			imv2.setImageBitmap(bitmap);
		 			imvFace2.setImageBitmap(null);
		 			mFaceNum2 = 0; 
		 		}
		 		
			}else{
				//分解字符串，获取脸的位置
			 	String[] tFaceStrs = tRetStr.split(";");
			 	int face_num = tFaceStrs.length;
			 	
			 	Bitmap bitmap2 = bitmap.copy(Config.ARGB_8888, true);
			 	Canvas canvas = new Canvas(bitmap2);
			 	Paint paint = new Paint();
			 	
			 	int tStokeWid = 1+(width+height)/300; 
				paint.setColor(Color.RED);
			 	paint.setStyle(Paint.Style.STROKE);//不填充
			 	paint.setStrokeWidth(tStokeWid);  //线的宽度
			 	
			 	for(int i=0; i<face_num; i++){
			 		String[] vals = tFaceStrs[i].split(",");
			 		int x = Integer.valueOf(vals[0]);
			 		int y = Integer.valueOf(vals[1]);
			 		int w = Integer.valueOf(vals[2]);
			 		int h = Integer.valueOf(vals[3]);
			 		int left = x ;
				 	int top = y ;
				 	int right = x + w; 
				 	int bottom = y + w; 
				 
				 	canvas.drawRect(left, top, right, bottom, paint);		 		
			 		if (vals.length < 5) {
						continue; 
					}
			 		//画特征点
			 		for(int j=0; j<5; j++){
			 			int ti = 4 + j*2; 
			 			int px = Integer.valueOf(vals[ti]);
				 		int py = Integer.valueOf(vals[ti+1]);
				 		Log.i("loadimg", "j="+j+",px=" + px +", py="+py);
				 		canvas.drawCircle(px, py, tStokeWid, paint);
			 		}
			 	}
			 	
			 	if(1 == mCurIndex){
			 		imv1.setImageBitmap(bitmap2);	
			 		imvFace1.setImageBitmap(tFaceBmp);
			 		mFaceNum1 = face_num;
			 		//textView1.setText("检测出"+face_num+"人");
			 		
			 	}else{
			 		mFaceNum2 = face_num;
			 		imv2.setImageBitmap(bitmap2);
			 		imvFace2.setImageBitmap(tFaceBmp);
			 		//textView2.setText("检测出"+face_num+"人");
			 	}			
			 	
			 	showCompare(); 
			} 
		}  		 
	}


	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		// Handle action bar item clicks here. The action bar will
		// automatically handle clicks on the Home/Up button, so long
		// as you specify a parent activity in AndroidManifest.xml.
		int id = item.getItemId();
//		if (id == R.id.action_settings) {
//			return true;
//		}
		return super.onOptionsItemSelected(item);
	}
}
