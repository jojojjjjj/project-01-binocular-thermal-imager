# Day 6: 温度数据解析 | Temperature Data Parsing

## 学习目标 | Learning Objectives

- 理解 MLX90640 完整的温度计算公式（像素温度和 Ta）
- 实现原始 ADC 数据到摄氏温度的完整转换
- 读取并合并两个半帧，获得完整的 32x24 温度矩阵
- 通过串口输出温度矩阵并验证数据合理性
- 参考开源固件代码，理解工程级驱动的实现方式

## 前置准备 | Prerequisites

- [ ] Day 5 成功读取了帧数据和 EEPROM 校准参数
- [ ] I2C 通信稳定，无频繁超时
- [ ] 已克隆开源固件仓库到本地

## 为什么学这个？| Why This Matters

今天是将"原始数字"变成"有意义信息"的关键一步。传感器输出的只是 ADC 采样的原始数字，必须经过一系列数学变换才能得到温度值。这个变换过程涉及增益补偿、偏移校正、发射率调整等，是传感器信号处理的经典流程。掌握了这个流程，你就能处理任何类型的传感器数据 -- 无论是温度、湿度、压力还是加速度传感器，核心思路都是：原始值 + 校准参数 = 物理量。

## 今日任务 | Today's Tasks

### Task 1: 参考开源驱动理解完整算法 (estimated 45 minutes)

**目标：** 阅读开源固件中的温度计算代码，理解完整的计算流程

**步骤：**

1. **打开开源固件仓库**
   - 找到 `thermal_bridge` 仓库中的 MLX90640 驱动文件
   - 通常文件名为 `mlx90640.c` 或类似
   - 搜索包含温度计算的关键函数

2. **理解温度计算的整体流程**
   - 完整流程分为以下步骤：
     ```
     1. 读取 EEPROM 校准参数（Day 5 已完成）
     2. 读取 RAM 帧数据（Day 5 已完成）
     3. 计算供电电压 VDD
     4. 计算环境温度 Ta
     5. 计算增益补偿
     6. 对每个像素：
        a. 应用增益补偿
        b. 计算红外辐射补偿 IR_Company
        c. 应用 Ta 补偿
        d. 应用发射率补偿
        e. 计算最终温度 To
     ```

3. **定位关键公式**
   - 在开源代码中找到以下公式：
     * VDD 计算：`VDD = (raw_VDD - Vdd25) / K_Vdd + 3.3`
     * Ta 计算：使用 PTAT、Vth、KsTa 等参数的复杂公式
     * 像素温度 To 计算：涉及 Alpha、KsTo、TGC 等多项补偿

4. **记录关键变量和公式**
   - 在笔记中记录每个公式中的变量含义
   - 注意数据类型转换（int16_t, uint16_t, float）

**预期结果：**
- 理解温度计算的完整流程
- 能定位开源代码中的关键函数
- 知道每个计算步骤的作用

**常见问题：**

**Q: 公式太复杂看不懂？**
A: 没关系，这是传感器的专用算法，由芯片厂商设计。我们不需要推导公式，只需要正确实现它。重点理解输入（原始数据 + 校准参数）和输出（温度值），中间的数学变换按数据手册和参考代码实现即可。

---

### Task 2: 实现完整的 Ta 计算 (estimated 45 minutes)

**目标：** 使用完整的校准参数计算传感器环境温度 Ta

**步骤：**

1. **补充 EEPROM 参数读取**
   - 添加以下参数的读取（在 Day 5 的基础上扩展）：

   ```c
   // 补充的校准参数
   float Vdd25;       // 25C 时的参考电压
   float K_Vdd;       // 电压补偿系数
   float Kv_PTAT;     // PTAT 电压系数
   float Kt_PTAT;     // PTAT 温度系数
   uint16_t VTH;      // 参考电压阈值
   float alpha_ptat;  // PTAT alpha 值
   ```

2. **实现 VDD 计算**

   ```c
   /**
    * @brief 计算当前供电电压
    * @param vdd_raw: RAM 中的 VDD 原始值
    * @retval 电压值（伏特）
    */
   float MLX90640_CalculateVDD(uint16_t vdd_raw)
   {
       // VDD = (vdd_raw - Vdd25) / K_Vdd + 3.3
       return ((float)vdd_raw - mlx_calib.Vdd25) / mlx_calib.K_Vdd + 3.3f;
   }
   ```

3. **实现完整 Ta 计算**

   ```c
   /**
    * @brief 计算传感器环境温度 Ta（完整版）
    * @param ptat_raw: RAM 中的 PTAT 原始值
    * @param vdd_raw: RAM 中的 VDD 原始值
    * @retval 环境温度（摄氏度）
    */
   float MLX90640_CalculateTa_Full(uint16_t ptat_raw, uint16_t vdd_raw)
   {
       float vdd = MLX90640_CalculateVDD(vdd_raw);

       // Ta 的计算公式（来自数据手册）
       // 简化版本：Ta = PTAT / (K_V_PTAT * VDD) - K_T_PTAT
       float ta = (float)ptat_raw;

       // 应用 VDD 补偿
       ta = ta / (1.0f + mlx_calib.Kv_PTAT * (vdd - 3.3f));

       // 转换为温度（近似，完整公式需要更多参数）
       // 参考 Melexis 官方 API 中的实现
       ta = (ta / mlx_calib.VTH) * (1.0f + mlx_calib.Kt_PTAT * (vdd - 3.3f));

       // 转换为摄氏度
       ta = ta - 273.15f;

       return ta;
   }
   ```

   **注意：** 以上是简化版公式。完整公式请参考 Melexis 官方提供的 `MLX90640_API.cpp` 中的 `MLX90640_GetTa()` 函数。完整公式考虑了更多补偿因素。

