# Day 4: MLX90640 传感器原理 | MLX90640 Sensor Principles

## 学习目标 | Learning Objectives

- 理解红外热成像的物理原理（黑体辐射、红外波段）
- 掌握 MLX90640 传感器的内部结构和寄存器地图
- 能阅读芯片数据手册中的关键参数
- 编写代码读取传感器的 ID 和配置信息
- 理解传感器的工作模式（刷新率、分辨率）

## 前置准备 | Prerequisites

- [ ] Day 3 I2C 扫描成功检测到 MLX90640（地址 0x33）
- [ ] 串口输出正常工作
- [ ] 已下载 MLX90640 数据手册（PDF）

## 为什么学这个？| Why This Matters

数据手册（Datasheet）是硬件工程师最重要的"课本"。每一个芯片都有数百页的数据手册，详细记录了它的每一个寄存器、每一个参数、每一个时序要求。学会阅读数据手册，就像学会了阅读"硬件的使用说明书"，你就能独立使用任何新芯片。今天我们将一起阅读 MLX90640 的数据手册，理解这个红外传感器是如何"看到"温度的。

## 今日任务 | Today's Tasks

### Task 1: 理解红外热成像原理 (estimated 40 minutes)

**目标：** 理解 MLX90640 测量温度的物理原理

**步骤：**

1. **学习红外辐射基础**
   - 任何温度高于绝对零度（-273.15C）的物体都会发射红外辐射
   - 物体温度越高，发射的红外辐射越强
   - 斯特藩-玻尔兹曼定律：辐射功率与温度的四次方成正比
   - MLX90640 工作在 8-14 微米波段（长波红外，LWIR）
   - 这个波段正好是常温物体（-40C ~ 300C）辐射最强的波段

2. **理解 MLX90640 的工作方式**
   - 32x24 = 768 个红外像素（也叫 thermopile array）
   - 每个像素由一个热电堆（thermopile）组成
   - 热电堆将红外辐射转换为微弱的电压信号
   - 内部 ADC 将电压信号转换为数字值
   - 内部信号处理器（DSP）进行温度补偿和计算

3. **理解测量范围和精度**
   - 测温范围：-40C ~ 300C（物体温度）
   - 传感器自身温度范围：-40C ~ 85C
   - 精度：约 +-1.5C（在室温附近）
   - 视场角（FOV）：110x75 度
   - 刷新率：最高 64Hz（但数据量大，我们通常用 2-8Hz）

4. **对比可见光和红外成像**
   - 可见光相机：接收物体反射的可见光（380-700nm 波长）
   - 红外热像仪：接收物体自身发射的红外辐射（8000-14000nm 波长）
   - 因此热像仪在完全黑暗的环境中也能工作
   - 这就是为什么热成像在消防、安防中有独特优势

**预期结果：**
- 能用自己的话解释红外热成像的原理
- 理解 MLX90640 的基本参数（分辨率、测温范围、精度）
- 知道为什么热像仪在黑暗中也能工作

**常见问题：**

**Q: 为什么热像仪看到的人是彩色的？人又不是彩色的？**
A: 热像仪看到的不是颜色，而是温度差异。APP 中的颜色是"伪彩色映射" -- 用颜色来表示温度（比如红色=高温，蓝色=低温），方便人眼观察。这在 Day 12 会学到。

---

### Task 2: 阅读 MLX90640 数据手册 (estimated 50 minutes)

**目标：** 学会从数据手册中提取关键信息，找到寄存器地址

**步骤：**

1. **下载数据手册**
   - 搜索 "MLX90640 datasheet" 或访问 Melexis 官网
   - 文档名称通常是 "MLX90640 IR Array Data Sheet"
   - 打开 PDF 文件

2. **找到关键参数（在数据手册前几页）**
   - 记录以下参数：
     * 供电电压范围：___
     * I2C 最大速率：___
     * 像素分辨率：___
     * 帧率范围：___
     * 测温精度：___
     * 工作温度范围：___

