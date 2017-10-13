package com.btlfinger.fingerprintunlock.ui.custom;

import com.btlfinger.fingerprintunlock.R;

import android.animation.ObjectAnimator;
import android.content.Context;
import android.util.AttributeSet;
import android .util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.animation.DecelerateInterpolator;
import android.widget.FrameLayout;
import android.widget.ImageView;

/**
 * 闪烁着的指纹，用以表示录取进度
 *
 * @author blestech
 * @since 2015-11-26
 */
public class IndexOfFingerProcess extends FrameLayout {

	public static final int fp_imgs[] = new int[] { R.drawable.process_bar_00 ,
			R.drawable.process_bar_00, R.drawable.process_bar_01,
			R.drawable.process_bar_02, R.drawable.process_bar_03,
			R.drawable.process_bar_04, R.drawable.process_bar_05,
			R.drawable.process_bar_06, R.drawable.process_bar_07,
			R.drawable.process_bar_08, R.drawable.process_bar_09,
			R.drawable.process_bar_10};

	public int mCurrentStep = 0;

	private View view;

	public IndexOfFingerProcess(Context context, AttributeSet attrs, int defStyle) {
		super(context, attrs, defStyle);
		init(context);
	}

	public IndexOfFingerProcess(Context context, AttributeSet attrs) {
		super(context, attrs);
		init(context);
	}

	public IndexOfFingerProcess(Context context) {
		super(context);
		init(context);
	}

	public void init(Context context) {
		LayoutInflater inflater = (LayoutInflater) context
				.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
		view = inflater.inflate(R.layout.fingerprocess, this);
		twikcleImg(context,0);
	}

	public void twikcleImg(Context context,int step) {
		ImageView imageViewAbove = (ImageView) view.findViewById(R.id.id_img_above);
		ImageView imageViewBelow = (ImageView) view.findViewById(R.id.id_img_below);
		int imgAboveId;
		int imgBelowId;
		
		if (step < 10) {
			
			imgAboveId = fp_imgs[step+1];
		    imgBelowId = fp_imgs[step];
			imageViewAbove.setImageResource(imgAboveId);
			imageViewBelow.setImageResource(imgBelowId);

			ObjectAnimator oa = ObjectAnimator.ofFloat(imageViewAbove, "alpha",0.0f, 1.0f);
			oa.setInterpolator(new DecelerateInterpolator());
			oa.setDuration(3000);
			oa.setRepeatCount(0);

			oa.start();
		} else {
			imgAboveId = fp_imgs[11];
		    imgBelowId = fp_imgs[10];
			imageViewAbove.setImageResource(imgAboveId);
			imageViewBelow.setImageResource(imgBelowId);
		}
		

	}
}
