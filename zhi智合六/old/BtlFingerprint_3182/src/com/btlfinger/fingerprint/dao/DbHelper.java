package com.btlfinger.fingerprint.dao;

import java.util.ArrayList;
import java.util.List;

import com.btlfinger.fingerprint.dao.FpsTable;
import com.btlfinger.fingerprintunlock.R;
import com.btlfinger.fingerprint.FingerPrintModel;
import com.btlfinger.fingerprint.FingerPrintManager;

import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.ContentProvider;
import android.content.ContentValues;
import android.content.Context;
import android.content.UriMatcher;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.net.Uri;
import android.util.Log;

/**
 * 数据库操作
 *
 * @author blestech
 * @since 2015-11-26
 */
public class DbHelper {
    private String TAG = "DbHelper";
    private Context mContext = null;
    public static SQLiteDatabase dbWrite;
    private ContentResolver mContentResolver = null;
    private static final UriMatcher MATCHER = new UriMatcher(UriMatcher.NO_MATCH);

    /*
     *按 fp_data_index 分组
     */
    public static String GROUPFP = "content://com.btlfinger.fingerprint.provider/" + FpsTable.TABLE_NAME;
    /*
     *不分组，也就是查询所有记录
     */
    public static String NOTGROUPFP = "content://com.btlfinger.fingerprint.provider/nogroup";

    public static Uri NOTGROUPFPURI = Uri.parse(NOTGROUPFP);
    public static Uri FPURI = Uri.parse(GROUPFP);

    public DbHelper(Context context) {
        mContext = context;
        dbWrite = FpProvider.database;
        mContentResolver = mContext.getContentResolver();
    }

    /******************** 插入数据begin ********************/

    public void insertNewFpData(String fp_name, int fp_data_index, int fp_template_size, int fp_data_path) {
        ContentValues values = new ContentValues();
        values.put(FpsTable.COL_FP_NAME, fp_name);
        values.put(FpsTable.COL_FP_DATA_INDEX, fp_data_index);
        values.put(FpsTable.COL_FP_TEMPLATE_SIZE, fp_template_size);
        values.put(FpsTable.COL_FP_DATA_PATH, fp_data_path);
        mContentResolver.insert(FPURI, values);
        Log.e(TAG, "index: "+fp_data_index);
        Log.e(TAG, "insertNewFpData1 success...");
    }

    public void insertNewFpData(FingerPrintModel finger) {
        insertNewFpData(finger.fp_name, finger.fp_data_index, finger.fp_template_size, finger.fp_data_path);
        Log.e(TAG, "index: "+finger.fp_data_index);
        Log.e(TAG, "insertNewFpData2 success...");
    }

    public void insertNewFpData(ArrayList<FingerPrintModel> fingers) {
        dbWrite.beginTransaction();
        try {
            for (int i = 0; i < fingers.size(); i++) {
                insertNewFpData(fingers.get(i));
            }
            dbWrite.setTransactionSuccessful();
        } finally {
            dbWrite.endTransaction();
        }

        Log.e(TAG, "insertNewFpDataList success...");
    }


    /**
     * Group by FpsTable.COL_FP_DATA_INDEX;
     */
    public Cursor queryAllFinger() {
        Log.e(TAG, "queryAllFinger success...");
        Cursor cursor = mContentResolver.query(FPURI, new String[] { FpsTable.COL_ID,FpsTable.COL_FP_NAME, FpsTable.COL_FP_DATA_INDEX,FpsTable.COL_FP_TEMPLATE_SIZE,FpsTable.COL_FP_DATA_PATH }, null, null, null);
        return cursor;
    }

    /*
     *得到指纹的个数
     */
    public int getFingerCount() {
        Cursor cursor = mContentResolver.query(FPURI, new String[] { FpsTable.COL_ID,FpsTable.COL_FP_NAME, FpsTable.COL_FP_DATA_INDEX,FpsTable.COL_FP_TEMPLATE_SIZE,FpsTable.COL_FP_DATA_PATH }, null, null, null);
        int count = cursor.getCount();
        cursor.close();
        return count;
    }

    /********************* 查询end  ********************/

    /******************** 更新begin ********************/

    /**
     * 重命名
     */
    public void updateFpNameById(String fp_new_name, String fp_id_string) {
        ContentValues values = new ContentValues();
        values.put(FpsTable.COL_FP_NAME, fp_new_name);
        mContentResolver.update(FPURI, values, "_id=?", new String[] { fp_id_string });
        Log.e(TAG, "updateFpNameById success...");
    }

    public void updateFpNameByIndex(String fp_new_name, String fp_index_string) {
        ContentValues values = new ContentValues();
        values.put(FpsTable.COL_FP_NAME, fp_new_name);
        mContentResolver.update(FPURI, values, "fp_data_index=?", new String[] { fp_index_string });
        Log.e(TAG, "updateFpNameByIndex success...");
    }
    //设置lunch package
    public void updateFpLunchByIndex(String fp_lunch_package, String fp_index_string) {
        ContentValues values = new ContentValues();
        values.put(FpsTable.COL_FP_LUNCH_PACKAGE, fp_lunch_package);
        mContentResolver.update(FPURI, values, "fp_data_index=?", new String[] { fp_index_string });
        Log.e(TAG, "updateFpLunchByIndex success...");
    }

