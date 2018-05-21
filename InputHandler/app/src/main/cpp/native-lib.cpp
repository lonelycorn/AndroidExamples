#include <jni.h>
#include <string>
#include <android/input.h>

// Not really useful since we cannot get the InputQueue associated with the application
class InputHandler {
public:
    InputHandler():
            m_input_queue(nullptr),
            m_looper(nullptr) {
        /*
        m_looper = ALooper_forThread();
        if (m_looper == nullptr) {
            // no loopers for current thread. prepare one
            m_looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
        }
         */
    }
    ~InputHandler() {

    }

    void OnInputEvent() {

    }
private:
    AInputQueue *m_input_queue;
    ALooper *m_looper;
};

extern "C"
JNIEXPORT jlong JNICALL
Java_com_lonelycorn_inputhandler_MainActivity_nativeCreateInputHandler(
        JNIEnv *env,
        jobject /* this */) {
    InputHandler *p = new InputHandler();
    return reinterpret_cast<jlong>(p);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_lonelycorn_inputhandler_MainActivity_nativeDestroyInputHandler(
        JNIEnv *env,
        jobject /* this */,
        jlong inputHandlerPointer) {
    InputHandler *p = reinterpret_cast<InputHandler *>(inputHandlerPointer);
    delete p;
    p = nullptr;
}
