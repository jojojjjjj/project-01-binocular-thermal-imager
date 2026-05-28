# Day 9: USB 数据传输实现 | USB Data Transmission Implementation

## 学习目标 | Learning Objectives

- 设计温度帧的数据格式（帧头 + 温度数据 + 帧尾）
- 实现温度数据的高效打包和 USB 发送
- 理解数据传输中的同步和可靠性问题
- 在电脑端验证接收到的温度数据
- 参考开源固件理解实际项目的数据协议

## 前置准备 | Prerequisites

- [ ] Day 8 USB CDC 收发测试成功
- [ ] Phase 2 的温度计算代码完整可用
- [ ] 电脑能识别 STM32 的 USB CDC 虚拟串口

## 为什么学这个？| Why This Matters

数据通信协议的设计是嵌入式系统开发中的核心技能。一个好的数据格式需要考虑：接收方如何知道一帧数据从哪里开始？数据有多少字节？如何校验数据的正确性？这些问题在所有通信系统中都存在，从 USB 到蓝牙到 Wi-Fi，都需要某种"协议"来保证数据的可靠传输。今天你将亲手设计一个简单但完整的通信协议。

## 今日任务 | Today's Tasks

### Task 1: 设计温度帧数据格式 (estimated 45 minutes)

**目标：** 定义温度数据从 STM32 传输到手机的数据格式

**步骤：**

1. **分析数据量**
   - 32 x 24 = 768 个像素
   - 每个像素温度用 float（4 字节）或 uint16（2 字节）
   - 使用 uint16 可以将温度编码为 0.01C 精度的整数（如 25.67C = 2567）
   - 总数据量：768 x 2 = 1536 字节
   - 加上帧头和帧尾，约 1550 字节/帧

2. **参考开源固件的数据格式**
   - 查看 thermal_bridge 仓库中的数据发送代码
   - 理解开源项目使用的数据格式
   - 可以直接使用或修改其格式

3. **设计帧格式**

   ```
   帧结构：
   +--------+--------+--------+----------------+--------+--------+
   | 帧头   | 帧长   | 帧序号 | 温度数据       | 校验和 | 帧尾   |
   | 2字节  | 2字节  | 2字节  | N字节          | 2字节  | 2字节  |
   +--------+--------+--------+----------------+--------+--------+

   帧头: 0xAA55 (2字节，固定标识)
   帧长: 整个帧的长度 (2字节, little-endian)
   帧序号: 递增序号 (2字节, 用于检测丢帧)
   温度数据: 768 x 2字节 = 1536字节
     - 每个像素用 int16_t 表示，单位 0.01C
     - 如 2567 表示 25.67C
     - -1000 表示 -10.00C
   校验和: 前面所有数据的 CRC16 (2字节)
   帧尾: 0x55AA (2字节，固定标识)

   总帧长: 2 + 2 + 2 + 1536 + 2 + 2 = 1546 字节
   ```

4. **定义数据结构**
   ```c
   // 帧头帧尾定义
   #define FRAME_HEADER     0xAA55
   #define FRAME_FOOTER     0x55AA

   // 帧数据结构
   typedef struct __attribute__((packed)) {
       uint16_t header;        // 帧头 0xAA55
       uint16_t length;        // 帧长度
       uint16_t frame_count;   // 帧序号
       int16_t  temp_data[768]; // 温度数据 (0.01C)
       uint16_t checksum;      // CRC16 校验
       uint16_t footer;        // 帧尾 0x55AA
   } ThermalFrame;
   ```

**预期结果：**
- 定义了完整的数据帧格式
- 理解帧头、帧尾、校验和的作用
- 数据结构定义完成

**常见问题：**

**Q: 为什么不直接发送 float 数据？**
A: float 是 4 字节，int16_t 是 2 字节。使用 int16_t + 0.01C 精度，数据量减半。而且整数在不同平台上的表示更一致，避免浮点字节序问题。

**Q: 1546 字节一帧，USB 能发送吗？**
A: USB Full Speed 的理论带宽是 12Mbps，发送 1546 字节只需要约 1ms。完全够用。

---

### Task 2: 实现数据打包和发送 (estimated 60 minutes)

**目标：** 将 float 温度矩阵打包为 int16_t 格式，通过 USB CDC 发送

**步骤：**

