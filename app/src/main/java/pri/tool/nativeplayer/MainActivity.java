package pri.tool.nativeplayer;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.content.ContextCompat;

import android.Manifest;
import android.content.Context;
import android.content.pm.PackageManager;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.SurfaceView;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.SeekBar;
import android.widget.TextView;

import com.tbruyelle.rxpermissions2.Permission;
import com.tbruyelle.rxpermissions2.RxPermissions;

import java.io.File;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.TimeZone;

import io.reactivex.functions.Consumer;

public class MainActivity extends AppCompatActivity implements SeekBar.OnSeekBarChangeListener {

    private static final String TAG = "MainActivity";
    private SurfaceView surfaceView;
    private NativePlayer player;
    private SeekBar seekBar;
    private TextView progressTextView;

    private boolean isTouch ;
    private boolean isSeek ;

    Button timeProviderButton;
    Button timeClientButton;

    TextView ipText;
    EditText ipEdit;

    // Used to load the 'native-lib' library on application startup.


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        checkPermission();

        initView();
        initData();
    }

    @Override
    protected void onResume() {
        super.onResume();
        player.prepare();
    }

    @Override
    protected void onStop() {
        super.onStop();
        player.stop();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        player.release();
    }

    private void initView() {
        surfaceView = findViewById(R.id.surfaceView);
        seekBar = findViewById(R.id.seek_bar);
        progressTextView = findViewById(R.id.textview_time);
    }

    private void initData() {
        player = new NativePlayer();
        player.setSurfaceView(surfaceView);

        player.setDataSource(new File(
                Environment.getExternalStorageDirectory() + File.separator + "Billons.mp4").getAbsolutePath());


        player.setOnPreparedListener(new NativePlayer.OnPreparedListener() {
            @Override
            public void onPrepared() {
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        seekBar.setMax(player.getDuration());
                    }
                });
                player.usePlayClockTime();
                player.start();
            }
        });

        player.setOnErrorListener(new NativePlayer.OnErrorListener() {
            @Override
            public void onError(int errorCode) {

            }
        });

        player.setOnProgressListener(new NativePlayer.OnProgressListener() {
            @Override
            public void onProgress(final int progress) {
                //progress: 当前的播放进度
           //     Log.e(TAG, "progress: " + progress);
                //duration

                //非人为干预进度条，让进度条自然的正常播放
                if (!isTouch){
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            int duration = player.getDuration();
                      //      Log.e(TAG, "duration: " + duration);
                            if (duration != 0) {
                                if(isSeek){
                                    isSeek = false;
                                    return;
                                }
                                seekBar.setProgress(progress);
                                updateTimeWidget(progress * 1000, duration * 1000);
                            }
                        }
                    });
                }
            }
        });

        ipText = this.findViewById(R.id.ipText);
        ipEdit = this.findViewById(R.id.ipEdit);

        timeProviderButton = findViewById(R.id.timeProvider);
        timeProviderButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                String ip = getLocalIpAddress(MainActivity.this);
                if (ip == null) {
                    ip = "192.168.0.102";

                }
                ipText.setText(ip);

                player.startNetTimeProvider(ip, 8090);
            }
        });

        timeClientButton = findViewById(R.id.timeClient);
        timeClientButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                String ip = ipEdit.getText().toString();

                player.startNetTimeClient(ip, 8090);
            }
        });
    }

    private void updateTimeWidget (int progress, int duration) {


        SimpleDateFormat df = new SimpleDateFormat("HH:mm:ss");
        df.setTimeZone(TimeZone.getTimeZone("UTC"));
        final String message = df.format(new Date(progress)) + " / " + df.format(new Date (duration));
        progressTextView.setText(message);
    }


    private void checkPermission() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M
                && ContextCompat.checkSelfPermission(this, Manifest.permission.READ_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED
                && ContextCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
            //requestLocationPermission();
            checkRxPermission();
        } else {
        }
    }

    public void checkRxPermission() {
        RxPermissions rxPermission = new RxPermissions(this);
        rxPermission
                .requestEach(
                        Manifest.permission.READ_EXTERNAL_STORAGE,
                        Manifest.permission.WRITE_EXTERNAL_STORAGE)
                .subscribe(new Consumer<Permission>() {
                    @Override
                    public void accept(Permission permission) throws Exception {
                        if (permission.granted) {
                            Log.d(TAG, " permission accept");
                        } else if (permission.shouldShowRequestPermissionRationale) {
                            // 用户拒绝了该权限，没有选中『不再询问』（Never ask again）,那么下次再次启动时，还会提示请求权限的对话框
                            Log.e(TAG, permission.name + " is denied. More info should be provided.");
                            finish();
                        } else {
                            // 用户拒绝了该权限，并且选中『不再询问』
                            Log.d(TAG, permission.name + " is denied.");
                            finish();
                        }
                    }
                });
    }

    @Override
    public void onProgressChanged(SeekBar seekBar, int i, boolean b) {
      //  Log.e(TAG, "onProgressChange:" + i);
    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {

    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {

    }

    public static String int2ip(int ipInt) {
        StringBuilder sb = new StringBuilder();
        sb.append(ipInt & 0xFF).append(".");
        sb.append((ipInt >> 8) & 0xFF).append(".");
        sb.append((ipInt >> 16) & 0xFF).append(".");
        sb.append((ipInt >> 24) & 0xFF);
        return sb.toString();
    }

    public static String getLocalIpAddress(Context context) {
        try {

            WifiManager wifiManager = (WifiManager) context
                    .getSystemService(Context.WIFI_SERVICE);
            WifiInfo wifiInfo = wifiManager.getConnectionInfo();
            int i = wifiInfo.getIpAddress();
            return int2ip(i);
        } catch (Exception ex) {
            return " 获取IP出错鸟!!!!请保证是WIFI,或者请重新打开网络!\n" + ex.getMessage();
        }
        // return null;
    }
}
