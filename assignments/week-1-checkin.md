# 第一周检查点 | Week 1 Check-in

## 概述 | Overview

第一周检查点评估学员在第 1-5 天的学习成果，重点检查 STM32 开发环境搭建、I2C 通信理解和 MLX90640 传感器基础驱动开发。

> Week 1 check-in evaluates learning outcomes from Days 1-5, focusing on STM32 development environment setup, I2C communication understanding, and initial MLX90640 sensor driver development.

---

## 检查时间 | Check-in Time

**Day 5 下午 | Day 5 Afternoon**

---

## 一、进度检查项 | Progress Check Items

### Day 1-2: STM32 基础 | STM32 Basics

- [ ] STM32CubeIDE 安装完成并能正常编译工程
- [ ] 能够使用 ST-Link 将程序下载到 STM32 开发板
- [ ] 成功运行第一个 GPIO 点灯程序（LED 闪烁）
- [ ] 理解 STM32 的时钟树配置（HSE/HSI/PLL）
- [ ] 理解 HAL 库与 LL 库的区别，知道项目中选择 HAL 库的原因
- [ ] 能够使用串口（UART）输出调试信息到电脑终端

### Day 3: I2C 通信协议 | I2C Communication Protocol

- [ ] 理解 I2C 协议的基本原理（起始位、地址、ACK/NACK、停止位）
- [ ] 理解 7 位地址机制和 R/W 位的含义
- [ ] 能够使用逻辑分析仪或示波器观察 I2C 波形
- [ ] 成功使用 STM32 HAL 库进行 I2C 写操作（`HAL_I2C_Master_Transmit`）
- [ ] 成功使用 STM32 HAL 库进行 I2C 读操作（`HAL_I2C_Master_Receive`）
- [ ] 能够对 I2C 总线上的设备进行扫描（I2C Scanner），发现已连接设备的地址

### Day 4-5: MLX90640 传感器基础 | MLX90640 Sensor Basics

- [ ] 理解 MLX90640 传感器的基本工作原理（红外热电堆阵列）
- [ ] 理解传感器的分辨率（32x24 像素）和视场角（FOV）
- [ ] 能够查阅 MLX90640 数据手册，找到关键寄存器地址
- [ ] 成功通过 I2C 读取 MLX90640 的芯片 ID 寄存器（地址 0x2407）
- [ ] 成功读取 MLX90640 的 EEPROM 配置数据
- [ ] 能够读取原始帧数据（Raw IR data）并通过串口输出到 PC
- [ ] 使用 Tera Term 或类似串口工具查看传感器数据输出

---

## 二、技术理解验证问题 | Technical Verification Questions

### STM32 基础问题 | STM32 Basics

**Q1.** STM32 的 GPIO 有哪几种工作模式？本项目连接 MLX90640 的 I2C 引脚应该配置为哪种模式？

> What GPIO modes does the STM32 support? Which mode should the I2C pins connected to MLX90640 be configured as?

**参考答案 | Reference Answer:**
GPIO 模式包括：推挽输出（Push-Pull Output）、开漏输出（Open-Drain Output）、复用推挽（AF Push-Pull）、复用开漏（AF Open-Drain）、模拟输入（Analog）、浮空输入（Floating Input）、上拉输入（Input Pull-Up）、下拉输入（Input Pull-Down）。I2C 的 SCL 和 SDA 引脚应配置为**复用开漏输出（AF Open-Drain）**，因为 I2C 协议要求总线是开漏的，需要外部上拉电阻。

---

**Q2.** 解释 STM32 时钟树中 HSE、HSI、PLL 的含义。为什么我们通常使用 PLL 倍频而不是直接使用 HSE？

> Explain the meaning of HSE, HSI, and PLL in the STM32 clock tree. Why do we typically use PLL multiplication instead of directly using HSE?

**参考答案 | Reference Answer:**
- HSE (High Speed External)：外部高速晶振，精度高但频率受限于晶振本身（通常 8 MHz）
- HSI (High Speed Internal)：内部高速 RC 振荡器，不需要外部元件但精度较低
- PLL (Phase Locked Loop)：锁相环，可以将输入时钟倍频到更高频率
使用 PLL 倍频是因为 MCU 需要更高的运行频率（如 72 MHz 或 168 MHz），而晶振频率通常较低（4-25 MHz），通过 PLL 倍频可以在保持精度的同时获得更高的系统时钟。

