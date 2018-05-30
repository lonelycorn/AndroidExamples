#include <jni.h>
#include <string>
#include "InertialSensor.h"

extern "C"
JNIEXPORT jstring JNICALL
Java_com_lonelycorn_sensors_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "run 'adb logcat | grep InertialSensor'";
    return env->NewStringUTF(hello.c_str());
}



extern "C"
JNIEXPORT jlong JNICALL
Java_com_lonelycorn_sensors_InertialSensor_nativeCreate(
        JNIEnv *env,
        jobject /* this */) {
    InertialSensor *s = new InertialSensor();
    return reinterpret_cast<jlong>(s);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_lonelycorn_sensors_InertialSensor_nativeDestroy(
        JNIEnv *env,
        jobject /* this */,
        jlong inertialSensorPointer) {
    InertialSensor *s = reinterpret_cast<InertialSensor *>(inertialSensorPointer);
    delete s;
    s = nullptr;
}


