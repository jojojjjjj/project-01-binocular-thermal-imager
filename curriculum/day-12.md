# Day 12: 伪彩色映射与热图像渲染 | Pseudo-Color Mapping & Thermal Image Rendering

> **主线说明（体验档）**：本课程主线是「PC 端 Python+OpenCV 可视化」（对齐 assignments.md 的 HW4）。今天用 `cv2.applyColorMap` + 自建查表 + `cv2.resize` 实现伪彩映射，几行就能把温度矩阵变成彩色热图，day-11 已给了读帧骨架。想做成手机 APP 的同学可参考 colourfate 开源的 Android 版（进阶，不作为主线要求）。

## 学习目标 | Learning Objectives

- 理解伪彩色映射的概念和用途
- 实现常见的伪彩色映射算法（Ironbow、Rainbow、White Hot）
- 用 OpenCV 的 `applyColorMap` 和自建查表把温度矩阵渲染成彩色图像
- 在 PC 屏幕上用 `cv2.imshow` 显示热图像，并切换多种配色
- 实现温度数据的插值放大（从 32×24 到 320×240）

## 前置准备 | Prerequisites

- [ ] Day 11 的 PC 可视化环境装好（pyserial/opencv/numpy 能导入）
- [ ] `scripts/usb_receiver.py` 能读帧并打印中心温度
- [ ] 看过 `software/tests/test_basic.py` 里的伪彩映射函数（可直接复用）

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

### Task 2: 用 Python 实现 Ironbow 伪彩映射 (estimated 60 minutes)

**目标：** 在 PC 端用 Python + OpenCV 实现伪彩色映射，把温度矩阵变成彩色图像

**步骤：**

1. **手写一个 Ironbow 查表函数**

   Ironbow 没有 OpenCV 内置 colormap，我们用一组断点（位置、R、G、B）自己线性插值。原理和 Task 1 讲的一样：温度归一化到 [0,1]，再在断点表里查颜色。

   ```python
   import numpy as np
   import cv2

   # Ironbow 断点：(位置, R, G, B)
   IRONBOW_STOPS = [
       (0.00,   0,   0,   0),    # 黑
       (0.10,  20,   0,  80),    # 深蓝
       (0.25,  60,   0, 160),    # 蓝
       (0.40, 160,   0, 160),    # 紫
       (0.55, 200,  30,  30),    # 红
       (0.70, 240, 100,   0),    # 橙
       (0.85, 250, 200,  20),    # 黄
       (1.00, 255, 255, 255),    # 白
   ]

   def build_lut(stops):
       """把断点表插值成一张 256 项的查找表（OpenCV applyColorMap 要 BGR）。"""
       stops = sorted(stops)
       positions = np.array([s[0] for s in stops])
       colors = np.array([s[1:] for s in stops], dtype=np.float32)
       # 在 [0,1] 上均匀取 256 个点
       samples = np.linspace(0, 1, 256)
       r = np.interp(samples, positions, colors[:, 0])
       g = np.interp(samples, positions, colors[:, 1])
       b = np.interp(samples, positions, colors[:, 2])
       lut = np.stack([b, g, r], axis=1).astype(np.uint8)  # BGR
       return lut

   IRONBOW_LUT = build_lut(IRONBOW_STOPS)

   def temp_to_ironbow(temps, t_min=None, t_max=None):
       """温度矩阵（24×32 或 768 个值）→ BGR 彩色图。"""
       arr = np.array(temps, dtype=np.float32).reshape(24, 32)
       if t_min is None: t_min = arr.min()
       if t_max is None: t_max = arr.max()
       norm = (arr - t_min) / max(t_max - t_min, 0.1)      # 归一化到 0-1
       idx = (norm * 255).astype(np.uint8)                  # 映射到 0-255
       color = cv2.applyColorMap(idx, cv2.COLORMAP_USER_LUT if False else None)  # 占位
       # 用我们自己的 LUT：直接查表
       color = IRONBOW_LUT[idx]                              # (24,32,3) BGR
       # 双线性插值放大到 320×240
       color = cv2.resize(color, (320, 240), interpolation=cv2.INTER_LINEAR)
       return color
   ```

   > 上面 `IRONBOW_LUT[idx]` 就是查表：idx 是 0-255 的索引，直接拿出对应的颜色，比逐像素循环快得多。`cv2.applyColorMap` 也能用，但 Ironbow 不在内置列表里，所以自己建 LUT 最直接。