---

**Q3.** `HAL_Init()` 函数做了哪些初始化工作？如果不调用它会怎样？

> What does the `HAL_Init()` function initialize? What happens if it is not called?

**参考答案 | Reference Answer:**
`HAL_Init()` 主要完成：配置 SysTick 定时器作为时基（默认 1ms 中断）、初始化 HAL 库的内部数据结构、设置 NVIC 优先级分组。如果不调用，所有依赖 HAL 时基的函数（如 `HAL_Delay()`）都无法正常工作，I2C 超时检测也会失效。

---

### I2C 通信问题 | I2C Communication Questions

**Q4.** 在 I2C 总线上，MLX90640 的默认 7 位地址是什么？如果总线上有两个 MLX90640，如何区分它们？

> What is the default 7-bit I2C address of the MLX90640? If there are two MLX90640 devices on the bus, how do you distinguish them?

**参考答案 | Reference Answer:**
MLX90640 的默认 I2C 地址是 0x33。通过修改传感器 EEPROM 中的地址寄存器可以为每个传感器分配不同的地址。在本项目中，由于需要双目（两个传感器），一个传感器保持默认地址 0x33，另一个需要改写为不同地址（如 0x33 以外的值），或者使用两个独立的 I2C 总线分别连接。

---

**Q5.** I2C 通信中，如果从设备没有发送 ACK（而是发送 NACK），可能的原因有哪些？请列举至少 3 种。

> In I2C communication, if the slave device does not send ACK (sends NACK instead), what are the possible reasons? List at least 3.

**参考答案 | Reference Answer:**
1. 从设备未正确连接（SCL/SDA 线路断开或短路）
2. I2C 地址错误（发送了错误的设备地址）
3. 总线上的上拉电阻缺失或阻值不合适
4. 从设备还未完成上电初始化
5. SCL 频率过高，超过了从设备的最大支持频率
6. 从设备处于错误状态或已被复位

---

**Q6.** MLX90640 的 I2C 通信最高支持多高的时钟频率？为什么我们在项目中通常设置较低的频率？

> What is the maximum I2C clock frequency supported by the MLX90640? Why do we typically set a lower frequency in the project?

**参考答案 | Reference Answer:**
MLX90640 支持 standard mode (100 kHz) 和 fast mode (400 kHz)，最高 400 kHz。设置较低频率的原因包括：提高长距离传输的信号完整性、减少总线上的噪声干扰、在调试阶段更容易用逻辑分析仪捕获和验证数据。

---

### MLX90640 传感器问题 | MLX90640 Sensor Questions

**Q7.** MLX90640 输出的原始数据是温度值吗？如果不是，需要进行哪些处理才能得到实际温度？

> Does the MLX90640 output raw temperature values? If not, what processing is needed to get actual temperatures?

**参考答案 | Reference Answer:**
不是直接的温度值。MLX90640 输出的是原始 IR（红外）ADC 数据，需要经过以下处理：
1. 从 EEPROM 读取校准参数（Vdd, Ta, emissivity, 等补偿系数）
2. 根据数据手册中的公式，使用校准参数和原始数据计算补偿后的像素温度值
3. 应用发射率（emissivity）校正
4. 可选：应用环境温度补偿

---

**Q8.** MLX90640 有哪两种刷新率模式？它们的区别是什么？在实时热成像应用中应该选择哪个？

> What are the two refresh rate modes of the MLX90640? What is the difference? Which should be chosen for real-time thermal imaging applications?

**参考答案 | Reference Answer:**
- MLX90640 支持两种刷新率：0.5 Hz（每 2 秒一帧）和 1 Hz（每秒一帧），同时还可以通过配置寄存器调整为 2 Hz、4 Hz、8 Hz、16 Hz、32 Hz、64 Hz。
- 较高刷新率可以提供更流畅的实时画面，但会增加噪声。
- 在实时热成像应用中，通常选择 8 Hz 或 16 Hz，在帧率和噪声之间取得平衡。

---

**Q9.** MLX90640 的视场角（FOV）是多少？如果要检测 2 米外的人体，每个人体像素大约覆盖多少面积？

> What is the FOV of the MLX90640? If detecting a human body at 2 meters distance, approximately what area does each pixel cover?

