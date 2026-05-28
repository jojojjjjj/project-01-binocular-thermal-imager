# Day 11: Android 开发环境 + OpenCV | Android Dev Environment + OpenCV

## 学习目标 | Learning Objectives

- 完成 Android Studio 的安装和基本配置
- 创建第一个 Android 项目并运行
- 理解 Android 项目的基本结构
- 配置 NDK 和 C++ 支持
- 集成 OpenCV Android SDK
- 理解 JNI 的基本概念

## 前置准备 | Prerequisites

- [ ] Phase 3 完成，固件通过 USB 稳定发送温度数据
- [ ] 已下载 Android Studio
- [ ] 有一台 Android 手机（Android 7.0+）或模拟器
- [ ] 手机已开启 USB 调试模式

## 为什么学这个？| Why This Matters

Android 是全球用户量最大的移动操作系统。学会 Android 开发，意味着你开发的嵌入式设备可以直接与数十亿台手机通信。今天搭建的 Android 开发环境，是我们将温度数据变成可视化热图像的基础。虽然 Android 开发的学习曲线比嵌入式编程更陡，但一旦掌握了基本框架，你就能开发出功能丰富的手机应用。

## 今日任务 | Today's Tasks

### Task 1: 安装和配置 Android Studio (estimated 45 minutes)

**目标：** 完成 Android Studio 安装，确保能创建和运行项目

**步骤：**

1. **安装 Android Studio**
   - 下载地址：https://developer.android.com/studio
   - 安装包约 1GB，下载可能需要一些时间
   - 安装过程中保持默认选项
   - 等待 Android SDK 和模拟器组件下载完成（可能需要额外 2-3GB 下载）

2. **配置 Android SDK**
   - 打开 Android Studio
   - 进入 File -> Settings -> Appearance & Behavior -> System Settings -> Android SDK
   - 确认已安装：
     * Android 10.0 (Q) 或更高版本的 SDK Platform
     * Android SDK Build-Tools
     * Android SDK Platform-Tools
     * NDK (Side by side) -- 重要：OpenCV 需要
     * CMake -- 重要：编译 C++ 代码需要

3. **配置手机开发模式**
   - 在手机上进入"设置" -> "关于手机"
   - 连续点击"版本号" 7 次启用开发者选项
   - 进入"开发者选项"
   - 开启"USB 调试"
   - 用 USB 线连接手机到电脑
   - 手机上弹出"允许 USB 调试"对话框，点击允许

4. **验证环境**
   - 在 Android Studio 中，设备列表应该显示你的手机
   - 如果没有显示，检查 USB 驱动是否安装

**预期结果：**
- Android Studio 安装成功
- SDK 和 NDK 配置完成
- 手机连接成功，USB 调试已开启

**常见问题：**

**Q: SDK 下载很慢？**
A: 可以配置国内镜像。在 SDK Manager 中设置代理，或者手动下载 SDK。

**Q: 手机连接不上？**
A:
1. 确认 USB 线支持数据传输
2. 安装手机厂商的 USB 驱动
3. 在手机上确认允许 USB 调试

---

### Task 2: 创建第一个 Android 项目 (estimated 60 minutes)

**目标：** 创建 ThermalEyes Android 项目，理解项目结构

**步骤：**

1. **创建新项目**
   - File -> New -> New Project
   - 选择 "Empty Views Activity" 模板
   - 配置：
     * Name: `ThermalEyes`
     * Package name: `com.example.thermaleyes`
     * Language: `Java`
     * Minimum SDK: `API 24 (Android 7.0)`
   - 点击 Finish

2. **理解项目结构**
   ```
   app/
   ├── src/
   │   ├── main/
   │   │   ├── java/com/example/thermaleyes/
   │   │   │   └── MainActivity.java   -- 主 Activity
   │   │   ├── res/
   │   │   │   ├── layout/
   │   │   │   │   └── activity_main.xml -- 界面布局
   │   │   │   └── values/
   │   │   │       └── strings.xml     -- 字符串资源
   │   │   └── AndroidManifest.xml     -- 应用清单
   │   └── ...
   ├── build.gradle                    -- 构建配置
   └── ...
   ```

3. **查看 MainActivity.java**
   - 打开 `app/src/main/java/.../MainActivity.java`
   - 理解以下关键概念：
     * `onCreate()`: Activity 创建时调用
     * `setContentView()`: 设置界面布局
     * Activity 是 Android 的"页面"，一个 APP 通常有多个 Activity

