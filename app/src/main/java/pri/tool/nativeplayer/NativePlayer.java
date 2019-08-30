package pri.tool.nativeplayer;

import android.media.MediaPlayer;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

public class NativePlayer implements SurfaceHolder.Callback {

    static {
        System.loadLibrary("native-lib");
    }

    //直播地址或媒体文件路径
    private String dataSource;
    private SurfaceHolder surfaceHolder;

    private OnErrorListener onErrorListener;
    private OnPreparedListener onPreparedListener;

    public void setDataSource(String dataSource) {
        this.dataSource = dataSource;
    }

    /**
     * 播放准备工作
     */
    public void prepare() {
        nativePrepare(dataSource);
    }

    public void start() {
        nativeStart();
    }

    public void setSurfaceView(SurfaceView surfaceView) {
        if (null != surfaceHolder) {
            surfaceHolder.removeCallback(this);
        }
        surfaceHolder = surfaceView.getHolder();
        surfaceHolder.addCallback(this);
    }

    /**
     * 供native反射调用
     * 表示播放器准备好了可以开始播放了
     */
    public void onPrepared() {
        if (onPreparedListener != null) {
            onPreparedListener.onPrepared();
        }
    }

    /**
     * 供native反射调用
     * 表示出错了
     */
    public void onError(int errorCode) {
        if (null != onErrorListener) {
            onErrorListener.onError(errorCode);
        }
    }


    @Override
    public void surfaceCreated(SurfaceHolder surfaceHolder) {
        nativeSetSurface(surfaceHolder.getSurface());
    }

    @Override
    public void surfaceChanged(SurfaceHolder surfaceHolder, int i, int i1, int i2) {

    }

    @Override
    public void surfaceDestroyed(SurfaceHolder surfaceHolder) {

    }

    public void setOnErrorListener(OnErrorListener listener) {
        this.onErrorListener = listener;
    }

    public void setOnPreparedListener(OnPreparedListener listener) {
        this.onPreparedListener = listener;
    }

    interface OnPreparedListener {
        void onPrepared();
    }

    public interface OnErrorListener {
        void onError(int errorCode);
    }


    private native void nativePrepare(String dataSource);

    private native void nativeStart();

    private native void nativeSetSurface(Surface surface);
}
