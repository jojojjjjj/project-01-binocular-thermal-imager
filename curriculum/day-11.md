# Day 11: PC 端热图可视化入门 | PC-Side Thermal Visualization Basics

## 学习目标 | Learning Objectives

- 配好 Python + OpenCV + pyserial 环境
- 用 pyserial 从 USB CDC 虚拟串口读出温度帧
- 理解一个 PC 端可视化程序的三个核心环节：串口接收 → 帧解析 → 伪彩映射显示
- 第一次在电脑屏幕上看到温度的彩色画面

## 前置准备 | Prerequisites

- [ ] Phase 3 完成，固件通过 USB 稳定发送温度数据
- [ ] 装好 Python 3（3.8+）
- [ ] 一根 USB-C 数据线（连开发板到电脑）

## 为什么学这个？| Why This Matters

到昨天为止，温度数据已经能从传感器读到 MCU、再通过 USB 发出来了——但你在电脑上看到的还只是一串串数字。今天我们要把这些数字变成屏幕上的彩色热图：红色热、蓝色冷，一眼就能看出哪里温度高。这是整个项目"看得见"的关键一步。

**日常类比**：就像天气预报的温度图——一堆温度数字（32×24=768 个）你看不过来，但涂上颜色（蓝色冷、红色热）就一目了然了。今天我们就是给温度数据"涂颜色"的那个人。

## 今日任务 | Today's Tasks

### Task 1: 配 Python 环境 + 装 OpenCV/pyserial (estimated 30 minutes)

**目标：** 装好可视化需要的三个库

**步骤：**

1. 打开命令行（Windows 用 cmd 或 PowerShell，Mac/Linux 用 Terminal）
2. 安装依赖：
   ```bash
   pip install pyserial opencv-python numpy
   ```
3. 验证安装：
   ```bash
   python -c "import serial, cv2, numpy; print('OK', cv2.__version__)"
   ```
   打印出 `OK 4.x.x` 就成功了

**预期结果：** 三个库都能正常导入

**常见问题：**

**Q: pip 装很慢？**
A: 换国内镜像：`pip install -i https://pypi.tuna.tsinghua.edu.cn/simple pyserial opencv-python numpy`

**Q: import cv2 报错？**
A: opencv-python 装的是 `cv2` 模块，名字对不上是正常的。确认装的是 `opencv-python` 不是 `opencv`。

---

### Task 2: 列出所有串口，找到 STM32 (estimated 30 minutes)

**目标：** 让 Python 找到开发板对应的串口

**步骤：**

1. 用 USB-C 线连开发板到电脑
2. 运行下面这段代码列出所有串口：
   ```python
   import serial.tools.list_ports
   ports = serial.tools.list_ports.comports()
   for p in ports:
       print(p.device, p.description)
   ```
3. 找到描述里带 "STM32" 或 "Virtual COM Port" 或 "USB Serial" 的那个，记下它的名字（Windows 形如 `COM5`，Mac/Linux 形如 `/dev/tty.usbmodemxxx` 或 `/dev/ttyACM0`）
4. 拔插一下 USB，看哪个串口消失/出现，确认就是它

**预期结果：** 能找到开发板对应的串口名

**常见问题：**

**Q: 一个串口都列不出来？**
A: 换一条 USB 数据线（很多是纯充电线）；确认固件里 USB CDC 已启用并烧录了。

---

### Task 3: 读一帧温度并打印 (estimated 60 minutes)

**目标：** 从串口读出一帧 1542 字节，解析成 32×24 温度矩阵，打印中心点温度

**步骤：**

1. 新建 `scripts/usb_receiver.py`
2. 按 HW3 的 protocol.md 帧格式（帧头 + 帧计数 + 1536 字节温度 + CRC），用 pyserial 循环读取
3. 可以直接复用 `software/tests/test_basic.py` 里的 `parse_thermal_packet` 函数（已写好帧头检查、CRC 校验、温度解码）
4. 打印帧计数和中心像素（row=12, col=16）的温度

**参考骨架：**
```python
import serial
from test_basic import parse_thermal_packet, PACKET_TOTAL_SIZE  # 复用现成函数

ser = serial.Serial('COM5', 115200, timeout=1)  # 串口名换成你的
buf = b''
while True:
    buf += ser.read(ser.in_waiting or 1)
    while len(buf) >= PACKET_TOTAL_SIZE:
        frame = buf[:PACKET_TOTAL_SIZE]
        buf = buf[PACKET_TOTAL_SIZE:]
        try:
            temps, count, crc_ok = parse_thermal_packet(frame)
            if crc_ok:
                center = temps[12 * 32 + 16]
                print(f"帧 {count}: 中心温度 {center:.1f}°C")
        except AssertionError:
            buf = buf[1:]  # 帧头没对齐，往后挪一个字节
```