4. **修改界面显示文字**
   - 打开 `res/layout/activity_main.xml`
   - 找到 `TextView`，修改文字为 "ThermalEyes 热成像仪"
   - 修改字体大小为 24sp

5. **运行项目**
   - 连接手机
   - 点击工具栏的 Run 按钮（绿色三角形）
   - 或按 Shift+F10
   - 等待编译和安装
   - 手机上应该显示你的 APP

**预期结果：**
- 项目创建成功
- 能在手机上运行
- 界面显示自定义文字

**常见问题：**

**Q: 编译报错 "SDK not found"？**
A: 检查 local.properties 文件中的 sdk.dir 路径是否正确。

**Q: 安装到手机失败？**
A: 确认手机已开启 USB 调试，并且电脑已授权。

---

### Task 3: 配置 NDK 和 C++ 支持 (estimated 45 minutes)

**目标：** 配置项目支持 C++ 和 JNI，为 OpenCV 集成做准备

**步骤：**

1. **启用 C++ 支持**
   - 在 `app/build.gradle` 中的 `android.defaultConfig` 下添加：

   ```gradle
   externalNativeBuild {
       cmake {
           cppFlags "-std=c++11"
       }
   }
   ndk {
       abiFilters 'armeabi-v7a', 'arm64-v8a'
   }
   ```

   - 在 `android` 块中添加：

   ```gradle
   externalNativeBuild {
       cmake {
           path "src/main/cpp/CMakeLists.txt"
           version "3.10.2"
       }
   }
   ```

2. **创建 C++ 代码目录**
   - 在 `app/src/main/` 下创建 `cpp` 目录
   - 创建 `CMakeLists.txt`：

   ```cmake
   cmake_minimum_required(VERSION 3.4.1)

   # 添加 native-lib
   add_library(native-lib SHARED native-lib.cpp)

   # 查找 log 库
   find_library(log-lib log)

   # 链接
   target_link_libraries(native-lib ${log-lib})
   ```

   - 创建 `native-lib.cpp`：

   ```cpp
   #include <jni.h>
   #include <string>
   #include <android/log.h>

   #define LOG_TAG "ThermalEyes"
   #define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

   extern "C" JNIEXPORT jstring JNICALL
   Java_com_example_thermaleyes_MainActivity_stringFromJNI(
           JNIEnv* env,
           jobject /* this */) {
       LOGI("JNI function called");
       std::string hello = "Hello from C++!";
       return env->NewStringUTF(hello.c_str());
   }
   ```

3. **在 MainActivity 中调用 JNI**
   ```java
   public class MainActivity extends AppCompatActivity {

       // 加载 native 库
       static {
           System.loadLibrary("native-lib");
       }

       // 声明 native 方法
       public native String stringFromJNI();

       @Override
       protected void onCreate(Bundle savedInstanceState) {
           super.onCreate(savedInstanceState);
           setContentView(R.layout.activity_main);

           // 调用 JNI 方法
           String msg = stringFromJNI();
           Log.d("ThermalEyes", msg);
           Toast.makeText(this, msg, Toast.LENGTH_SHORT).show();
       }
   }
   ```

4. **编译运行**
   - 同步 Gradle：File -> Sync Project with Gradle Files
   - 编译运行
   - 应该看到 Toast 显示 "Hello from C++!"

**预期结果：**
- C++ 代码编译成功
- JNI 调用正常工作
- 理解 JNI 的基本使用方式

**常见问题：**

**Q: CMake 找不到？**
A: 在 SDK Manager 中安装 CMake 和 NDK。

**Q: JNI 函数名不对？**
A: JNI 函数名必须遵循 `Java_包名_类名_方法名` 的格式，用下划线分隔。包名中的点也用下划线替换。

---

### Task 4: 集成 OpenCV Android SDK (estimated 60 minutes)

**目标：** 将 OpenCV Android SDK 集成到项目中

**步骤：**

1. **下载 OpenCV Android SDK**
   - 访问：https://opencv.org/releases/
   - 下载 Android 版本（opencv-android-4.x.x.zip）
   - 解压到方便的位置