2. **接上 Day 11 的主循环，显示 Ironbow 热图**

   把 Day 11 里打印温度的那段，换成调用 `temp_to_ironbow` 再 `cv2.imshow`：

   ```python
   # 在 Day 11 的 while 循环里：
   if crc_ok:
       img = temp_to_ironbow(temps)
       cv2.putText(img, f"center: {center:.1f}C", (10, 20),
                   cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 1)
       cv2.imshow('thermal', img)
       cv2.waitKey(1)  # 必须调用，否则窗口不刷新
   ```

3. **运行验证**
   - 运行脚本，窗口应弹出彩色热图
   - 手掌覆盖传感器时，手的位置应明显偏暖色（红/橙/黄）
   - 室温区域应偏冷色（黑/蓝/紫）

**预期结果：**
- Ironbow 查表函数完成
- 屏幕上显示彩色热图，手掌测试有明显的暖色区

**常见问题：**

**Q: 画面全是一种颜色？**
A: 归一化没做对。确认 `(arr - t_min) / (t_max - t_min)` 把温度范围铺满了 0-1，再乘 255。如果 `t_max - t_min` 太小（比如都是 25°C），除以一个最小值 0.1 防止除零。

**Q: 颜色看起来不对（BGR/RGB 颠倒）？**
A: OpenCV 用 BGR 顺序，`cv2.imshow` 也按 BGR 显示。查表时 LUT 存成 BGR（断点表的 R/G/B 在 `np.stack` 时排成 `[b,g,r]`）就不会反。

---

### Task 3: 多配色切换显示热图像 (estimated 60 minutes)

**目标：** 在 PC 窗口里用按键切换配色方案，对比 Ironbow 和 Rainbow 的效果

**步骤：**

1. **再加一种配色：Rainbow**

   Rainbow 可以直接用 OpenCV 内置的 `cv2.COLORMAP_JET`（最接近经典彩虹：蓝→青→绿→黄→红）。我们封装一个函数，按选择返回不同配色：

   ```python
   def temp_to_color(temps, scheme='ironbow', t_min=None, t_max=None):
       arr = np.array(temps, dtype=np.float32).reshape(24, 32)
       if t_min is None: t_min = arr.min()
       if t_max is None: t_max = arr.max()
       norm = (arr - t_min) / max(t_max - t_min, 0.1)
       idx = (norm * 255).astype(np.uint8)

       if scheme == 'ironbow':
           color = IRONBOW_LUT[idx]                      # 自建查表
       elif scheme == 'rainbow':
           color = cv2.applyColorMap(idx, cv2.COLORMAP_JET)   # 内置
       elif scheme == 'white_hot':
           gray = cv2.merge([idx, idx, idx])             # 灰度，热=白
           color = gray
       else:
           color = IRONBOW_LUT[idx]

       return cv2.resize(color, (320, 240), interpolation=cv2.INTER_LINEAR)
   ```

2. **按键切换配色**

   在主循环里监听键盘：按 `1`/`2`/`3` 切换 Ironbow/Rainbow/White Hot。

   ```python
   scheme = 'ironbow'  # 当前配色
   while True:
       # ... 读帧、解析（复用 Day 11） ...
       if crc_ok:
           img = temp_to_color(temps, scheme)
           cv2.putText(img, f"[{scheme}] center: {center:.1f}C", (10, 20),
                       cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 1)
           cv2.imshow('thermal', img)
           key = cv2.waitKey(1) & 0xFF
           if key == ord('1'): scheme = 'ironbow'
           elif key == ord('2'): scheme = 'rainbow'
           elif key == ord('3'): scheme = 'white_hot'
           elif key == ord('q'): break
   ```

