# Day 7: 传感器校验与数据优化 | Sensor Calibration & Data Optimization

## 学习目标 | Learning Objectives

- 理解坏像素的概念及检测方法
- 实现简单的坏像素补偿算法
- 学习并实现基本的数据滤波（移动平均）
- 优化传感器刷新率和数据传输效率
- 完成 Phase 2 里程碑检查

## 前置准备 | Prerequisites

- [ ] Day 6 温度计算完成，能输出 32x24 温度矩阵
- [ ] 温度数据基本合理（室温范围内）
- [ ] 已完成 Phase 2 之前所有的课程内容

## 为什么学这个？| Why This Matters

真实的传感器数据从来不是"完美的"。任何传感器都会有噪声、坏像素、漂移等问题。学会处理这些不完美的数据，是从"实验室原型"走向"实用产品"的关键一步。在工业界，信号处理和数据校准通常占传感器项目 50% 以上的开发时间。今天学的坏像素补偿和滤波技术，是信号处理领域最基础也最实用的方法。

## 今日任务 | Today's Tasks

### Task 1: 检测和标记坏像素 (estimated 45 minutes)

**目标：** 找出温度矩阵中的坏像素，建立坏像素掩码

**步骤：**

1. **理解坏像素的来源**
   - 制造工艺导致个别热电堆损坏或偏差过大
   - 表现为：温度值异常（-999, 0, 或极大值）
   - MLX90640 通常有 0-5 个坏像素
   - 每个传感器的坏像素位置不同

2. **编写坏像素检测代码**

   ```c
   #define MAX_BAD_PIXELS 16
   uint16_t bad_pixels[MAX_BAD_PIXELS];  // 存储坏像素索引
   uint8_t bad_pixel_count = 0;

   /**
    * @brief 检测坏像素
    * 坏像素的判定条件：
    * 1. 温度值 < -40C（超出测量范围）
    * 2. 温度值 > 300C（超出测量范围）
    * 3. 与周围像素偏差超过 20C
    */
   void MLX90640_DetectBadPixels(void)
   {
       bad_pixel_count = 0;

       for (int row = 0; row < 24; row++) {
           for (int col = 0; col < 32; col++) {
               int idx = row * 32 + col;
               float temp = temperature_data[idx];

               // 条件1: 超出范围
               if (temp < -40.0f || temp > 300.0f) {
                   if (bad_pixel_count < MAX_BAD_PIXELS) {
                       bad_pixels[bad_pixel_count++] = idx;
                   }
                   continue;
               }

               // 条件2: 与邻居偏差过大（仅内部像素）
               if (row > 0 && row < 23 && col > 0 && col < 31) {
                   float left  = temperature_data[idx - 1];
                   float right = temperature_data[idx + 1];
                   float up    = temperature_data[idx - 32];
                   float down  = temperature_data[idx + 32];
                   float neighbor_avg = (left + right + up + down) / 4.0f;
                   float diff = temp - neighbor_avg;
                   if (diff < 0) diff = -diff;  // 取绝对值

                   if (diff > 20.0f) {
                       if (bad_pixel_count < MAX_BAD_PIXELS) {
                           bad_pixels[bad_pixel_count++] = idx;
                       }
                   }
               }
           }
       }

       printf("Bad pixels detected: %d\r\n", bad_pixel_count);
       for (int i = 0; i < bad_pixel_count; i++) {
           int idx = bad_pixels[i];
           printf("  Pixel[%d,%d] (idx=%d): %.1fC\r\n",
                  idx / 32, idx % 32, idx, temperature_data[idx]);
       }
   }
   ```

3. **测试坏像素检测**
   - 运行检测，记录发现的坏像素数量和位置
   - 通常发现 0-3 个坏像素是正常的

**预期结果：**
- 检测到坏像素（如果有的话）
- 每个坏像素的位置和异常值被记录
- 理解坏像素的判定标准

---

### Task 2: 坏像素补偿与数据滤波 (estimated 60 minutes)

