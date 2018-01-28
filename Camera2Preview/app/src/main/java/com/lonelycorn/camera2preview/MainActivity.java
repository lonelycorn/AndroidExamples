package com.lonelycorn.camera2preview;

import android.content.Context;
import android.app.Activity;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraMetadata;
import android.hardware.camera2.TotalCaptureResult;
import android.os.Bundle;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraDevice;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.CaptureRequest;
import android.os.Handler;
import android.os.HandlerThread;
import android.util.Size;
import android.view.TextureView;
import android.widget.Toast;
import android.util.Log;
import android.view.Surface;
import android.graphics.SurfaceTexture;

import java.util.Arrays;


public class MainActivity extends Activity {
    private static final String TAG = "Camera2Preview";
    private static final boolean VERBOSE_DEBUG = true;

    /* Return the global CameraManager */
    private final CameraManager getCameraManager() {
        return (CameraManager) getSystemService(Context.CAMERA_SERVICE);
    }
    /* The opened CameraDevice */
    private CameraDevice cameraDevice = null;

    /* The image size */
    private Size previewImageSize = null;

    /* View used to preview images */
    private TextureView textureView = null;

    private HandlerThread backgroundThread;
    private Handler backgroundHandler;
    private void startBackgroundThread(){
        backgroundThread = new HandlerThread("Camera background");
        backgroundThread.start();
        backgroundHandler = new Handler(backgroundThread.getLooper());
    }
    private void stopBackgroundThread(){
        backgroundThread.quitSafely();
        try {
            backgroundThread.join();
            backgroundThread = null;
            backgroundHandler = null;
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        textureView = (TextureView) findViewById(R.id.textureView);
        assert(textureView != null);
        final TextureView.SurfaceTextureListener listener = new TextureView.SurfaceTextureListener() {
            @Override
            public void onSurfaceTextureAvailable(SurfaceTexture surfaceTexture, int i, int i1) {
                Log.i(TAG, "onSurfaceTextureAvailable");
                setUpPreview();
            }

            @Override
            public void onSurfaceTextureSizeChanged(SurfaceTexture surfaceTexture, int i, int i1) {
                Log.i(TAG, "onSurfaceTextureSizeChanged");
            }

            @Override
            public boolean onSurfaceTextureDestroyed(SurfaceTexture surfaceTexture) {
                Log.i(TAG, "onSurfaceTextureDestroyed");
                return false;
            }

            @Override
            public void onSurfaceTextureUpdated(SurfaceTexture surfaceTexture) {
                // This is called when the SurfaceTexture is updated (e.g. new image captured from camera)
                //Log.i(TAG, "onSurfaceTextureUpdated");
            }
        };
        textureView.setSurfaceTextureListener(listener);
    }

    @Override
    protected void onResume(){
        Log.i(TAG, "onResume");
        super.onResume();
        startBackgroundThread();
        setUpCamera();
    }
    @Override
    protected void onPause(){
        Log.i(TAG, "onPause");
        closeCamera();
        stopBackgroundThread();
        super.onPause();
    }

    private void closeCamera(){
        if (cameraDevice == null) {
            Log.e(TAG, "Camera not opened");
            return;
        }

        cameraDevice.close();
        cameraDevice = null;
    }
    private void setUpCamera(){
        if (cameraDevice != null) {
            Log.e(TAG, "Camera already opened");
            return;
        }

        try {
            // enumerate camera id, and try to open the first one
            final String[] allCameraId = getCameraManager().getCameraIdList();
            if (VERBOSE_DEBUG) {
                String msg = "available camera ID's:";
                for (int i = 0; i < allCameraId.length; ++i) {
                    msg = msg + " " + allCameraId[i];
                }
                Log.d(TAG, msg);
            }
            final String cameraId = allCameraId[0];
            Log.d(TAG, "using camera ID: " + cameraId);

            // get image size
            final Size[] allSize = getCameraManager()
                    .getCameraCharacteristics(cameraId)
                    .get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP)
                    .getOutputSizes(SurfaceTexture.class);
            if (VERBOSE_DEBUG){
                String msg = "available image sizes:";
                for (int i = 0; i < allSize.length; ++i){
                    msg = msg + " " + allSize[i].toString();
                }
                Log.d(TAG, msg);
            }
            previewImageSize = allSize[0];
            Log.d(TAG, "using image size: " + previewImageSize.toString());

            // set up callbacks for CameraDevice connection / disconnection / error handling
            final CameraDevice.StateCallback stateCallback = new CameraDevice.StateCallback() {
                @Override
                public void onOpened(CameraDevice camera) {
                    Log.i(TAG, "CameraDevice.StateCallback: onOpened");
                    assert(cameraDevice == null);
                    cameraDevice = camera;
                }
                @Override
                public void onDisconnected(CameraDevice camera) {
                    Log.i(TAG, "CameraDevice.StateCallback: onDisconnected");
                    closeCamera();
                }
                @Override
                public void onError(CameraDevice camera, int error) {
                    Log.i(TAG, "CameraDevice.StateCallback: onError: " + String.valueOf(error));
                    closeCamera();
                }
            };

            getCameraManager().openCamera(cameraId, stateCallback, null);
        } catch (CameraAccessException e) {
            final String msg = "cannot set up camera: access exception";
            Toast.makeText(MainActivity.this, msg, Toast.LENGTH_SHORT).show();
            Log.e(TAG, msg);
            e.printStackTrace();
        } catch (SecurityException e) {
            final String msg = "cannot set up camera: security exception";
            Toast.makeText(MainActivity.this, msg, Toast.LENGTH_SHORT).show();
            Log.e(TAG, msg);
        }
    }

