# Day 12: 伪彩色映射与热图像渲染 | Pseudo-Color Mapping & Thermal Image Rendering

## 学习目标 | Learning Objectives

- 理解伪彩色映射的概念和用途
- 实现常见的伪彩色映射算法（Ironbow、Rainbow、White Hot）
- 使用 OpenCV 将温度矩阵渲染为彩色图像
- 在 Android 界面上显示热图像
- 实现温度数据的插值放大（从 32x24 到 320x240）

## 前置准备 | Prerequisites

- [ ] Day 11 Android 项目创建成功，OpenCV 集成完成
- [ ] JNI 调用正常工作
- [ ] 已了解 ThermalEyes APP 源码结构

## 为什么学这个？| Why This Matters

热成像仪最引人注目的特征就是那些色彩斑斓的"热图"。但传感器本身并不输出颜色 -- 它只输出温度数值。"伪彩色映射"就是将温度数值转换为颜色的算法。这个看似简单的步骤，实际上涉及到色彩科学和人眼感知的知识。一个好的配色方案，能让人眼快速识别温度差异。今天你将亲手实现几种经典的伪彩色映射方案，让温度数据"变得可见"。

## 今日任务 | Today's Tasks

### Task 1: 理解伪彩色映射原理 (estimated 40 minutes)

**目标：** 理解温度到颜色的映射方法和常见配色方案

**步骤：**

1. **为什么需要伪彩色？**
   - 人眼对灰度的分辨能力有限（约 100 级）
   - 人眼对色彩的分辨能力强得多（数千种）
   - 将温度映射为颜色，可以让人眼快速识别微小的温度差异
   - 不同颜色代表不同温度范围

2. **常见伪彩色方案**
   - **Ironbow（铁虹）**：最常用的热成像配色
     * 低温（暗色）：黑色 -> 深蓝 -> 紫色
     * 中温（暖色）：红色 -> 橙色 -> 黄色
     * 高温（亮色）：白色
   - **Rainbow（彩虹）**：经典彩虹色
     * 蓝 -> 青 -> 绿 -> 黄 -> 红
   - **White Hot（白热）**：军事常用
     * 黑（冷）-> 白（热），灰度线性映射
   - **Black Hot（黑热）**：白热反转
     * 白（冷）-> 黑（热）

3. **映射算法原理**
   - 将温度范围 [T_min, T_max] 映射到 [0.0, 1.0]
   - 用归一化值在颜色查找表中查找 R, G, B 值
   - 查找表可以是线性插值的断点表

   ```
   归一化: ratio = (T - T_min) / (T_max - T_min)
   ratio 范围: 0.0 (最冷) -> 1.0 (最热)
   ```

**预期结果：**
- 理解伪彩色映射的概念
- 知道常见的配色方案
- 理解归一化和颜色查找表的原理

---

### Task 2: 实现 C++ 伪彩色映射函数 (estimated 60 minutes)

**目标：** 在 JNI 层用 C++ + OpenCV 实现伪彩色映射

**步骤：**

1. **编写 Ironbow 配色方案**

   ```cpp
   // native-lib.cpp
   #include <opencv2/core.hpp>
   #include <opencv2/imgproc.hpp>
   #include <android/bitmap.h>

   /**
    * @brief Ironbow 伪彩色映射
    * @param value 归一化值 [0.0, 1.0]
    * @return BGR 颜色值（OpenCV 使用 BGR 顺序）
    */
   cv::Vec3b ironbowColor(float value) {
       // 限制范围
       if (value < 0.0f) value = 0.0f;
       if (value > 1.0f) value = 1.0f;

       // Ironbow 断点表（位置, R, G, B）
       struct ColorStop { float pos; uchar r, g, b; };
       ColorStop stops[] = {
           {0.00f,   0,   0,   0},    // 黑
           {0.10f,  20,   0,  80},    // 深蓝
           {0.25f,  60,   0, 160},    // 蓝
           {0.40f, 160,   0, 160},    // 紫
           {0.55f, 200,  30,  30},    // 红
           {0.70f, 240, 100,   0},    // 橙
           {0.85f, 250, 200,  20},    // 黄
           {1.00f, 255, 255, 255},    // 白
       };

       // 在断点之间线性插值
       for (int i = 0; i < 7; i++) {
           if (value >= stops[i].pos && value <= stops[i+1].pos) {
               float t = (value - stops[i].pos) / (stops[i+1].pos - stops[i].pos);
               uchar r = stops[i].r + t * (stops[i+1].r - stops[i].r);
               uchar g = stops[i].g + t * (stops[i+1].g - stops[i].g);
               uchar b = stops[i].b + t * (stops[i+1].b - stops[i].b);
               return cv::Vec3b(b, g, r);  // BGR 顺序
           }
       }
       return cv::Vec3b(255, 255, 255);
   }
   ```

