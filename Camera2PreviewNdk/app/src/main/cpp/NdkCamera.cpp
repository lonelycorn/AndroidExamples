#include "NdkCamera.h"
#include "Debug.h"

NdkCamera::CaptureTarget::CaptureTarget():
        native_window(nullptr),
        capture_session_output(nullptr),
        camera_output_target(nullptr),
        capture_request(nullptr) {
}

NdkCamera::CaptureTarget::~CaptureTarget(){
}

bool NdkCamera::CaptureTarget::Register(ANativeWindow *window,
                                        ACameraDevice *camera,
                                        ACaptureSessionOutputContainer *container) {
    // TODO: check retval
    // get a ref to preview_window
    native_window = window;
    ANativeWindow_acquire(native_window);

    // create and set up capture request
    ACameraDevice_createCaptureRequest(camera, TEMPLATE_PREVIEW, &capture_request);
    ACameraOutputTarget_create(native_window, &camera_output_target);
    ACaptureRequest_addTarget(capture_request, camera_output_target);

    // add target to OutputContainer
    ACaptureSessionOutput_create(native_window, &capture_session_output);
    ACaptureSessionOutputContainer_add(container, capture_session_output);

    return true;
}

bool NdkCamera::CaptureTarget::Deregister(ACaptureSessionOutputContainer *container){
    // TODO: check retval
    ACaptureRequest_removeTarget(capture_request, camera_output_target);
    ACaptureRequest_free(capture_request);
    ACameraOutputTarget_free(camera_output_target);

    ACaptureSessionOutputContainer_remove(container, capture_session_output);
    ACaptureSessionOutput_free(capture_session_output);

    // release the ref to preview_window
    ANativeWindow_release(native_window);

    native_window = nullptr;
    capture_session_output = nullptr;
    camera_output_target = nullptr;
    capture_request = nullptr;

    return true;
}

NdkCamera::NdkCamera():
        m_verbose_logging(true),
        m_camera_manager(nullptr),
        m_camera_id(),
        m_camera_device(nullptr),
        m_camera_capture_session(nullptr),
        m_capture_session_output_container(nullptr),
        m_preview(),
        m_capture_session_status(CaptureSessionStatus::CLOSED)
{
    LOGI("Constructor");
    // instantiate a camera manager
    m_camera_manager = ACameraManager_create();
    ASSERT(m_camera_manager, "Failed to create CameraManager");

    // instantiate a capture session output container
    ACaptureSessionOutputContainer_create(&m_capture_session_output_container);
    ASSERT(m_capture_session_output_container, "Failed to create CaptureSessionOutputContainer");
}

NdkCamera::~NdkCamera()
{
    LOGI("Destructor");
    ASSERT(m_camera_manager, "No camera manager available");

    // release cameras
    if (IsOpen())
    {
        Close();
    }

    if (m_preview.native_window != nullptr) {
        LOGE("Forgot to clear preview");
        m_preview.Deregister(m_capture_session_output_container);
    }

    // destroy session output container
    ACaptureSessionOutputContainer_free(m_capture_session_output_container);

    // destroy camera manager
    ACameraManager_delete(m_camera_manager);
}

bool NdkCamera::Open()
{
    LOGI("Open");
    if (IsOpen())
    {
        LOGE("Already opened camera %s", m_camera_id.c_str());
        return false;
    }

    if (!EnumerateCamera())
    {
        LOGE("No available camera");
        return false;
    }

    camera_status_t retval = ACameraManager_openCamera(
            m_camera_manager,
            m_camera_id.c_str(),
            GetCameraDeviceStateCallbacks(),
            &m_camera_device);
    if (retval != ACAMERA_OK)
    {
        LOGE("Failed to open camera %s", m_camera_id.c_str());
        return false;
    }
    LOGI("Opened camera %s", m_camera_id.c_str());

    // TODO: initialize camera controls (exposure, sensitivity, etc)
    return true;
}

bool NdkCamera::Close()
{
    LOGI("Close");
    if (!IsOpen())
    {
        LOGE("No camera opened");
        return false;
    }
    LOGI("Closed camera %s", m_camera_id.c_str());
    ACameraDevice_close(m_camera_device);
    m_camera_device = nullptr;
    m_camera_id.clear();
    return true;
}