3. **运行对比**
   - 分别按 1/2/3 切换，观察同一个场景在三种配色下的差别
   - 手掌测试时，留意哪种配色让"热区边界"最清楚

**预期结果：**
- 窗口能显示热图
- 按键能在 Ironbow / Rainbow / White Hot 之间切换
- 不同配色的冷暖分布一致，只是"上色"不同

**常见问题：**

**Q: 按键没反应？**
A: `cv2.waitKey(1) & 0xFF` 要接住返回值再判断。窗口必须是焦点（点一下窗口再按键）。`waitKey` 的参数是毫秒，填 0 会阻塞等待，要填 1 让循环继续转。

**Q: JET（Rainbow）看起来颜色断层？**
A: JET 本身色阶就是分段的，正常现象。这正是 Ironbow 更受欢迎的原因之一——亮度单调递增，更符合人眼感知。

---

### Task 4: 画面上叠色标条 (estimated 30 minutes)

**目标：** 在热图旁边画一条温度色标条，让人一眼看懂"什么颜色对应什么温度"

**步骤：**

1. **生成一条色标条**

   色标条就是把"从最冷到最热"的颜色竖着排一条。我们直接造一组从 0 到 255 的索引，套用同一个 LUT：

   ```python
   def make_colorbar(scheme='ironbow', height=240, width=30):
       # 从上(热)到下(冷)：索引从 255 到 0
       ramp = np.linspace(255, 0, height).astype(np.uint8)         # (height,)
       ramp = np.tile(ramp[:, None], (1, width))                   # (height, width)
       if scheme == 'rainbow':
           bar = cv2.applyColorMap(ramp, cv2.COLORMAP_JET)
       elif scheme == 'white_hot':
           bar = cv2.merge([ramp, ramp, ramp])
       else:
           bar = IRONBOW_LUT[ramp]                                  # 自建 LUT
       return bar
   ```

2. **把色标条拼到热图右边**

   ```python
   img = temp_to_color(temps, scheme)
   bar = make_colorbar(scheme)
   combined = np.hstack([img, bar])                                # 热图 + 色标条
   cv2.imshow('thermal', combined)
   ```

3. **（可选）标几个温度刻度**
   用 `cv2.putText` 在色标条上标 25°C / 30°C / 35°C 的位置，帮助读数。

**预期结果：**
- 热图右侧出现一条色标条
- 顶部是高温色（白/红），底部是低温色（黑/蓝）

---

## 今日作业 | Homework

1. **热图像渲染**（必须）
   - 屏幕上成功显示 Ironbow 热图
   - Ironbow 查表函数正确实现
   - 提交运行截图

2. **算法笔记**（必须）
   - 解释伪彩色映射的原理
   - 画出 Ironbow 配色方案的断点图
   - 解释双线性插值放大的作用

3. **配色方案比较**（推荐）
   - 实现至少 2 种配色方案（Ironbow + Rainbow 或 White Hot）
   - 按键切换，比较不同方案的效果
   - 记录你的偏好和理由

4. **挑战任务**（可选）
   - 画面中心画十字线，显示该点精确温度
   - 加滑块/按键动态调整 minTemp/maxTemp，看热图如何变化

## 明日预告 | Tomorrow's Preview

明天我们将实现图像融合——把热图和电脑摄像头拍的可见光画面叠在一起！这是双目热成像仪的核心功能。还会把 USB 接收、伪彩、融合串成一个完整的 PC 可视化脚本，准备进入全系统联调。

## 参考资源 | References

- **OpenCV 颜色映射**：https://docs.opencv.org/master/d3/d50/group__imgproc__colormap.html
- **伪彩色算法详解**：https://www.flir.com/discover/cores/thermal-imaging-colormaps-explained/
- **OpenCV resize 插值**：https://docs.opencv.org/master/da/d54/group__imgproc__transform.html
- **本课程现成函数**：`software/tests/test_basic.py`（伪彩映射、高斯频率分解，可直接复用）
- **colourfate 开源 Android APP（进阶参考）**：https://github.com/colourfate/ThermalEyes

---

*预计完成时间：6-7 小时*
*Estimated completion time: 6-7 hours*
