# Day 8: USB CDC 原理与配置 | USB CDC Principles & Configuration

## 学习目标 | Learning Objectives

- 理解 USB 协议的基本概念（设备、端点、描述符）
- 了解 USB CDC 类的用途和工作方式
- 在 CubeMX 中配置 STM32F411 的 USB OTG FS 为 CDC 模式
- 实现 USB 虚拟串口的基本收发功能
- 在电脑上验证 USB CDC 通信

## 前置准备 | Prerequisites

- [ ] Phase 2 全部完成，温度数据稳定输出
- [ ] 开发板通过 USB 连接到电脑
- [ ] 电脑已安装串口驱动（Windows 10+ 通常自动安装）

## 为什么学这个？| Why This Matters

USB 是当今世界最通用的接口标准。你的鼠标、键盘、U 盘、手机充电器，都用 USB。USB CDC（Communications Device Class）让 STM32 可以模拟一个串口设备，这意味着你的传感器可以通过 USB 线直接与手机通信，而不需要额外的转接芯片。理解 USB CDC 的原理，你就掌握了嵌入式设备与计算机/手机通信的核心技术。

## 今日任务 | Today's Tasks

### Task 1: 理解 USB 和 CDC 基础 (estimated 50 minutes)

**目标：** 建立 USB 协议的基本认知，理解 CDC 类的作用

**步骤：**

1. **USB 协议基础概念**
   - **USB 设备**：STM32 作为 USB 设备连接到主机（电脑/手机）
   - **端点（Endpoint）**：USB 通信的"管道"，数据通过端点传输
     * 端点 0：控制端点，用于设备配置
     * 端点 1, 2, 3...：数据端点，用于批量数据传输
   - **描述符（Descriptor）**：描述设备能力的数据结构
     * 设备描述符：告诉主机"我是谁"
     * 配置描述符：告诉主机"我能做什么"
     * 接口描述符：定义具体的功能
   - **枚举（Enumeration）**：主机识别设备的过程

2. **USB CDC 类**
   - CDC（Communications Device Class）是 USB 的一个标准类
   - 功能：模拟串口通信
   - 好处：
     * 操作系统和手机原生支持，不需要额外驱动
     * 简单易用，像操作串口一样操作 USB
     * 速度快（USB 2.0 Full Speed = 12Mbps）
   - CDC 使用两个端点：
     * 一个用于控制/状态
     * 一个用于批量数据传输（Bulk IN / Bulk OUT）

3. **理解数据流**
   ```
   STM32 (USB Device)  <--- USB 线 --->  电脑/手机 (USB Host)

   发送方向（STM32 -> 电脑）:
   1. STM32 调用 CDC_Transmit_FS()
   2. 数据通过 USB Bulk IN 端点发送
   3. 电脑的串口工具接收数据

   接收方向（电脑 -> STM32）:
   1. 电脑通过串口工具发送数据
   2. 数据通过 USB Bulk OUT 端点到达 STM32
   3. STM32 在 CDC 接收回调中处理数据
   ```

**预期结果：**
- 理解 USB 设备、端点、描述符的概念
- 知道 CDC 类的作用和优势
- 理解数据在 USB 上的传输方向

**常见问题：**

**Q: USB CDC 和 UART 串口有什么区别？**
A: 物理层面完全不同（USB 是差分信号，UART 是单端信号），但软件层面看起来一样 -- 都是通过"串口"读写数据。CDC 的优势是速度快、即插即用、不需要额外的串口转接芯片。

---

### Task 2: 在 CubeMX 中配置 USB CDC (estimated 60 minutes)

**目标：** 配置 STM32F411 的 USB 外设为 CDC 模式

**步骤：**

1. **创建新工程（或在现有工程上修改）**
   - 工程名：`ThermalEyes_USB_CDC`
   - 芯片：STM32F411CEU6