4. **使用官方 API 作为参考**
   - 从 Melexis 官方 GitHub 下载 API：https://github.com/melexis/mlx90640-library
   - 找到 `MLX90640_API.cpp` 中的 `MLX90640_GetTa()` 函数
   - 与我们自己的实现进行对比
   - 如果时间允许，直接移植官方 API

**预期结果：**
- Ta 计算结果接近室温（20-30C 范围）
- 理解 VDD 补偿和 PTAT 计算的作用

**常见问题：**

**Q: 计算的 Ta 值偏差大？**
A: 优先使用 Melexis 官方 API 的实现。我们的简化版可能有参数遗漏。直接移植官方库是最可靠的方式。

---

### Task 3: 实现像素温度计算 (estimated 75 minutes)

**目标：** 实现完整的像素温度计算，输出 32x24 温度矩阵

**步骤：**

1. **考虑使用官方 API**
   - 最可靠的方式是直接移植 Melexis 官方的 `MLX90640_API.c/.h`
   - 这个 API 已经处理了所有复杂的计算和边界情况
   - 移植步骤：
     * 将 `MLX90640_API.c` 和 `.h` 复制到你的工程
     * 修改 I2C 读写接口，替换为 STM32 HAL 函数
     * 调用 `MLX90640_DumpEE()` 读取 EEPROM
     * 调用 `MLX90640_GetFrameData()` 读取帧数据
     * 调用 `MLX90640_CalculateTo()` 计算温度

2. **如果自己实现，以下是核心流程**

   ```c
   #define MLX90640_PIXEL_NUM 768  // 32 * 24 = 768 个像素

   float temperature_data[MLX90640_PIXEL_NUM];  // 存储最终温度

   /**
    * @brief 计算所有像素的温度值
    * @param emissivity: 目标发射率（通常 0.95-1.0）
    * @retval 无
    */
   void MLX90640_CalculateAllTemperatures(float emissivity)
   {
       float ta = MLX90640_CalculateTa_Full(ptat_value, vdd_value);

       for (int i = 0; i < MLX90640_PIXEL_NUM; i++) {
           // 获取像素原始值
           // 注意：像素数据在 frame_data 中的位置需要根据半帧映射
           int16_t ir_raw = (int16_t)frame_data[i + 64];  // 跳过控制参数

           // 应用增益补偿
           float ir_compensated = (float)ir_raw / mlx_calib.gain;

           // 应用 VDD 补偿
           // ...

           // 应用 Ta 补偿
           // ...

           // 应用发射率补偿
           ir_compensated /= emissivity;

           // 转换为温度
           // To = sqrt(ir_compensated / alpha + Ta^4) - 273.15
           // 使用简化的平方根公式
           float ta_k = ta + 273.15f;  // 转换为开尔文
           float to_k_sq = ir_compensated / mlx_calib.alpha_ptat + ta_k * ta_k * ta_k * ta_k;

           if (to_k_sq > 0) {
               float to_k = powf(to_k_sq, 0.25f);
               temperature_data[i] = to_k - 273.15f;
           } else {
               temperature_data[i] = -999.0f;  // 无效值标记
           }
       }
   }
   ```

3. **推荐：移植官方 API**
   - 如果上面的简化版本结果不理想，强烈建议移植官方 API
   - 步骤：
     * 下载 https://github.com/melexis/mlx90640-library
     * 将 `MLX90640_API.c` 和 `MLX90640_API.h` 复制到工程 `Core/Src` 目录
     * 修改其中的 I2C 读写函数：

   ```c
   // 替换原来的平台相关函数
   int MLX90640_I2CRead(uint8_t slave_addr, uint16_t start_addr, uint16_t n_words, uint16_t *data)
   {
       // 使用 STM32 HAL 实现
       return HAL_I2C_Mem_Read(&hi2c1, slave_addr << 1, start_addr,
                               I2C_MEMADD_SIZE_16BIT,
                               (uint8_t*)data, n_words * 2, 100) == HAL_OK ? 0 : -1;
   }

   int MLX90640_I2CWrite(uint8_t slave_addr, uint16_t write_addr, uint16_t data)
   {
       uint8_t buf[2] = { data >> 8, data & 0xFF };
       return HAL_I2C_Mem_Write(&hi2c1, slave_addr << 1, write_addr,
                                I2C_MEMADD_SIZE_16BIT, buf, 2, 100) == HAL_OK ? 0 : -1;
   }
   ```

