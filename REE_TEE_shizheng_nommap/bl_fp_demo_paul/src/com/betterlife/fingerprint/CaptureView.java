package com.betterlife.fingerprint;

import java.io.File;
import java.io.FileOutputStream;

import com.betterlife.bl_fp_demo_paul.MainActivity;
import com.betterlife.bl_fp_demo_paul.R;
import com.betterlife.fingerprint.FpNative;

import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.Bitmap.CompressFormat;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.graphics.Bitmap.Config;
import android.graphics.Point;
import android.graphics.Rect;
import android.net.Uri;
import android.os.Environment;
import android.util.AttributeSet;
import android.util.Log;
//import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.os.*;

public class CaptureView extends SurfaceView implements SurfaceHolder.Callback {
	private SurfaceHolder surHolder;
	private MyThread mThread = null;
	private Paint mScorePaint = new Paint(); // 分数的画图句柄
	private Paint mTipsPaint = new Paint(); // 提示的画图句柄n
	private Paint mTitlePaint = new Paint(); // 标题的画图句柄n
	private Paint TitleRectPaint = new Paint(); // 标题区域的画图句柄
	private Paint mPixcelPaint = new Paint(); // 提示的画图句柄n
	private static Bitmap result;
	private static Bitmap drawScore;
	private static Bitmap obmp;
	private int W = 112; // 位图宽
	private int H = 96; // 位图高
	private Bitmap newBitmap;

	private int tipsHeight = 36; // 文字高
	private int tipsWidth = tipsHeight * 4; // 文字宽

	private int titleHeight = 42; // 标题文字高
	private int titleWidth = titleHeight * 4; // 标题文字宽

	private Rect titleRect; // 标题区域

	private FpNative mFpNative = null;
	private int nScroePassCount = 0;
	public static boolean bStop = false;

	public static boolean bExit = false;

	private byte srcBytes[] = null;
	private int histogramBytes[] = null;
	private int index_of_max_count = 0;
	private int countArrays[] = null;

	private int action_params[] = null;
	private int action_result[] = null;
	private Canvas mCanvas = null;
	private float mScale = 2f;

	private Point userTipPoint = new Point();
	private Point scorePoint = new Point();
	private Point imagePoint = new Point();
	private Point titlePoint = new Point();

	private Point firstLineImagePoint = new Point();

	private Point secondLineImagePoint = new Point();

	private byte CurrentPicNo = 1;

	private byte[] savedBitmapIndex = new byte[6];

	private byte[][] savedBitmapData = null;

	private final int prams_size = 30;

	public int agc = 0xf9;

	private final int HISTOGRAM_HEIGHT = 300;

	public CaptureView(Context context) {
		this(context, null);
	}

	public CaptureView(Context context, AttributeSet attrs, int defStyle) {
		super(context, attrs, defStyle);
	}

	public CaptureView(Context context, AttributeSet attrs, int defStyleAttr,
			int defStyleRes) {
		super(context, attrs, defStyleAttr);
	}

	public CaptureView(Context context, AttributeSet attrs) {
		super(context, attrs);

		Log.d("CaptureView", "CaptureView");
		FpNative.FpInit(FingerprintData.getInstance());
		W = FingerprintData.getInstance().width; // 位图宽
		H = FingerprintData.getInstance().height; // 位图高
		Log.d("sjx", "11width:" + W + "height:" + H);
		savedBitmapData = new byte[6][W * H * 3 + 1078];
		 
		this.setKeepScreenOn(true);
		this.setFocusable(true);

		surHolder = this.getHolder();
		surHolder.addCallback(this);

		mScorePaint.setAntiAlias(true);
		mScorePaint.setTextSize(26);
		mScorePaint.setColor(Color.RED);

		mTipsPaint.setAntiAlias(true);
		mTipsPaint.setTextSize(tipsHeight);
		mTipsPaint.setColor(Color.BLUE);

		mTitlePaint.setAntiAlias(true);
		mTitlePaint.setTextSize(titleHeight);
		mTitlePaint.setColor(Color.BLACK);

		TitleRectPaint.setColor(Color.GRAY);

		// mPixcelPaint.setAntiAlias(true);
		// mPixcelPaint.setTextSize(26);
		// mPixcelPaint.setStrokeWidth((float) 20.0);
		mPixcelPaint.setColor(Color.BLACK);

		srcBytes = new byte[W * H * 3 + 1078];
		histogramBytes = new int[256];
		countArrays = new int[256];
		action_result = new int[prams_size];
		action_params = new int[prams_size];

		ClearSavedIndex();

		mFpNative = new FpNative();
	}

