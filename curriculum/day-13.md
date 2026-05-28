# Day 13: 图像融合与 UI 完善 | Image Fusion & UI Refinement

## 学习目标 | Learning Objectives

- 理解图像融合的概念和常用算法（透明度叠加、加权融合）
- 使用 OpenCV 实现可见光图像与热图像的融合
- 集成 UVCAndroid 库获取 USB 摄像头可见光画面
- 集成 UsbSerial 库接收 STM32 发送的温度数据
- 完善 APP 的用户界面和交互功能

## 前置准备 | Prerequisites

- [ ] Day 12 伪彩色热图像渲染成功
- [ ] ThermalEyes APP 项目能编译运行
- [ ] 已浏览开源 APP 仓库的代码结构

## 为什么学这个？| Why This Matters

图像融合是"双目"热成像仪的核心卖点。单纯的热图像虽然能显示温度分布，但缺乏空间细节 -- 你可能看到一个热源，但看不清它是什么。将热图像与可见光图像融合后，就能同时看到物体的外观轮廓和温度分布。这种技术在实际应用中非常有价值：消防员可以同时看到火焰的温度和房间结构，维修人员可以看到设备的温度异常和具体部件。

## 今日任务 | Today's Tasks

### Task 1: 实现图像融合算法 (estimated 45 minutes)

**目标：** 用 OpenCV 实现可见光图像与热图像的加权融合

**步骤：**

1. **理解融合原理**
   - 可见光图像提供空间细节（边缘、纹理、颜色）
   - 热图像提供温度分布信息
   - 融合 = alpha * 可见光图像 + (1-alpha) * 热图像
   - alpha 值控制融合比例：0=纯热图，1=纯可见光

2. **实现 C++ 融合函数**

   ```cpp
   /**
    * @brief 融合可见光图像和热图像
    * @param visible 可见光图像 (BGR, 320x240)
    * @param thermal 热图像 (BGR, 320x240)
    * @param alpha 融合系数 [0.0, 1.0]，0=纯热图，1=纯可见光
    * @return 融合后的图像
    */
   cv::Mat fuseImages(const cv::Mat& visible, const cv::Mat& thermal, float alpha) {
       CV_Assert(visible.size() == thermal.size());
       CV_Assert(visible.type() == CV_8UC3);
       CV_Assert(thermal.type() == CV_8UC3);

       cv::Mat result(visible.size(), CV_8UC3);

       for (int y = 0; y < visible.rows; y++) {
           for (int x = 0; x < visible.cols; x++) {
               cv::Vec3b v = visible.at<cv::Vec3b>(y, x);
               cv::Vec3b t = thermal.at<cv::Vec3b>(y, x);

               // 加权融合
               result.at<cv::Vec3b>(y, x) = cv::Vec3b(
                   (uchar)(alpha * v[0] + (1.0f - alpha) * t[0]),
                   (uchar)(alpha * v[1] + (1.0f - alpha) * t[1]),
                   (uchar)(alpha * v[2] + (1.0f - alpha) * t[2])
               );
           }
       }

       return result;

       // 或者使用 OpenCV 内置函数（更高效）：
       // cv::addWeighted(visible, alpha, thermal, 1.0 - alpha, 0, result);
   }
   ```

3. **使用 OpenCV 高效版本**
   ```cpp
   cv::Mat fuseImagesFast(const cv::Mat& visible, const cv::Mat& thermal, float alpha) {
       cv::Mat result;
       cv::addWeighted(visible, alpha, thermal, 1.0 - alpha, 0.0, result);
       return result;
   }
   ```

4. **添加 JNI 接口**
   ```cpp
   extern "C" JNIEXPORT jobject JNICALL
   Java_com_example_thermaleyes_MainActivity_fuseImages(
           JNIEnv* env, jobject thiz,
           jobject visibleBitmap, jfloatArray tempData,
           jfloat alpha, jfloat minTemp, jfloat maxTemp) {

       // 将 Android Bitmap 转换为 OpenCV Mat
       // ... (使用 AndroidBitmap_lockPixels)

       // 渲染热图像
       // ... (使用 Day 12 的函数)

       // 融合
       cv::Mat fused = fuseImagesFast(visibleMat, thermalMat, alpha);

       // 转换回 Bitmap
       // ...

       return resultBitmap;
   }
   ```

**预期结果：**
- 图像融合函数完成
- 能调整融合比例

**常见问题：**

**Q: 融合后图像颜色偏暗？**
A: 确保两个输入图像的像素值范围一致（0-255）。如果热图像使用了不同的值范围，需要先归一化。

