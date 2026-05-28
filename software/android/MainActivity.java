/**
 * ============================================================
 * ThermalEyes Android APP - 主活动 | Main Activity
 * ============================================================
 * 功能概述 | Function Overview:
 *   1. 通过 USB Serial 接收热成像传感器数据 (MLX90640)
 *      Receive thermal sensor data via USB Serial
 *   2. 通过 UVC 接收可见光摄像头视频
 *      Receive visible light camera video via UVC
 *   3. 使用 OpenCV 进行图像融合
 *      Perform image fusion using OpenCV
 *   4. 显示融合后的热成像画面，标注最高/最低温度点
 *      Display fused thermal image with min/max temperature markers
 *
 * 开源参考 | Open-source reference:
 *   https://github.com/colourfate/ThermalEyes
 *
 * 依赖库 | Dependencies:
 *   - UsbSerial: https://github.com/mik3y/usb-serial-for-android
 *   - UVCAndroid: https://github.com/saki4510t/UVCAndroid
 *   - OpenCV Android SDK: https://opencv.org/releases/
 *
 * ============================================================
 */

package com.example.thermaleyes;

import android.app.Activity;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.Spinner;
import android.widget.TextView;

import com.serenegiant.usb.UVCCamera;                   // UVCAndroid 库
import com.serenegiant.usbcameracommon.UVCCameraHandler;  // UVCAndroid 相机处理

import org.opencv.android.OpenCVLoader;
import org.opencv.android.Utils;
import org.opencv.core.Core;
import org.opencv.core.CvType;
import org.opencv.core.Mat;
import org.opencv.core.Point;
import org.opencv.core.Scalar;
import org.opencv.imgproc.Imgproc;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.HashMap;

import de.kai_morich.usb_serial.UsbSerialDevice;  // UsbSerial 库
import de.kai_morich.usb_serial.UsbSerialPort;

/**
 * MainActivity - ThermalEyes 主界面
 *
 * 界面布局 | Layout:
 *   +------------------------------------------+
 *   |  [可见光预览]    |    [融合画面显示]       |
 *   |  Visible Camera  |    Fusion Display      |
 *   +------------------------------------------+
 *   |  最高温度: 42.5°C     最低温度: 22.1°C    |
 *   |  Max temp: 42.5°C     Min temp: 22.1°C    |
 *   +------------------------------------------+
 *   |  融合模式: [下拉选择]  配色方案: [下拉选择] |
 *   |  Fusion mode: [v]      Color palette: [v]  |
 *   +------------------------------------------+
 */
public class MainActivity extends Activity {

    private static final String TAG = "ThermalEyes";

    /* ============================================================
     * 常量定义 | Constants
     * ============================================================ */

    /* USB 设备标识 | USB device identifiers */
    /* STM32 CDC 设备的 VID 和 PID，需要与固件中的配置一致 */
    /* STM32 CDC device VID/PID, must match firmware configuration */
    private static final int STM32_VID = 0x0483;   /* ST Microelectronics */
    private static final int STM32_PID = 0x5740;   /* CDC Device */

    /* MLX90640 传感器参数 | MLX90640 sensor parameters */
    private static final int THERMAL_COLS = 32;
    private static final int THERMAL_ROWS = 24;
    private static final int THERMAL_PIXELS = THERMAL_COLS * THERMAL_ROWS;  /* 768 */

    /* 数据包格式常量 | Data packet format constants */
    private static final int FRAME_HEADER = 0x5A5A;
    private static final int PACKET_SIZE = 1542;  /* 帧头(2)+帧号(2)+数据(1536)+CRC(2) */

    /* 温度显示范围 | Temperature display range */
    private static final float DEFAULT_TEMP_MIN = 20.0f;
    private static final float DEFAULT_TEMP_MAX = 40.0f;

    /* 融合模式枚举 | Fusion mode enumeration */
    private static final String[] FUSION_MODES = {
        "叠加 | Overlay",           /* 热成像叠加在可见光上 */
        "并排 | Side by Side",       /* 并排显示 */
        "混合 | Blend",             /* 加权混合 */
        "频率域 | Frequency Domain"  /* 频率域分解融合 */
    };