2. **实现温度矩阵到图像的转换**

   ```cpp
   /**
    * @brief 将温度数据转换为伪彩色图像
    * @param tempData 温度数据数组 (768 个 float)
    * @param width 宽度 (32)
    * @param height 高度 (24)
    * @param minTemp 显示最低温度
    * @param maxTemp 显示最高温度
    * @return OpenCV Mat (BGR 格式)
    */
   cv::Mat temperaturesToImage(float* tempData, int width, int height,
                               float minTemp, float maxTemp) {
       // 创建输出图像
       cv::Mat image(height, width, CV_8UC3);

       float range = maxTemp - minTemp;
       if (range < 0.1f) range = 0.1f;  // 防止除零

       for (int y = 0; y < height; y++) {
           for (int x = 0; x < width; x++) {
               float temp = tempData[y * width + x];
               float normalized = (temp - minTemp) / range;
               image.at<cv::Vec3b>(y, x) = ironbowColor(normalized);
           }
       }

       // 使用双线性插值放大图像（从 32x24 到 320x240）
       cv::Mat resized;
       cv::resize(image, resized, cv::Size(320, 240), 0, 0, cv::INTER_LINEAR);

       return resized;
   }
   ```

3. **创建 JNI 接口函数**

   ```cpp
   extern "C" JNIEXPORT jobject JNICALL
   Java_com_example_thermaleyes_MainActivity_renderThermalImage(
           JNIEnv* env, jobject thiz,
           jfloatArray tempData, jint width, jint height,
           jfloat minTemp, jfloat maxTemp) {

       // 获取 Java float 数组
       jfloat* data = env->GetFloatArrayElements(tempData, nullptr);

       // 转换为 OpenCV 图像
       cv::Mat image = temperaturesToImage((float*)data, width, height,
                                           minTemp, maxTemp);

       env->ReleaseFloatArrayElements(tempData, data, JNI_ABORT);

       // 将 OpenCV Mat 转换为 Android Bitmap
       // 创建 Bitmap 对象
       jclass bitmapClass = env->FindClass("android/graphics/Bitmap");
       jmethodID createBitmapMethod = env->GetStaticMethodID(
           bitmapClass, "createBitmap",
           "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;");

       jclass configClass = env->FindClass("android/graphics/Bitmap$Config");
       jfieldID argb8888Field = env->GetStaticFieldID(configClass, "ARGB_8888",
           "Landroid/graphics/Bitmap$Config;");
       jobject argb8888 = env->GetStaticObjectField(configClass, argb8888Field);

       jobject bitmap = env->CallStaticObjectMethod(bitmapClass, createBitmapMethod,
           image.cols, image.rows, argb8888);

       // 将 Mat 数据复制到 Bitmap
       cv::Mat rgba;
       cv::cvtColor(image, rgba, cv::COLOR_BGR2RGBA);

       AndroidBitmapInfo info;
       AndroidBitmap_getInfo(env, bitmap, &info);

       void* pixels = nullptr;
       AndroidBitmap_lockPixels(env, bitmap, &pixels);

       memcpy(pixels, rgba.data, rgba.total() * rgba.elemSize());

       AndroidBitmap_unlockPixels(env, bitmap);

       return bitmap;
   }
   ```