---

### Task 2: 集成 USB 通信功能 (estimated 75 minutes)

**目标：** 在 APP 中实现 USB 串口通信，接收固件发送的温度数据

**步骤：**

1. **添加 USB Host 权限**
   - 在 `AndroidManifest.xml` 中添加：
   ```xml
   <uses-feature android:name="android.hardware.usb.host" />
   <uses-permission android:name="android.permission.USB_PERMISSION" />
   ```

2. **集成 UsbSerial 库**
   - 在 `app/build.gradle` 中添加依赖：
   ```gradle
   implementation 'com.github.mik3y:usb-serial-for-android:3.4.6'
   ```
   - 或参考 ThermalEyes APP 仓库的依赖配置

3. **创建 USB 通信管理类**
   ```java
   public class UsbSerialManager {
       private static final String TAG = "UsbSerial";
       private UsbManager usbManager;
       private UsbSerialPort serialPort;
       private boolean isConnected = false;

       // 帧头帧尾
       private static final int FRAME_HEADER = 0xAA55;
       private static final int FRAME_SIZE = 1546;

       public interface TemperatureCallback {
           void onTemperatureData(float[] temperatures, float minTemp, float maxTemp);
       }

       private TemperatureCallback callback;

       public UsbSerialManager(Context context) {
           usbManager = (UsbManager) context.getSystemService(Context.USB_SERVICE);
       }

       public void setCallback(TemperatureCallback cb) {
           this.callback = cb;
       }

       public boolean connect() {
           // 查找所有 USB 设备
           for (UsbDevice device : usbManager.getDeviceList().values()) {
               // 查找 CDC 设备
               UsbSerialDriver driver = UsbSerialProber.getDefaultProber().probeDevice(device);
               if (driver != null) {
                   // 请求权限
                   if (!usbManager.hasPermission(device)) {
                       // 需要先请求权限
                       return false;
                   }

                   // 打开连接
                   UsbDeviceConnection connection = usbManager.openDevice(device);
                   serialPort = driver.getPorts().get(0);
                   serialPort.open(connection);
                   serialPort.setParameters(115200, 8, UsbSerialPort.STOPBITS_1,
                                               UsbSerialPort.PARITY_NONE);

                   isConnected = true;
                   startReading();
                   return true;
               }
           }
           return false;
       }

       private void startReading() {
           new Thread(() -> {
               byte[] buffer = new byte[4096];
               while (isConnected) {
                   try {
                       int len = serialPort.read(buffer, 100);
                       if (len > 0) {
                           parseData(buffer, len);
                       }
                   } catch (Exception e) {
                       Log.e(TAG, "Read error: " + e.getMessage());
                   }
               }
           }).start();
       }

       private void parseData(byte[] data, int len) {
           // 查找帧头 0xAA55
           // 解析帧数据
           // 转换为温度数组
           // 调用回调
           if (callback != null) {
               float[] temps = new float[768];
               // ... 解析逻辑 ...
               callback.onTemperatureData(temps, minTemp, maxTemp);
           }
       }

       public void disconnect() {
           isConnected = false;
           if (serialPort != null) {
               try { serialPort.close(); } catch (Exception e) {}
           }
       }
   }
   ```

   **注意：** 这是一个框架代码。完整的帧解析逻辑需要根据 Day 9 定义的帧格式来实现。参考 ThermalEyes APP 仓库中的实际实现。

4. **在 MainActivity 中使用**
   ```java
   private UsbSerialManager usbManager;

   @Override
   protected void onCreate(Bundle savedInstanceState) {
       // ...
       usbManager = new UsbSerialManager(this);
       usbManager.setCallback(new UsbSerialManager.TemperatureCallback() {
           @Override
           public void onTemperatureData(float[] temperatures, float minTemp, float maxTemp) {
               runOnUiThread(() -> {
                   Bitmap bitmap = renderThermalImage(temperatures, 32, 24, minTemp, maxTemp);
                   thermalImageView.setImageBitmap(bitmap);
                   tempInfoText.setText(String.format("%.1fC ~ %.1fC", minTemp, maxTemp));
               });
           }
       });
   }

   @Override
   protected void onResume() {
       super.onResume();
       usbManager.connect();
   }

   @Override
   protected void onPause() {
       super.onPause();
       usbManager.disconnect();
   }
   ```

**预期结果：**
- APP 能通过 USB 接收温度数据
- 实时显示热图像

**常见问题：**

**Q: 找不到 USB 设备？**
A: 确认手机支持 USB Host 模式（不是所有手机都支持）。使用 OTG 线连接。