2. **配置 USB OTG FS**
   - 在 CubeMX 左侧找到 "Connectivity" -> "USB_OTG_FS"
   - Mode 选择 **Device_Only**（只作为 USB 设备，不需要主机功能）
   - 参数保持默认：
     * Speed: Full Speed (12 Mbps)
     * 其他参数默认

3. **配置 USB Device 类为 CDC**
   - 在 CubeMX 左侧找到 "Middleware" -> "USB_DEVICE"
   - Class For FS IP 选择 **Communication Device Class (CDC)**
   - 参数设置：
     * Packet Size: 64（USB Full Speed 最大包大小）
     * 其他保持默认

4. **配置时钟**
   - USB 需要 48MHz 的精确时钟
   - 进入 Clock Configuration 页面
   - 确认 USB 时钟为 48MHz（CubeMX 通常会自动配置）
   - 如果 USB 时钟不正确，USB 将无法工作

5. **确认引脚配置**
   - USB OTG FS 使用的引脚：
     * PA11: USB_DM（数据负）
     * PA12: USB_DP（数据正）
   - 这些引脚在开发板上已经连接到 USB 接口，不需要额外连线

6. **生成代码**
   - 保存并生成代码
   - 注意：CubeMX 会创建以下新文件：
     * `usb_device.c` -- USB 设备初始化
     * `usbd_cdc_if.c` -- CDC 接口回调函数
     * `usbd_desc.c` -- USB 描述符

**预期结果：**
- CubeMX 成功配置 USB CDC
- 生成的代码包含 USB 和 CDC 相关文件
- 无编译错误

**常见问题：**

**Q: 编译报错 "USB_OTG_FS not found"？**
A: 确认你选择的芯片是 STM32F411CEU6（不是其他型号）。F411 有 USB OTG FS 外设。

**Q: Clock Configuration 中 USB 时钟显示红色？**
A: USB 需要 48MHz 时钟。调整 PLL 配置确保 USB 时钟分频器输出 48MHz。

---

### Task 3: 实现 USB CDC 收发 (estimated 60 minutes)

**目标：** 编写代码通过 USB CDC 发送和接收数据

**步骤：**

1. **理解 CDC 接口文件**
   - 打开 `USB_DEVICE/App/usbd_cdc_if.c`
   - 找到以下关键函数：
     * `CDC_Receive_FS()` -- 接收数据的回调函数
     * `CDC_Transmit_FS()` -- 发送数据的函数
   - 这些函数已经在 CubeMX 生成的代码中

2. **编写 USB 发送测试代码**
   - 在 `main.c` 中添加：

   ```c
   /* USER CODE BEGIN Includes */
   #include "usbd_cdc_if.h"  // CDC 接口头文件
   #include <string.h>
   #include <stdio.h>
   /* USER CODE END Includes */

   // 在 while(1) 中添加发送测试
   /* USER CODE BEGIN 3 */
   char msg[] = "Hello from STM32 USB CDC!\r\n";

   // 通过 USB CDC 发送字符串
   CDC_Transmit_FS((uint8_t*)msg, strlen(msg));

   HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
   HAL_Delay(1000);
   /* USER CODE END 3 */
   ```

3. **编译和烧录**
   - 编译工程
   - 烧录到 STM32
   - **重要：** 烧录后需要重新插拔 USB 线（或按 RESET）
   - 因为 STM32 的 USB 需要重新枚举

4. **在电脑上验证**
   - 打开设备管理器（Windows：右键开始菜单 -> 设备管理器）
   - 展开"端口 (COM 和 LPT)"
   - 应该看到一个新的 COM 口，名称类似 "STM32 Virtual COM Port"
   - 记录 COM 口号（如 COM3）
   - 打开串口工具（PuTTY / Tera Term），连接到该 COM 口
   - 波特率：随意设置（USB CDC 忽略波特率设置）
   - 应该看到每秒输出一行 "Hello from STM32 USB CDC!"

