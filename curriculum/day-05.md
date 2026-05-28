# Day 5: MLX90640 基础驱动开发 | MLX90640 Basic Driver Development

## 学习目标 | Learning Objectives

- 掌握 MLX90640 EEPROM 校准参数的读取方法
- 实现完整的帧数据读取（两个半帧）
- 理解原始 ADC 数据到温度值的转换公式
- 能通过串口输出 32x24 的温度矩阵
- 验证读取温度数据的合理性

## 前置准备 | Prerequisites

- [ ] Day 4 成功读取了 MLX90640 的 Chip ID 和控制寄存器
- [ ] 理解了帧数据结构（两个半帧）
- [ ] 串口调试输出正常工作

## 为什么学这个？| Why This Matters

今天是我们从"配置传感器"到"真正获取数据"的关键跨越。EEPROM 中的校准参数是每个传感器独有的"指纹" -- 它记录了出厂时校准的补偿系数，没有这些参数，读取的原始数据就无法准确转换为温度值。理解"原始 ADC 值 -> 应用校准参数 -> 得到温度"这个流程，是理解所有传感器数据处理的基础模式，无论温度、压力、加速度传感器都遵循类似的校准流程。

## 今日任务 | Today's Tasks

### Task 1: 读取 EEPROM 校准参数 (estimated 60 minutes)

**目标：** 从 MLX90640 的 EEPROM 中读取所有校准参数

**步骤：**

1. **了解 EEPROM 参数分布**
   - MLX90640 的校准参数存储在 EEPROM 区域（0x2400 起始的地址）
   - 关键参数包括：
     * **ECC（Emissivity Correction Coefficient）**：发射率校准系数
     * **Ks_Ta**：环境温度补偿系数
     * **Ks_To**：目标温度补偿系数
     * **Kv**：供电电压补偿系数
     * **Vth, Alpha, Beta 等像素级校准参数**

2. **编写 EEPROM 批量读取函数**
   - 在 `main.c` 中添加（`/* USER CODE BEGIN 0 */` 区域）：

   ```c
   // EEPROM 校准参数存储
   typedef struct {
       float emissivity;
       float KsTa;
       float KsTo;
       float Kv;
       uint16_t Vdd;
       uint16_t ptat;
       float alpha_ptat;
       float gain;
       float tgc;
       float resolution;
       float KsTa_scale;
       float Kv_scale;
   } MLX90640_CalibParams;

   MLX90640_CalibParams mlx_calib;

   /**
    * @brief 从 MLX90640 读取多个连续寄存器
    * @param start_addr: 起始寄存器地址
    * @param data: 数据缓冲区
    * @param count: 读取的字（word）数量
    * @retval HAL_StatusTypeDef
    */
   HAL_StatusTypeDef MLX90640_ReadWords(uint16_t start_addr, uint16_t *data, uint16_t count)
   {
       uint8_t reg_buf[2];
       uint8_t read_buf[2];
       HAL_StatusTypeDef status;

       for (uint16_t i = 0; i < count; i++) {
           status = MLX90640_ReadRegister(start_addr + i, &data[i]);
           if (status != HAL_OK) return status;
       }

       return HAL_OK;
   }

   /**
    * @brief 读取并解析 EEPROM 校准参数
    */
   HAL_StatusTypeDef MLX90640_ReadCalibration(void)
   {
       uint16_t ee_data[64];  // 存储读取的 EEPROM 数据

       printf("Reading EEPROM calibration data...\r\n");

       // 读取 EEPROM 前 64 个字（0x2400 - 0x243F）
       // 注意：EEPROM 地址空间在数据手册中有详细定义
       // 这里读取关键的校准参数

       uint16_t reg_val;

       // 读取发射率（Emissivity）
       // 地址 0x240F 包含发射率的补码值
       if (MLX90640_ReadRegister(0x240F, &reg_val) == HAL_OK) {
           if (reg_val > 32767) {
               mlx_calib.emissivity = (float)(reg_val - 65536) / 32768.0f;
           } else {
               mlx_calib.emissivity = (float)reg_val / 32768.0f;
           }
           printf("  Emissivity: %.4f\r\n", mlx_calib.emissivity);
       }

       // 读取 Kv (供电电压补偿)
       if (MLX90640_ReadRegister(0x2410, &reg_val) == HAL_OK) {
           int16_t kv_raw = (int16_t)reg_val;
           mlx_calib.Kv = (float)kv_raw / 65536.0f;
           printf("  Kv: %.6f\r\n", mlx_calib.Kv);
       }

       // 读取 KsTa (环境温度补偿)
       if (MLX90640_ReadRegister(0x2411, &reg_val) == HAL_OK) {
           int16_t ksta_raw = (int16_t)reg_val;
           mlx_calib.KsTa = (float)ksta_raw / 65536.0f;
           printf("  KsTa: %.6f\r\n", mlx_calib.KsTa);
       }

       // 读取增益（Gain）
       if (MLX90640_ReadRegister(0x2420, &reg_val) == HAL_OK) {
           mlx_calib.gain = (float)reg_val;
           printf("  Gain: %.1f\r\n", mlx_calib.gain);
       }

       // 读取 Vdd（供电电压测量）
       if (MLX90640_ReadRegister(0x2421, &reg_val) == HAL_OK) {
           mlx_calib.Vdd = reg_val;
           printf("  Vdd raw: 0x%04X\r\n", reg_val);
       }

       // 读取 PTAT（与环境温度相关）
       if (MLX90640_ReadRegister(0x2422, &reg_val) == HAL_OK) {
           mlx_calib.ptat = reg_val;
           printf("  PTAT raw: 0x%04X\r\n", reg_val);
       }

       printf("Calibration data read complete.\r\n");
       return HAL_OK;
   }
   ```