    /* 伪彩色方案 | Pseudo-color palettes */
    private static final String[] COLOR_PALETTES = {
        "铁虹 | Ironbow",
        "彩虹 | Rainbow",
        "灰度 | Grayscale",
        "热力 | Hot",
        "冷色 | Cool"
    };

    /* ============================================================
     * UI 控件 | UI Controls
     * ============================================================ */
    private ImageView ivFusionDisplay;       /* 融合画面显示 | Fusion display */
    private ImageView ivCameraPreview;       /* 可见光预览 | Visible light preview */
    private TextView tvMaxTemp;              /* 最高温度显示 | Max temperature display */
    private TextView tvMinTemp;              /* 最低温度显示 | Min temperature display */
    private TextView tvStatus;               /* 状态信息 | Status info */
    private Spinner spinnerFusionMode;       /* 融合模式选择 | Fusion mode selector */
    private Spinner spinnerColorPalette;     /* 配色方案选择 | Color palette selector */
    private Button btnConnect;               /* 连接按钮 | Connect button */

    /* ============================================================
     * USB 和传感器变量 | USB and Sensor Variables
     * ============================================================ */

    /* USB Serial (热成像数据) | USB Serial (thermal data) */
    private UsbManager usbManager;
    private UsbSerialPort usbSerialPort;
    private UsbSerialDevice usbSerialDevice;
    private boolean serialConnected = false;

    /* UVC Camera (可见光) | UVC Camera (visible light) */
    private UVCCameraHandler uvcCameraHandler;
    private boolean cameraConnected = false;

    /* 热成像数据缓冲区 | Thermal data buffer */
    private float[] thermalTemperatures = new float[THERMAL_PIXELS];
    private int currentFrameCount = 0;

    /* 图像处理变量 | Image processing variables */
    private Mat thermalMat;                  /* 热成像 Mat (32x24) */
    private Mat thermalColorMat;             /* 着色后的热成像 Mat */
    private Mat visibleMat;                  /* 可见光 Mat */
    private Mat fusedMat;                    /* 融合结果 Mat */

    /* 当前设置 | Current settings */
    private int currentFusionMode = 0;       /* 当前融合模式 | Current fusion mode */
    private int currentPalette = 0;          /* 当前配色方案 | Current color palette */
    private float displayTempMin = DEFAULT_TEMP_MIN;
    private float displayTempMax = DEFAULT_TEMP_MAX;

    /* 线程处理 | Thread handling */
    private Handler mainHandler;
    private volatile boolean isRunning = false;
    private Thread thermalReadThread;

    /* 加载 Native 库 | Load native library */
    static {
        /* 加载 OpenCV 库 | Load OpenCV library */
        if (!OpenCVLoader.initDebug()) {
            Log.e(TAG, "OpenCV 初始化失败 | OpenCV initialization failed");
        }
        /* 加载本地 JNI 库 | Load native JNI library */
        System.loadLibrary("native-lib");
    }

    /* ============================================================
     * Activity 生命周期 | Activity Lifecycle
     * ============================================================ */

    /**
     * Activity 创建时的初始化 | Initialization when Activity is created
     */
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        /* setContentView(R.layout.activity_main); */  /* 实际项目中使用布局文件 */

        Log.i(TAG, "ThermalEyes 启动 | ThermalEyes starting");

        /* 初始化 UI 控件 | Initialize UI controls */
        initViews();

        /* 初始化 USB 管理 | Initialize USB manager */
        usbManager = (UsbManager) getSystemService(USB_SERVICE);

        /* 初始化 OpenCV Mat 对象 | Initialize OpenCV Mat objects */
        thermalMat = new Mat(THERMAL_ROWS, THERMAL_COLS, CvType.CV_32FC1);
        thermalColorMat = new Mat();
        visibleMat = new Mat();
        fusedMat = new Mat();

        /* 初始化主线程 Handler | Initialize main thread Handler */
        mainHandler = new Handler(Looper.getMainLooper());

