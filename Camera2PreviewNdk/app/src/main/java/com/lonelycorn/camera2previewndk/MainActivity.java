package com.lonelycorn.camera2previewndk;

import android.app.Activity;
import android.graphics.SurfaceTexture;
import android.os.Bundle;
import android.util.Log;
import android.view.Surface;
import android.view.TextureView;
import android.widget.Toast;

public class MainActivity extends Activity {
    private static final String TAG = "Camera2PreviewNdk";
    private TextureView textureView;

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.i(TAG, "onCreate");
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        textureView = (TextureView) findViewById(R.id.textureView);
        assert(textureView != null);
        final TextureView.SurfaceTextureListener listener = new TextureView.SurfaceTextureListener() {
            @Override
            public void onSurfaceTextureAvailable(SurfaceTexture surfaceTexture, int i, int i1) {
                Log.i(TAG, "onSurfaceTextureAvailable");
                Surface surface = new Surface(surfaceTexture);
                if (!ndkCameraSetPreviewWindow(surface)) {
                    Log.e(TAG, "failed to set preview window");
                    return;
                }
                // TODO: instead of calling StartSession, mark it as ready, and let NdkCamera decide when to start
                if (!ndkCameraStartSession()) {
                    Log.e(TAG, "failed to start session");
                    return;
                }
                final String msg = "Successfully started";
                Toast.makeText(MainActivity.this, msg, Toast.LENGTH_SHORT).show();
            }

            @Override
            public void onSurfaceTextureSizeChanged(SurfaceTexture surfaceTexture, int i, int i1) {
                Log.i(TAG, "onSurfaceTextureSizeChanged");
            }

            @Override
            public boolean onSurfaceTextureDestroyed(SurfaceTexture surfaceTexture) {
                Log.i(TAG, "onSurfaceTextureDestroyed");
                ndkCameraClearPreviewWindow();
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
    protected void onResume() {
        Log.i(TAG, "onResume");
        super.onResume();
        ndkCameraOpen();
    }

    @Override
    protected void onPause() {
        Log.i(TAG, "onPuase");
        ndkCameraStopSession();
        ndkCameraClose();
        super.onPause();
    }

    // return true if an NDK camera was successfully opened
    private native boolean ndkCameraOpen();
    // return true if an NDK camera was successfully closed
    private native boolean ndkCameraClose();
    // return true if successfully acquired preview window
    private native boolean ndkCameraSetPreviewWindow(Surface surface);
    // return true if successfully released preview window
    private native boolean ndkCameraClearPreviewWindow();
    // return true if successfully started capture session
    private native boolean ndkCameraStartSession();
    // return true if successfully stopped capture session
    private native boolean ndkCameraStopSession();

}