bool NdkCamera::IsOpen()
{
    return (m_camera_device != nullptr);
}

bool NdkCamera::StartSession()
{
    LOGI("CreateSession");
    // TODO: check all retval below
    camera_status_t retval;

    retval = ACameraDevice_createCaptureSession(
            m_camera_device,
            m_capture_session_output_container,
            GetCameraCaptureSessionStateCallbacks(),
            &m_camera_capture_session);
    if (retval != ACAMERA_OK) {
        LOGE("Failed to create capture session; error = %d", static_cast<int>(retval));
        return false;
    }
    retval = ACameraCaptureSession_setRepeatingRequest(
            m_camera_capture_session,
            GetCameraCaptureSessionCaptureCallbacks(),
            1, // numRequests
            &m_preview.capture_request,
            nullptr); // captureSequenceId
    if (retval != ACAMERA_OK) {
        LOGE("Failed to set repeating request; error = %d", static_cast<int>(retval));
        return false;
    }

    return true;
}

bool NdkCamera::StopSession()
{
    LOGI("DestroySession");
    if (m_camera_capture_session == nullptr) {
        LOGE("Camera capture session never started");
        return false;
    }

    camera_status_t retval;

    retval = ACameraCaptureSession_stopRepeating(m_camera_capture_session);
    if (retval != ACAMERA_OK) {
        LOGE("Failed to stop repeating");
        return false;
    }

    ACameraCaptureSession_close(m_camera_capture_session);
    m_camera_capture_session = nullptr;
    return true;
}

bool NdkCamera::SetPreviewWindow(ANativeWindow *preview_window)
{
    LOGI("SetPreviewWindow");
    if (m_preview.native_window != nullptr) {
        if (m_preview.native_window != preview_window) {
            LOGE("Preview already started on a different window");
            return false;
        }
        else {
            LOGE("Preview already started on this window");
            return false;
        }
    }

    if (m_camera_device == nullptr) {
        LOGE("Camera not opened");
        return false;
    }

    if (!m_preview.Register(preview_window,
                            m_camera_device,
                            m_capture_session_output_container))
    {
        LOGE("Failed to set preview window");
        return false;
    }

    return true;
}

bool NdkCamera::ClearPreviewWindow()
{
    LOGI("ClearPreviewWindow");
    // TODO: wait for session to stop
    if (m_preview.native_window == nullptr) {
        LOGE("Preview window was never set");
        return false;
    }

    m_preview.Deregister(m_capture_session_output_container);
    return true;
}

bool NdkCamera::EnumerateCamera()
{
    LOGI("EnumerateCamera");
    ASSERT(m_camera_manager != nullptr, "No instance for CameraManager");

    camera_status_t retval;

    // get all connected cameras
    ACameraIdList *camera_id_list = nullptr;
    retval = ACameraManager_getCameraIdList(m_camera_manager, &camera_id_list);
    if (retval != ACAMERA_OK) {
        LOGE("Failed to get camera ID list. retval = %d", static_cast<int>(retval));
        return false;
    }
    if (m_verbose_logging)
    {
        std::string msg;
        for (int i = 0; i < camera_id_list->numCameras; ++i)
        {
            msg = msg + " " + camera_id_list->cameraIds[i];
        }
        LOGI("connected cameras: %s", msg.c_str());
    }

    // walk through the list and pick up the first back-facing camera
    bool found = false;
    for (int i = 0; i < camera_id_list->numCameras; ++i)
    {
        if (found)
        {
            break;
        }

        const char *camera_id = camera_id_list->cameraIds[i]; // NOT a copy

        // get all metadata
        ACameraMetadata *camera_meta_data;
        retval = ACameraManager_getCameraCharacteristics(
                m_camera_manager,
                camera_id,
                &camera_meta_data);
        if (retval != ACAMERA_OK) {
            LOGE("Failed to get metadata for camera %s", camera_id);
            return false;
        }
        // check if it is back-facing
        int32_t tag_count = 0;
        const uint32_t *tags = nullptr;
        ACameraMetadata_getAllTags(camera_meta_data, &tag_count, &tags); // NOT a copy; no need to free
        for (int idx = 0; idx < tag_count; ++idx)
        {
            if (ACAMERA_LENS_FACING == tags[idx])
            {
                ACameraMetadata_const_entry lens_info = {0};
                retval = ACameraMetadata_getConstEntry(
                        camera_meta_data,
                        tags[idx],
                        &lens_info);
                if (retval != ACAMERA_OK) {
                    LOGE("Failed to get lens info for camera %s", camera_id);
                    continue;
                }
                auto facing = static_cast<acamera_metadata_enum_android_lens_facing_t>(lens_info.data.u8[0]);
                if (ACAMERA_LENS_FACING_BACK == facing)
                {
                    // found the one!
                    m_camera_id = std::string(camera_id);
                    found = true;
                }
                break;
            }
        }
        ACameraMetadata_free(camera_meta_data);
    }
    ACameraManager_deleteCameraIdList(camera_id_list);
    return found;
}

