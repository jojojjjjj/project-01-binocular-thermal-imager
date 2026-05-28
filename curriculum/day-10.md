# Day 10: 固件集成与测试 | Firmware Integration & Testing

## 学习目标 | Learning Objectives

- 将 I2C 读取、温度计算、USB 发送三大模块整合为完整固件
- 实现稳定的固件主循环
- 掌握嵌入式系统的调试技巧
- 完成 Phase 3 里程碑检查
- 为 Android APP 开发准备好稳定的固件基础

## 前置准备 | Prerequisites

- [ ] Day 9 USB 数据传输成功，Python 能解析温度帧
- [ ] Phase 2 的温度计算代码完整
- [ ] 串口调试工具可用

## 为什么学这个？| Why This Matters

系统集成是嵌入式开发中最具挑战性的环节。各个模块独立工作时可能都很正常，但组合在一起时往往会暴露出时序冲突、资源竞争、内存不足等问题。今天的集成工作，将锻炼你从"模块开发者"向"系统工程师"过渡的能力。一个好的固件架构，能让后续的 APP 开发事半功倍。

## 今日任务 | Today's Tasks

### Task 1: 整合固件代码 (estimated 60 minutes)

**目标：** 将所有模块整合到一个工程中，确保编译通过

**步骤：**

1. **创建最终工程**
   - 工程名：`ThermalEyes_Firmware`
   - CubeMX 配置：
     * I2C1: 400kHz Fast Mode, PB6/PB7
     * USART1: 115200, PA9/PA10（调试用）
     * USB_OTG_FS: Device_Only, CDC
     * GPIO: PC13 LED 输出
     * Clock: HCLK=100MHz, USB=48MHz

2. **组织代码文件**
   ```
   Core/Src/
   ├── main.c              -- 主程序入口
   ├── mlx90640.c          -- MLX90640 驱动（新建）
   ├── thermal_frame.c     -- 温度帧打包（新建）
   ├── usb_transport.c     -- USB 传输（新建）
   └── ...
   Core/Inc/
   ├── mlx90640.h          -- MLX90640 头文件
   ├── thermal_frame.h     -- 温度帧头文件
   ├── usb_transport.h     -- USB 传输头文件
   └── ...
   ```

3. **将之前写的代码按模块整理**
   - `mlx90640.c/h`：I2C 读写、EEPROM 校准、帧读取、温度计算、坏像素补偿
   - `thermal_frame.c/h`：帧打包、CRC16 校验
   - `usb_transport.c/h`：USB CDC 发送封装

4. **编写主程序**
   ```c
   /* main.c */
   #include "mlx90640.h"
   #include "thermal_frame.h"
   #include "usb_transport.h"

   int main(void)
   {
       HAL_Init();
       SystemClock_Config();
       MX_GPIO_Init();
       MX_I2C1_Init();
       MX_USART1_UART_Init();
       MX_USB_DEVICE_Init();

       /* USER CODE BEGIN 2 */
       printf("\r\n=== ThermalEyes Firmware v1.0 ===\r\n");

       // 初始化 MLX90640
       if (MLX90640_Init() != HAL_OK) {
           printf("FATAL: MLX90640 init failed!\r\n");
           Error_Handler();
       }
       printf("MLX90640 initialized.\r\n");

       // 等待 USB 枚举完成
       HAL_Delay(1000);
       printf("USB CDC ready.\r\n");
       printf("Starting main loop...\r\n");

       /* USER CODE END 2 */

       while (1)
       {
           /* USER CODE BEGIN 3 */
           // 1. 读取传感器数据
           if (MLX90640_ReadBothSubpages() == HAL_OK) {

               // 2. 计算温度
               MLX90640_CalculateTemperatures(0.95f);

               // 3. 数据后处理
               MLX90640_CompensateBadPixels();

               // 4. 打包数据帧
               PackThermalFrame();

               // 5. 通过 USB 发送
               SendThermalFrame();

               // 6. LED 指示
               HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
           }

           // 控制帧率
           HAL_Delay(50);  // 约 10-15 FPS
           /* USER CODE END 3 */
       }
   }
   ```

**预期结果：**
- 所有代码整合到一个工程中
- 编译无错误
- 代码结构清晰

**常见问题：**

**Q: 编译时提示重复定义？**
A: 检查头文件是否使用了 `#ifndef` 保护。确保每个 `.h` 文件都有：
```c
#ifndef __MLX90640_H
#define __MLX90640_H
// ...
#endif
```

**Q: 链接错误找不到函数？**
A: 确保所有 `.c` 文件都在工程中（CubeIDE 通常自动包含 Src 目录下的所有 .c 文件）。

---

### Task 2: 添加错误处理和状态机 (estimated 60 minutes)

**目标：** 为主循环添加完善的错误处理和状态管理

**步骤：**

1. **定义系统状态**
   ```c
   typedef enum {
       STATE_INIT,        // 初始化
       STATE_IDLE,        // 空闲等待
       STATE_READING,     // 读取传感器
       STATE_PROCESSING,  // 数据处理
       STATE_SENDING,     // USB 发送
       STATE_ERROR        // 错误状态
   } SystemState;

   SystemState current_state = STATE_INIT;
   uint32_t error_count = 0;
   uint32_t frame_count = 0;
   ```

