#ifndef EGLRENDERING_RENDERER_H
#define EGLRENDERING_RENDERER_H

#include <atomic>
#include <thread>
#include <mutex>
#include <EGL/egl.h> // interface between window manager and GL

class Renderer {
public:
    Renderer();
    ~Renderer();

    void start();
    void stop();
    void setWindow(ANativeWindow *window);
    void setRotation(float angle);

private:

    enum Message
    {
        NONE = 0,
        WINDOW_SET,
        FORCE_EXIT,
        COUNT
    };
    std::atomic<int> m_message; // NOTE: maybe a queue is better...
    std::thread m_thread; // background thread that does the actual rendering
    std::mutex m_api_mutex;

    ANativeWindow *m_window;

    // EGL stuff
    EGLDisplay m_display;
    EGLSurface m_surface;
    EGLContext m_context;

    // graphics control
    std::atomic<float> m_angle;


    void renderLoop();
    void drawFrame();

    bool initialize();
    void destroy();

};


#endif //EGLRENDERING_RENDERER_H
