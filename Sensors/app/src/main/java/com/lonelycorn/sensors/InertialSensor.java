package com.lonelycorn.sensors;


public class InertialSensor {
    private long inertialSensorPointer;

    InertialSensor() {
        inertialSensorPointer = 0;
    }

    public void onResume() {
        inertialSensorPointer = nativeCreate();
    }

    public void onPause() {
        nativeDestroy(inertialSensorPointer);
        inertialSensorPointer = 0;
    }

    private static native long nativeCreate();
    private static native void nativeDestroy(long inertialSensorPointer);
}
