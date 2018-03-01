#include "Renderer.h"
#include "Debug.h"

#include <android/native_window.h>
#include <GLES/gl.h> // graphics
#include <cassert>

#define LOG_TAG "RENDERER"

// XYZ
static const GLint VERTICES[][3] = {
        { -0x10000, -0x10000, -0x10000 },
        {  0x10000, -0x10000, -0x10000 },
        {  0x10000,  0x10000, -0x10000 },
        { -0x10000,  0x10000, -0x10000 },
        { -0x10000, -0x10000,  0x10000 },
        {  0x10000, -0x10000,  0x10000 },
        {  0x10000,  0x10000,  0x10000 },
        { -0x10000,  0x10000,  0x10000 }
};

// RGB-alpha
static const GLint COLORS[][4] = {
        { 0x00000, 0x00000, 0x00000, 0x10000 },
        { 0x10000, 0x00000, 0x00000, 0x10000 },
        { 0x10000, 0x10000, 0x00000, 0x10000 },
        { 0x00000, 0x10000, 0x00000, 0x10000 },
        { 0x00000, 0x00000, 0x10000, 0x10000 },
        { 0x10000, 0x00000, 0x10000, 0x10000 },
        { 0x10000, 0x10000, 0x10000, 0x10000 },
        { 0x00000, 0x10000, 0x10000, 0x10000 }
};

static const GLubyte INDICES[] = {
        0, 4, 5,    0, 5, 1,
        1, 5, 6,    1, 6, 2,
        2, 6, 7,    2, 7, 3,
        3, 7, 4,    3, 4, 0,
        4, 7, 6,    4, 6, 5,
        3, 0, 1,    3, 1, 2
};

Renderer::Renderer():
    m_message(Message::NONE),
    m_thread(),
    m_api_mutex(),
    m_window(nullptr),
    m_display(EGL_NO_DISPLAY),
    m_surface(EGL_NO_SURFACE),
    m_context(EGL_NO_CONTEXT),
    m_angle(0) {
    LOGI("Renderer()");
}

Renderer::~Renderer() {
    LOGI("~Renderer");
    if (m_thread.joinable()) {
        LOGE("background thread still running");
        assert(false);
    }
}

void Renderer::start() {
    std::lock_guard<std::mutex> lock(m_api_mutex);
    LOGI("start()");
    m_thread = std::thread([&](){this->renderLoop();});
}

void Renderer::stop() {
    std::lock_guard<std::mutex> lock(m_api_mutex);
    LOGI("stop()");
    if (!m_thread.joinable()) {
        LOGE("never started");
        return;
    }
    m_message = Message::FORCE_EXIT;
    m_thread.join();
}

void Renderer::setWindow(ANativeWindow *window) {
    std::lock_guard<std::mutex> lock(m_api_mutex);
    LOGI("setWindow()");
    if (m_window) {
        LOGE("window was already set");
        assert(false);
    } else {
        m_message = Message::WINDOW_SET;
        m_window = window;
    }
}

void Renderer::setRotation(float angle) {
    std::lock_guard<std::mutex> lock(m_api_mutex);
    LOGI("setRotation()");
    m_angle.exchange(angle);
}

void Renderer::renderLoop() {
    LOGI("renderLoop() start");

    bool forceExit = false;

    while (!forceExit) {
        //int msg = m_message.exchange(static_cast<int>(Message::NONE));
        int msg = m_message.exchange(Message::NONE);
        assert(msg < Message::COUNT);
        switch (static_cast<Message>(msg)) {
        case Message::WINDOW_SET:
            initialize();
            break;
        case Message::FORCE_EXIT:
            forceExit = true;
            destroy();
            break;
            default:
            // do nothing
            break;
        }

        if (m_display) {
            drawFrame();
            if (!eglSwapBuffers(m_display, m_surface)) {
                LOGE("eglSwapBuffer returned error %d", eglGetError());
            }

        }
    }
    LOGI("renderLoop() stop");
}

void Renderer::drawFrame() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    GLfloat angle = m_angle;
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -3.0f);
    glRotatef(angle, 0.0f, 1.0f, 0.0f);
    glRotatef(angle * 0.1f, 1.0f, 0.0f, 0.0f);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glFrontFace(GL_CW);
    glVertexPointer(3, GL_FIXED, 0, VERTICES);
    glColorPointer(4, GL_FIXED, 0, COLORS);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, INDICES);
}

bool Renderer::initialize() {
    LOGI("initialize()");
    const EGLint attribs[] = {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_NONE
    };
    EGLConfig config;
    EGLint numConfigs;
    EGLint format;
    EGLint width;
    EGLint height;
    GLfloat ratio;

    if ((m_display = eglGetDisplay(EGL_DEFAULT_DISPLAY)) == EGL_NO_DISPLAY) {
        LOGE("eglGetDisplay() returned error %d", eglGetError());
        return false;
    }
    if (!eglInitialize(m_display, 0, 0)) {
        LOGE("eglInitialize() returned error %d", eglGetError());
        return false;
    }

    if (!eglChooseConfig(m_display, attribs, &config, 1, &numConfigs)) {
        LOGE("eglChooseConfig() returned error %d", eglGetError());
        destroy();
        return false;
    }

    if (!eglGetConfigAttrib(m_display, config, EGL_NATIVE_VISUAL_ID, &format)) {
        LOGE("eglGetConfigAttrib() returned error %d", eglGetError());
        destroy();
        return false;
    }

    ANativeWindow_setBuffersGeometry(m_window, 0, 0, format);

    if (!(m_surface = eglCreateWindowSurface(m_display, config, m_window, 0))) {
        LOGE("eglCreateWindowSurface() returned error %d", eglGetError());
        destroy();
        return false;
    }

    if (!(m_context = eglCreateContext(m_display, config, 0, 0))) {
        LOGE("eglCreateContext() returned error %d", eglGetError());
        destroy();
        return false;
    }

    if (!eglMakeCurrent(m_display, m_surface, m_surface, m_context)) {
        LOGE("eglMakeCurrent() returned error %d", eglGetError());
        destroy();
        return false;
    }

    if (!eglQuerySurface(m_display, m_surface, EGL_WIDTH, &width) ||
        !eglQuerySurface(m_display, m_surface, EGL_HEIGHT, &height)) {
        LOGE("eglQuerySurface() returned error %d", eglGetError());
        destroy();
        return false;
    }

    glDisable(GL_DITHER);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
    glClearColor(0, 0, 0, 0);
    glEnable(GL_CULL_FACE);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);

    glViewport(0, 0, width, height);

    ratio = (GLfloat) width / height;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustumf(-ratio, ratio, -1, 1, 1, 10);

    return true;
}

void Renderer::destroy() {
    LOGI("destroy()");

    eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(m_display, m_context);
    eglDestroySurface(m_display, m_surface);
    eglTerminate(m_display);

    m_display = EGL_NO_DISPLAY;
    m_surface = EGL_NO_SURFACE;
    m_context = EGL_NO_CONTEXT;
}