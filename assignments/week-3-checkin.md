# 第三周检查点 | Week 3 Check-in

## 概述 | Overview

第三周检查点评估学员在第 11-15 天的学习成果，重点检查 Android 应用开发、OpenCV 热成像渲染、系统整体集成以及最终演示准备。这是项目的收尾阶段，要求完整的端到端系统演示。

> Week 3 check-in evaluates learning outcomes from Days 11-15, focusing on Android app development, OpenCV thermal imaging rendering, full system integration, and final presentation preparation. This is the project's closing phase, requiring a complete end-to-end system demonstration.

---

## 检查时间 | Check-in Time

**Day 15（最终展示日）| Day 15 (Final Presentation Day)**

---

## 一、进度检查项 | Progress Check Items

### Day 11: Android USB Host 通信 | Android USB Host Communication

- [ ] Android Studio 项目创建成功，能在真机上运行
- [ ] 理解 Android USB Host API 的基本使用流程
- [ ] 实现了 USB 设备枚举和权限请求
- [ ] 能识别连接的 STM32 设备（通过 VID/PID 或设备类）
- [ ] 建立了 USB 通信连接，能接收 STM32 发送的数据
- [ ] 能正确解析数据帧（帧头识别、数据提取、校验验证）
- [ ] 处理了 USB 设备断开和重连的场景

### Day 12: OpenCV 热成像渲染 | OpenCV Thermal Imaging Rendering

- [ ] 成功集成 OpenCV Android SDK 到项目中
- [ ] 理解热成像伪彩色（Pseudo-color）映射的原理
- [ ] 实现了至少一种伪彩色方案（如 Ironbow、Rainbow、Inferno）
- [ ] 能将 32x24 的温度矩阵渲染为可视化的热成像图像
- [ ] 实现了图像缩放/插值（从 32x24 放大到可显示的分辨率）
- [ ] 添加了温度标尺/色标（Color Bar）显示
- [ ] 帧率满足实时显示需求（至少 4 FPS）

### Day 13: 双目画面合成与 UI | Dual-View Composition and UI

- [ ] 实现了左右双目画面的并排（Side-by-Side）显示
- [ ] 理解双目视觉的原理：两个不同视角如何增强深度感知
- [ ] 添加了触摸测温功能（点击画面显示该点温度）
- [ ] 实现了最高/最低温度追踪和标注
- [ ] UI 布局合理：热成像画面占主要区域，控制区域不遮挡画面
- [ ] 添加了温度阈值报警功能（可选：超过设定温度时变色提示）
- [ ] 应用有合理的权限声明（USB 权限等）

### Day 14: 系统集成 | System Integration

- [ ] STM32 固件 + USB 数据线 + Android 手机端到端联通
- [ ] 实时热成像画面流畅，无明显延迟（延迟 < 500ms）
- [ ] 双目画面同步显示，无明显时间差
- [ ] 系统能稳定运行至少 10 分钟
- [ ] 整体外观整洁：硬件固定良好，线缆管理有序
- [ ] 如果制作了外壳/支架，结构稳固且便于使用

### Day 15: 最终展示准备 | Final Presentation Preparation

- [ ] 完成了最终演示排练至少 2 次
- [ ] 准备了 PPT 或演示文稿
- [ ] 准备了备用方案（如预先录制好的演示视频）
- [ ] 所有源代码已提交到 Git 仓库
- [ ] README.md 文档完善
- [ ] 准备好回答老师和同学的提问

---

## 二、技术理解验证问题 | Technical Verification Questions

### Android 开发问题 | Android Development Questions

**Q1.** 在 Android 中使用 USB Host API 与 STM32 通信，需要哪些关键步骤？`UsbManager`、`UsbDevice`、`UsbDeviceConnection`、`UsbEndpoint` 各自的作用是什么？

> What are the key steps to communicate with STM32 using Android USB Host API? What are the roles of `UsbManager`, `UsbDevice`, `UsbDeviceConnection`, and `UsbEndpoint`?

