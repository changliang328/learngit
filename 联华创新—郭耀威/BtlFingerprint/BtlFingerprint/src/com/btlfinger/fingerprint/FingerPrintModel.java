package com.btlfinger.fingerprint;

import android.os.Parcel;
import android.os.Parcelable;

/**
 * 指纹信息抽象类
 * 
 * @author blestech
 * @since 2015-11-26
 */
public class FingerPrintModel implements Parcelable {
	/*
	 * 每个指纹应该保存的数据文件个数，也是每次录取指纹的时录取的次数
	 */
	public static final int FP_DATA_ITEM = 5;
	public int fp_id;
	/*
	 * 指纹名称
	 */
	public String fp_name;
	/*
	 * 指纹索引，每个指纹都有五个数据文件，他们对应的fp_data_index都相同
	 */
	public int fp_data_index;
	
	/*
	 *按压指纹新增属性：模板长度
	 */
	public int fp_template_size;
	
	/*
	 * 指纹数据保存文件名
	 */
	public int fp_data_path;
	
	/*
	 * 指纹数据快捷拉起
	 */
	public String fp_lunch_package;

	public FingerPrintModel() {

	}

	@Override
	public int describeContents() {

		return 0;
	}

	@Override
	public void writeToParcel(Parcel dest, int flags) {

		dest.writeInt(fp_id);
		dest.writeString(fp_name);
		dest.writeInt(fp_data_index);
		dest.writeInt(fp_template_size);
		dest.writeInt(fp_data_path);
		dest.writeString(fp_lunch_package);
	}

	public static final Parcelable.Creator<FingerPrintModel> CREATOR = new Parcelable.Creator<FingerPrintModel>() {

		@Override
		public FingerPrintModel createFromParcel(Parcel source) {
			FingerPrintModel model = new FingerPrintModel();
			model.fp_id = source.readInt();
			model.fp_name = source.readString();
			model.fp_data_index = source.readInt();
			model.fp_template_size = source.readInt();
			model.fp_data_path = source.readInt();
			model.fp_lunch_package = source.readString();
			return model;
		}

		@Override
		public FingerPrintModel[] newArray(int size) {
			return new FingerPrintModel[size];
		}
	};

}