**预期结果：**
- 伪彩色映射函数完成
- JNI 接口可以将温度数组转换为 Bitmap

**常见问题：**

**Q: 编译时找不到 AndroidBitmap 头文件？**
A: 在 CMakeLists.txt 中添加：
```cmake
find_library(android-lib android)
target_link_libraries(native-lib ${android-lib})
```

---

### Task 3: 在 Android 界面显示热图像 (estimated 60 minutes)

**目标：** 创建 Android UI，使用 ImageView 显示热图像

**步骤：**

1. **修改布局文件**
   - 编辑 `res/layout/activity_main.xml`：

   ```xml
   <?xml version="1.0" encoding="utf-8"?>
   <LinearLayout xmlns:android="http://schemas.android.com/apk/res/android"
       android:layout_width="match_parent"
       android:layout_height="match_parent"
       android:orientation="vertical"
       android:gravity="center"
       android:background="#000000">

       <TextView
           android:id="@+id/titleText"
           android:layout_width="wrap_content"
           android:layout_height="wrap_content"
           android:text="ThermalEyes 热成像"
           android:textColor="#FFFFFF"
           android:textSize="20sp"
           android:layout_marginBottom="10dp"/>

       <ImageView
           android:id="@+id/thermalImage"
           android:layout_width="320dp"
           android:layout_height="240dp"
           android:scaleType="fitCenter"
           android:background="#333333"/>

       <TextView
           android:id="@+id/tempInfoText"
           android:layout_width="wrap_content"
           android:layout_height="wrap_content"
           android:text="温度: --C ~ --C"
           android:textColor="#FFFFFF"
           android:textSize="16sp"
           android:layout_marginTop="10dp"/>

       <Button
           android:id="@+id/testButton"
           android:layout_width="wrap_content"
           android:layout_height="wrap_content"
           android:text="生成测试热图"
           android:layout_marginTop="10dp"/>

   </LinearLayout>
   ```

2. **修改 MainActivity**
   ```java
   public class MainActivity extends AppCompatActivity {

       static {
           System.loadLibrary("native-lib");
       }

       private ImageView thermalImageView;
       private TextView tempInfoText;

       // JNI 方法声明
       public native Bitmap renderThermalImage(float[] tempData, int width, int height,
                                                float minTemp, float maxTemp);

       @Override
       protected void onCreate(Bundle savedInstanceState) {
           super.onCreate(savedInstanceState);
           setContentView(R.layout.activity_main);

           thermalImageView = findViewById(R.id.thermalImage);
           tempInfoText = findViewById(R.id.tempInfoText);

           Button testButton = findViewById(R.id.testButton);
           testButton.setOnClickListener(v -> generateTestThermalImage());
       }

       private void generateTestThermalImage() {
           // 生成测试温度数据（模拟一个热源在中心）
           float[] testData = new float[768];
           float minTemp = 20.0f;
           float maxTemp = 35.0f;

           for (int y = 0; y < 24; y++) {
               for (int x = 0; x < 32; x++) {
                   // 中心高，边缘低
                   float dx = x - 16.0f;
                   float dy = y - 12.0f;
                   float dist = (float)Math.sqrt(dx*dx + dy*dy);
                   float maxDist = (float)Math.sqrt(16*16 + 12*12);
                   float temp = 35.0f - (dist / maxDist) * 15.0f;
                   testData[y * 32 + x] = temp;
               }
           }

           // 渲染热图像
           Bitmap bitmap = renderThermalImage(testData, 32, 24, minTemp, maxTemp);
           if (bitmap != null) {
               thermalImageView.setImageBitmap(bitmap);
               tempInfoText.setText(String.format("温度: %.1fC ~ %.1fC", minTemp, maxTemp));
           }
       }
   }
   ```

