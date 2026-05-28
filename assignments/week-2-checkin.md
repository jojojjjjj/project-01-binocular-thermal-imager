# 第二周检查点 | Week 2 Check-in

## 概述 | Overview

第二周检查点评估学员在第 6-10 天的学习成果，重点检查 MLX90640 完整驱动开发、温度数据计算、USB CDC 通信实现以及固件集成。

> Week 2 check-in evaluates learning outcomes from Days 6-10, focusing on complete MLX90640 driver development, temperature data calculation, USB CDC communication implementation, and firmware integration.

---

## 检查时间 | Check-in Time

**Day 10 下午 | Day 10 Afternoon**

---

## 一、进度检查项 | Progress Check Items

### Day 6: MLX90640 完整帧数据读取 | Full Frame Data Reading

- [ ] 理解 MLX90640 的分页读取机制（Page 0 和 Page 1）
- [ ] 理解状态寄存器（0x8000）中数据就绪标志位的含义
- [ ] 能够正确读取完整的一帧数据（32x24 = 768 个像素）
- [ ] 实现了等待数据就绪的轮询机制或中断机制
- [ ] 帧数据读取速度能满足目标帧率（至少 4 Hz）

### Day 7: 温度补偿算法 | Temperature Compensation Algorithm

- [ ] 从 EEPROM 读取所有校准参数（Vdd coefficients, Ta coefficients, emissivity 等）
- [ ] 正确实现 Vdd（电源电压）补偿计算
- [ ] 正确实现 Ta（环境温度）补偿计算
- [ ] 实现像素温度到摄氏度的转换公式
- [ ] 计算出的温度值在合理范围内（与实际环境温度对比误差 < 2 度）
- [ ] 理解发射率（emissivity）的概念及其对温度测量的影响

### Day 8: USB CDC 基础 | USB CDC Basics

- [ ] 理解 USB CDC（Communications Device Class）虚拟串口的概念
- [ ] 在 STM32CubeIDE 中正确配置 USB 外设为 CDC 模式
- [ ] 能够通过 USB CDC 发送字符串到 PC
- [ ] 能够通过 USB CDC 接收 PC 发送的数据
- [ ] 理解 USB 端点（Endpoint）的概念和配置
- [ ] USB 枚举成功，PC 能识别为 COM 端口

### Day 9: 数据传输协议 | Data Transmission Protocol

- [ ] 设计了简单的二进制数据传输协议（帧头 + 数据 + 校验）
- [ ] 实现了温度数据的打包发送（将 float 或 int16 温度数据序列化）
- [ ] 实现了双传感器数据的时间同步（两个 MLX90640 数据交替或同时发送）
- [ ] 添加了帧序号用于检测丢包
- [ ] PC 端使用 Python 脚本能正确接收和解析传感器数据
- [ ] 数据传输速率满足实时显示需求

### Day 10: 固件集成 | Firmware Integration

- [ ] MLX90640 驱动、USB CDC 和主循环正确集成
- [ ] 系统能持续稳定运行至少 5 分钟不崩溃
- [ ] 双目传感器数据都能正确读取和传输
- [ ] 实现了基本的错误恢复机制（I2C 通信失败时重试或复位）
- [ ] 代码使用 FreeRTOS 或主循环 + 状态机合理组织任务调度
- [ ] 固件整体结构清晰，模块间耦合度低

---

## 二、技术理解验证问题 | Technical Verification Questions

### MLX90640 驱动问题 | MLX90640 Driver Questions

**Q1.** MLX90640 的 768 个像素数据存储在哪些 RAM 地址中？为什么需要分两页读取？每页包含多少个像素数据？

> Which RAM addresses store the 768 pixel data of the MLX90640? Why do we need to read in two pages? How many pixel data does each page contain?

**参考答案 | Reference Answer:**
MLX90640 的像素数据存储在 RAM 地址 0x0400-0x063F（Page 0，IR 数据前半部分 512 个半字）和 0x0700-0x07FF（Page 1，IR 数据后半部分 256 个半字）。实际上，768 个像素的 IR 原始数据分布在 Page 0 的 0x0400-0x05FF（768 个半字）。两页机制与传感器的内部 ADC 采样策略有关：传感器交替在两页之间进行测量，每一"子帧"只更新一半的数据。读取时需要检查状态寄存器确认哪一页有新数据，然后读取对应页面。

---

**Q2.** 在计算 MLX90640 的像素温度时，为什么需要先读取 Vdd 和 Ta 补偿数据？如果不做补偿会怎样？

> Why do we need to read Vdd and Ta compensation data before calculating pixel temperatures? What happens without compensation?

**参考答案 | Reference Answer:**
Vdd 补偿是为了消除电源电压波动对 ADC 测量精度的影响。Ta 补偿是为了消除传感器自身温度对红外测量的干扰。如果不做补偿，温度读数会出现明显偏差（可能数度到十几度的误差），不同环境温度下测量结果不一致，温度数据不可用。

---

