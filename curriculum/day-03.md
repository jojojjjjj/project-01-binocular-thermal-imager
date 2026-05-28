# Day 3: I2C 通信原理与实践 | I2C Communication Principles & Practice

## 学习目标 | Learning Objectives

- 理解 I2C 通信协议的工作原理（起始、地址、数据、停止）
- 掌握 STM32 I2C 外设的 CubeMX 配置方法
- 实现 I2C 总线扫描，检测连接的设备
- 成功扫描到 MLX90640 传感器（地址 0x33）
- 理解 I2C 通信中常见的问题和调试方法

## 前置准备 | Prerequisites

- [ ] Day 2 LED 闪烁实验成功完成
- [ ] 已准备好 MLX90640 传感器模块
- [ ] 已准备好 4 根杜邦线（母对母）
- [ ] 已阅读 MLX90640 数据手册的前两页

## 为什么学这个？| Why This Matters

I2C（Inter-Integrated Circuit）是电子世界中最常用的通信协议之一。从你手机里的陀螺仪、到笔记本电脑的电池管理芯片、再到我们的 MLX90640 热传感器，都使用 I2C 与主控芯片通信。理解 I2C 协议，意味着你能与数百种不同的传感器和外围芯片"对话"。今天学的 I2C 扫描技巧，是硬件调试中最实用的工具之一 -- 当传感器"没反应"时，第一步就是用 I2C 扫描看看它是否"在线"。

## 今日任务 | Today's Tasks

### Task 1: 理解 I2C 协议原理 (estimated 50 minutes)

**目标：** 从信号层面理解 I2C 通信过程，能画出一次完整 I2C 读操作的时序图

**步骤：**

1. **学习 I2C 基本概念**
   - I2C 只需要两根信号线：
     * **SCL（Serial Clock）**：时钟线，由主设备（Master/STM32）控制
     * **SDA（Serial Data）**：数据线，双向传输
   - 每个设备有唯一的 7 位地址（如 MLX90640 的地址是 0x33）
   - 通信由主设备发起，从设备响应
   - I2C 是"开漏"（Open-Drain）输出，需要外部上拉电阻
   - 常见速率：标准模式 100kHz，快速模式 400kHz

2. **理解一次 I2C 写操作的时序**
   ```
   主设备发出:
   [起始位] [7位设备地址 + W(0)] [ACK] [8位寄存器地址] [ACK] [8位数据] [ACK] [停止位]

   示例：向 MLX90640 的寄存器 0x800D 写入数据 0x01
   START -> 0x66(地址0x33<<1|W=0) -> ACK -> 0x0D(低8位) -> ACK -> 0x80(高8位) -> ACK -> 0x01 -> ACK -> STOP
   ```
   - 注意：MLX90640 使用 16 位寄存器地址，所以地址分两次发送
   - ACK（应答）：每发送完一个字节，接收方拉低 SDA 表示"收到了"

3. **理解一次 I2C 读操作的时序**
   ```
   主设备发出:
   [起始位] [设备地址+W] [ACK] [寄存器地址(16位)] [ACK]
   [重新起始位] [设备地址+R] [ACK] [读取数据] [NACK] [停止位]
   ```
   - 读操作分两步：先"告诉"从设备要读哪个寄存器，然后再读取数据
   - NACK（非应答）：最后一个字节后主设备发 NACK，表示"读完了"

4. **画出时序图**
   - 在纸上画出一次 I2C 写操作的 SCL 和 SDA 波形
   - 标注起始位、地址、数据、ACK、停止位的位置
   - 这将帮助你理解后面的代码

**预期结果：**
- 能画出 I2C 通信的基本时序图
- 理解主从设备、地址、ACK 的概念
- 知道 MLX90640 的 I2C 地址是 0x33

**常见问题：**

**Q: 为什么 MLX90640 用 16 位寄存器地址？**
A: 因为 MLX90640 有很多寄存器，8 位地址（最多 256 个）不够用。16 位地址可以寻址 65536 个寄存器位置。

**Q: I2C 地址 0x33 和 0x66 是什么关系？**
A: 0x33 是 7 位地址。在通信时，地址左移 1 位，最低位表示读/写方向。0x33 << 1 = 0x66（写方向），0x33 << 1 | 1 = 0x67（读方向）。HAL 库会自动处理这个移位。

---

### Task 2: 连接 MLX90640 传感器 (estimated 30 minutes)

**目标：** 正确连接 MLX90640 到 STM32 开发板

**步骤：**

1. **确认接线关系**
   - 仔细查看接线表：

   | MLX90640 引脚 | STM32F411 引脚 | 说明 |
   |---------------|---------------|------|
   | VIN (VCC)     | 3.3V          | 供电（注意：确认模块支持 3.3V） |
   | GND           | GND           | 地线 |
   | SDA           | PB7 (I2C1_SDA)| I2C 数据线 |
   | SCL           | PB6 (I2C1_SCL)| I2C 时钟线 |