**目标：** 用邻域插值补偿坏像素，实现移动平均滤波

**步骤：**

1. **坏像素补偿**

   ```c
   /**
    * @brief 补偿坏像素（使用邻域平均值）
    */
   void MLX90640_CompensateBadPixels(void)
   {
       for (int i = 0; i < bad_pixel_count; i++) {
           int idx = bad_pixels[i];
           int row = idx / 32;
           int col = idx % 32;

           float sum = 0;
           int count = 0;

           // 取上下左右邻居的平均值
           if (row > 0) {
               sum += temperature_data[(row - 1) * 32 + col];
               count++;
           }
           if (row < 23) {
               sum += temperature_data[(row + 1) * 32 + col];
               count++;
           }
           if (col > 0) {
               sum += temperature_data[row * 32 + (col - 1)];
               count++;
           }
           if (col < 31) {
               sum += temperature_data[row * 32 + (col + 1)];
               count++;
           }

           if (count > 0) {
               temperature_data[idx] = sum / count;
           }
       }
   }
   ```

2. **实现移动平均滤波**

   ```c
   #define FILTER_SIZE 3  // 滤波窗口大小

   float temp_history[FILTER_SIZE][MLX90640_PIXEL_NUM];
   uint8_t history_index = 0;
   uint8_t history_filled = 0;

   /**
    * @brief 更新温度历史并计算移动平均
    */
   void MLX90640_ApplyMovingAverage(void)
   {
       // 保存当前帧到历史
       for (int i = 0; i < MLX90640_PIXEL_NUM; i++) {
           temp_history[history_index][i] = temperature_data[i];
       }

       history_index = (history_index + 1) % FILTER_SIZE;
       if (!history_filled && history_index == 0) {
           history_filled = 1;
       }

       // 计算移动平均
       if (history_filled) {
           for (int i = 0; i < MLX90640_PIXEL_NUM; i++) {
               float sum = 0;
               for (int j = 0; j < FILTER_SIZE; j++) {
                   sum += temp_history[j][i];
               }
               temperature_data[i] = sum / FILTER_SIZE;
           }
       }
   }
   ```

3. **空间域简单滤波（可选）**

   ```c
   /**
    * @brief 空间域均值滤波（3x3 窗口）
    * @note 简化版，不处理边界
    */
   void MLX90640_SpatialFilter(void)
   {
       float filtered[MLX90640_PIXEL_NUM];

       for (int row = 1; row < 23; row++) {
           for (int col = 1; col < 31; col++) {
               int idx = row * 32 + col;
               float sum = 0;

               // 3x3 邻域
               for (int dr = -1; dr <= 1; dr++) {
                   for (int dc = -1; dc <= 1; dc++) {
                       sum += temperature_data[(row + dr) * 32 + (col + dc)];
                   }
               }
               filtered[idx] = sum / 9.0f;
           }
       }

       // 边界像素保持原值
       for (int row = 1; row < 23; row++) {
           for (int col = 1; col < 31; col++) {
               temperature_data[row * 32 + col] = filtered[row * 32 + col];
           }
       }
   }
   ```

**预期结果：**
- 坏像素被补偿，温度矩阵中不再有异常值
- 滤波后的数据更平滑，噪声降低
- 理解时间域滤波和空间域滤波的区别

**常见问题：**

**Q: 滤波后温度变化延迟了？**
A: 移动平均滤波确实会引入延迟。FILTER_SIZE 越大，延迟越明显，但噪声抑制越好。需要根据应用场景权衡。

---

### Task 3: 优化数据采集效率 (estimated 45 minutes)

**目标：** 分析数据采集的时间开销，优化数据读取策略

**步骤：**

