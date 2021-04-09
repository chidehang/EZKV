package com.cdh.ezkv;

import android.content.Context;
import android.util.Log;

/**
 * Created by chidehang on 2021/3/23
 */
public class EZKV {

    private static final String TAG = "EZKV";

    private final long nativeHandle;

    private static String rootDir = null;

    private EZKV(long handle) {
        nativeHandle = handle;
    }

    /**
     * 初始化
     */
    public static String initialize(Context context) {
        String root = context.getFilesDir().getAbsolutePath() + "/ezkv";

        // 加载库
        System.loadLibrary("ezkv");

        nInitialize(root);
        EZKV.rootDir = root;
        return EZKV.rootDir;
    }

    /**
     * 获取默认EZKV实例
     */
    public static EZKV defaultEZKV() {
        if (rootDir == null) {
            throw new IllegalStateException("尚未初始化。");
        }
        long handle = nGetDefaultEZKV();
        if (handle == 0) {
            return null;
        }
        Log.d(TAG, "获取默认EZKV: " + handle);
        return new EZKV(handle);
    }

    public static long pageSize() {
        return nPageSize();
    }

    public boolean encode(String key, int value) {
        return nEncodeInt(nativeHandle, key, value);
    }

    public int decodeInt(String key, int defaultValue) {
        return nDecodeInt(nativeHandle, key, defaultValue);
    }

    public boolean encode(String key, String value) {
        return nEncodeString(nativeHandle, key, value);
    }

    public String decodeString(String key, String defaultValue) {
        return nDecodeString(nativeHandle, key, defaultValue);
    }

    public void removeValueForKey(String key) {
        nRemoveValueForKey(nativeHandle, key);
    }

    public long totalSize() {
        return nTotalSize(nativeHandle);
    }

    public long count() {
        return nCount(nativeHandle);
    }

    // ----------------- native begin -----------------
    private static native void nInitialize(String rootDir);

    private static native long nGetDefaultEZKV();

    private static native long nPageSize();

    private native boolean nEncodeInt(long handle, String key, int value);

    private native int nDecodeInt(long handle, String key, int defaultValue);

    private native boolean nEncodeString(long handle, String key, String value);

    private native String nDecodeString(long handle, String key, String defaultValue);

    private native void nRemoveValueForKey(long handle, String key);

    private native long nTotalSize(long handle);

    private native long nCount(long handle);
    // ----------------- native end -----------------
}
