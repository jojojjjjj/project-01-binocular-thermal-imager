# Day 13: 图像融合与 UI 完善 | Image Fusion & UI Refinement

> **主线说明（体验档）**：本课程主线是「PC 端 Python+OpenCV 可视化」（对齐 assignments.md 的 HW4）。今天用 `cv2.VideoCapture(0)` 取电脑摄像头可见光画面 + `cv2.addWeighted` 做透明度叠加，再用高斯频率分解做高频增强融合，几行就能跑（day-11/12 已给骨架）。最后把 USB 接收、伪彩、融合串成一个完整的 PC 脚本，准备进入全系统联调。想做成手机 APP 的同学可参考 colourfate 开源的 Android 版（进阶，不作为主线要求）。

## 学习目标 | Learning Objectives

- 理解图像融合的概念和常用算法（透明度叠加、高频增强）
- 用 OpenCV 实现可见光图像与热图像的融合
- 用 `cv2.VideoCapture(0)` 取电脑摄像头画面
- 把 USB 接收、伪彩映射、融合、显示串成一个完整的 PC 可视化脚本

## 前置准备 | Prerequisites

- [ ] Day 12 的 Ironbow/Rainbow 伪彩映射能显示
- [ ] `scripts/usb_receiver.py` 能读帧并显示热图
- [ ] 电脑有摄像头（笔记本自带或 USB 摄像头都行；没有也能跑纯热图模式）

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

2. **实现透明度叠加融合**

   OpenCV 的 `cv2.addWeighted` 一行就能做加权融合：

   ```python
   def fuse_alpha(visible, thermal, alpha=0.6):
       """透明度叠加：alpha=0.6 偏可见光，0.4 偏热图。两张图要同尺寸。"""
       visible = cv2.resize(visible, (320, 240))
       thermal = cv2.resize(thermal, (320, 240))
       return cv2.addWeighted(visible, alpha, thermal, 1.0 - alpha, 0)
   ```

3. **实现高频增强融合**

   另一种思路：把热图的"边缘"（高频）叠到可见光上，既保留可见光的细节，又让热区有强调。做法是高斯模糊得到低频，高频=热图-低频，再叠加。`software/tests/test_basic.py` 里已有高斯频率分解函数可参考。

   ```python
   def fuse_highfreq(visible, thermal, ksize=5, gain=1.5):
       """高频增强：把热图的边缘信息叠到可见光上。"""
       visible = cv2.resize(visible, (320, 240))
       thermal = cv2.resize(thermal, (320, 240))
       low = cv2.GaussianBlur(thermal, (ksize, ksize), 0)   # 低频（平滑的底）
       high = cv2.subtract(thermal, low)                    # 高频（边缘/热点轮廓）
       # 把高频加到可见光上，gain 控制增强强度
       fused = cv2.addWeighted(visible, 1.0, high, gain, 0)
       return fused
   ```

**预期结果：**
- 两种融合函数都写好
- 能调整 alpha / gain 看效果变化

**常见问题：**

**Q: 融合后图像颜色偏暗或发白？**
A: 确保两个输入都是 0-255 的 uint8、同尺寸。`addWeighted` 的权重之和接近 1 不会整体变亮/变暗；高频增强里 `gain` 太大会让画面过曝，调小一点。

**Q: 热图和可见光对不上？**
A: 电脑摄像头和 MLX90640 物理位置不同，画面有视差，本项目接受近似对齐即可（不必做精确配准）。把摄像头尽量靠近 MLX90640、朝同一方向能减轻错位。

---

### Task 2: 接摄像头 + 把融合接进主循环 (estimated 60 minutes)

**目标：** 用电脑摄像头取可见光画面，把 Task 1 的融合函数接进 USB 读帧主循环

**步骤：**

1. **打开电脑摄像头**

   `cv2.VideoCapture(0)` 就能取本机摄像头（0 是默认摄像头编号）。没有摄像头也能跑，退化为只显示热图。

   ```python
   cap = cv2.VideoCapture(0)               # 0=笔记本自带摄像头；外接 USB 摄像头可能是 1
   has_cam = cap.isOpened()
   if not has_cam:
       print("没找到摄像头，将只显示热图模式")
   ```

2. **在主循环里同时取热图和可见光，做融合**

   把 day-11/12 的读帧循环扩成：读一帧温度 → 渲染热图 → 读一帧可见光 → 融合 → 显示。用按键切显示模式。

   ```python
   mode = 'alpha'   # alpha / highfreq / thermal_only / visible_only
   while True:
       # 1) 读温度帧（复用 day-11 的 parse_thermal_packet）
       buf += ser.read(ser.in_waiting or 1)
       while len(buf) >= PACKET_TOTAL_SIZE:
           frame = buf[:PACKET_TOTAL_SIZE]; buf = buf[PACKET_TOTAL_SIZE:]
           try:
               temps, count, crc_ok = parse_thermal_packet(frame)
           except AssertionError:
               buf = buf[1:]; continue
           if not crc_ok:
               continue
           thermal = temp_to_color(temps, scheme)        # day-12 的伪彩函数

           # 2) 读可见光（有摄像头时）
           visible = None
           if has_cam:
               ok, visible = cap.read()
               if ok:
                   visible = cv2.resize(visible, (320, 240))

           # 3) 按当前模式合成画面
           if mode == 'thermal_only' or visible is None:
               out = thermal
           elif mode == 'visible_only':
               out = visible
           elif mode == 'highfreq':
               out = fuse_highfreq(visible, thermal)
           else:  # alpha
               out = fuse_alpha(visible, thermal)

           cv2.putText(out, f"[{mode}] center: {temps[12*32+16]:.1f}C",
                       (10, 20), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255,255,255), 1)
           cv2.imshow('thermal', out)
           key = cv2.waitKey(1) & 0xFF
           if key == ord('1'): mode = 'alpha'
           elif key == ord('2'): mode = 'highfreq'
           elif key == ord('3'): mode = 'thermal_only'
           elif key == ord('4'): mode = 'visible_only'
           elif key == ord('q'): break
   cap.release(); cv2.destroyAllWindows()
   ```

3. **运行验证**
   - 按 1/2/3/4 切换四种模式，确认都能显示
   - 手掌测试时，融合模式下应能同时看到手的轮廓（可见光）和热区（暖色）

**预期结果：**
- 脚本能同时接收 USB 温度帧 + 摄像头画面
- 四种模式可按键切换
- 没装摄像头时自动退化为纯热图，不报错

**常见问题：**

**Q: 摄像头打不开 / VideoCapture(0) 返回 False？**
A: 换成 `cv2.VideoCapture(1)` 试试（外接摄像头编号可能不同）；确认没被别的程序占用（如 Zoom/微信）。实在没有就跑纯热图模式，不影响主线。

**Q: 画面很卡？**
A: 摄像头读取和 USB 读取都在主循环里，会互相拖。可以接受——本项目不追求高帧率，2-4 FPS 够看。想优化可以把摄像头读取放到单独线程，进阶可做。

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