1. **测量各步骤的耗时**
   ```c
   // 在 main.c 中使用 HAL_GetTick() 测量时间
   uint32_t t_start, t_end;

   t_start = HAL_GetTick();
   MLX90640_ReadSubpage(0);
   t_end = HAL_GetTick();
   printf("Subpage0 read: %lu ms\r\n", t_end - t_start);

   t_start = HAL_GetTick();
   MLX90640_ReadSubpage(1);
   t_end = HAL_GetTick();
   printf("Subpage1 read: %lu ms\r\n", t_end - t_start);

   t_start = HAL_GetTick();
   MLX90640_CalculateAllTemperatures(0.95f);
   t_end = HAL_GetTick();
   printf("Temperature calc: %lu ms\r\n", t_end - t_start);
   ```

2. **分析瓶颈**
   - I2C 读取通常是最耗时的步骤
   - 832 words x 2 bytes = 1664 bytes
   - 在 400kHz I2C 下，理论最小时间约 33ms
   - 实际可能有额外开销

3. **优化策略**
   - 使用 DMA 方式读取 I2C（减少 CPU 等待）
   - 选择合适的刷新率（2-4Hz 对本项目足够）
   - 只在数据就绪时读取（使用状态寄存器）

4. **配置 DMA（如果时间允许）**
   - 在 CubeMX 中为 I2C1 启用 DMA
   - 使用 `HAL_I2C_Mem_Read_DMA()` 替代阻塞式读取
   - 在 DMA 完成回调中处理数据

**预期结果：**
- 知道各步骤的耗时
- 选择合适的刷新率
- 理解 DMA 对性能的提升

---

### Task 4: Phase 2 里程碑检查 (estimated 30 minutes)

**目标：** 验证 Phase 2 的所有功能是否完成

**步骤：**

1. **运行完整的测试流程**
   - 初始化传感器
   - 读取校准参数
   - 配置刷新率
   - 读取两个半帧
   - 计算温度矩阵
   - 检测和补偿坏像素
   - 应用滤波
   - 输出最终温度矩阵

2. **验证以下功能**
   - [ ] I2C 通信正常
   - [ ] 校准参数读取成功
   - [ ] 帧数据读取完整
   - [ ] 温度计算结果合理
   - [ ] 坏像素已补偿
   - [ ] 滤波效果可见
   - [ ] 串口输出正常

3. **完成 Phase 2 检查报告**
   - 记录测试结果
   - 记录遇到的问题和解决方案
   - 准备进入 Phase 3（USB 通信）

**预期结果：**
- 所有检查项通过
- 稳定地输出温度数据
- 准备好进入 USB 通信阶段

---

## 今日作业 | Homework

1. **数据处理代码**（必须）
   - 完成坏像素补偿和滤波功能
   - 提交完整的温度矩阵输出截图
   - 对比处理前后的数据差异

2. **Phase 2 总结报告**（必须）
   - 写一份 Phase 2 的学习总结（500 字以上）
   - 包含：学到的关键技术、遇到的问题、解决方案
   - 画出完整的数据处理流程图

3. **性能测试**（推荐）
   - 记录各步骤的耗时
   - 测试不同刷新率下的帧率
   - 测试手掌检测的最大距离

4. **挑战任务**（可选）
   - 实现异常值检测：当检测到温度超过阈值时闪烁 LED 报警
   - 实现简单的运动检测：对比前后两帧的差异

## 明日预告 | Tomorrow's Preview

明天我们进入 Phase 3！你将学习 USB 通信协议，配置 STM32 的 USB CDC（虚拟串口），让温度数据通过 USB 实时传输到电脑。这是设备从"独立工作"变成"与手机通信"的关键一步。

## 参考资源 | References

- **坏像素补偿算法**：https://ieeexplore.ieee.org/document/8569593
- **移动平均滤波**：https://en.wikipedia.org/wiki/Moving_average
- **STM32 I2C DMA 配置**：https://deepbluembedded.com/stm32-i2c-dma-tutorial/
- **Melexis MLX90640 应用笔记**：https://www.melexis.com/en/documents/documentation/application-notes

---

*预计完成时间：5-6 小时*
*Estimated completion time: 5-6 hours*