**Q: USB 权限被拒绝？**
A: 需要在 AndroidManifest 中声明 USB 设备的 vendor ID 和 product ID，或使用 USB_DEVICE_ATTACHED intent。

---

### Task 3: 完善 UI 界面 (estimated 45 minutes)

**目标：** 完善 APP 的用户界面，添加控制选项和信息显示

**步骤：**

1. **添加控件**
   - 融合比例滑块（SeekBar）
   - 配色方案选择（Spinner/RadioButton）
   - 连接状态指示
   - 中心温度显示

2. **更新布局**
   ```xml
   <!-- 在现有布局基础上添加 -->
   <SeekBar
       android:id="@+id/fusionSeekBar"
       android:layout_width="match_parent"
       android:layout_height="wrap_content"
       android:max="100"
       android:progress="50"
       android:layout_margin="10dp"/>

   <TextView
       android:id="@+id/fusionLabel"
       android:layout_width="wrap_content"
       android:layout_height="wrap_content"
       android:text="融合比例: 50%"
       android:textColor="#FFFFFF"/>

   <TextView
       android:id="@+id/centerTempText"
       android:layout_width="wrap_content"
       android:layout_height="wrap_content"
       android:text="中心温度: --C"
       android:textColor="#FF4444"
       android:textSize="18sp"/>

   <TextView
       android:id="@+id/connectionStatus"
       android:layout_width="wrap_content"
       android:layout_height="wrap_content"
       android:text="未连接"
       android:textColor="#FF0000"/>
   ```

3. **添加交互逻辑**
   ```java
   SeekBar fusionSeekBar = findViewById(R.id.fusionSeekBar);
   fusionSeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
       @Override
       public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
           float alpha = progress / 100.0f;
           fusionLabel.setText(String.format("融合比例: %d%%", progress));
           // 更新融合参数
       }
       // ...
   });
   ```

**预期结果：**
- UI 包含所有必要的控件
- 交互流畅

---

### Task 4: 集成 UVC 摄像头（可选/高级） (estimated 45 minutes)

**目标：** 使用 UVCAndroid 库获取 USB 可见光摄像头画面

**步骤：**

1. **了解 UVCAndroid**
   - UVC（USB Video Class）是 USB 摄像头的标准协议
   - UVCAndroid 库让 Android 可以直接读取 USB 摄像头画面
   - 参考 ThermalEyes APP 的实现方式

2. **集成 UVCAndroid**
   - 参考 https://github.com/saki4510t/UVCAndroid
   - 或者参考 ThermalEyes APP 中的 UVC 集成方式
   - 这一步比较复杂，如果时间不够可以跳过，使用模拟数据

3. **获取可见光帧**
   - 注册帧回调
   - 将 YUV/RGB 帧转换为 OpenCV Mat
   - 传递给融合函数

**预期结果：**
- 能获取 USB 摄像头画面
- 画面与热图像可以进行融合

**注意：** UVC 集成是本课程中技术难度最高的部分。如果时间不够，可以：
- 使用手机内置摄像头替代 USB 摄像头
- 使用静态测试图像模拟可见光输入
- 在最终演示中使用预录的可见光视频

---

## 今日作业 | Homework

1. **图像融合实现**（必须）
   - 成功实现至少一种融合算法
   - 提交融合效果图截图

2. **USB 通信**（必须）
   - APP 能通过 USB 接收温度数据
   - 实时显示热图像
   - 提交运行截图

3. **UI 完善**（推荐）
   - 完成基本的 UI 控件
   - 融合比例可调节
   - 温度信息显示正确

4. **Phase 4 总结**（必须）
   - 总结 Android 开发中学到的关键技术
   - 记录遇到的最大挑战和解决方案

## 明日预告 | Tomorrow's Preview

明天我们将进行全系统联调！固件 + APP 整合，确保端到端的数据流通畅。你还将进行性能优化和 Bug 修复，让整个系统稳定运行。

## 参考资源 | References

- **OpenCV addWeighted**：https://docs.opencv.org/master/d2/de8/group__core__array.html
- **usb-serial-for-android**：https://github.com/mik3y/usb-serial-for-android
- **UVCAndroid**：https://github.com/saki4510t/UVCAndroid
- **ThermalEyes APP 源码**：https://github.com/colourfate/ThermalEyes
- **Android USB Host 文档**：https://developer.android.com/guide/topics/connectivity/usb/host

---

*预计完成时间：6-8 小时*
*Estimated completion time: 6-8 hours*