	@Override
	public void surfaceCreated(SurfaceHolder holder) {
		mScale = 0;// CaptureActivity.sScale;
		// imagePoint.set((getWidth() - ((int) (W * mScale))) / 2,
		// (getHeight() - ((int) (H * mScale))) / 2);

		Log.d("CaptureView", "surfaceCreated");

		userTipPoint.set(32, getHeight() - 32);

		titlePoint.set((getWidth() - titleWidth) / 2, titleHeight + 12);

		titleRect = new Rect(0, 0, getWidth(), titleHeight + 32);

		imagePoint.set((getWidth() - ((int) (W * mScale))) / 2,
				titleRect.bottom + 24);

		scorePoint.set((getWidth() - tipsWidth) / 2,
				(int) ((imagePoint.y + 96 + 4) * mScale));

		firstLineImagePoint.set(10, imagePoint.y
				+ (int) ((imagePoint.y + 96 + 20) * mScale));

		secondLineImagePoint.set(10, imagePoint.y
				+ (int) ((firstLineImagePoint.y + 96 + 20) * mScale));

		drawBackground();

		mThread = new MyThread();

		mThread.start();
	}

	@Override
	public void surfaceChanged(SurfaceHolder holder, int format, int width,
			int height) {
	}

	@Override
	public void surfaceDestroyed(SurfaceHolder holder) {
		mThread = null;
	}

	private int calculate_Histogram(byte srcBytes[], int countArrays[],
			int histogramBytes[]) {
		int indexMax = 0;
		int temval = 0;
		float divfactor = 0;
		int curColor = 0;
		String pixcel_value_hex_str;
		for (int i = 0; i < countArrays.length; i++) {
			countArrays[i] = 0;
		}
		for (int i = 0; i < H; i++)
			for (int j = 0; j < W; j++) {
				temval = srcBytes[(i * W + j)] & (0xFF);
				// Log.d("CaptureView", "temval" + temval+",i"+i+" ,j"+j);
				// (temval&(0xff)||(temval&(0xff)<<8)||((temval&(0xff)<<16)||((temval&(0xff)<<24)
				curColor = ((temval & 0xff) | ((temval & 0xff) << 8)
						| (temval & 0xff) << 16 | 0xff << 24);// |(temval&0xff)<<24);
				// pixcel_value_hex_str = Integer.toHexString(curColor);
				// Log.d("CaptureView", "temval" +
				// temval+",i"+i+" ,j"+j+"curColor"+pixcel_value_hex_str);
				mPixcelPaint.setColor(curColor);
				mCanvas.drawPoint(imagePoint.x + j, imagePoint.y + i,
						mPixcelPaint);
				countArrays[temval]++;
			}
		indexMax = 0;
		for (int i = 1; i <= 255; i++) {
			if (countArrays[indexMax] < countArrays[i]) {
				// Log.d("CaptureView", "indexMax=" + indexMax + " value="
				// + countArrays[indexMax] + ", i" + i + "="
				// + countArrays[i]);
				indexMax = i;
			}
		}
		divfactor = (float) countArrays[indexMax] / 256;
		Log.d("CaptureView", "indexMax=" + indexMax + " value="
				+ countArrays[indexMax] + ",divfactor=" + divfactor);

		for (int i = 0; i <= 255; i++) {
			histogramBytes[i] = (int) (countArrays[i] / divfactor);
		}
		mCanvas.drawLine(imagePoint.x, imagePoint.y+HISTOGRAM_HEIGHT , imagePoint.x
				+ 255, imagePoint.y+HISTOGRAM_HEIGHT, mScorePaint);
		mCanvas.drawLine(imagePoint.x, imagePoint.y+HISTOGRAM_HEIGHT , imagePoint.x
				, imagePoint.y+ HISTOGRAM_HEIGHT - 256, mScorePaint);		//mCanvas.drawRect( new Rect(imagePoint.x, imagePoint.y, getWidth()-imagePoint.x-256, getHeight() -  imagePoint.y-256), TitleRectPaint);
		for (int i = 0; i <= 255; i++) {
			mCanvas.drawLine(imagePoint.x + i, imagePoint.y+HISTOGRAM_HEIGHT , imagePoint.x
					+ i, imagePoint.y + HISTOGRAM_HEIGHT - histogramBytes[i], mScorePaint);
		}
		// imagePoint.x,
		// imagePoint.y+256,imagePoint.x+i,imagePoint.y+256-histogramBytes[i]
		return 0;
	}

	public void drawScore() {
		if (bStop)
			return;
		action_params[0] = MainActivity.sDacp;
		action_result[0] = 0;
		action_result[1] = 0;
		action_result[2] = 0;
		action_result[3] = 0;
		int success = mFpNative.FpGetFingerRawImage(srcBytes, action_params,
				action_result);

		if (bExit)
			return;
		mCanvas = surHolder.lockCanvas();
		mCanvas.drawColor(Color.WHITE);
		mCanvas.drawRect(titleRect, TitleRectPaint);
		if (success == 0) {
			calculate_Histogram(srcBytes, countArrays, histogramBytes);
		} else {
		}
		Paint p = new Paint();
		surHolder.unlockCanvasAndPost(mCanvas);
	}