**参考答案 | Reference Answer:**
关键步骤：
1. 获取 `UsbManager` 实例（系统服务）
2. 通过 `UsbManager.getDeviceList()` 枚举已连接的 USB 设备
3. 找到目标 `UsbDevice`（通过 VID/PID 匹配）
4. 请求用户授权 `UsbManager.requestPermission()`
5. 打开 `UsbDeviceConnection`
6. 找到正确的 `UsbEndpoint`（IN endpoint 用于接收数据）
7. 使用 `connection.bulkTransfer()` 进行数据收发

各组件作用：
- `UsbManager`：管理 USB 设备的枚举和权限
- `UsbDevice`：代表一个已连接的 USB 设备
- `UsbDeviceConnection`：与 USB 设备的实际通信连接
- `UsbEndpoint`：通信端点，定义了数据传输的方向和类型

---

**Q2.** 在 Android 中进行 USB 数据接收时，为什么不能在主线程（UI 线程）中执行？应该如何处理？

> Why should USB data reception not be performed on the Android main (UI) thread? How should it be handled?

**参考答案 | Reference Answer:**
USB 的 `bulkTransfer()` 是阻塞操作，会一直等待直到收到数据或超时。在主线程执行会导致：
1. UI 界面冻结（ANR - Application Not Responding）
2. 系统可能杀掉应用

正确做法：
- 创建一个后台线程（`Thread` 或 `HandlerThread`）专门用于 USB 数据接收
- 接收到的数据通过 `Handler`、`runOnUiThread()` 或 `LiveData` 传递到 UI 线程进行显示
- 注意线程同步，避免并发访问共享数据结构

---

**Q3.** Android Activity 的生命周期对 USB 连接有什么影响？在 `onPause()` 或 `onDestroy()` 中应该做什么处理？

> How does the Android Activity lifecycle affect USB connections? What should be handled in `onPause()` or `onDestroy()`?

**参考答案 | Reference Answer:**
- `onPause()`：Activity 进入后台时调用。应暂停数据接收线程，避免后台占用资源。但不应关闭 USB 连接，因为用户可能很快返回。
- `onDestroy()`：Activity 销毁时调用。应关闭 USB 连接（`connection.close()`），停止接收线程，释放所有 USB 资源。
- `onResume()`：恢复时重新检查 USB 设备状态，可能需要重新请求权限。
- 正确的生命周期管理确保应用稳定运行，避免资源泄漏。

---

### OpenCV 热成像问题 | OpenCV Thermal Imaging Questions

**Q4.** 什么是伪彩色（Pseudo-color）映射？为什么热成像需要使用伪彩色而不是灰度显示？常见的伪彩色方案有哪些？

> What is pseudo-color mapping? Why do thermal images use pseudo-color instead of grayscale? What are common pseudo-color schemes?

**参考答案 | Reference Answer:**
伪彩色映射是将温度值映射到彩色空间的过程。使用伪彩色而不是灰度的原因：
1. 人眼对颜色的分辨能力远强于灰度（能区分约 1000 种颜色，但只能区分约 100 个灰度级别）
2. 彩色显示能更直观地突出温度差异
3. 常见的标准化色标方案更易于解读

常见方案：
- **Ironbow**：黑色 -> 紫色 -> 红色 -> 橙色 -> 黄色 -> 白色（最常见）
- **Rainbow**：蓝色 -> 青色 -> 绿色 -> 黄色 -> 红色
- **Inferno**：黑色 -> 深紫 -> 红橙 -> 黄色 -> 白色
- **Hot**：黑色 -> 红色 -> 黄色 -> 白色

---

**Q5.** 将 32x24 的温度矩阵放大到手机屏幕分辨率（如 1080x1920），常用的插值方法有哪些？各有什么优缺点？

> What are the common interpolation methods for scaling a 32x24 temperature matrix to phone screen resolution (e.g., 1080x1920)? What are the pros and cons of each?