**Q3.** MLX90640 的发射率（emissivity）默认值是多少？如果测量金属表面的温度，需要怎样调整？为什么？

> What is the default emissivity value of the MLX90640? How should it be adjusted for measuring metal surfaces? Why?

**参考答案 | Reference Answer:**
默认发射率为 1.0（完美黑体）。金属表面的发射率通常很低（0.05-0.3），需要将 MLX90640 的发射率参数调整为实际值，否则测得的温度会远低于实际温度。这是因为低发射率表面反射的环境红外辐射会干扰测量。实际操作中，可以在金属表面贴黑色绝缘胶带（发射率接近 1.0）来简化测量。

---

### USB CDC 问题 | USB CDC Questions

**Q4.** USB CDC 和 UART 串口有什么区别？在本项目中为什么选择 USB CDC 而不是 UART 来传输热成像数据？

> What is the difference between USB CDC and UART? Why choose USB CDC instead of UART for transmitting thermal imaging data in this project?

**参考答案 | Reference Answer:**
- USB CDC 是通过 USB 协议模拟串口通信，实际传输速率远高于传统 UART（USB 2.0 Full Speed 理论可达 12 Mbps，而 UART 通常最高 115200 bps = 约 115 KB/s）
- 本项目需要传输 768 个像素 x 2 字节 x 2 个传感器 x 至少 4 Hz = 约 123 KB/s 的数据量，UART 无法满足此带宽需求
- USB CDC 使用手机 USB 接口即可通信，不需要额外的 USB 转 TTL 适配器
- USB CDC 自带 CRC 校验和纠错机制，数据传输更可靠

---

**Q5.** 在 STM32 中配置 USB CDC 时，需要配置哪些关键参数？USB 中断的优先级应该如何设置？

> What are the key parameters to configure when setting up USB CDC on STM32? How should USB interrupt priorities be set?

**参考答案 | Reference Answer:**
关键配置参数：
- USB 模式：Device（从设备模式）
- USB 类：CDC（Communications Device Class）
- 端点配置：至少需要 3 个端点（Control Endpoint 0, CDC Data IN/OUT, CDC Command IN）
- 包大小（Max Packet Size）：通常 64 字节（Full Speed）
- VID/PID：可使用默认值或自定义

中断优先级：USB 中断优先级应高于普通 I2C 操作，通常设置为抢占优先级 0-1（较高优先级），确保不会因为 I2C 操作阻塞而丢失 USB 数据。

---

**Q6.** 你设计的数据传输协议是什么样的？为什么需要帧头和校验和？

> What does your data transmission protocol look like? Why do we need frame headers and checksums?

**参考答案 | Reference Answer:**
典型的协议格式：
```
[0xAA] [0x55] [帧序号 1B] [传感器ID 1B] [数据长度 2B] [温度数据 ... NB] [CRC8 1B]
```
- 帧头（0xAA 0x55）：用于接收端识别数据帧的起始位置，防止数据错位
- 帧序号：检测丢包，接收端可以发现丢失的帧
- 传感器 ID：区分左目和右目传感器数据
- CRC 校验：检测传输错误，确保数据完整性

---

### 固件集成问题 | Firmware Integration Questions

**Q7.** 如果两个 MLX90640 传感器共用一条 I2C 总线，在读取数据时需要注意什么？如果两个传感器的数据更新周期不同步会怎样？

> If two MLX90640 sensors share one I2C bus, what should be noted when reading data? What happens if the two sensors have different data update cycles?

**参考答案 | Reference Answer:**
注意事项：
- I2C 总线同一时刻只能有一个主设备发起通信，需要确保对两个传感器的读取操作不会同时发起
- 读取操作需要等待各自的数据就绪标志位，不能假设两个传感器同步更新
- 如果两个传感器更新周期不同步，需要分别跟踪每个传感器的状态，在各自数据就绪时分别读取

不同步的影响：如果强行同步读取，可能读到其中某个传感器的旧数据，导致双目画面出现时间错位。解决方案是使用双缓冲或为每个传感器独立管理读取时机。

---

**Q8.** 在主循环中同时处理 I2C 传感器读取和 USB 数据发送，可能出现什么问题？如何解决？

> What problems might occur when simultaneously handling I2C sensor reading and USB data transmission in the main loop? How to solve them?

**参考答案 | Reference Answer:**
可能的问题：
1. I2C 读取是阻塞操作，可能导致 USB 数据发送不及时，造成 USB 缓冲区溢出
2. USB 发送占用 CPU 时间，可能导致错过传感器数据就绪窗口
3. 主循环执行时间不确定，无法保证稳定的帧率

解决方案：
- 使用 DMA 进行 I2C 读取（非阻塞方式）
- 使用中断或 DMA 进行 USB 发送
- 使用 FreeRTOS 创建独立的传感器读取任务和 USB 发送任务，通过消息队列传递数据
- 或者使用状态机架构，在主循环中通过状态标志协调各模块

---

## 三、代码审查清单 | Code Review Checklist