2. **连接硬件**
   - **先断开 STM32 的 USB 线**（安全第一！）
   - 用杜邦线按上表连接 MLX90640 和 STM32
   - 再次检查接线：VCC 接 VCC，GND 接 GND，不要接反
   - **特别注意：** SDA 和 SCL 不要接反，否则通信会失败
   - 确认连接无误后，再插上 USB 线给 STM32 供电

3. **上电检查**
   - 给 STM32 上电后，用手轻触 MLX90640 的金属盖
   - 如果感觉不到明显发热，说明供电正常
   - 如果感觉发烫，立即断电，检查接线

**预期结果：**
- MLX90640 正确连接到 STM32
- 上电后传感器无异常发热
- 准备好进行 I2C 通信测试

**常见问题：**

**Q: 模块上标的是 SDA/SCL 还是其他？**
A: 不同厂商的模块标注可能不同。常见的标注方式有：
- SDA/SCL（最标准）
- DATA/CLK
- 有时标注为 I2C_SDA/I2C_SCL
如果不确定，查阅你购买店铺的商品详情页。

**Q: 接错了会不会烧坏？**
A: VCC 和 GND 接反可能烧坏传感器，所以接线时一定要仔细。SDA 和 SCL 接反一般不会烧坏，但通信会失败。

---

### Task 3: 配置 I2C 并实现总线扫描 (estimated 75 minutes)

**目标：** 配置 STM32 的 I2C1 外设，编写代码扫描总线上的所有设备

**步骤：**

1. **在 CubeMX 中配置 I2C1**
   - 打开 Day 2 的工程（或创建新工程 `ThermalEyes_I2C`）
   - 在 CubeMX 中：
     * 点击左侧 "Connectivity" -> "I2C1"
     * Mode 选择 "I2C"
     * 参数设置（Parameter Settings）：
       - I2C Speed Mode: Fast Mode (400 kHz)
       - I2C Clock Speed: 400000
     * 确认引脚映射：PB6=SCL, PB7=SDA
   - 保存并重新生成代码

2. **编写 I2C 扫描代码**
   - 打开 `main.c`
   - 在 `/* USER CODE BEGIN Includes */` 区域添加：

   ```c
   #include <stdio.h>
   #include <string.h>
   ```

   - 在 `/* USER CODE BEGIN PV */` 区域添加变量：

   ```c
   uint8_t i2c_scan_buf[1];
   char uart_buf[64];
   ```

   - 在 `main()` 函数的 `while(1)` 循环中替换为：

   ```c
   /* USER CODE BEGIN 3 */
   // I2C 总线扫描
   HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);  // LED 闪烁表示正在扫描

   uint8_t found = 0;
   for (uint8_t addr = 1; addr < 127; addr++) {
       // 尝试向该地址发送数据，看是否有 ACK
       HAL_StatusTypeDef result = HAL_I2C_IsDeviceReady(
           &hi2c1,
           addr << 1,   // HAL 库需要 8 位地址（左移 1 位）
           3,           // 尝试 3 次
           5            // 超时 5ms
       );

       if (result == HAL_OK) {
           found++;
       }
   }

   // 根据 LED 闪烁模式显示结果
   if (found > 0) {
       // 快闪 3 次：找到设备
       for (int i = 0; i < 6; i++) {
           HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
           HAL_Delay(100);
       }
   } else {
       // 慢闪 1 次：没找到设备
       HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
       HAL_Delay(1000);
       HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
       HAL_Delay(1000);
   }

   HAL_Delay(2000);  // 等待 2 秒再扫描一次
   /* USER CODE END 3 */
   ```

3. **编译、烧录、测试**
   - 编译并烧录到开发板
   - 观察 LED 闪烁模式
   - 如果快闪 3 次：恭喜，MLX90640 被成功检测到！
   - 如果慢闪 1 次：没有找到设备，进入故障排查

**预期结果：**
- I2C 扫描成功检测到 MLX90640（地址 0x33）
- LED 显示找到设备（快闪模式）
- 理解 I2C 扫描的原理和代码逻辑

**常见问题：**

**Q: 扫描不到任何设备？**
A: 按以下步骤排查：
1. 检查接线（VCC, GND, SDA, SCL 四根线）
2. 确认 SDA 和 SCL 没有接反
3. 检查 I2C 引脚配置是否正确（PB6=SCL, PB7=SDA）
4. 确认传感器模块是否支持 3.3V 供电
5. 检查杜邦线是否接触良好（换一根试试）

**Q: 扫描到多个设备？**
A: 如果你连接了其他 I2C 设备，可能会扫描到多个地址。MLX90640 的默认地址是 0x33。如果看到其他地址，检查是否有其他 I2C 设备连在总线上。