2. **导入 OpenCV 模块**
   - 方式一（推荐）：使用预编译库
   - 将 `sdk/native/libs/` 下的 `.so` 文件复制到 `app/src/main/jniLibs/`
     ```
     app/src/main/jniLibs/
     ├── arm64-v8a/
     │   └── libopencv_java4.so
     └── armeabi-v7a/
         └── libopencv_java4.so
     ```
   - 将 `sdk/native/static/libs/` 下的 OpenCV 头文件配置到 CMakeLists.txt

   - 方式二：参考开源 APP 项目 ThermalEyes 的配置方式
   - 克隆 APP 仓库：https://github.com/colourfate/ThermalEyes
   - 查看其 `build.gradle` 和 `CMakeLists.txt` 中 OpenCV 的配置方式

3. **修改 CMakeLists.txt**
   ```cmake
   cmake_minimum_required(VERSION 3.4.1)

   # 设置 OpenCV 路径（根据实际位置修改）
   set(OpenCV_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../../libs/opencv/sdk/native/jni")

   # 查找 OpenCV
   find_package(OpenCV REQUIRED)

   # 添加 native 库
   add_library(native-lib SHARED native-lib.cpp)

   # 包含 OpenCV 头文件
   target_include_directories(native-lib PRIVATE ${OpenCV_INCLUDE_DIRS})

   # 查找 log 库
   find_library(log-lib log)

   # 链接
   target_link_libraries(native-lib ${OpenCV_LIBS} ${log-lib})
   ```

4. **测试 OpenCV**
   - 在 `native-lib.cpp` 中添加：

   ```cpp
   #include <opencv2/core.hpp>
   #include <opencv2/imgproc.hpp>

   extern "C" JNIEXPORT jint JNICALL
   Java_com_example_thermaleyes_MainActivity_testOpenCV(
           JNIEnv* env, jobject) {
       // 创建一个简单的 OpenCV Mat 测试
       cv::Mat test(32, 24, CV_32F);
       return test.rows;  // 应该返回 32
   }
   ```

   - 在 MainActivity 中调用：
   ```java
   public native int testOpenCV();
   // ...
   int rows = testOpenCV();
   Log.d("ThermalEyes", "OpenCV Mat rows: " + rows);
   ```

**预期结果：**
- OpenCV 集成成功
- C++ 代码能使用 OpenCV 的 Mat 类
- 编译运行无错误

**常见问题：**

**Q: 找不到 OpenCV 包？**
A: 检查 CMakeLists.txt 中 OpenCV_DIR 路径是否正确。路径必须指向包含 `OpenCVConfig.cmake` 的目录。

**Q: 链接错误 "undefined reference to cv::..."？**
A: 确保在 target_link_libraries 中包含了 ${OpenCV_LIBS}。

**Q: 运行时崩溃 " UnsatisfiedLinkError"？**
A: 确保对应的 ABI（arm64-v8a 或 armeabi-v7a）的 .so 文件已复制到 jniLibs 目录。

---

## 今日作业 | Homework

1. **Android 项目创建**（必须）
   - 成功创建并运行 ThermalEyes 项目
   - JNI 调用正常工作
   - 提交项目截图

2. **OpenCV 集成**（必须）
   - OpenCV 成功集成
   - native 代码能使用 OpenCV 的 Mat 类
   - 提交运行日志

3. **Android 学习笔记**（必须）
   - 解释以下概念：Activity, Layout, JNI, NDK, Gradle
   - 画出 Android 项目与 OpenCV 的关系图
   - 记录环境搭建中遇到的问题

4. **探索开源 APP**（推荐）
   - 克隆 ThermalEyes APP 仓库
   - 浏览其项目结构
   - 找到 OpenCV 图像处理相关的代码位置

## 明日预告 | Tomorrow's Preview

明天我们将进入图像处理的核心！你将学习伪彩色映射算法，将温度数据转换为彩色热图像。你将第一次在手机屏幕上看到温度的可视化效果！

## 参考资源 | References

- **Android Studio 下载**：https://developer.android.com/studio
- **Android 开发者文档**：https://developer.android.com/docs
- **OpenCV Android 教程**：https://docs.opencv.org/master/d5/df8/tutorial_dev_with_OCV_on_Android.html
- **ThermalEyes APP 源码**：https://github.com/colourfate/ThermalEyes
- **JNI 教程**：https://developer.android.com/ndk/guides/

---

*预计完成时间：6-7 小时*
*Estimated completion time: 6-7 hours*