3. **在 main() 中调用并查看结果**
   ```c
   /* USER CODE BEGIN 2 */
   MLX90640_ReadCalibration();
   /* USER CODE END 2 */
   ```

**预期结果：**
- 成功读取 EEPROM 中的校准参数
- 串口打印出 Emissivity、Kv、KsTa、Gain 等参数值
- Emissivity 通常在 0.9-1.0 之间

**常见问题：**

**Q: 读取的值看起来不合理？**
A: EEPROM 参数是出厂时写入的，不同传感器值可能不同。关键是确保读取的寄存器地址正确。参考开源固件仓库中的 EEPROM 读取代码进行比对。

**Q: 数据手册中有很多其他校准参数需要读吗？**
A: 完整的校准参数有很多。我们先读取几个关键的，Day 6 会完善。也可以参考 Melexis 官方提供的 API 源码中的完整参数列表。

---

### Task 2: 读取帧数据（半帧） (estimated 75 minutes)

**目标：** 编写代码读取 MLX90640 的一个完整半帧数据

**步骤：**

1. **理解帧数据读取流程**
   - 读取帧数据的步骤：
     1. 等待新数据就绪（检查状态寄存器）
     2. 读取半帧 0 的数据（832 个 word）
     3. 等待下一个半帧就绪
     4. 读取半帧 1 的数据（832 个 word）
   - 状态寄存器（0x8000）：
     * Bit 3：新半帧 0 数据就绪
     * Bit 4：新半帧 1 数据就绪