**参考答案 | Reference Answer:**
MLX90640 的标准 FOV 为 110 度 x 75.7 度。在 2 米距离处，整个画幅覆盖大约 5.5 m x 3.0 m 的区域。每个像素（32x24 = 768 个像素）大约覆盖 17 cm x 12.5 cm 的面积。

---

## 三、代码审查清单 | Code Review Checklist

### 工程结构 | Project Structure

- [ ] STM32CubeIDE 工程创建正确，文件结构清晰
- [ ] 使用了 `.ioc` 配置文件正确初始化 I2C 外设
- [ ] 主程序逻辑清晰，没有在 `main()` 中堆砌大量代码
- [ ] I2C 相关操作封装为独立函数（如 `MLX90640_Init()`, `MLX90640_ReadRaw()`）

### 代码质量 | Code Quality

- [ ] 所有函数有中文/英文注释说明功能、参数和返回值
- [ ] I2C 操作有超时处理和错误返回值检查
- [ ] 魔数（Magic Numbers）已替换为有意义的宏定义（如 `#define MLX90640_ADDR 0x33`）
- [ ] 串口调试输出使用统一格式（如 `[INFO]`, `[ERROR]`, `[DEBUG]`）
- [ ] 代码缩进一致，使用 4 空格或 Tab 统一风格

### 硬件相关 | Hardware Related

- [ ] I2C 引脚配置了内部上拉或确认外部有上拉电阻（通常 4.7kOhm）
- [ ] I2C 时钟频率配置正确（不超过 400 kHz）
- [ ] GPIO 初始化顺序正确（先初始化 I2C，再操作传感器）
- [ ] 传感器有足够的上电时间后再进行 I2C 通信

---

## 四、自我评估量表 | Self-Assessment Rubric

| 评估维度 | 自我评分 (1-5) | 说明 |
|----------|---------------|------|
| STM32 开发环境搭建 | ___ / 5 | 能否独立完成环境配置和程序下载 |
| I2C 协议理解深度 | ___ / 5 | 能否清楚解释协议细节并调试通信问题 |
| MLX90640 数据手册阅读 | ___ / 5 | 能否从数据手册中找到关键信息 |
| 代码编写规范 | ___ / 5 | 代码是否清晰、有注释、有错误处理 |
| 问题解决能力 | ___ / 5 | 遇到 bug 时能否独立排查和解决 |

**评分标准：**
- 5 分：完全掌握，能独立完成并帮助他人
- 4 分：基本掌握，偶尔回顾资料即可完成
- 3 分：理解概念，实践中有少量困难
- 2 分：概念模糊，需要大量指导
- 1 分：完全不理解，需要重新学习

---

## 五、下周准备 | Preparation for Next Week

### 需要完成的事项 | Items to Complete

1. **确保 MLX90640 原始数据读取正常**
   - 串口能持续输出原始帧数据
   - 数据值在合理范围内（不为全 0 或全 0xFFFF）

2. **阅读以下资料 | Read the following materials**
   - MLX90640 数据手册第 7 节（RAM 内存映射）
   - MLX90640 数据手册第 8 节（EEPROM 内存映射）
   - MLX90640 数据手册第 11 节（温度计算公式）

3. **预习下周内容 | Preview next week's content**
   - Day 6: MLX90640 完整帧数据读取（分页读取机制）
   - Day 7: 温度补偿算法和像素温度计算
   - Day 8-10: USB CDC 虚拟串口通信基础

### 建议准备 | Suggested Preparations

- [ ] 安装 USB CDC 驱动（Windows 10/11 通常自动识别）
- [ ] 准备 USB 数据线（确保支持数据传输，不是仅充电线）
- [ ] 预习 USB CDC 概念：了解虚拟串口的基本原理
- [ ] 准备 Android Studio 开发环境（提前下载安装）

---

## 通过标准 | Pass Criteria

- [ ] STM32 开发环境正常工作，能编译和下载程序
- [ ] GPIO 和 UART 基础实验完成
- [ ] I2C 通信实验成功，能扫描到 MLX90640 设备
- [ ] 能读取 MLX90640 的芯片 ID 和原始数据
- [ ] 代码有基本注释和错误处理
- [ ] 能回答上述技术问题中的至少 5 个

**总分 >= 60 分为通过 | Total score >= 60 is passing**

---

*最后更新：2026-05-27 | Last updated: 2026-05-27*