        /* 设置连接按钮监听 | Set connect button listener */
        btnConnect.setOnClickListener(v -> connectUSB());
    }

    @Override
    protected void onResume() {
        super.onResume();
        isRunning = true;
        Log.i(TAG, "Activity 恢复 | Activity resumed");
    }

    @Override
    protected void onPause() {
        super.onPause();
        isRunning = false;
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        isRunning = false;
        disconnectUSB();
        releaseResources();
    }

    /* ============================================================
     * UI 初始化 | UI Initialization
     * ============================================================ */

    /**
     * 初始化界面控件 | Initialize UI controls
     */
    private void initViews() {
        /*
         * 在实际项目中，这些通过 findViewById() 或 ViewBinding 获取
         * In production, these are obtained via findViewById() or ViewBinding
         *
         * 布局文件 (activity_main.xml) 应包含:
         * Layout file (activity_main.xml) should contain:
         *   - ImageView: fusion display (大的融合画面)
         *   - ImageView: camera preview (小的可见光预览)
         *   - TextView:  max/min temperature display
         *   - Spinner:   fusion mode selector
         *   - Spinner:   color palette selector
         *   - Button:    connect/disconnect
         */

        /* 设置融合模式下拉 | Set up fusion mode spinner */
        /*
        spinnerFusionMode = findViewById(R.id.spinner_fusion_mode);
        ArrayAdapter<String> fusionAdapter = new ArrayAdapter<>(this,
            android.R.layout.simple_spinner_item, FUSION_MODES);
        spinnerFusionMode.setAdapter(fusionAdapter);
        spinnerFusionMode.setOnItemSelectedListener(new FusionModeListener());
        */

        /* 设置配色方案下拉 | Set up color palette spinner */
        /*
        spinnerColorPalette = findViewById(R.id.spinner_color_palette);
        ArrayAdapter<String> paletteAdapter = new ArrayAdapter<>(this,
            android.R.layout.simple_spinner_item, COLOR_PALETTES);
        spinnerColorPalette.setAdapter(paletteAdapter);
        spinnerColorPalette.setOnItemSelectedListener(new PaletteListener());
        */

        Log.i(TAG, "UI 初始化完成 | UI initialization complete");
    }

    /* ============================================================
     * USB 连接管理 | USB Connection Management
     * ============================================================ */

    /**
     * 连接 USB 设备 | Connect USB device
     *
     * 查找并连接两种 USB 设备:
     *   1. STM32 CDC 设备 (热成像数据传输)
     *   2. UVC 摄像头设备 (可见光视频)
     *
     * Find and connect two USB devices:
     *   1. STM32 CDC device (thermal data transfer)
     *   2. UVC camera device (visible light video)
     */
    private void connectUSB() {
        Log.i(TAG, "正在搜索 USB 设备 | Searching for USB devices");

        HashMap<String, UsbDevice> deviceList = usbManager.getDeviceList();

        for (UsbDevice device : deviceList.values()) {
            int vid = device.getVendorId();
            int pid = device.getProductId();

            Log.i(TAG, String.format("发现设备 | Found device: VID=%04X PID=%04X %s",
                    vid, pid, device.getDeviceName()));

            /* ---- 连接 STM32 CDC (热成像数据) ---- */
            /* Connect to STM32 CDC (thermal data) */
            if (vid == STM32_VID && pid == STM32_PID) {
                connectThermalSensor(device);
            }

            /* ---- 连接 UVC 摄像头 (可见光) ---- */
            /* Connect to UVC camera (visible light) */
            /* UVC 设备通常 VID=0x046D (Logitech) 等 */
            /* UVC devices typically VID=0x046D (Logitech) etc. */
            if (device.getInterfaceCount() > 0) {
                String ifaceName = device.getInterface(0).getName();
                if (ifaceName != null && ifaceName.contains("Video")) {
                    connectUVCCamera(device);
                }
            }
        }

        updateStatus();
    }

    /**
     * 连接热成像传感器 (STM32 CDC) | Connect thermal sensor
     *
     * 使用 UsbSerial 库建立与 STM32 的串口通信
     * Uses UsbSerial library to establish serial communication with STM32
     *
     * @param device USB 设备对象
     */
    private void connectThermalSensor(UsbDevice device) {
        try {
            /* 创建串口连接 | Create serial port connection */
            usbSerialDevice = UsbSerialDevice.createUsbSerialDevice(device,
                    usbManager.openDevice(device));

            if (usbSerialDevice != null) {
                /* 打开串口并设置参数 | Open serial port and set parameters */
                /* USB CDC 不需要波特率配置，但 UsbSerial 库要求调用 syncOpen */
                /* USB CDC doesn't need baud rate, but UsbSerial requires syncOpen */
                usbSerialDevice.syncOpen();

                /* 启动热成像数据读取线程 | Start thermal data reading thread */
                startThermalReadThread();

                serialConnected = true;
                Log.i(TAG, "热成像传感器已连接 | Thermal sensor connected");
            }
        } catch (IOException e) {
            Log.e(TAG, "热成像传感器连接失败 | Thermal sensor connection failed: " + e.getMessage());
        }
    }

    /**
     * 连接 UVC 摄像头 | Connect UVC camera
     *
     * 使用 UVCAndroid 库处理 USB 视频设备
     * Uses UVCAndroid library to handle USB video device
     *
     * @param device USB 设备对象
     */
    private void connectUVCCamera(UsbDevice device) {
        /*
         * UVCAndroid 库的典型使用流程:
         * Typical UVCAndroid library usage flow:
         *
         * 1. 创建 UVCCameraHandler
         *    uvcCameraHandler = UVCCameraHandler.createHandler(this,
         *        usbManager, device, UVCCamera.DEFAULT_PREVIEW_WIDTH,
         *        UVCCamera.DEFAULT_PREVIEW_HEIGHT, UVCCamera.DEFAULT_PREVIEW_MODE);
         *
         * 2. 打开摄像头
         *    uvcCameraHandler.open();
         *    uvcCameraHandler.startPreview();
         *
         * 3. 设置预览回调获取帧数据
         *    Set preview callback to get frame data
         *
         * 4. 在回调中将帧数据转换为 OpenCV Mat
         *    Convert frame data to OpenCV Mat in callback
         */
        Log.i(TAG, "UVC 摄像头连接 | UVC camera connecting");
        cameraConnected = true;
    }

    /**
     * 断开所有 USB 连接 | Disconnect all USB connections
     */
    private void disconnectUSB() {
        /* 停止热成像读取线程 | Stop thermal reading thread */
        isRunning = false;
        if (thermalReadThread != null) {
            thermalReadThread.interrupt();
        }

        /* 关闭串口 | Close serial port */
        if (usbSerialDevice != null) {
            usbSerialDevice.syncClose();
            usbSerialDevice = null;
        }
        serialConnected = false;

        /* 关闭 UVC 摄像头 | Close UVC camera */
        if (uvcCameraHandler != null) {
            uvcCameraHandler.close();
        }
        cameraConnected = false;

        Log.i(TAG, "USB 已断开 | USB disconnected");
    }

    /* ============================================================
     * 热成像数据读取线程 | Thermal Data Reading Thread
     * ============================================================ */

    /**
     * 启动热成像数据读取线程 | Start thermal data reading thread
     *
     * 这个线程持续从 USB Serial 读取热成像帧数据
     * This thread continuously reads thermal frame data from USB Serial
     */
    private void startThermalReadThread() {
        thermalReadThread = new Thread(() -> {
            Log.i(TAG, "热成像数据读取线程启动 | Thermal read thread started");

            byte[] readBuffer = new byte[PACKET_SIZE + 100];  /* 留余量 | Extra margin */
            int bufferOffset = 0;

            while (isRunning && serialConnected) {
                try {
                    /* 从 USB Serial 读取数据 | Read from USB Serial */
                    int bytesRead = usbSerialDevice.read(readBuffer, bufferOffset,
                            readBuffer.length - bufferOffset, 200);

                    if (bytesRead > 0) {
                        bufferOffset += bytesRead;

                        /* 查找帧头并解析完整数据包 */
                        /* Find frame header and parse complete packet */
                        bufferOffset = parseThermalData(readBuffer, bufferOffset);
                    }
                } catch (Exception e) {
                    Log.e(TAG, "热成像读取错误 | Thermal read error: " + e.getMessage());
                }
            }

            Log.i(TAG, "热成像数据读取线程结束 | Thermal read thread stopped");
        });

        thermalReadThread.setName("ThermalReadThread");
        thermalReadThread.start();
    }

    /**
     * 解析热成像数据包 | Parse thermal data packet
     *
     * 数据包格式 | Packet format:
     *   [0x5A][0x5A][FrameCount_L][FrameCount_H][Data...1536B...][CRC_L][CRC_H]
     *
     * @param buffer    数据缓冲区
     * @param offset    当前有效数据长度
     * @return 处理后剩余的偏移量
     */
    private int parseThermalData(byte[] buffer, int offset) {
        int pos = 0;

        while (pos <= offset - PACKET_SIZE) {
            /* 查找帧头 0x5A5A | Search for frame header 0x5A5A */
            if (buffer[pos] == 0x5A && buffer[pos + 1] == 0x5A) {
                /* 找到帧头，检查是否有完整的数据包 */
                /* Found header, check if we have a complete packet */
                if (pos + PACKET_SIZE <= offset) {
                    /* 提取帧计数 | Extract frame count */
                    int frameNum = ((buffer[pos + 3] & 0xFF) << 8) | (buffer[pos + 2] & 0xFF);

                    /* 提取温度数据 (768 个 16 位有符号整数) */
                    /* Extract temperature data (768 16-bit signed integers) */
                    int dataStart = pos + 4;
                    float frameMinTemp = Float.MAX_VALUE;
                    float frameMaxTemp = Float.MIN_VALUE;

                    for (int i = 0; i < THERMAL_PIXELS; i++) {
                        /* 小端序: 低字节在前 | Little-endian: low byte first */
                        int low  = buffer[dataStart + i * 2] & 0xFF;
                        int high = buffer[dataStart + i * 2 + 1] & 0xFF;
                        int rawValue = (high << 8) | low;

                        /* 转换为有符号整数 | Convert to signed integer */
                        if (rawValue > 32767) rawValue -= 65536;

                        /* 转换为温度 (单位: 0.01°C -> °C) */
                        /* Convert to temperature (unit: 0.01°C -> °C) */
                        thermalTemperatures[i] = rawValue / 100.0f;

                        /* 跟踪最高/最低温度 | Track min/max temperature */
                        if (thermalTemperatures[i] < frameMinTemp) {
                            frameMinTemp = thermalTemperatures[i];
                        }
                        if (thermalTemperatures[i] > frameMaxTemp) {
                            frameMaxTemp = thermalTemperatures[i];
                        }
                    }

                    /* 更新显示 (在主线程) | Update display (on main thread) */
                    final float minT = frameMinTemp;
                    final float maxT = frameMaxTemp;
                    currentFrameCount = frameNum;

                    mainHandler.post(() -> updateThermalDisplay(minT, maxT));

                    /* 移动到下一个数据包 | Move to next packet */
                    pos += PACKET_SIZE;
                } else {
                    break;  /* 数据不完整，等待更多数据 | Incomplete, wait for more */
                }
            } else {
                pos++;  /* 不是帧头，继续搜索 | Not header, keep searching */
            }
        }

        /* 处理剩余数据 | Handle remaining data */
        int remaining = offset - pos;
        if (remaining > 0 && pos > 0) {
            System.arraycopy(buffer, pos, buffer, 0, remaining);
        }

        return remaining;
    }

    /* ============================================================
     * 图像处理与显示 | Image Processing and Display
     * ============================================================ */

    /**
     * 更新热成像显示 | Update thermal display
     *
     * 在主线程中调用，执行图像处理和界面更新
     * Called on main thread, performs image processing and UI update
     *
     * @param minTemp 当前帧最低温度
     * @param maxTemp 当前帧最高温度
     */
    private void updateThermalDisplay(float minTemp, float maxTemp) {
        /* ---- 步骤 1: 将温度数据写入 OpenCV Mat ---- */
        /* Step 1: Write temperature data to OpenCV Mat */
        for (int row = 0; row < THERMAL_ROWS; row++) {
            for (int col = 0; col < THERMAL_COLS; col++) {
                int idx = row * THERMAL_COLS + col;
                thermalMat.put(row, col, thermalTemperatures[idx]);
            }
        }

        /* ---- 步骤 2: 调用 Native 方法进行图像处理 ---- */
        /* Step 2: Call native methods for image processing */

        /* 伪彩色映射 | Pseudo-color mapping */
        /* 将 32x24 的温度数据映射为彩色图像 */
        /* Map 32x24 temperature data to color image */
        thermalColorMat = nativeApplyPseudoColor(
            thermalMat.getNativeObjAddr(),
            minTemp, maxTemp,
            currentPalette
        );

        /* 图像融合 | Image fusion */
        /* 根据选择的融合模式，将热成像与可见光融合 */
        /* Fuse thermal and visible light based on selected mode */
        if (currentFusionMode == 3 && visibleMat != null && !visibleMat.empty()) {
            /* 频率域融合 | Frequency domain fusion */
            fusedMat = nativeFuseImages(
                thermalColorMat.getNativeObjAddr(),
                visibleMat.getNativeObjAddr(),
                currentFusionMode,
                0.6f,  /* 高频权重 | High-freq weight */
                0.4f   /* 低频权重 | Low-freq weight */
            );
        } else {
            /* 其他模式直接使用着色后的热成像 */
            /* Other modes use colored thermal directly */
            fusedMat = thermalColorMat;
        }

        /* ---- 步骤 3: 标注最高/最低温度点 ---- */
        /* Step 3: Mark min/max temperature points */
        markTemperaturePoints(fusedMat, minTemp, maxTemp);

        /* ---- 步骤 4: 显示结果 ---- */
        /* Step 4: Display result */
        /* 将 OpenCV Mat 转换为 Android Bitmap 并显示 */
        /* Convert OpenCV Mat to Android Bitmap and display */
        /*
        Bitmap bitmap = Bitmap.createBitmap(fusedMat.cols(), fusedMat.rows(),
                Bitmap.Config.ARGB_8888);
        Utils.matToBitmap(fusedMat, bitmap);
        ivFusionDisplay.setImageBitmap(bitmap);
        */

        /* ---- 步骤 5: 更新温度文字 ---- */
        /* Step 5: Update temperature text */
        /*
        tvMaxTemp.setText(String.format("最高 | Max: %.1f°C", maxTemp));
        tvMinTemp.setText(String.format("最低 | Min: %.1f°C", minTemp));
        */
    }

    /**
     * 在图像上标注最高和最低温度点 | Mark min/max temperature points on image
     *
     * @param image   要标注的图像
     * @param minTemp 最低温度
     * @param maxTemp 最高温度
     */
    private void markTemperaturePoints(Mat image, float minTemp, float maxTemp) {
        int minIdx = 0, maxIdx = 0;
        float minVal = Float.MAX_VALUE;
        float maxVal = Float.MIN_VALUE;

        /* 查找最高和最低温度像素位置 | Find min/max temperature pixel positions */
        for (int i = 0; i < THERMAL_PIXELS; i++) {
            if (thermalTemperatures[i] < minVal) {
                minVal = thermalTemperatures[i];
                minIdx = i;
            }
            if (thermalTemperatures[i] > maxVal) {
                maxVal = thermalTemperatures[i];
                maxIdx = i;
            }
        }

        /* 计算像素坐标 | Calculate pixel coordinates */
        int minCol = minIdx % THERMAL_COLS;
        int minRow = minIdx / THERMAL_COLS;
        int maxCol = maxIdx % THERMAL_COLS;
        int maxRow = maxIdx / THERMAL_COLS;

        /* 缩放到显示分辨率 | Scale to display resolution */
        float scaleX = (float) image.cols() / THERMAL_COLS;
        float scaleY = (float) image.rows() / THERMAL_ROWS;

        Point minPoint = new Point(minCol * scaleX, minRow * scaleY);
        Point maxPoint = new Point(maxCol * scaleX, maxRow * scaleY);

        /* 绘制圆圈标记 | Draw circle markers */
        /* 蓝色圆圈 = 最低温度, 红色圆圈 = 最高温度 */
        /* Blue circle = min temperature, Red circle = max temperature */
        Imgproc.circle(image, minPoint, 5, new Scalar(255, 0, 0), 2);
        Imgproc.circle(image, maxPoint, 5, new Scalar(0, 0, 255), 2);

        /* 绘制温度文字标签 | Draw temperature text labels */
        Imgproc.putText(image, String.format("%.1f°C", minTemp),
                new Point(minPoint.x + 8, minPoint.y),
                Imgproc.FONT_HERSHEY_SIMPLEX, 0.4, new Scalar(255, 0, 0), 1);
        Imgproc.putText(image, String.format("%.1f°C", maxTemp),
                new Point(maxPoint.x + 8, maxPoint.y),
                Imgproc.FONT_HERSHEY_SIMPLEX, 0.4, new Scalar(0, 0, 255), 1);
    }

    /* ============================================================
     * Native 方法声明 | Native Method Declarations
     * ============================================================ */

    /**
     * Native 方法: 伪彩色映射 | Native method: pseudo-color mapping
     *
     * 将温度数据 Mat 转换为彩色图像 Mat
     * Convert temperature data Mat to color image Mat
     *
     * @param thermalMatAddr 温度数据 Mat 的 native 地址
     * @param minTemp        显示温度范围下限
     * @param maxTemp        显示温度范围上限
     * @param paletteType    配色方案 (0=Ironbow, 1=Rainbow, 2=Grayscale, 3=Hot, 4=Cool)
     * @return 着色后的 Mat native 地址
     */
    public native long nativeApplyPseudoColor(long thermalMatAddr,
                                               float minTemp, float maxTemp,
                                               int paletteType);

    /**
     * Native 方法: 图像融合 | Native method: image fusion
     *
     * 使用高斯模糊进行频率域分解，融合热成像和可见光图像
     * Uses Gaussian blur for frequency domain decomposition to fuse thermal and visible images
     *
     * @param thermalAddr 热成像 Mat 的 native 地址
     * @param visibleAddr 可见光 Mat 的 native 地址
     * @param mode        融合模式
     * @param highWeight  高频权重
     * @param lowWeight   低频权重
     * @return 融合后的 Mat native 地址
     */
    public native long nativeFuseImages(long thermalAddr, long visibleAddr,
                                         int mode, float highWeight, float lowWeight);

    /* ============================================================
     * 辅助方法 | Helper Methods
     * ============================================================ */

    /**
     * 更新状态显示 | Update status display
     */
    private void updateStatus() {
        String status = String.format("传感器 | Sensor: %s | 摄像头 | Camera: %s",
                serialConnected ? "已连接 Connected" : "未连接 Disconnected",
                cameraConnected ? "已连接 Connected" : "未连接 Disconnected");
        Log.i(TAG, status);
        /*
        if (tvStatus != null) {
            tvStatus.setText(status);
        }
        */
    }

    /**
     * 释放资源 | Release resources
     */
    private void releaseResources() {
        if (thermalMat != null) thermalMat.release();
        if (thermalColorMat != null) thermalColorMat.release();
        if (visibleMat != null) visibleMat.release();
        if (fusedMat != null) fusedMat.release();
    }

    /* ============================================================
     * Spinner 监听器 | Spinner Listeners
     * ============================================================ */

    /**
     * 融合模式选择监听器 | Fusion mode selection listener
     */
    private class FusionModeListener implements AdapterView.OnItemSelectedListener {
        @Override
        public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
            currentFusionMode = position;
            Log.i(TAG, "融合模式 | Fusion mode: " + FUSION_MODES[position]);
        }

        @Override
        public void onNothingSelected(AdapterView<?> parent) {}
    }

    /**
     * 配色方案选择监听器 | Color palette selection listener
     */
    private class PaletteListener implements AdapterView.OnItemSelectedListener {
        @Override
        public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
            currentPalette = position;
            Log.i(TAG, "配色方案 | Color palette: " + COLOR_PALETTES[position]);
        }

        @Override
        public void onNothingSelected(AdapterView<?> parent) {}
    }
}