**Q: 扫描结果不稳定？**
A:
1. 检查接触是否良好
2. 尝试降低 I2C 速率到 100kHz（Standard Mode）
3. 确认上拉电阻是否存在（有些模块自带，有些需要外接）

---

### Task 4: 添加串口调试输出 (estimated 40 minutes)

**目标：** 配置 USART 输出，将 I2C 扫描结果通过串口发送到电脑显示

**步骤：**

1. **在 CubeMX 中配置 USART1**
   - 在 CubeMX 中点击 Connectivity -> USART1
   - Mode: Asynchronous（异步模式）
   - 参数设置：
     * Baud Rate: 115200
     * Word Length: 8 Bits
     * Stop Bits: 1
     * Parity: None
   - 确认引脚：PA9=TX, PA10=RX

2. **重定向 printf 到串口**
   - 在 `main.c` 中添加以下代码（在 `/* USER CODE BEGIN 0 */` 区域）：

   ```c
   // 重定向 fputc，使 printf 输出到 USART
   #ifdef __GNUC__
   #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
   #else
   #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
   #endif

   PUTCHAR_PROTOTYPE
   {
       HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
       return ch;
   }
   ```

   - 还需要添加编译选项：在工程属性中，MCU Settings 勾选 "Use float with printf"

3. **修改 I2C 扫描代码，添加串口输出**

   ```c
   /* USER CODE BEGIN 3 */
   printf("\r\n=== I2C Bus Scan ===\r\n");

   uint8_t found = 0;
   for (uint8_t addr = 1; addr < 127; addr++) {
       HAL_StatusTypeDef result = HAL_I2C_IsDeviceReady(
           &hi2c1, addr << 1, 3, 5
       );

       if (result == HAL_OK) {
           printf("  Device found at address: 0x%02X\r\n", addr);
           found++;
       }
   }

   printf("Scan complete. %d device(s) found.\r\n", found);

   if (found > 0) {
       for (int i = 0; i < 6; i++) {
           HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
           HAL_Delay(100);
       }
   }

   HAL_Delay(3000);
   /* USER CODE END 3 */
   ```

4. **使用串口工具查看输出**
   - 烧录程序后，打开串口调试工具（PuTTY / Tera Term / Arduino 串口监视器）
   - 设置：COM 端口（查看设备管理器确认），波特率 115200
   - 应该看到每 3 秒输出一次扫描结果

**预期结果：**
- 串口工具中显示扫描结果
- 能看到 MLX90640 的地址 0x33
- 理解串口调试的基本方法

**常见问题：**

**Q: 串口没有输出？**
A:
1. 确认 USART1 引脚配置正确（PA9=TX）
2. 确认串口工具选择了正确的 COM 端口
3. 确认波特率一致（115200）
4. 如果使用 USB-TTL 转换器，确认 TX/RX 交叉连接

**Q: printf 中文乱码？**
A: 确保串口工具和源代码文件使用相同的编码（UTF-8）。或者避免在 printf 中使用中文。

---

## 今日作业 | Homework

1. **I2C 扫描实验**（必须）
   - 成功扫描到 MLX90640 传感器
   - 截图串口输出结果
   - 提交代码

2. **原理理解**（必须）
   - 在笔记中画出 I2C 写操作的时序图（SCL/SDA 波形）
   - 用自己的话解释：I2C 通信需要哪几根线？为什么需要上拉电阻？主设备和从设备的区别是什么？

3. **调试实践**（推荐）
   - 故意把 SDA 和 SCL 接反，观察扫描结果
   - 故意断开一根线，观察扫描结果
   - 记录不同故障现象，建立调试经验

4. **挑战任务**（可选）
   - 修改扫描代码，使其只扫描一次并持续闪烁 LED 表示设备在线
   - 或者添加第二块 I2C 设备（如果有），验证多设备共存

## 明日预告 | Tomorrow's Preview

明天我们将深入 MLX90640 传感器！你将学习红外热成像的物理原理，阅读传感器的寄存器地图，并编写代码读取传感器的配置信息。这是真正"和传感器对话"的开始。

## 参考资源 | References

- **I2C 协议教程**：https://learn.sparkfun.com/tutorials/i2c/all
- **MLX90640 数据手册**：https://www.melexis.com/en/product/mlx90640
- **STM32 I2C HAL 教程**：https://deepbluembedded.com/stm32-i2c-tutorial/
- **I2C 总线扫描原理**：https://learn.adafruit.com/scanning-i2c-addresses
- **逻辑分析仪使用教程**：B站搜索"逻辑分析仪 I2C"

---

*预计完成时间：5-6 小时*
*Estimated completion time: 5-6 hours*
