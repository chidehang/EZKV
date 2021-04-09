//
// Created by chidh on 2021/3/23.
//
#include <cstdint>
#include <jni.h>
#include <string>
#include "EZKVPredef.h"
#include "EZKVLog.h"
#include "EZKV.h"

using namespace std;

// 库加载时缓存
static jclass g_cls = nullptr;
static jfieldID g_fileID = nullptr;
static JavaVM *g_currentJVM = nullptr;

static int registerNativeMethods(JNIEnv *env, jclass cls);

extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    g_currentJVM = vm;
    JNIEnv *env;
    if (vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return -1;
    }

    if (g_cls) {
        env->DeleteGlobalRef(g_cls);
    }
    static const char *clsName = "com/cdh/ezkv/EZKV";
    jclass instance = env->FindClass(clsName);
    if (!instance) {
        LOGE("fail to locate class: %s", clsName);
        return -2;
    }
    g_cls = reinterpret_cast<jclass>(env->NewGlobalRef(instance));
    if (!g_cls) {
        LOGE("fail to create global reference for %s", clsName);
        return -3;
    }

    // 动态注册函数
    int ret = registerNativeMethods(env, g_cls);
    if (ret != 0) {
        LOGE("fail to register native methods for class %s, ret = %d", clsName, ret);
        return -4;
    }

    g_fileID = env->GetFieldID(g_cls, "nativeHandle", "J");
    if (!g_fileID) {
        LOGE("fail to locate fileID");
        return -5;
    }

    return JNI_VERSION_1_6;
}

#define EZKV_JNI static

namespace ezkv {

    static string jstring2string(JNIEnv *env, jstring jstr) {
        if (jstr) {
            const char *str = env->GetStringUTFChars(jstr, nullptr);
            if (str) {
                string result(str);
                env->ReleaseStringUTFChars(jstr, str);
                return result;
            }
        }
        return "";
    }

    static jstring string2jstring(JNIEnv *env, const string &str) {
        return env->NewStringUTF(str.c_str());
    }

    EZKV_JNI void nInitialize(JNIEnv *env, jclass cls, jstring rootDir) {
        if (!rootDir) {
            return;
        }
        const char *kstr = env->GetStringUTFChars(rootDir, nullptr);
        if (kstr) {
            EZKV::initializeEZKV(kstr);
            env->ReleaseStringUTFChars(rootDir, kstr);
        }
    }

    EZKV_JNI jlong nGetDefaultEZKV(JNIEnv *env, jclass cls) {
        EZKV *kv = nullptr;
        kv = EZKV::defaultEZKV();
        return (jlong) kv;
    }

    EZKV_JNI jlong nPageSize(JNIEnv *env, jclass cls) {
        return DEFAULT_EZKV_SIZE;
    }

    EZKV_JNI jboolean nEncodeInt(JNIEnv *env, jobject obj, jlong handle, jstring oKey, jint value) {
        EZKV *kv = reinterpret_cast<EZKV *>(handle);
        if (kv && oKey) {
            string key = jstring2string(env, oKey);
            return (jboolean) kv->set((int32_t) value, key);
        }
        return (jboolean) false;
    }

    EZKV_JNI jint nDecodeInt(JNIEnv *env, jobject obj, jlong handle, jstring oKey, jint defaultValue) {
        EZKV *kv = reinterpret_cast<EZKV *>(handle);
        if (kv && oKey) {
            string key = jstring2string(env, oKey);
            return (jint) kv->getInt32(key, defaultValue);
        }
        return defaultValue;
    }

    EZKV_JNI jboolean nEncodeString(JNIEnv *env, jobject obj, jlong handle, jstring oKey, jstring oValue) {
        EZKV *kv = reinterpret_cast<EZKV *>(handle);
        if (kv && oKey) {
            string key = jstring2string(env, oKey);
            if (oValue) {
                string value = jstring2string(env, oValue);
                return (jboolean) kv->set(value, key);
            } else {
                kv->removeValueForKey(key);
                return (jboolean) true;
            }
        }
        return (jboolean) false;
    }

    EZKV_JNI jstring nDecodeString(JNIEnv *env, jobject obj, jlong handle, jstring oKey, jstring defaultValue) {
        EZKV *kv = reinterpret_cast<EZKV *>(handle);
        if (kv && oKey) {
            string key = jstring2string(env, oKey);
            string value;
            bool hasValue = kv->getString(key, value);
            if (hasValue) {
                return string2jstring(env, value);
            }
        }
        return defaultValue;
    }

    EZKV_JNI void nRemoveValueForKey(JNIEnv *env, jobject obj, jlong handle, jstring oKey) {
        EZKV *kv = reinterpret_cast<EZKV *>(handle);
        if (kv && oKey) {
            string key = jstring2string(env, oKey);
            kv->removeValueForKey(key);
        }
    }

    EZKV_JNI jlong nTotalSize(JNIEnv *env, jobject obj, jlong handle) {
        EZKV *kv = reinterpret_cast<EZKV *>(handle);
        if (kv) {
            jlong size = kv->totalSize();
            return size;
        }
        return 0;
    }

    EZKV_JNI jlong nCount(JNIEnv *env, jobject obj, jlong handle) {
        EZKV *kv = reinterpret_cast<EZKV *>(handle);
        if (kv) {
            jlong size = kv->count();
            return size;
        }
        return 0;
    }

} // end namespace ezkv

static JNINativeMethod g_methods[] = {
        {"nInitialize", "(Ljava/lang/String;)V", (void *) ezkv::nInitialize},
        {"nGetDefaultEZKV", "()J", (void *) ezkv::nGetDefaultEZKV},
        {"nPageSize", "()J", (void *) ezkv::nPageSize},
        {"nEncodeInt", "(JLjava/lang/String;I)Z", (void *) ezkv::nEncodeInt},
        {"nDecodeInt", "(JLjava/lang/String;I)I", (void *) ezkv::nDecodeInt},
        {"nEncodeString", "(JLjava/lang/String;Ljava/lang/String;)Z", (void *) ezkv::nEncodeString},
        {"nDecodeString", "(JLjava/lang/String;Ljava/lang/String;)Ljava/lang/String;", (void *) ezkv::nDecodeString},
        {"nRemoveValueForKey", "(JLjava/lang/String;)V", (void *) ezkv::nRemoveValueForKey},
        {"nTotalSize", "(J)J", (void *) ezkv::nTotalSize},
        {"nCount", "(J)J", (void *) ezkv::nCount}
};

static int registerNativeMethods(JNIEnv *env, jclass cls) {
    return env->RegisterNatives(cls, g_methods, sizeof(g_methods) / sizeof(g_methods[0]));
}