/* Callbacks for the camera pipeline */

void NdkCamera::OnDisconnected(ACameraDevice *dev)
{
    LOGI("Camera %s disconnected", m_camera_id.c_str());
    ASSERT(dev == m_camera_device, "Wrong camera disconnected");
    Close();
}

void NdkCamera::OnError(ACameraDevice *dev, int error)
{
    LOGI("Camera %s on error %d", m_camera_id.c_str(), error);
    ASSERT(dev == m_camera_device, "Wrong camera on error");
    Close();
}

void CameraDeviceOnDisconnected(void *context, ACameraDevice *dev)
{
    reinterpret_cast<NdkCamera *>(context)->OnDisconnected(dev);
}

void CameraDeviceOnError(void *context, ACameraDevice *dev, int err)
{
    reinterpret_cast<NdkCamera *>(context)->OnError(dev, err);
}

ACameraDevice_stateCallbacks *NdkCamera::GetCameraDeviceStateCallbacks()
{
    static ACameraDevice_stateCallbacks callbacks = {
            .context = this,
            // surprisingly, there's no onOpened() which is available in the Java Android SDK
            .onDisconnected = ::CameraDeviceOnDisconnected,
            .onError = ::CameraDeviceOnError,
    };
    return &callbacks;
}

void NdkCamera::OnCaptureSessionStatusChange(CaptureSessionStatus s) {
    LOGI("OnCaptureSessionStatusChange");
    if (s >= CaptureSessionStatus::COUNT) {
        LOGE("Invalid status: %d", static_cast<int>(s));
    }
    static const char *status_names[] = {"CLOSED", "READY", "ACTIVE"};
    LOGI("CaptureSessionStatus: %s -> %s", status_names[m_capture_session_status], status_names[s]);
    m_capture_session_status = s;
}
void OnCameraCaptureSessionActive(void *context, ACameraCaptureSession *s) {
    reinterpret_cast<NdkCamera *>(context)->
            OnCaptureSessionStatusChange(NdkCamera::CaptureSessionStatus::ACTIVE);
}

void OnCameraCaptureSessionReady(void *context, ACameraCaptureSession *s) {
    reinterpret_cast<NdkCamera *>(context)->
            OnCaptureSessionStatusChange(NdkCamera::CaptureSessionStatus::READY);
}

void OnCameraCaptureSessionClosed(void *context, ACameraCaptureSession *s) {
    reinterpret_cast<NdkCamera *>(context)->
            OnCaptureSessionStatusChange(NdkCamera::CaptureSessionStatus::CLOSED);
}

ACameraCaptureSession_stateCallbacks *
NdkCamera::GetCameraCaptureSessionStateCallbacks()
{
    static ACameraCaptureSession_stateCallbacks callbacks = {
            .context = this,
            .onActive = ::OnCameraCaptureSessionActive,
            .onReady = ::OnCameraCaptureSessionReady,
            .onClosed = ::OnCameraCaptureSessionClosed,
    };
    return &callbacks;
}

ACameraCaptureSession_captureCallbacks *
NdkCamera::GetCameraCaptureSessionCaptureCallbacks()
{
    return nullptr;
}