	// 绘制指纹位图
	private boolean drawBmp(byte bytes[], int x, int y) {
		obmp = BitmapFactory.decodeByteArray(bytes, 0, bytes.length);
		if (obmp == null) {
			mCanvas.drawText("Bmp NULL", x, y, mScorePaint);
			return false;
		} else {
			Bitmap sbmp = scaleBmp(obmp, mScale);
			// Bitmap tbmp = toGreyBmp(sbmp);
			mCanvas.drawBitmap(sbmp, x, y, mScorePaint);
			if (!obmp.isRecycled())
				obmp.recycle();
			// if (!tbmp.isRecycled())
			// tbmp.recycle();
			if (!sbmp.isRecycled())
				sbmp.recycle();
			return true;
		}
	}

	public void drawBackground() {
		mCanvas = surHolder.lockCanvas();
		mCanvas.drawColor(Color.WHITE);
		mCanvas.drawRect(titleRect, TitleRectPaint);
		// mCanvas.drawText(getResources().getString(R.string.ma_capture_tip_init),userTipPoint.x,
		// userTipPoint.y, mTipsPaint);
		// mCanvas.drawText(getResources().getString(R.string.capture_title),
		// titlePoint.x - 50, titlePoint.y, mTitlePaint);

		Paint p = new Paint();

		// p.setColor(Color.BLUE);// 设置绿色
		// p.setStrokeWidth(4.0f);
		// mCanvas.drawLine(0, titlePoint.y + 260, getWidth(), titlePoint.y +
		// 260,p);// 画线
		surHolder.unlockCanvasAndPost(mCanvas);
	}

	/**
	 * 将彩色图转换为灰度图
	 * 
	 * @bmp 位图
	 * @return 返回转换好的位图
	 */
	private Bitmap toGreyBmp(Bitmap bmp) {
		int width = bmp.getWidth(); // 获取位图的宽
		int height = bmp.getHeight(); // 获取位图的高
		int[] pixels = new int[width * height]; // 通过位图的大小创建像素点数组
		bmp.getPixels(pixels, 0, width, 0, 0, width, height);
		int alpha = 0xFF << 24;
		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				int grey = pixels[width * i + j];
				int red = ((grey & 0x00FF0000) >> 16);
				int green = ((grey & 0x0000FF00) >> 8);
				int blue = (grey & 0x000000FF);
				grey = (red + green + blue) / 3;
				grey = alpha | (grey << 16) | (grey << 8) | grey;
				pixels[width * i + j] = grey;
			}
		}
		result = Bitmap.createBitmap(width, height, Config.RGB_565);
		result.setPixels(pixels, 0, width, 0, 0, width, height);
		return result;
	}

	private Bitmap scaleBmp(Bitmap bmp, float scale) {
		Matrix matrix = new Matrix();
		matrix.postScale(scale, scale);
		newBitmap = Bitmap.createBitmap(bmp, 0, 0, bmp.getWidth(),
				bmp.getHeight(), matrix, true);
		Log.d("CaptureView", "scale:" + scale + "width:" + newBitmap.getWidth()
				+ "height:" + newBitmap.getHeight());
		return newBitmap;
	}

	public static void saveImage() {
		if (obmp != null) {
			try {
				File file = new File(Environment.getExternalStorageDirectory(),
						System.currentTimeMillis() + ".jpg");
				FileOutputStream stream = new FileOutputStream(file);
				result.compress(CompressFormat.JPEG, 100, stream);
				stream.close();
				// 模拟一个sd卡被插入的事件
				Intent intent = new Intent();
				intent.setAction(Intent.ACTION_MEDIA_MOUNTED);
				intent.setData(Uri.fromFile(Environment
						.getExternalStorageDirectory()));
				sendBroadcast(intent);
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
	}

	private static void sendBroadcast(Intent intent) {
		Context mBase = null;
		mBase.sendBroadcast(intent);

	}

	class MyThread extends Thread {
		@Override
		public void run() {
			while (!bExit) {
				if (!bStop) {
					drawScore();
				}

				Log.d("CaptureView", "MyThread-run");
			}
			//mFpNative.FpCancelAction(0);
			Log.d("CaptureView", "MyThread-exit");

		}
	}

	public void killThread() {
		mThread.interrupted();
	}

	private Point GetLocaionOfPic(int no) {
		Point tempPoint = new Point();

		if (no < 3) {

			tempPoint.x = firstLineImagePoint.x;
			tempPoint.y = firstLineImagePoint.y;

		} else {
			tempPoint.x = secondLineImagePoint.x;
			tempPoint.y = secondLineImagePoint.y;
		}
		return tempPoint;
	}

	private int GetSavedIndex() {
		int i;
		for (i = 0; i < savedBitmapIndex.length; i++) {
			if (savedBitmapIndex[i] != 0)
				break;
		}
		if (i != savedBitmapIndex.length)
			return i;
		for (i = 1; i < savedBitmapIndex.length; i++)
			savedBitmapIndex[i - 1] = savedBitmapIndex[i];

		return savedBitmapIndex.length - 1;
	}

	private void ClearSavedIndex() {
		for (int i = 0; i < savedBitmapIndex.length; i++)
			savedBitmapIndex[i] = 0;

	}

	@Override
	protected void onDetachedFromWindow() {
		// TODO Auto-generated method stub
		super.onDetachedFromWindow();
		Log.d("CaptureView", "onDetachedFromWindow");
	}
}