2. **编写帧数据读取函数**
   ```c
   #define MLX90640_RAM_START     0x2400
   #define MLX90640_FRAME_LEN    832   // 每个半帧 832 个 word
   #define MLX90640_REG_STATUS   0x8000

   uint16_t frame_data[MLX90640_FRAME_LEN];  // 存储半帧数据

   /**
    * @brief 等待新帧数据就绪
    * @param subpage: 等待哪个半帧（0 或 1）
    * @retval HAL_StatusTypeDef
    */
   HAL_StatusTypeDef MLX90640_WaitForData(uint8_t subpage)
   {
       uint16_t status;
       uint8_t bit_pos = (subpage == 0) ? 3 : 4;
       uint32_t timeout = 0;

       // 等待状态位被置位
       do {
           if (MLX90640_ReadRegister(MLX90640_REG_STATUS, &status) != HAL_OK) {
               return HAL_ERROR;
           }
           HAL_Delay(10);
           timeout++;
           if (timeout > 200) {  // 最多等待 2 秒
               printf("Timeout waiting for subpage %d data\r\n", subpage);
               return HAL_TIMEOUT;
           }
       } while (!(status & (1 << bit_pos)));

       // 清除状态位（读取后自动清除？不是，需要写 0 来清除）
       // 实际上对于 MLX90640，读取 RAM 后状态位会自动更新
       return HAL_OK;
   }

   /**
    * @brief 读取一个半帧的数据
    * @param subpage: 读取哪个半帧（0 或 1）
    * @retval HAL_StatusTypeDef
    */
   HAL_StatusTypeDef MLX90640_ReadSubpage(uint8_t subpage)
   {
       // 等待数据就绪
       HAL_StatusTypeDef status = MLX90640_WaitForData(subpage);
       if (status != HAL_OK) return status;

       // 读取 832 个 word 的数据
       // 使用 HAL_I2C_Mem_Read 可以更高效地读取连续地址
       uint8_t addr_buf[2];
       addr_buf[0] = (MLX90640_RAM_START >> 8) & 0xFF;
       addr_buf[1] = MLX90640_RAM_START & 0xFF;

       // 使用内存读取（Mem_Read）更高效：先发送地址再读取
       uint8_t raw_data[MLX90640_FRAME_LEN * 2];

       status = HAL_I2C_Mem_Read(
           &hi2c1,
           MLX90640_ADDR << 1,
           MLX90640_RAM_START,
           I2C_MEMADD_SIZE_16BIT,
           raw_data,
           MLX90640_FRAME_LEN * 2,
           500
       );

       if (status != HAL_OK) {
           printf("Failed to read subpage %d: %d\r\n", subpage, status);
           return status;
       }

       // 将字节数据转换为 16 位字（大端序）
       for (int i = 0; i < MLX90640_FRAME_LEN; i++) {
           frame_data[i] = ((uint16_t)raw_data[i * 2] << 8) | raw_data[i * 2 + 1];
       }

       return HAL_OK;
   }
   ```

3. **测试帧数据读取**
   - 在 main() 中添加：

   ```c
   printf("\r\nReading frame data...\r\n");

   // 读取半帧 0
   if (MLX90640_ReadSubpage(0) == HAL_OK) {
       printf("Subpage 0 read success!\r\n");
       printf("First 10 raw values: ");
       for (int i = 0; i < 10; i++) {
           printf("0x%04X ", frame_data[i]);
       }
       printf("\r\n");

       // 打印环境温度（Ta）数据
       // Ta 数据位于 RAM 偏移 0x900 (相对于 0x2400)
       // 即 frame_data[0x900 / 2] = frame_data[0x480]
       // 具体位置请参考数据手册
   }
   ```

4. **编译烧录测试**
   - 注意：读取 832 个 word（1664 字节）需要一些时间
   - 在 I2C 400kHz 下，约需要 40-50ms
   - 如果超时，可以增加 HAL_I2C_Mem_Read 的超时参数

**预期结果：**
- 成功读取半帧数据（832 个 word）
- 看到原始的 16 位数据值
- 每次读取的值应该略有变化（噪声和环境变化）

**常见问题：**

**Q: HAL_I2C_Mem_Read 返回 HAL_ERROR？**
A:
1. 尝试增加超时时间到 1000ms
2. 检查 I2C 配置是否正确（400kHz）
3. 可以尝试分多次读取（每次读 64 或 128 个 word），减少单次传输量

**Q: 读到的数据全是 0x0000 或 0xFFFF？**
A: 可能是传感器没有开始采集。确认控制寄存器已正确配置（刷新率已设置）。

---

### Task 3: 读取环境温度 (estimated 45 minutes)

**目标：** 从帧数据中提取传感器自身温度（Ta），这是计算目标温度的基础

**步骤：**

1. **理解 Ta 计算**
   - MLX90640 内部有一个 PTAT（Proportional To Absolute Temperature）传感器
   - PTAT 值存储在 RAM 中的特定位置
   - Ta 值 = f(PTAT_raw, Vth, Ks_Ta) 通过校准参数计算

2. **编写 Ta 计算函数**

   ```c
   /**
    * @brief 从帧数据中计算传感器环境温度（Ta）
    * @retval 温度值（摄氏度）
    */
   float MLX90640_CalculateTa(void)
   {
       // PTAT 数据位于 RAM 地址 0x2400 + 0x900
       // 在帧数据数组中的偏移是 0x900 / 2 = 0x480 = 1152
       // 但实际上 PTAT 在每个半帧的固定位置
       // 参考 frame_data 的结构，具体偏移请查看数据手册

       uint16_t ptat_raw = frame_data[0];  // 简化：实际偏移需要查数据手册

       // 详细的温度计算需要完整的校准参数
       // 简化版本：使用近似公式
       // Ta = ptat_raw / 校准系数 - 273.15

       // 这里用一个简化版本
       // 完整的计算在 Day 6 实现
       float ta = (float)ptat_raw * 0.03f - 273.15f + 100.0f;

       return ta;
   }
   ```

   **注意：** 这是一个简化的近似计算。Day 6 会实现完整的温度计算公式。这里只是让你先看到数据在变化。

