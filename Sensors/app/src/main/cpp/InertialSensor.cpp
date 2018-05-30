#include "InertialSensor.h"
#include <cassert>
#include <android/log.h>

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "InertialSensor", __VA_ARGS__)

#define IDENTIFIER 42

static void printSensorInfo(const ASensor *s) {
    LOGD("\n===== SENSOR INFO =====");
    LOGD("name: %s", ASensor_getName(s));
    LOGD("type: %s", ASensor_getStringType(s));
    LOGD("vendor: %s", ASensor_getVendor(s));
    LOGD("resolution: %.7g", ASensor_getResolution(s));
    LOGD("min delay: %d us", ASensor_getMinDelay(s));
    LOGD("FIFO max event count: %d", ASensor_getFifoMaxEventCount(s));
    LOGD("reporting mode: %d", ASensor_getReportingMode(s));
}

static int64_t lastAccelTimestamp = 0;
static int64_t lastGyroTimestamp = 0;
static int64_t lastMagTimestamp = 0;
/* Per the implementation of ASensorManager_createEventQueue(), fd is the file descriptor of the
 * queue, and is useless here.
 *
 *  // Code snippet copied from Android Gingerbread
 *  ASensorEventQueue* ASensorManager_createEventQueue(ASensorManager* manager,
 *      ALooper* looper, int ident, ALooper_callbackFunc callback, void* data)
 *  {
 *     sp<SensorEventQueue> queue =
 *             static_cast<SensorManager*>(manager)->createEventQueue();
 *     if (queue != 0) {
 *         ALooper_addFd(looper, queue->getFd(), ident, ALOOPER_EVENT_INPUT, callback, data);
 *         queue->looper = looper;
 *         queue->incStrong(manager);
 *     }
 *     return static_cast<ASensorEventQueue*>(queue.get());
 *  }
 */
static int eventCallback(int fd, int events, void *data) {
    //LOGD("callback: fd = %d, events = %d", fd, events);
    ASensorEvent event;
    ASensorEventQueue *queue = reinterpret_cast<InertialSensor *>(data)->GetEventQueue();
    double dt;
    while (ASensorEventQueue_getEvents(queue, &event, 1) > 0) {
        switch (event.type) {
            case ASENSOR_TYPE_ACCELEROMETER:
                dt = (event.timestamp - lastAccelTimestamp) * 1e-6;
                lastAccelTimestamp = event.timestamp;
                LOGD("Accel callback, dt = %6.3f, value = (%+6.3f, %+6.3f, %+6.3f)",
                     dt, event.data[0], event.data[1], event.data[2]);
                break;
            case ASENSOR_TYPE_GYROSCOPE:
                dt = (event.timestamp - lastGyroTimestamp) * 1e-6;
                lastGyroTimestamp = event.timestamp;
                LOGD("Gyro callback, dt = %6.3f, value = (%+6.3f, %+6.3f, %+6.3f)",
                     dt, event.data[0], event.data[1], event.data[2]);
                break;
            case ASENSOR_TYPE_MAGNETIC_FIELD:
                dt = (event.timestamp - lastMagTimestamp) * 1e-6;
                lastMagTimestamp = event.timestamp;
                LOGD("Mag callback, dt = %6.3f, value = (%+6.3f, %+6.3f, %+6.3f)",
                     dt, event.data[0], event.data[1], event.data[2]);
                break;
            default:
                assert(false);
        }
    }

    return 1; // continue receiving things
}

InertialSensor::InertialSensor():
    m_manager(nullptr),
    m_looper(nullptr) {

    m_manager = ASensorManager_getInstance();
    assert(m_manager);

    m_accel = ASensorManager_getDefaultSensor(m_manager, ASENSOR_TYPE_ACCELEROMETER);
    m_gyro = ASensorManager_getDefaultSensor(m_manager, ASENSOR_TYPE_GYROSCOPE);
    m_mag = ASensorManager_getDefaultSensor(m_manager, ASENSOR_TYPE_MAGNETIC_FIELD);
    assert(m_accel && m_gyro && m_mag);
    printSensorInfo(m_accel);
    printSensorInfo(m_gyro);
    printSensorInfo(m_mag);

    m_looper = ALooper_forThread();
    if (!m_looper) {
        m_looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
    }
    assert(m_looper);

    //m_event_queue = ASensorManager_createEventQueue(m_manager, m_looper, IDENTIFIER, nullptr, nullptr);
    m_event_queue = ASensorManager_createEventQueue(
            m_manager, m_looper,
            IDENTIFIER,
            eventCallback,
            reinterpret_cast<void *>(this));
    assert(m_event_queue);

    int retval;
    retval = ASensorEventQueue_enableSensor(m_event_queue, m_accel);
    assert(retval == 0);
    retval = ASensorEventQueue_setEventRate(m_event_queue, m_accel, 67*1000); // 15 Hz
    assert(retval == 0);
    retval = ASensorEventQueue_enableSensor(m_event_queue, m_gyro);
    assert(retval == 0);
    retval = ASensorEventQueue_setEventRate(m_event_queue, m_gyro, 50*1000); // 20 Hz
    assert(retval == 0);
    retval = ASensorEventQueue_enableSensor(m_event_queue, m_mag);
    assert(retval == 0);
    retval = ASensorEventQueue_setEventRate(m_event_queue, m_mag, 100*1000); // 10 Hz
    assert(retval == 0);
    (void) retval;
}

InertialSensor::~InertialSensor() {
    ASensorEventQueue_disableSensor(m_event_queue, m_accel);
    ASensorEventQueue_disableSensor(m_event_queue, m_gyro);
    ASensorEventQueue_disableSensor(m_event_queue, m_mag);
}

void InertialSensor::Update() {
    //ALooper_pollAll(0, nullptr, nullptr, nullptr);
}