**预期结果：**
- 所有 768 个像素都有温度值
- 温度值在合理范围内（接近室温）
- 对着热物体（如手掌）时，对应区域的温度明显升高

**常见问题：**

**Q: 自己实现的温度值不准确？**
A: 强烈建议使用 Melexis 官方 API。官方 API 考虑了所有补偿因素和边界情况。自己实现容易遗漏参数或公式有误。

**Q: 移植官方 API 编译报错？**
A:
1. 确保工程中启用了 `math.h`（需要 `#include <math.h>`）
2. 可能需要在链接器设置中启用浮点运算库
3. 在 CubeIDE 工程属性中，检查 MCU Settings -> Use float with printf/newlib

---

### Task 4: 串口输出温度矩阵并验证 (estimated 45 minutes)

**目标：** 将 32x24 温度矩阵通过串口格式化输出，并验证数据合理性

**步骤：**

1. **编写温度矩阵输出函数**

   ```c
   /**
    * @brief 通过串口输出温度矩阵
    */
   void PrintTemperatureMatrix(void)
   {
       printf("\r\n=== Temperature Matrix (32x24) ===\r\n");

       // 输出统计信息
       float min_temp = 999.0f, max_temp = -999.0f, sum = 0.0f;

       for (int i = 0; i < MLX90640_PIXEL_NUM; i++) {
           if (temperature_data[i] < min_temp) min_temp = temperature_data[i];
           if (temperature_data[i] > max_temp) max_temp = temperature_data[i];
           sum += temperature_data[i];
       }

       float avg_temp = sum / MLX90640_PIXEL_NUM;
       printf("Min: %.1fC | Max: %.1fC | Avg: %.1fC\r\n", min_temp, max_temp, avg_temp);

       // 以 8x6 的简化格式输出（每 4 个像素取平均）
       printf("\r\nSimplified view (8x6 blocks):\r\n");
       for (int row = 0; row < 24; row += 4) {
           for (int col = 0; col < 32; col += 4) {
               float block_sum = 0;
               int count = 0;
               for (int r = row; r < row + 4 && r < 24; r++) {
                   for (int c = col; c < col + 4 && c < 32; c++) {
                       block_sum += temperature_data[r * 32 + c];
                       count++;
                   }
               }
               printf("%5.1f ", block_sum / count);
           }
           printf("\r\n");
       }
   }
   ```

2. **在 while(1) 中持续输出**
   ```c
   /* USER CODE BEGIN 3 */
   // 读取两个半帧
   if (MLX90640_ReadSubpage(0) == HAL_OK) {
       if (MLX90640_ReadSubpage(1) == HAL_OK) {
           MLX90640_CalculateAllTemperatures(0.95f);
           PrintTemperatureMatrix();
       }
   }
   HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
   /* USER CODE END 3 */
   ```

3. **验证数据合理性**
   - 将传感器放在桌面上，观察温度是否在 20-30C 范围
   - 用手掌放在传感器前方 10cm 处，观察对应区域温度是否升高
   - 用一杯热水放在传感器前方，观察温度变化
   - 记录观察结果

**预期结果：**
- 串口输出 8x6 简化温度矩阵
- 室温环境下，所有温度值在 20-30C 范围
- 手掌靠近时，中心区域温度升高到 30-35C

**常见问题：**

**Q: 温度矩阵中有些值异常（-999 或极大值）？**
A: 这通常是坏像素（defective pixel）。每个 MLX90640 可能有几个坏像素。可以在后处理中用周围像素的平均值替换。Day 7 会处理这个问题。

**Q: 输出速度很慢？**
A: 串口打印大量数据需要时间。可以降低输出频率（比如每 5 秒输出一次），或者只输出统计信息而非完整矩阵。

---

## 今日作业 | Homework

1. **温度计算验证**（必须）
   - 成功计算出温度矩阵
   - 用手掌验证温度变化
   - 提交串口输出截图（展示手掌测试前后的温度变化）

2. **算法笔记**（必须）
   - 画出温度计算的流程图
   - 记录关键公式和变量含义
   - 标注你在实现中遇到的问题

3. **精度测试**（推荐）
   - 用温度计测量室温，与传感器读数对比
   - 测量热水温度（注意安全！），与传感器读数对比
   - 记录偏差值

4. **代码优化**（可选）
   - 如果还没移植官方 API，尝试移植并对比结果
   - 比较自实现和官方 API 的精度差异

## 明日预告 | Tomorrow's Preview

明天我们将对传感器数据进行校验和优化。你将学习如何处理坏像素、添加简单的滤波算法、调整刷新率以获得最佳性能。至此，Phase 2 的传感器驱动开发就完成了！

## 参考资源 | References

- **Melexis 官方 MLX90640 API**：https://github.com/melexis/mlx90640-library
- **MLX90640 数据手册**（Temperature Calculation 章节）
- **开源固件 thermal_bridge**：https://github.com/colourfate/thermal_bridge
- **STM32 浮点运算配置**：CubeIDE 工程属性 -> MCU Settings
- **C 语言 math.h 函数**：https://en.cppreference.com/w/c/numeric/math

---

*预计完成时间：5-7 小时*
*Estimated completion time: 5-7 hours*