3. **运行测试**
   - 编译运行
   - 点击"生成测试热图"按钮
   - 应该看到中心偏暖色、边缘偏冷色的热图像
   - 类似一个从中心向外扩散的"热点"

**预期结果：**
- 界面正确显示
- 点击按钮后出现伪彩色热图像
- 热图像呈现中心高温、边缘低温的效果

**常见问题：**

**Q: 点击按钮后没有图像？**
A:
1. 检查 Logcat 中是否有错误日志
2. 确认 JNI 函数名正确
3. 确认 Bitmap 创建成功

**Q: 图像显示变形？**
A: 检查 ImageView 的 scaleType 和宽高比。320:240 = 4:3。

---

### Task 4: 添加色标条 (estimated 30 minutes)

**目标：** 在热图像旁边显示温度色标条，帮助用户理解颜色含义

**步骤：**

1. **在布局中添加色标条**
   ```xml
   <LinearLayout
       android:layout_width="wrap_content"
       android:layout_height="240dp"
       android:orientation="horizontal"
       android:layout_marginTop="10dp">

       <ImageView
           android:id="@+id/thermalImage"
           android:layout_width="320dp"
           android:layout_height="240dp"
           ... />

       <!-- 色标条 -->
       <ImageView
           android:id="@+id/colorBar"
           android:layout_width="30dp"
           android:layout_height="240dp"
           android:layout_marginLeft="5dp"/>

   </LinearLayout>
   ```

2. **生成色标条图像**
   - 可以在 C++ 中生成一个垂直渐变条
   - 或者在 Java 中使用 Canvas 绘制

   ```java
   private Bitmap generateColorBar(float minTemp, float maxTemp) {
       int width = 30;
       int height = 240;
       float[] barData = new float[height * width];

       // 从上到下：高温到低温
       for (int y = 0; y < height; y++) {
           float ratio = 1.0f - (float)y / height;
           float temp = minTemp + ratio * (maxTemp - minTemp);
           for (int x = 0; x < width; x++) {
               barData[y * width + x] = temp;
           }
       }

       return renderThermalImage(barData, width, height, minTemp, maxTemp);
   }
   ```

**预期结果：**
- 热图像旁边显示色标条
- 上方为高温色，下方为低温色

---

## 今日作业 | Homework

1. **热图像渲染**（必须）
   - 成功显示测试热图像
   - Ironbow 配色方案正确实现
   - 提交运行截图

2. **算法笔记**（必须）
   - 解释伪彩色映射的原理
   - 画出 Ironbow 配色方案的断点图
   - 解释双线性插值放大的作用

3. **配色方案比较**（推荐）
   - 实现至少 2 种配色方案（Ironbow + Rainbow 或 White Hot）
   - 添加切换按钮，比较不同方案的效果
   - 记录你的偏好和理由

4. **挑战任务**（可选）
   - 实现温度十字线：在热图像中心显示一个十字标记，显示该点的精确温度
   - 添加温度范围滑块，动态调整 minTemp/maxTemp

## 明日预告 | Tomorrow's Preview

明天我们将实现图像融合 -- 将热图像与可见光图像叠加显示！这是双目热成像仪的核心功能。你还将学习如何通过 USB 接收固件发送的实时数据，完善 APP 的通信功能。

## 参考资源 | References

- **OpenCV 颜色映射**：https://docs.opencv.org/master/d3/d50/group__imgproc__colormap.html
- **伪彩色算法详解**：https://www.flir.com/discover/cores/thermal-imaging-colormaps-explained/
- **OpenCV resize 插值**：https://docs.opencv.org/master/da/d54/group__imgproc__transform.html
- **Android Bitmap 操作**：https://developer.android.com/reference/android/graphics/Bitmap
- **ThermalEyes APP 源码**：https://github.com/colourfate/ThermalEyes

---

*预计完成时间：6-7 小时*
*Estimated completion time: 6-7 hours*
