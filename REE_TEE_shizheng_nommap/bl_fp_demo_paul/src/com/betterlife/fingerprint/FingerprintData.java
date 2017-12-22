package com.betterlife.fingerprint;

public class FingerprintData {
	public int width = 112;
	public int height = 96;
	public int capdacp = 100;

	public static FingerprintData gFingerprintData = null;

	public static FingerprintData getInstance() {
		if (null == gFingerprintData) {
			gFingerprintData = new FingerprintData();
		}
		return gFingerprintData;
	}

}