1. **编写 CRC16 校验函数**
   ```c
   /**
    * @brief 计算 CRC16 校验和
    * @param data: 数据指针
    * @param len: 数据长度
    * @retval CRC16 值
    */
   uint16_t CRC16_Calculate(uint8_t *data, uint16_t len)
   {
       uint16_t crc = 0xFFFF;
       for (uint16_t i = 0; i < len; i++) {
           crc ^= data[i];
           for (uint8_t j = 0; j < 8; j++) {
               if (crc & 0x0001) {
                   crc = (crc >> 1) ^ 0xA001;
               } else {
                   crc >>= 1;
               }
           }
       }
       return crc;
   }
   ```

2. **编写帧打包函数**
   ```c
   ThermalFrame tx_frame;
   uint16_t frame_counter = 0;

   /**
    * @brief 打包温度数据为一帧
    */
   void PackThermalFrame(void)
   {
       tx_frame.header = FRAME_HEADER;
       tx_frame.length = sizeof(ThermalFrame);
       tx_frame.frame_count = frame_counter++;

       // 将 float 温度转换为 int16_t (0.01C 精度)
       for (int i = 0; i < 768; i++) {
           // 限制温度范围
           float temp = temperature_data[i];
           if (temp < -40.0f) temp = -40.0f;
           if (temp > 300.0f) temp = 300.0f;
           tx_frame.temp_data[i] = (int16_t)(temp * 100.0f);
       }

       // 计算校验和（覆盖 header 到 temp_data 结束）
       tx_frame.checksum = CRC16_Calculate(
           (uint8_t*)&tx_frame,
           sizeof(ThermalFrame) - 4  // 减去 checksum 和 footer 的大小
       );

       tx_frame.footer = FRAME_FOOTER;
   }
   ```

3. **编写 USB 发送函数**
   ```c
   /**
    * @brief 通过 USB CDC 发送温度帧
    * @retval 0=成功, -1=失败
    */
   int SendThermalFrame(void)
   {
       // USB CDC 单次最大发送 64 字节
       // 需要分多次发送完整帧
       uint8_t *data = (uint8_t*)&tx_frame;
       uint16_t total_len = sizeof(ThermalFrame);
       uint16_t offset = 0;

       while (offset < total_len) {
           uint16_t chunk_size = total_len - offset;
           if (chunk_size > 64) chunk_size = 64;

           uint8_t result = CDC_Transmit_FS(data + offset, chunk_size);
           if (result != USBD_OK) {
               return -1;
           }

           offset += chunk_size;

           // USB 发送需要间隔
           HAL_Delay(1);
       }

       return 0;
   }
   ```

   **注意：** 分包发送的方式可能不是最优的。更好的方式是直接发送整个帧，让 USB 驱动自动分包。可以尝试：

   ```c
   // 直接发送整个帧（推荐尝试）
   CDC_Transmit_FS((uint8_t*)&tx_frame, sizeof(ThermalFrame));
   ```

4. **在 while(1) 中整合发送**
   ```c
   /* USER CODE BEGIN 3 */
   // 读取传感器数据
   MLX90640_ReadSubpage(0);
   MLX90640_ReadSubpage(1);

   // 计算温度
   MLX90640_CalculateAllTemperatures(0.95f);

   // 补偿坏像素
   MLX90640_CompensateBadPixels();

   // 打包数据
   PackThermalFrame();

   // 通过 USB 发送
   SendThermalFrame();

   // LED 闪烁
   HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);

   HAL_Delay(100);  // 控制帧率
   /* USER CODE END 3 */
   ```

**预期结果：**
- 温度数据被打包为标准帧格式
- USB CDC 成功发送数据帧
- 电脑串口工具中收到二进制数据

**常见问题：**

**Q: CDC_Transmit_FS 返回 USBD_BUSY？**
A: USB 发送是异步的，上一次发送还没完成就再次调用会返回 BUSY。解决方法：
1. 在两次发送之间增加延时
2. 检查发送完成标志后再发送

**Q: 电脑收到的数据量不对？**
A: 串口工具可能将二进制数据作为文本显示，有些字节被解释为控制字符。使用支持十六进制显示的串口工具（如 Tera Term 的 Hex View）。

---

### Task 3: 验证数据传输 (estimated 45 minutes)

**目标：** 在电脑端验证接收到的数据是否正确

**步骤：**

1. **使用十六进制查看工具**
   - 推荐使用 Tera Term 或 RealTerm
   - 设置为十六进制显示模式
   - 查找帧头 0xAA55，确认帧结构正确