**预期结果：**
- 设备管理器中出现新的 COM 口
- 串口工具中看到周期性输出
- 理解 USB CDC 的基本使用方法

**常见问题：**

**Q: 设备管理器中没有出现 COM 口？**
A:
1. 重新插拔 USB 线
2. 检查 USB 配置是否正确（Device_Only + CDC）
3. 检查 USB 时钟是否为 48MHz
4. 尝试安装 STM32 CDC 驱动（Windows 10+ 通常不需要）
5. 检查 USB 线是否支持数据传输（有些线只能充电）

**Q: COM 口出现了但串口工具无法连接？**
A:
1. 确保没有其他程序占用该 COM 口
2. 关闭其他串口工具
3. 尝试不同的波特率设置

**Q: 串口工具打开了但没有数据？**
A:
1. 确认 `CDC_Transmit_FS()` 的返回值（USBD_OK 表示成功）
2. 检查 `MX_USB_DEVICE_Init()` 是否在 main() 中被调用
3. 在代码中添加 LED 闪烁来确认程序在运行

---

### Task 4: 实现 USB CDC 接收功能 (estimated 30 minutes)

**目标：** 编写代码接收电脑通过 USB CDC 发送的数据

**步骤：**

1. **修改 CDC 接收回调函数**
   - 打开 `USB_DEVICE/App/usbd_cdc_if.c`
   - 找到 `CDC_Receive_FS()` 函数
   - 在 `USER CODE BEGIN 6` 区域添加处理代码：

   ```c
   /* USER CODE BEGIN 6 */
   // 将接收到的数据回显（Echo）
   uint8_t echo_buf[64];
   uint16_t len = hRxLenCloned;  // 接收到的数据长度

   if (len > 0 && len < 64) {
       // 回显
       CDC_Transmit_FS(Buf, len);
   }

   // 重新启动接收
   USBD_CDC_SetRxBuffer(&hUsbDeviceFS, &Buf[0]);
   USBD_CDC_ReceivePacket(&hUsbDeviceFS);
   /* USER CODE END 6 */
   ```

   **注意：** 具体的回调函数参数和接收缓冲区管理方式可能因 CubeMX 版本而异。请参考生成的代码中的注释。

2. **测试收发**
   - 在串口工具中输入文字并发送
   - 应该看到 STM32 将发送的内容回显回来

**预期结果：**
- 能接收电脑发送的数据
- 实现简单的回显功能

---

## 今日作业 | Homework

1. **USB CDC 收发测试**（必须）
   - 成功配置 USB CDC
   - 实现字符串发送和回显
   - 提交设备管理器截图和串口通信截图

2. **USB 协议笔记**（必须）
   - 解释以下概念：端点、描述符、枚举
   - 画出 USB CDC 的数据流图
   - 说明 USB CDC 相比 UART 的优势

3. **性能测试**（推荐）
   - 测量 USB CDC 的最大传输速率
   - 对比 USB CDC 和 UART 的速度差异
   - 记录测试方法和结果

4. **挑战任务**（可选）
   - 实现一个简单的命令解析器：通过串口发送命令控制 LED
   - 例如：发送 "ON" 点亮 LED，发送 "OFF" 关闭 LED

## 明日预告 | Tomorrow's Preview

明天我们将把温度数据通过 USB CDC 发送到电脑！你将定义温度帧的数据格式，编写打包和发送代码，在电脑串口工具中看到实时的温度数据流。

## 参考资源 | References

- **USB 基础教程**：https://www.beyondlogic.org/usbnutshell/usb1.shtml
- **STM32 USB CDC 教程**：https://deepbluembedded.com/stm32-usb-cdc-virtual-serial-port-tutorial/
- **USB CDC 类规范**：https://www.usb.org/document-library/class-definitions-communication-devices-12
- **STM32CubeIDE USB 配置**：ST 官方 UM1884 文档

---

*预计完成时间：5-6 小时*
*Estimated completion time: 5-6 hours*