### MLX90640 驱动模块 | MLX90640 Driver Module

- [ ] 驱动代码独立为 `.c/.h` 文件，与主程序解耦
- [ ] 初始化函数检查传感器连接状态（读取芯片 ID 并验证）
- [ ] EEPROM 校准参数在初始化时一次性读取并缓存
- [ ] 温度计算函数参数清晰，返回值含义明确
- [ ] 帧数据读取有超时保护（不会无限等待）
- [ ] 双传感器各有独立的驱动实例或上下文结构体

### USB 通信模块 | USB Communication Module

- [ ] USB CDC 初始化代码正确，不影响其他外设
- [ ] 发送函数有缓冲区满检测，不会覆盖未发送的数据
- [ ] 接收函数有解析状态机，能正确处理不完整或损坏的帧
- [ ] USB 连接/断开事件有处理逻辑
- [ ] 协议定义在头文件中，方便 Android 端参考

### 主程序集成 | Main Program Integration

- [ ] `main()` 函数逻辑清晰，能看出整体工作流程
- [ ] 没有使用 `HAL_Delay()` 进行长时间的忙等待（会阻塞其他任务）
- [ ] 错误处理完善：I2C 错误有重试机制，USB 断开有重连逻辑
- [ ] 全局变量使用合理，有明确的 volatile 声明（中断共享变量）
- [ ] 资源使用合理：RAM 和 Flash 使用率在 MCU 限制范围内

### 代码规范 | Code Standards

- [ ] 函数命名有意义（如 `MLX90640_ReadFrame()` 而非 `read_data()`）
- [ ] 宏定义使用大写命名（如 `#define FRAME_HEADER_0 0xAA`）
- [ ] 注释覆盖率 > 50%，关键算法有详细说明
- [ ] 使用 `const` 修饰只读数据，使用 `static` 限制作用域
- [ ] Git 提交记录清晰，每次提交有有意义的 commit message

---

## 四、自我评估量表 | Self-Assessment Rubric

| 评估维度 | 自我评分 (1-5) | 说明 |
|----------|---------------|------|
| MLX90640 帧数据读取 | ___ / 5 | 能否稳定读取完整的 32x24 帧数据 |
| 温度计算准确性 | ___ / 5 | 计算的温度值是否接近实际值 |
| USB CDC 通信实现 | ___ / 5 | 能否稳定发送和接收数据 |
| 数据协议设计 | ___ / 5 | 协议是否可靠、易于解析 |
| 双传感器集成 | ___ / 5 | 两个传感器数据是否都能正确获取 |
| 代码质量和模块化 | ___ / 5 | 代码是否清晰、模块化、有注释 |
| 系统稳定性 | ___ / 5 | 系统能否持续稳定运行 |

**评分标准：**
- 5 分：完全掌握，功能稳定运行，代码质量高
- 4 分：功能完成，偶有小问题，能独立调试
- 3 分：基本功能实现，稳定性和代码质量有待提高
- 2 分：部分功能有问题，需要协助才能完成
- 1 分：未能实现，需要大量指导

---

## 五、下周准备 | Preparation for Next Week

### 需要完成的事项 | Items to Complete

1. **确保固件端数据传输稳定**
   - PC 端 Python 脚本能持续接收双目传感器数据
   - 数据帧无丢失或乱序
   - 连续运行 10 分钟无崩溃

2. **阅读以下资料 | Read the following materials**
   - Android USB Host API 官方文档
   - OpenCV Android SDK 入门教程
   - Android Activity 生命周期概念

3. **预习下周内容 | Preview next week's content**
   - Day 11: Android 项目创建和 USB Host 通信
   - Day 12: OpenCV 热成像伪彩色渲染
   - Day 13: 双目画面合成和 UI 完善

### 提前安装 | Pre-installation

- [ ] Android Studio 最新版本
- [ ] Android NDK（用于 OpenCV native 库）
- [ ] OpenCV Android SDK 4.x
- [ ] 准备 Android 测试设备（开启开发者模式和 USB 调试）

---

## 通过标准 | Pass Criteria

### 基本要求（必须） | Basic Requirements (Mandatory)

- [ ] MLX90640 能读取完整帧数据并通过串口显示
- [ ] 温度计算基本正确（误差 < 3 度）
- [ ] USB CDC 能在 PC 上枚举为 COM 端口
- [ ] 能通过 USB 发送温度数据到 PC
- [ ] 至少一个传感器数据能正确传输
- [ ] 代码有基本注释

### 优秀标准（额外） | Excellence Criteria (Additional)

- [ ] 双传感器数据都能正确读取和传输
- [ ] 数据协议设计合理，有帧头和校验
- [ ] 温度计算误差 < 1 度
- [ ] 代码模块化程度高，错误处理完善
- [ ] 系统能持续稳定运行超过 10 分钟

**总分 >= 70 分为通过，>= 90 分为优秀**

| Total >= 70 is passing, >= 90 is excellent |

---

*最后更新：2026-05-27 | Last updated: 2026-05-27*