3. **找到寄存器地图（Register Map）**
   - 在数据手册中搜索 "Register Map" 或 "Register Summary"
   - 找到以下关键寄存器：
     * **0x2400 - 0x273F**：RAM 区域，存储像素数据（64 words = 128 bytes 用于 32x24 像素的一半）
     * **0x2400 - 0x241F**：上半帧像素数据（32x12 = 384 值，但实际按半帧存储）
     * **0x2500 - 0x251F**：下半帧像素数据
     * **0x2800 - 0x2803**：Ta（环境温度，传感器自身温度）
     * **0x240D - 0x2420**：控制寄存器
     * **0x2C00 - 0x2C01**：状态寄存器
     * **0x3000 - 0x3406**：EEPROM 区域，存储校准参数

4. **理解帧数据结构**
   - MLX90640 每帧数据分两个半帧（subpage）传输
   - 半帧 0（Subpage 0）：包含偶数行像素 + 部分控制参数
   - 半帧 1（Subpage 1）：包含奇数行像素 + 部分控制参数
   - 需要读取两个半帧才能获得完整的 32x24 图像
   - 每个半帧包含 832 个 16 位字（word）

5. **理解控制寄存器**
   - 寄存器 0x800D：控制寄存器 1
     * Bit 9-8：刷新率设置（00=0.5Hz, 01=1Hz, 10=2Hz, 11=4Hz, ...）
     * Bit 10：测量分辨率（0=16-bit, 1=17-bit, 2=18-bit, 3=19-bit）
   - 寄存器 0x800E：控制寄存器 2（I2C 地址等）
   - 寄存器 0x800F：I2C 地址配置

**预期结果：**
- 能在数据手册中找到关键寄存器地址
- 理解帧数据的结构（两个半帧）
- 知道控制寄存器的位置和作用

**常见问题：**

**Q: 数据手册全是英文看不懂？**
A: 数据手册确实比较难读。重点看表格中的参数和寄存器地址，文字描述可以跳过。也可以尝试使用翻译工具。最重要的是找到寄存器地址和参数含义。

**Q: 为什么分两个半帧？**
A: 因为 MLX90640 内部 ADC 逐行扫描，每帧需要分两步读取。这是传感器的硬件设计决定的，无法更改。

---

### Task 3: 读取传感器 ID 和配置 (estimated 60 minutes)

**目标：** 编写代码通过 I2C 读取 MLX90640 的 ID 寄存器和控制寄存器

**步骤：**

1. **创建新工程或复用 Day 3 工程**
   - 工程名：`ThermalEyes_MLX90640`
   - 确保已配置 I2C1（400kHz）和 USART1（115200）

2. **编写 MLX90640 读取函数**
   - 在 `main.c` 中 `/* USER CODE BEGIN 0 */` 区域添加：

   ```c
   // MLX90640 I2C 地址
   #define MLX90640_ADDR 0x33

   // MLX90640 寄存器地址
   #define MLX90640_REG_CHIP_ID      0x2407
   #define MLX90640_REG_CONTROL1     0x800D
   #define MLX90640_REG_CONTROL2     0x800E
   #define MLX90640_REG_STATUS       0x8000

   /**
    * @brief 从 MLX90640 读取 16 位寄存器
    * @param reg_addr: 16 位寄存器地址
    * @param data: 读取数据的存储指针
    * @retval HAL_StatusTypeDef
    */
   HAL_StatusTypeDef MLX90640_ReadRegister(uint16_t reg_addr, uint16_t *data)
   {
       uint8_t buf[2];
       uint8_t reg_buf[2];

       // 寄存器地址是 16 位，需要先发送高字节再发送低字节
       reg_buf[0] = (reg_addr >> 8) & 0xFF;  // 高字节
       reg_buf[1] = reg_addr & 0xFF;          // 低字节

       // 发送寄存器地址
       HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(
           &hi2c1,
           MLX90640_ADDR << 1,  // 8 位地址（写方向）
           reg_buf,
           2,                    // 发送 2 字节地址
           100                   // 超时 100ms
       );

       if (status != HAL_OK) return status;

       // 接收 2 字节数据
       status = HAL_I2C_Master_Receive(
           &hi2c1,
           (MLX90640_ADDR << 1) | 1,  // 8 位地址（读方向）
           buf,
           2,
           100
       );

       if (status != HAL_OK) return status;

       // MLX90640 使用大端序（高字节在前）
       *data = ((uint16_t)buf[0] << 8) | buf[1];

       return HAL_OK;
   }
   ```

