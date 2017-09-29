package com.btlfinger.fingerprintunlock.ui.support;

import java.util.Timer;
import java.util.TimerTask;

import com.btlfinger.fingerprintunlock.PackagesConstant;
import com.btlfinger.fingerprintunlock.support.PreferenceUtils;
import com.btlfinger.fingerprintunlock.R;
import com.btlfinger.service.aidl.IFpsFingerClient;
import com.btlfinger.service.aidl.IFpsFingerManager;
import com.btlfinger.aidlservice.FingerService;


import android.app.ActionBar;
import android.app.Activity;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.RemoteException;
import android.provider.Settings;
import android.util.Log;
import android.view.MenuItem;
import android.view.View;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.widget.Toast;
/**
 * 锁屏界面
 *
 * @author blestech
 * @since 2015-11-26
 */
public class LockActivity extends Activity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        // TODO Auto-generated method stub
        super.onCreate(savedInstanceState);

        setContentView(R.layout.unlockactivity);
        initService();
    }

    public void initService() {
        Intent intent = new Intent();
        intent.setAction("com.btlfinger.aidlservice");
        bindService(intent, mConnection, Context.BIND_AUTO_CREATE);
    }

    private ServiceConnection mConnection = new ServiceConnection() {

        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            // TODO Auto-generated method stub
            Log.d("LockActivity","ServiceConnection");
            fm = IFpsFingerManager.Stub.asInterface(service);
            try {
                if (fm != null)
                    fm.listen(client);
            } catch (RemoteException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }

            lockUIHandler.sendEmptyMessageDelayed(MATCH_AGAIN, 200);
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            // TODO Auto-generated method stub
            fm = null;
        }
    };

    private IFpsFingerClient client = new IFpsFingerClient.Stub() {

        @Override
        public void getValue(int type, int score) throws RemoteException {

            Log.i("LockActivity", "type = " + type + "---score = " + score);

            if (type == 3) {
                if (score > 0) {
                    lockUIHandler.sendEmptyMessage(MATCH_SUCCESS);
                    lockUIHandler.removeCallbacks(fingerUpRunnable);
                } else if (score != -2047) {
                    Log.i("LockActivity", "---start another match---");
                    Message msg = lockUIHandler.obtainMessage(MATCH_FAIL);
                    msg.arg1 = score;
                    lockUIHandler.sendMessage(msg);
                }
            } else if (type == /*FpsFingerManagerService.ISFINGERUP_WHAT*/7) {
                if (score == -1) {
                    Log.i("LockActivity", "match failed");
                    lockUIHandler.postDelayed(fingerUpRunnable, 10);
                } else {
                    lockUIHandler.sendEmptyMessage(MATCH_AGAIN);
                    Log.i("LockActivity", "match again");
                }
            }
        }
    };

    private IFpsFingerManager fm;

    private static final int MATCH_SUCCESS = 1;
    private static final int MATCH_FAIL = 2;
    private static final int MATCH_AGAIN = 3;

    private static final int MAXFAILLIMIT = 3;
    private int MatchFailNumber= 0;

    private Handler lockUIHandler = new Handler() {
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case MATCH_SUCCESS: {
                MatchFailNumber = 0;
                finish();
            }
            break;

            case MATCH_FAIL:
                MatchFailNumber ++;
                if( MatchFailNumber >= 4) {

                    MatchFailNumber = 0;
                    Intent intent = new Intent();
                    intent.setFlags(0);
                    intent.setClass(LockActivity.this, PasswdActivity.class);
                    startActivity(intent);
                    LockActivity.this.finish();

                    Toast toast = Toast.makeText(LockActivity.this, "尝试超过三次，请输入密码。",Toast.LENGTH_SHORT);
                    toast.show();
                    return;
                }
                try {
                    fm.matchUp(0);
                } catch (RemoteException e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
                }

                break;
            case MATCH_AGAIN:
                try {
                    if (fm != null)
                        fm.matchUp(0);
                } catch (RemoteException e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
                }
                break;

            default:
                break;
            }

        }
    };

    Toast mToast = null;
    public void showToast(boolean flag) {
        String text = null;
        if (flag) {
            text = getString(R.string.fp_tryagain);
        } else {
            text = getString(R.string.pwd_tryagain);
        }

        if (mToast == null) {
            mToast = Toast.makeText(LockActivity.this, text,Toast.LENGTH_SHORT);
        } else {
            mToast.setText(text);
            mToast.setDuration(Toast.LENGTH_SHORT);
        }
        mToast.show();
    }

    private Runnable fingerUpRunnable = new Runnable() {

        @Override
        public void run() {
            try {
                if (fm != null)
                    fm.isFingerUp();
            } catch (RemoteException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
    };


    @Override
    public boolean onOptionsItemSelected(MenuItem menuItem) {
        switch (menuItem.getItemId()) {

        case android.R.id.home:
            finish();
            break;
        default:
            break;
        }

        return true;
    }

    @Override
    protected void onResume() {
        // TODO Auto-generated method stub
        try {
            if (fm != null)
                fm.listen(client);
        } catch (RemoteException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }

        super.onResume();
    }

    @Override
    protected void onPause() {
        // TODO Auto-generated method stub
        try {
            if (fm != null)
                fm.cancle(0);
        } catch (RemoteException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
        try {
            if (fm != null)
                fm.powerDown();
        } catch (RemoteException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
        super.onPause();

    }
}
