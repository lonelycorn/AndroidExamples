#pragma once
#include <android/sensor.h>
#include <android/looper.h>

class InertialSensor {
public:
    InertialSensor();
    ~InertialSensor();

    ASensorEventQueue *GetEventQueue() const {
        return m_event_queue;
    }

    void Update();
private:
    ASensorManager *m_manager;
    const ASensor *m_accel;
    const ASensor *m_gyro;
    const ASensor *m_mag;
    ASensorEventQueue *m_event_queue;
    ALooper *m_looper;

};