    /******************** 更新end ********************/

    /******************** 删除begin ********************/
    /**
     * 删除数据库中的指纹
     *
     * @param fp_id
     */
    public void deleteFpById(String fp_id) {
        mContentResolver.delete(FPURI, "_id=?", new String[] { fp_id });
        Log.e(TAG, "deleteFpById success...");
    }

    public void deleteFpByIndex(String fp_index_string) {
        mContentResolver.delete(FPURI, "fp_data_index=?", new String[] { fp_index_string });
        Log.e(TAG, "deleteFpByIndex success...");
    }

    /******************** 删除end ********************/

    /**
     * 获取新指纹名称后缀
     *
     * @return
     */
    public int getNewFpNameIndex () {
        int _id = 1;
        String title = "";
        String title_pre= mContext.getString(R.string.fingerprint);

        Cursor cursor = mContentResolver.query(FPURI, new String[] {FpsTable.COL_FP_DATA_INDEX,FpsTable.COL_FP_NAME }, null, null, FpsTable.COL_FP_DATA_INDEX + " asc");

        List<String> idList = new ArrayList<String>();

        while (cursor.moveToNext()) {
            _id = cursor.getInt(cursor.getColumnIndex(FpsTable.COL_FP_DATA_INDEX));
            title = cursor.getString(cursor.getColumnIndex(FpsTable.COL_FP_NAME));
            Log.e(TAG, "_id : " + _id + " | title : " + title);
            idList.add(title);
        }
        cursor.close();

        Log.e(TAG,"current max id : "+_id);

        for(int i = 1; i <= _id; i++) {
            String title_string = title_pre + i;
            if(!idList.contains(title_string)) {
                Log.e(TAG,"title string: "+title_string);
                return i;
            }
        }
        Log.e(TAG, "all records exist, so get max id : " + idList.size() + 1);
        return idList.size() + 1;
    }

    /*
     *获取当前指纹的最大索引
     */
    public int getCurrentMaxFpIndex() {
        int maxindex = 0;
        Cursor cursor = mContentResolver.query(FPURI, new String[] {FpsTable.COL_FP_DATA_INDEX }, null, null, FpsTable.COL_FP_DATA_INDEX + " desc");

        if (cursor.moveToFirst()) {
            maxindex = cursor.getInt(cursor.getColumnIndex(FpsTable.COL_FP_DATA_INDEX));
        }
        cursor.close();

        Log.e(TAG, "getCurrentMaxFpIndex success..." + maxindex);
        return maxindex;
    }

    /*
     *获取当前指纹的最大ID
     */
    public int getCurrentMaxFpId() {
        int maxid = 0;
        Cursor cursor = mContentResolver.query(FPURI, new String[] {FpsTable.COL_ID}, null, null, FpsTable.COL_FP_DATA_INDEX + " desc");
        if (cursor.moveToFirst()) {
            maxid = cursor.getInt(cursor.getColumnIndex(FpsTable.COL_ID));
        }
        cursor.close();
        Log.e(TAG, "getCurrentMaxFpId success..." + maxid);
        return maxid;
    }


    public List<Integer> getAllFileIndex() {
        List<Integer> indexList = new ArrayList<Integer>();
        Cursor cursor = mContentResolver.query(NOTGROUPFPURI, new String[] {FpsTable.COL_FP_DATA_PATH }, null, null, null);
        while (cursor.moveToNext()) {
            indexList.add(cursor.getInt(cursor.getColumnIndex(FpsTable.COL_FP_DATA_PATH)));
        }
        cursor.close();
        Log.e(TAG, "getAllFileIndex success..." + indexList.size());
        return indexList;
    }

    public List<Integer> getAllTemplateLength() {
        List<Integer> templateSizeList = new ArrayList<Integer>();
        Cursor cursor = mContentResolver.query(NOTGROUPFPURI, new String[] {FpsTable.COL_FP_TEMPLATE_SIZE}, null, null, null);
        while (cursor.moveToNext()) {
            templateSizeList.add(cursor.getInt(cursor.getColumnIndex(FpsTable.COL_FP_TEMPLATE_SIZE)));
        }
        cursor.close();
        Log.e(TAG, "getAllTemplateLength success..." + templateSizeList.size());
        return templateSizeList;
    }

    /**
     * 得到某个指纹的所有数据文件路径
     *
     * @param index
     * @return
     */
    public List<String> getFpFilePathByIndex(String fp_index_string) {
        List<String> filepathList = new ArrayList<String>();
        Cursor cursor = mContentResolver.query(NOTGROUPFPURI, new String[] {FpsTable.COL_FP_DATA_PATH}, "fp_data_index=?", new String[] {fp_index_string}, null);
        while (cursor.moveToNext()) {
            filepathList.add(FingerPrintManager.FP_DATA_DIR + cursor.getString(cursor.getColumnIndex(FpsTable.COL_FP_DATA_PATH)));
        }
        cursor.close();
        Log.e(TAG, "getFpFilePathByIndex success...");
        return filepathList;
    }
}