**参考答案 | Reference Answer:**
常见插值方法：
1. **最近邻插值（Nearest Neighbor）**：最快，但画面有明显的方块感
2. **双线性插值（Bilinear）**：速度适中，画面平滑，是热成像应用的最佳平衡点
3. **双三次插值（Bicubic）**：质量最好但速度最慢，可能不适合实时应用

在 OpenCV 中使用 `Imgproc.resize(src, dst, new Size(width, height), 0, 0, Imgproc.INTER_LINEAR)` 实现双线性插值。由于热成像数据分辨率低（32x24），双线性插值是推荐选择。

---

**Q6.** 在 OpenCV 中如何实现温度到颜色的映射？请描述使用 Look-Up Table (LUT) 的方法。

> How to implement temperature-to-color mapping in OpenCV? Describe the Look-Up Table (LUT) approach.

**参考答案 | Reference Answer:**
LUT 方法步骤：
1. 将温度值归一化到 0-255 范围：`normalized = (temp - minTemp) / (maxTemp - minTemp) * 255`
2. 创建一个 256x1x3 的颜色查找表（LUT），每个索引对应一个 RGB 颜色值
3. 使用 `Core.LUT(src, lut, dst)` 进行高效的颜色映射
4. 也可以使用 `Imgproc.applyColorMap()` 函数直接应用 OpenCV 内置的色标

LUT 方法的优势：映射速度快（O(n)），颜色方案可以灵活自定义，适合实时应用。

---

### 系统集成问题 | System Integration Questions

**Q7.** 整个系统的端到端延迟由哪些部分组成？如何优化以减少延迟？

> What components make up the end-to-end latency of the entire system? How can it be optimized?

**参考答案 | Reference Answer:**
延迟组成：
1. **传感器采集延迟**：MLX90640 采集一帧数据的时间（如 1/8 Hz = 125ms at 8 Hz）
2. **I2C 读取延迟**：MCU 通过 I2C 读取帧数据的时间（约 10-30ms）
3. **USB 传输延迟**：数据从 MCU 传输到 Android 的时间（约 1-5ms）
4. **协议解析延迟**：Android 解析数据帧的时间（约 1ms）
5. **渲染延迟**：OpenCV 渲染热成像画面的时间（约 5-20ms）

优化方法：
- 提高 MLX90640 刷新率（但会增加噪声）
- 使用 DMA 加速 I2C 和 USB 传输
- 在 Android 端使用双缓冲：接收线程写缓冲 A，渲染线程读缓冲 B
- 使用 `Mat` 对象池避免频繁内存分配

---

**Q8.** 双目热成像相比单目热成像有什么优势？在软件层面如何利用双目数据？

> What advantages does binocular thermal imaging have over monocular? How can dual-sensor data be utilized at the software level?

**参考答案 | Reference Answer:**
双目优势：
1. **更宽的视场角**：两个传感器可以覆盖更大的监测范围
2. **立体视觉**：如果两个传感器有部分重叠的视野，理论上可以估计目标的距离
3. **冗余性**：一个传感器故障时另一个仍可工作
4. **温度交叉验证**：同一目标出现在两个视野中时可以交叉验证温度读数

软件利用方式：
- 并排显示（Side-by-Side）：最常见的呈现方式
- 画面拼接（Stitching）：如果视野有重叠，可以拼接成全景热图
- 温度差异分析：比较同一区域在两个传感器中的读数差异

---

## 三、最终展示准备清单 | Final Presentation Preparation Checklist

### 硬件准备 | Hardware Preparation

- [ ] STM32 开发板供电正常（USB 或电池）
- [ ] 两个 MLX90640 传感器固定在支架上，位置稳定
- [ ] USB 数据线连接可靠（连接 STM32 和 Android 手机）
- [ ] 线缆整理整齐，不影响演示操作
- [ ] 准备了不同温度的测试物体（如热水杯、冰块、手）
- [ ] 有备用电池或充电方案

### 软件准备 | Software Preparation