2. **编写简单的 Python 验证脚本**
   - 在电脑上创建 Python 脚本来解析数据：

   ```python
   import serial
   import struct

   # 打开串口（修改为你的 COM 口）
   ser = serial.Serial('COM3', 115200, timeout=1)

   FRAME_HEADER = 0xAA55
   FRAME_SIZE = 1546

   print("Waiting for thermal frames...")

   while True:
       # 查找帧头
       data = ser.read(2)
       if len(data) == 2:
           header = struct.unpack('<H', data)[0]
           if header == FRAME_HEADER:
               # 读取剩余数据
               remaining = ser.read(FRAME_SIZE - 2)
               full_frame = data + remaining

               if len(full_frame) == FRAME_SIZE:
                   # 解析帧
                   header, length, count = struct.unpack('<HHH', full_frame[0:6])
                   temp_data = struct.unpack('<768h', full_frame[6:1542])
                   checksum, footer = struct.unpack('<HH', full_frame[1542:1546])

                   # 验证帧尾
                   if footer == 0x55AA:
                       # 转换温度
                       temps = [t / 100.0 for t in temp_data]
                       min_t = min(temps)
                       max_t = max(temps)
                       avg_t = sum(temps) / len(temps)
                       print(f"Frame {count}: min={min_t:.1f}C max={max_t:.1f}C avg={avg_t:.1f}C")

   ser.close()
   ```

3. **运行验证脚本**
   - 安装 Python pyserial 库：`pip install pyserial`
   - 修改 COM 口号为你的设备
   - 运行脚本
   - 应该看到每帧的温度统计信息

**预期结果：**
- Python 脚本成功解析温度帧
- 显示正确的帧序号和温度数据
- 手掌测试时看到温度变化

**常见问题：**

**Q: Python 找不到帧头？**
A:
1. 确认 COM 口正确
2. 检查帧格式定义是否与代码一致
3. 先用十六进制查看工具确认数据格式

**Q: 帧尾不正确？**
A: 可能是帧对齐问题。需要确保每次都从帧头开始读取。

---

### Task 4: 参考开源固件优化协议 (estimated 30 minutes)

**目标：** 对比开源固件的数据协议，优化自己的实现

**步骤：**

1. **阅读 thermal_bridge 仓库的 USB 发送代码**
   - 找到温度数据发送相关的函数
   - 理解其数据格式和发送策略
   - 注意它如何处理 USB 发送失败的情况

2. **优化点**
   - 添加发送失败的计数和 LED 指示
   - 优化发送间隔，确保 USB 稳定
   - 考虑是否需要增加帧类型标识（用于区分不同数据）

3. **添加调试输出**
   ```c
   // 每隔一段时间输出调试信息
   static uint32_t last_debug = 0;
   uint32_t now = HAL_GetTick();
   if (now - last_debug > 5000) {  // 每 5 秒
       printf("Frames sent: %u, Errors: %u\r\n",
              frame_counter, usb_error_count);
       last_debug = now;
   }
   ```

**预期结果：**
- 数据传输稳定可靠
- 参考开源代码优化了发送策略
- 添加了基本的调试输出

---

## 今日作业 | Homework

1. **USB 数据传输完成**（必须）
   - 成功通过 USB CDC 发送温度帧
   - Python 脚本成功解析数据
   - 提交 Python 脚本和解析结果截图

2. **协议文档**（必须）
   - 写一份简短的数据协议文档
   - 包含帧格式、字段说明、示例
   - 这将作为 APP 开发的接口文档

3. **稳定性测试**（推荐）
   - 持续发送 1000 帧，统计丢帧率
   - 测试不同帧率下的稳定性
   - 记录测试结果

4. **挑战任务**（可选）
   - 在 Python 端实现简单的温度可视化（用 matplotlib 显示温度矩阵）
   - 实现双向通信：电脑发送命令控制传感器参数

## 明日预告 | Tomorrow's Preview

明天我们将把所有固件代码整合到一起，完成完整的固件系统：I2C 读取 + 温度计算 + USB 发送。你将进行固件的综合测试，确保所有模块协同工作，然后就可以进入 APP 开发阶段了。

## 参考资源 | References

- **开源固件 thermal_bridge**：https://github.com/colourfate/thermal_bridge
- **USB CDC 数据传输**：https://www.st.com/en/development-tools/stm32cubeide.html
- **Python pyserial**：https://pyserial.readthedocs.io/
- **CRC16 算法**：https://en.wikipedia.org/wiki/Cyclic_redundancy_check
- **STM32 USB CDC 发送大数据**：ST 社区论坛搜索 "CDC_Transmit_FS large data"

---

*预计完成时间：5-6 小时*
*Estimated completion time: 5-6 hours*
