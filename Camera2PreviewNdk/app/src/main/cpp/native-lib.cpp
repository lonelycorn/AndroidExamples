#include <jni.h>
#include <android/native_window_jni.h>
#include "NdkCamera.h"

static NdkCamera ndkCamera;

extern "C" JNIEXPORT jboolean JNICALL
Java_com_lonelycorn_camera2previewndk_MainActivity_ndkCameraOpen(
        JNIEnv *env,
        jobject /* this */) {
    return (jboolean) ndkCamera.Open();
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_lonelycorn_camera2previewndk_MainActivity_ndkCameraClose(
        JNIEnv *env,
        jobject /* this */) {
    return (jboolean) ndkCamera.Close();
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_lonelycorn_camera2previewndk_MainActivity_ndkCameraSetPreviewWindow(
        JNIEnv *env,
        jobject /* this */,
        jobject surface) {
    ANativeWindow *window = ANativeWindow_fromSurface(env, surface);
    bool success = ndkCamera.SetPreviewWindow(window);
    ANativeWindow_release(window);
    return (jboolean) success;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_lonelycorn_camera2previewndk_MainActivity_ndkCameraClearPreviewWindow(
        JNIEnv *env,
        jobject /* this */) {
    bool success = ndkCamera.ClearPreviewWindow();
    return (jboolean) success;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_lonelycorn_camera2previewndk_MainActivity_ndkCameraStartSession(
        JNIEnv *env,
        jobject /* this */) {
    return (jboolean) ndkCamera.StartSession();
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_lonelycorn_camera2previewndk_MainActivity_ndkCameraStopSession(
        JNIEnv *env,
        jobject /* this */) {
    return (jboolean) ndkCamera.StopSession();
}