- [ ] STM32 固件为最终版本，已充分测试
- [ ] Android APK 为最终版本，已安装在手机上
- [ ] 应用启动后能自动检测和连接 USB 设备
- [ ] 测试了多种场景下的表现（不同温度物体、不同距离）
- [ ] 准备了演示视频作为备用方案

### 演示材料 | Presentation Materials

- [ ] PPT 或演示文稿制作完成
- [ ] 包含项目概述、技术架构、核心代码展示
- [ ] 包含系统架构图和数据流图
- [ ] 准备了挑战与解决方案的讲解
- [ ] 准备了 3-5 个可演示的场景

### 文档材料 | Documentation

- [ ] README.md 完善（项目介绍、硬件清单、接线说明、编译步骤）
- [ ] 代码有充分注释
- [ ] 接线图/原理图文档完整
- [ ] Git 提交记录清晰

---

## 四、演示就绪评估 | Demo Readiness Assessment

### 功能完整性检查 | Feature Completeness Check

| 功能 | 状态 | 备注 |
|------|------|------|
| MLX90640 传感器数据读取 | [ ] | 双目数据都能读取 |
| 温度计算和补偿 | [ ] | 误差 < 2 度 |
| USB CDC 数据传输 | [ ] | 稳定无丢包 |
| Android USB 接收 | [ ] | 自动识别设备 |
| 热成像伪彩色渲染 | [ ] | 至少一种色标 |
| 双目画面显示 | [ ] | 并排显示 |
| 触摸测温功能 | [ ] | 点击显示温度 |
| 最高/最低温度标注 | [ ] | 实时更新 |

### 稳定性检查 | Stability Check

- [ ] 连续运行 5 分钟无崩溃
- [ ] USB 拔插后能自动重连
- [ ] 传感器遮挡恢复后数据正常
- [ ] 应用切换到后台再恢复后正常工作

### 演示流程排练 | Demo Flow Rehearsal

1. **开场介绍**（1 分钟）：项目名称、功能概述
2. **硬件展示**（1 分钟）：展示 STM32 + 双 MLX90640 硬件
3. **实时演示**（3-5 分钟）：
   - 连接 USB，启动 Android 应用
   - 展示实时热成像画面
   - 用手/热水杯/冰块演示温度变化
   - 演示触摸测温功能
   - 演示双目画面效果
4. **技术讲解**（3-5 分钟）：架构、核心技术、挑战
5. **总结展望**（1-2 分钟）：收获、改进方向

---

## 自我评估量表 | Self-Assessment Rubric

| 评估维度 | 自我评分 (1-5) | 说明 |
|----------|---------------|------|
| Android USB 通信实现 | ___ / 5 | 能否稳定接收数据并处理异常 |
| OpenCV 渲染效果 | ___ / 5 | 热成像画面是否清晰美观 |
| 系统集成完整性 | ___ / 5 | 端到端系统是否稳定运行 |
| UI/UX 设计 | ___ / 5 | 应用界面是否友好易用 |
| 项目文档质量 | ___ / 5 | 文档是否完整清晰 |
| 演示准备充分度 | ___ / 5 | 演示是否流畅、有说服力 |

---

## 通过标准 | Pass Criteria

### 基本要求（必须） | Basic Requirements (Mandatory)

- [ ] Android 应用能连接 STM32 并显示热成像画面
- [ ] 至少一个传感器的数据能正确显示
- [ ] 伪彩色渲染基本正确
- [ ] 系统能运行演示
- [ ] 有基本的项目文档

### 优秀标准（额外） | Excellence Criteria (Additional)

- [ ] 双目画面都能正确显示
- [ ] 触摸测温功能完善
- [ ] 画面流畅（> 4 FPS），延迟低
- [ ] 系统稳定运行 10 分钟以上
- [ ] UI 精美，有温度标尺和报警功能
- [ ] 代码质量高，文档完善

**总分 >= 70 分为通过，>= 90 分为优秀**

| Total >= 70 is passing, >= 90 is excellent |

---

*最后更新：2026-05-27 | Last updated: 2026-05-27*