    private void setUpPreview(){
        try{
            SurfaceTexture surfaceTexture = textureView.getSurfaceTexture();
            assert(surfaceTexture != null);
            surfaceTexture.setDefaultBufferSize(previewImageSize.getWidth(), previewImageSize.getHeight());

            Surface surface = new Surface(surfaceTexture); // create one for that control
            final CaptureRequest.Builder captureRequestBuilder = cameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW);
            captureRequestBuilder.addTarget(surface);

            final CameraCaptureSession.StateCallback stateCallback = new CameraCaptureSession.StateCallback() {
                @Override
                public void onConfigured(CameraCaptureSession cameraCaptureSession) {
                    Log.i(TAG, "CameraCaptureSession.StateCallback: onConfigured");
                    assert(cameraDevice != null);
                    // set auto-focus, auto-exposure, auto-white-balance
                    captureRequestBuilder.set(CaptureRequest.CONTROL_MODE, CameraMetadata.CONTROL_MODE_AUTO);
                    final CameraCaptureSession.CaptureCallback captureCallback = new CameraCaptureSession.CaptureCallback() {
                        @Override
                        public void onCaptureCompleted(CameraCaptureSession session, CaptureRequest request, TotalCaptureResult result) {
                            if (VERBOSE_DEBUG) {
                                Log.d(TAG, "CameraCaptureSession.CaptureCallback: onCaptureCompleted frame " + String.valueOf(result.getFrameNumber()));
                            }
                            super.onCaptureCompleted(session, request, result);
                        }
                        @Override
                        public void onCaptureStarted(CameraCaptureSession session, CaptureRequest request, long timestamp, long frameNumber) {
                            if (VERBOSE_DEBUG) {
                                Log.d(TAG, "CameraCaptureSession.CaptureCallback: onCaptureStarted frame " + String.valueOf(frameNumber) + " @ time " + String.valueOf(timestamp));
                            }
                            super.onCaptureStarted(session, request, timestamp, frameNumber);
                        }
                    };
                    try {
                        // use background thread to handle these request, so that the main activity isn't blocked
                        cameraCaptureSession.setRepeatingRequest(captureRequestBuilder.build(), captureCallback, backgroundHandler);
                    } catch (CameraAccessException e) {
                        final String msg = "cannot update preview: access exception";
                        Toast.makeText(MainActivity.this, msg, Toast.LENGTH_SHORT).show();
                        Log.e(TAG, msg);
                        e.printStackTrace();
                    }
                }

                @Override
                public void onConfigureFailed(CameraCaptureSession cameraCaptureSession) {
                    Log.i(TAG, "CameraCaptureSession.StateCallback: onConfigureFailed");
                }
            };

            cameraDevice.createCaptureSession(Arrays.asList(surface), stateCallback, null);

        } catch (CameraAccessException e) {
            final String msg = "cannot set up preview: access exception";
            Toast.makeText(MainActivity.this, msg, Toast.LENGTH_SHORT).show();
            Log.e(TAG, msg);
            e.printStackTrace();
        }
    }
}