2. **实现状态机**
   ```c
   while (1)
   {
       switch (current_state)
       {
       case STATE_INIT:
           if (MLX90640_Init() == HAL_OK) {
               printf("Sensor init OK\r\n");
               current_state = STATE_IDLE;
           } else {
               printf("Sensor init FAILED, retrying...\r\n");
               HAL_Delay(1000);
           }
           break;

       case STATE_IDLE:
           // 等待新数据就绪
           current_state = STATE_READING;
           break;

       case STATE_READING:
           if (MLX90640_ReadBothSubpages() == HAL_OK) {
               current_state = STATE_PROCESSING;
           } else {
               error_count++;
               if (error_count > 10) {
                   current_state = STATE_ERROR;
               } else {
                   current_state = STATE_IDLE;
               }
           }
           break;

       case STATE_PROCESSING:
           MLX90640_CalculateTemperatures(0.95f);
           MLX90640_CompensateBadPixels();
           PackThermalFrame();
           current_state = STATE_SENDING;
           break;

       case STATE_SENDING:
           if (SendThermalFrame() == 0) {
               frame_count++;
               HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
           } else {
               error_count++;
           }
           current_state = STATE_IDLE;
           HAL_Delay(50);
           break;

       case STATE_ERROR:
           // 快闪 LED 表示错误
           HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
           printf("ERROR STATE! Errors: %lu, Frames: %lu\r\n",
                  error_count, frame_count);
           HAL_Delay(200);

           // 尝试恢复
           if (MLX90640_Init() == HAL_OK) {
               error_count = 0;
               current_state = STATE_IDLE;
           }
           break;
       }

       // 定期输出统计信息
       static uint32_t last_stats = 0;
       if (HAL_GetTick() - last_stats > 10000) {  // 每 10 秒
           printf("Stats: frames=%lu, errors=%lu\r\n", frame_count, error_count);
           last_stats = HAL_GetTick();
       }
   }
   ```

3. **添加看门狗（可选但推荐）**
   - 在 CubeMX 中启用 IWDG（独立看门狗）
   - 在主循环中定期喂狗
   - 看门狗超时时自动复位系统

**预期结果：**
- 状态机运行稳定
- 错误时自动恢复
- 有统计信息输出

---

### Task 3: 综合测试 (estimated 60 minutes)

**目标：** 对完整固件进行全面测试

**步骤：**

1. **功能测试**
   - 测试项：
     * [ ] 上电后正常初始化
     * [ ] USB 枚举成功
     * [ ] 温度数据正确
     * [ ] 帧发送稳定
     * [ ] LED 正常闪烁
     * [ ] 手掌测试温度变化

2. **稳定性测试**
   - 连续运行 10 分钟，观察是否有异常
   - 检查错误计数是否增长
   - 记录帧率（每秒发送几帧）

3. **边缘条件测试**
   - 拔掉传感器再插回，观察恢复行为
   - 快速插拔 USB，观察重连
   - 覆盖传感器（模拟极端温度），观察数据

4. **性能测试**
   ```c
   // 在主循环中测量帧率
   static uint32_t fps_counter = 0;
   static uint32_t fps_timer = 0;

   fps_counter++;
   if (HAL_GetTick() - fps_timer >= 1000) {
       printf("FPS: %lu\r\n", fps_counter);
       fps_counter = 0;
       fps_timer = HAL_GetTick();
   }
   ```

**预期结果：**
- 所有功能测试通过
- 稳定运行 10+ 分钟无崩溃
- 帧率在 5-15 FPS 范围

**常见问题：**

**Q: 运行一段时间后帧率下降？**
A: 可能是 USB 发送阻塞。检查 USB 缓冲区是否溢出。增加帧间延时。

**Q: 偶尔出现读取错误？**
A: I2C 通信有时会被中断。确保 I2C 操作不会被其他中断打断。可以临时关闭中断。

---

### Task 4: Phase 3 里程碑检查 (estimated 30 minutes)

**目标：** 完成 Phase 3 的所有验证

**步骤：**

1. **Phase 3 检查清单**
   - [ ] USB CDC 配置正确，电脑能识别 COM 口
   - [ ] 数据帧格式完整（帧头、数据、校验、帧尾）
   - [ ] Python 脚本能正确解析温度帧
   - [ ] 手掌测试能看到温度变化
   - [ ] 固件运行稳定（10 分钟无崩溃）
   - [ ] 错误处理和恢复机制工作

2. **准备 APP 开发的接口文档**
   - 整理以下信息供 Day 11 使用：
     * USB 设备信息（VID/PID）
     * 数据帧格式（帧头、长度、数据编码、校验）
     * 波特率：不适用（USB CDC 忽略波特率）
     * 帧率：约 X FPS

3. **备份固件代码**
   - 将工程提交到 Git
   - 确保代码有清晰的注释

**预期结果：**
- Phase 3 全部通过
- 固件稳定可用
- APP 开发所需的所有接口信息已整理

---

## 今日作业 | Homework

1. **固件集成完成**（必须）
   - 所有模块整合完成
   - 状态机运行正常
   - 提交完整的工程代码

2. **Phase 3 总结报告**（必须）
   - 描述固件的整体架构
   - 记录测试结果（功能、稳定性、性能）
   - 列出已知问题和解决方案

3. **接口文档**（必须）
   - 写一份面向 APP 开发的接口说明
   - 包含数据格式、通信参数、示例

4. **Git 提交**（推荐）
   - 将最终固件代码提交到 Git 仓库
   - 使用清晰的 commit message

## 明日预告 | Tomorrow's Preview

明天我们进入 Phase 4 -- Android APP 开发！你将安装 Android Studio，配置 NDK 和 OpenCV，创建第一个 Android 项目。这是一个全新的技术领域，做好准备！

## 参考资源 | References

- **开源固件 thermal_bridge**：https://github.com/colourfate/thermal_bridge
- **STM32 状态机设计模式**：https://www.embedded.com/state-machines-for-embedded-systems/
- **STM32 看门狗配置**：https://deepbluembedded.com/stm32-iwdg-tutorial/
- **嵌入式系统测试方法**：https://www.eetimes.com/embedded-system-testing-strategies/

---

*预计完成时间：6-7 小时*
*Estimated completion time: 6-7 hours*