3. **在 main() 中读取并打印传感器信息**
   - 在 `while(1)` 之前添加：

   ```c
   /* USER CODE BEGIN 2 */
   printf("\r\n=== MLX90640 Sensor Info ===\r\n");

   uint16_t chip_id = 0;
   if (MLX90640_ReadRegister(MLX90640_REG_CHIP_ID, &chip_id) == HAL_OK) {
       printf("Chip ID: 0x%04X\r\n", chip_id);
   } else {
       printf("ERROR: Failed to read Chip ID\r\n");
   }

   uint16_t ctrl1 = 0;
   if (MLX90640_ReadRegister(MLX90640_REG_CONTROL1, &ctrl1) == HAL_OK) {
       printf("Control Register 1: 0x%04X\r\n", ctrl1);
       // 解析刷新率（Bit 9-8）
       uint8_t refresh_rate = (ctrl1 >> 8) & 0x07;
       printf("  Refresh Rate Setting: %d\r\n", refresh_rate);
       // 解析分辨率（Bit 10-11 之间有差异，具体看数据手册）
       uint8_t resolution = (ctrl1 >> 10) & 0x03;
       printf("  ADC Resolution: %d-bit\r\n", 16 + resolution);
   }

   uint16_t ctrl2 = 0;
   if (MLX90640_ReadRegister(MLX90640_REG_CONTROL2, &ctrl2) == HAL_OK) {
       printf("Control Register 2: 0x%04X\r\n", ctrl2);
   }

   printf("=============================\r\n");
   /* USER CODE END 2 */
   ```

4. **编译、烧录、查看结果**
   - 编译并烧录
   - 打开串口工具查看输出
   - 预期看到类似输出：
   ```
   === MLX90640 Sensor Info ===
   Chip ID: 0x0122
   Control Register 1: 0x0304
     Refresh Rate Setting: 3
     ADC Resolution: 18-bit
   Control Register 2: 0x1C60
   =============================
   ```
   - 你的具体数值可能不同，这取决于传感器版本和默认配置

**预期结果：**
- 成功读取到 Chip ID（通常为 0x0122 或类似值）
- 能读取控制寄存器并解析刷新率和分辨率参数
- 理解 16 位寄存器地址的 I2C 读取流程

**常见问题：**

**Q: 读取失败返回 HAL_ERROR？**
A:
1. 确认 Day 3 的 I2C 扫描能找到设备
2. 检查寄存器地址是否正确（16 位，大端序发送）
3. 尝试增加超时时间
4. 确认 I2C 速率配置正确

**Q: Chip ID 读出来是 0x0000 或 0xFFFF？**
A: 0x0000 可能是读到了错误的寄存器，0xFFFF 可能是总线没有应答。检查 I2C 地址和寄存器地址。

---

### Task 4: 配置传感器工作模式 (estimated 45 minutes)

**目标：** 编写代码修改 MLX90640 的刷新率和分辨率设置

**步骤：**

