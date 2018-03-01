#include <jni.h>
#include <cassert>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include "Renderer.h"

static ANativeWindow *window = nullptr;
static Renderer *renderer = nullptr;

extern "C"
JNIEXPORT void JNICALL
Java_com_lonelycorn_eglrendering_MainActivity_nativeOnStart(
        JNIEnv *env,
        jobject /* this */) {
    assert(!renderer);
    renderer = new Renderer();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_lonelycorn_eglrendering_MainActivity_nativeOnResume(
        JNIEnv *env,
        jobject /* this */) {
    assert(renderer);
    renderer->start();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_lonelycorn_eglrendering_MainActivity_nativeOnPause(
        JNIEnv *env,
        jobject /* this */) {
    assert(renderer);
    renderer->stop();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_lonelycorn_eglrendering_MainActivity_nativeOnStop(
        JNIEnv *env,
        jobject /* this */) {
    assert(renderer);
    delete renderer;
    renderer = nullptr;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_lonelycorn_eglrendering_MainActivity_nativeSetSurface(
        JNIEnv *env,
        jobject /* this */,
        jobject surface) {
    assert(renderer);
    if (surface != nullptr) {
        assert(!window);
        window = ANativeWindow_fromSurface(env, surface);
        renderer->setWindow(window);
    } else {
        assert(window);
        ANativeWindow_release(window);
        window = nullptr;
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_lonelycorn_eglrendering_MainActivity_nativeSetRotation(
        JNIEnv *env,
        jobject /* this */,
        jfloat angle) {
    assert(renderer);
    renderer->setRotation((float) angle);
}