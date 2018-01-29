#ifndef CAMERA2PREVIEWNDK_NDKCAMERA_H
#define CAMERA2PREVIEWNDK_NDKCAMERA_H

#include <cstdint>
#include <string>

#include <camera/NdkCameraManager.h>
#include <camera/NdkCameraError.h>
#include <camera/NdkCameraCaptureSession.h>
#include <camera/NdkCameraDevice.h>
#include <camera/NdkCameraMetadataTags.h>
#include <camera/NdkCaptureRequest.h>
#include <camera/NdkCameraMetadata.h>



class NdkCamera{
public:
    NdkCamera();
    ~NdkCamera();

    bool Open();
    bool Close();
    bool IsOpen();

    bool StartSession();
    bool StopSession();

    bool SetPreviewWindow(ANativeWindow *preview_window);
    bool ClearPreviewWindow();

    void OnError(ACameraDevice *dev, int error);
    void OnDisconnected(ACameraDevice *dev);


    enum CaptureSessionStatus: unsigned int {
        CLOSED,
        READY,
        ACTIVE,
        COUNT,
    };
    void OnCaptureSessionStatusChange(CaptureSessionStatus s);

private:
    ACameraDevice_stateCallbacks *GetCameraDeviceStateCallbacks();
    ACameraCaptureSession_stateCallbacks *GetCameraCaptureSessionStateCallbacks();
    ACameraCaptureSession_captureCallbacks *GetCameraCaptureSessionCaptureCallbacks();

    /** Sets m_camera_id to the first back-facing available camera.
     */
    bool EnumerateCamera();

    bool m_verbose_logging;

    // handle to enumerate cameras and create CameraDevice
    ACameraManager *m_camera_manager;

    // ID of the opened camera. Empty if no camera is opened.
    // using std::string to avoid memory management nightmare
    std::string m_camera_id;

    // The opened camera
    ACameraDevice *m_camera_device;

    // A "Context" in which CameraRequest will be submitted
    ACameraCaptureSession *m_camera_capture_session;
    ACaptureSessionOutputContainer *m_capture_session_output_container;

    struct CaptureTarget {
        ANativeWindow *native_window;
        ACaptureSessionOutput *capture_session_output;
        ACameraOutputTarget *camera_output_target;
        ACaptureRequest *capture_request;

        CaptureTarget();
        ~CaptureTarget();
        bool Register(ANativeWindow *window,
                      ACameraDevice *camera,
                      ACaptureSessionOutputContainer *container);
        bool Deregister(ACaptureSessionOutputContainer *container);
    };

    CaptureTarget m_preview;

    CaptureSessionStatus m_capture_session_status;

};

#endif //CAMERA2PREVIEWNDK_NDKCAMERA_H