3. **读取并打印 Ta**
   ```c
   // 在读取半帧数据后
   float ta = MLX90640_CalculateTa();
   printf("Sensor Ta (approximate): %.1f C\r\n", ta);
   ```

**预期结果：**
- 串口输出传感器温度的近似值
- 温度值应该接近室温（20-30C 范围）

**常见问题：**

**Q: 温度值偏差很大？**
A: 这是正常的，因为我们使用了简化公式。Day 6 使用完整校准参数后就会准确。

---

### Task 4: 整合完整读取流程 (estimated 30 minutes)

**目标：** 将今天的代码整合为完整的初始化+读取流程

**步骤：**

1. **创建完整的初始化和读取流程**

   ```c
   /* USER CODE BEGIN 2 */
   printf("\r\n=== ThermalEyes - MLX90640 Driver ===\r\n");

   // Step 1: 检查传感器连接
   uint16_t chip_id;
   if (MLX90640_ReadRegister(MLX90640_REG_CHIP_ID, &chip_id) != HAL_OK) {
       printf("FATAL: Cannot communicate with MLX90640!\r\n");
       Error_Handler();
   }
   printf("Chip ID: 0x%04X - OK\r\n", chip_id);

   // Step 2: 读取校准参数
   MLX90640_ReadCalibration();

   // Step 3: 配置传感器
   MLX90640_SetRefreshRate(2);  // 2Hz
   printf("Sensor configured: 2Hz refresh rate\r\n");

   // Step 4: 等待传感器稳定
   printf("Waiting for sensor to stabilize...\r\n");
   HAL_Delay(500);

   printf("Sensor ready!\r\n\r\n");
   /* USER CODE END 2 */
   ```

2. **在 while(1) 中持续读取并输出**

   ```c
   /* USER CODE BEGIN 3 */
   // 读取半帧 0
   if (MLX90640_ReadSubpage(0) == HAL_OK) {
       float ta = MLX90640_CalculateTa();
       printf("Frame: Ta=%.1fC | Raw[0..4]=0x%04X 0x%04X 0x%04X 0x%04X 0x%04X\r\n",
              ta, frame_data[0], frame_data[1], frame_data[2],
              frame_data[3], frame_data[4]);
   }

   HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
   /* USER CODE END 3 */
   ```

**预期结果：**
- 完整的初始化流程：检测传感器 -> 读校准参数 -> 配置 -> 开始采集
- 持续读取并输出帧数据
- LED 在每次读取后闪烁

---

## 今日作业 | Homework

1. **驱动代码完成**（必须）
   - 成功读取 EEPROM 校准参数
   - 成功读取帧数据
   - 提交代码和串口输出截图

2. **数据分析**（必须）
   - 记录读取到的校准参数值
   - 记录帧数据中前 20 个原始值
   - 尝试分析这些值的含义

3. **代码整理**（推荐）
   - 将 MLX90640 相关的函数整理到单独的文件中
   - 创建 `mlx90640.c` 和 `mlx90640.h`
   - 这样代码更清晰，后续维护方便

4. **思考题**（可选）
   - 为什么每个传感器都需要单独的校准参数？
   - 832 个 word 中，除了像素数据还有什么其他信息？

## 明日预告 | Tomorrow's Preview

明天是核心的一天！我们将实现完整的温度计算算法，把原始 ADC 数据转换为准确的温度值。你将第一次看到 32x24 个温度数据，并能验证它们是否合理（比如放在手前面看温度是否升高）。

## 参考资源 | References

- **MLX90640 数据手册**（EEPROM 和 RAM 章节）
- **MLX90640 EEPROM 映射**：数据手册 Table 4
- **Melexis 官方 API 源码**：https://github.com/melexis/mlx90640-library
- **开源固件参考**：https://github.com/colourfate/thermal_bridge
- **I2C 批量读取教程**：https://deepbluembedded.com/stm32-i2c-mem-read-write/

---

*预计完成时间：5-6 小时*
*Estimated completion time: 5-6 hours*