1. **编写寄存器写入函数**
   - 在 `/* USER CODE BEGIN 0 */` 区域添加：

   ```c
   /**
    * @brief 向 MLX90640 写入 16 位寄存器
    * @param reg_addr: 16 位寄存器地址
    * @param data: 要写入的 16 位数据
    * @retval HAL_StatusTypeDef
    */
   HAL_StatusTypeDef MLX90640_WriteRegister(uint16_t reg_addr, uint16_t data)
   {
       uint8_t buf[4];

       // 构造发送缓冲区：[地址高字节][地址低字节][数据高字节][数据低字节]
       buf[0] = (reg_addr >> 8) & 0xFF;
       buf[1] = reg_addr & 0xFF;
       buf[2] = (data >> 8) & 0xFF;
       buf[3] = data & 0xFF;

       return HAL_I2C_Master_Transmit(
           &hi2c1,
           MLX90640_ADDR << 1,
           buf,
           4,
           100
       );
   }

   /**
    * @brief 设置 MLX90640 刷新率
    * @param rate: 刷新率代码 (0=0.5Hz, 1=1Hz, 2=2Hz, 3=4Hz, 4=8Hz, 5=16Hz, 6=32Hz, 7=64Hz)
    * @retval HAL_StatusTypeDef
    */
   HAL_StatusTypeDef MLX90640_SetRefreshRate(uint8_t rate)
   {
       uint16_t ctrl1;

       // 先读取当前值
       HAL_StatusTypeDef status = MLX90640_ReadRegister(MLX90640_REG_CONTROL1, &ctrl1);
       if (status != HAL_OK) return status;

       // 清除刷新率位（Bit 9-7，共3位）
       ctrl1 &= ~(0x07 << 7);
       // 设置新的刷新率
       ctrl1 |= (rate & 0x07) << 7;

       // 写回
       return MLX90640_WriteRegister(MLX90640_REG_CONTROL1, ctrl1);
   }
   ```

2. **配置传感器并验证**
   - 在 main() 中添加：

   ```c
   // 设置刷新率为 2Hz（rate=2）
   printf("\r\nSetting refresh rate to 2Hz...\r\n");
   if (MLX90640_SetRefreshRate(2) == HAL_OK) {
       HAL_Delay(100);  // 等待设置生效

       // 回读验证
       uint16_t ctrl1_new;
       MLX90640_ReadRegister(MLX90640_REG_CONTROL1, &ctrl1_new);
       printf("New Control Register 1: 0x%04X\r\n", ctrl1_new);

       uint8_t new_rate = (ctrl1_new >> 7) & 0x07;
       printf("New Refresh Rate Setting: %d\r\n", new_rate);
   } else {
       printf("Failed to set refresh rate!\r\n");
   }
   ```

3. **测试不同配置**
   - 尝试设置不同的刷新率（0-7），每次回读验证
   - 记录每次设置后的寄存器值

**预期结果：**
- 成功修改传感器的刷新率设置
- 回读验证寄存器值已改变
- 理解"读-改-写"的寄存器操作模式

**常见问题：**

**Q: 写入后回读值没有变化？**
A: 某些寄存器是只读的（如 EEPROM 中的校准参数）。控制寄存器应该可写。确认你写入的寄存器地址正确。

**Q: 设置刷新率后需要等待多久？**
A: 设置后需要等待至少一个帧周期才能生效。例如设置 2Hz 后等待 500ms。建议设置后等待 100-200ms。

---

## 今日作业 | Homework

1. **传感器信息读取**（必须）
   - 成功读取并记录 MLX90640 的 Chip ID
   - 读取并解析控制寄存器的各字段
   - 提交串口输出截图和代码

2. **数据手册笔记**（必须）
   - 在笔记中整理 MLX90640 的关键参数表
   - 列出你最常用的 5 个寄存器地址及其用途
   - 画出传感器帧数据结构图

3. **配置实验**（推荐）
   - 尝试设置不同的刷新率，验证回读结果
   - 记录不同刷新率设置对应的控制寄存器值

4. **思考题**（可选）
   - 为什么红外热像仪的分辨率（32x24）比普通相机低这么多？
   - 如果要测量一个 100C 的物体，传感器需要满足什么条件？

## 明日预告 | Tomorrow's Preview

明天我们将编写 MLX90640 的完整驱动程序！你将从传感器读取实际的像素数据，获取 32x24 个温度值。这将是你第一次"看到"温度数据。

## 参考资源 | References

- **MLX90640 数据手册**：https://www.melexis.com/en/product/mlx90640
- **MLX90640 寄存器地图**：数据手册中的 Register Map 章节
- **红外热成像原理**：https://en.wikipedia.org/wiki/Thermographic_camera
- **Melexis MLX90640 应用笔记**：https://www.melexis.com/en/documents/documentation/application-notes
- **开源 MLX90640 驱动参考**：https://github.com/colourfate/thermal_bridge

---

*预计完成时间：5-6 小时*
*Estimated completion time: 5-6 hours*