5. 运行，用手掌覆盖传感器，观察中心温度是否上升

**预期结果：** 控制台持续打印帧计数和中心温度，手掌覆盖时温度明显上升

**常见问题：**

**Q: 一直打印不出来 / 帧计数不涨？**
A: 串口名对不对；波特率对 USB CDC 无影响但填 115200 即可；确认固件确实在发数据（先用串口工具看原始字节）。

**Q: CRC 一直失败？**
A: 你的 protocol.md 帧格式可能和 test_basic.py 的 0x5A5A 帧头不一致——以固件实际发送的为准，调整解析函数的帧头/校验。

---

### Task 4: 伪彩色映射，第一次显示热图 (estimated 60 minutes)

**目标：** 把温度矩阵变成彩色画面用 OpenCV 显示出来

**步骤：**

1. 写一个伪彩映射函数：温度 → 归一化（0-1）→ 查颜色表 → RGB
2. 先用最简单的 Ironbow 配色（蓝→紫→红→橙→黄→白）
3. 用 `cv2.resize` 把 32×24 放大到 320×240（双线性插值，画面更平滑）
4. 用 `cv2.imshow` 显示，用 `cv2.waitKey(1)` 保持窗口刷新
5. 在画面上叠加中心温度文字 `cv2.putText`

**参考骨架：**
```python
import cv2
import numpy as np

def temp_to_color(temps):
    arr = np.array(temps, dtype=np.float32).reshape(24, 32)
    # 归一化到 0-255
    norm = cv2.normalize(arr, None, 0, 255, cv2.NORM_MINMAX).astype(np.uint8)
    # 用 OpenCV 自带 colormap（JET 近似 Rainbow，IRON 没有内置可手写查表）
    color = cv2.applyColorMap(norm, cv2.COLORMAP_JET)
    # 放大
    color = cv2.resize(color, (320, 240), interpolation=cv2.INTER_LINEAR)
    return color

# 在主循环里：
img = temp_to_color(temps)
cv2.putText(img, f"center: {center:.1f}C", (10, 20), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255,255,255), 1)
cv2.imshow('thermal', img)
cv2.waitKey(1)
```

6. 运行，你应该第一次在屏幕上看到彩色热图！手掌测试时手的位置应该明显变红

**预期结果：** 弹出窗口显示彩色热图，手掌测试有明显热区

**常见问题：**

**Q: 窗口卡死不刷新？**
A: `cv2.waitKey(1)` 必须在循环里调用，它是 OpenCV 刷新窗口的触发点。

**Q: 画面全是单色？**
A: 归一化没做对——确认 `cv2.normalize` 把温度范围映射到了 0-255。

---

## 今日作业 | Homework

1. **环境就绪**（必须）
   - pyserial/opencv/numpy 都能导入
   - 提交 import 成功的截图

2. **读帧 + 打印温度**（必须）
   - scripts/usb_receiver.py 能持续读帧并打印中心温度
   - 提交控制台输出截图（含手掌测试温度上升）

3. **显示热图**（必须）
   - 用 cv2.imshow 显示彩色热图
   - 提交热图截图（手掌测试有热区）

4. **概念笔记**（必须）
   - 用自己的话解释：串口接收、帧解析、伪彩映射三个环节各做什么
   - 画出"温度数据 → 彩色画面"的流程图

## 明日预告 | Tomorrow's Preview

明天我们深入伪彩映射：比较 Ironbow 和 Rainbow 两种配色的差别，理解为什么专业热像仪默认用 Ironbow。还会加上可见光摄像头，开始做图像融合。

## 参考资源 | References

- **pyserial 文档**：https://pyserial.readthedocs.io/
- **OpenCV Python 教程**：https://docs.opencv.org/master/d6/d00/tutorial_py_root.html
- **本课程现成函数**：`software/tests/test_basic.py`（parse_thermal_packet、伪彩映射、高斯频率分解）
- **colourfate 开源 Android APP（进阶参考）**：https://github.com/colourfate/ThermalEyes

---

*预计完成时间：3-4 小时*
*Estimated completion time: 3-4 hours*
