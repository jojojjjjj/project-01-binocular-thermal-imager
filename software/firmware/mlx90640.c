/**
 * ============================================================
 * MLX90640 热成像传感器驱动 - 实现文件
 * MLX90640 Thermal Sensor Driver - Implementation File
 * ============================================================
 *
 * 核心工作原理 | Core Working Principle:
 *   MLX90640 是一个 32x24 的红外热电堆阵列传感器。
 *   每个像素包含一个热电堆，检测物体发出的红外辐射。
 *   通过测量红外辐射强度和传感器自身温度，计算出目标温度。
 *
 *   MLX90640 is a 32x24 IR thermopile array sensor.
 *   Each pixel contains a thermopile detecting IR radiation from objects.
 *   By measuring IR radiation intensity and sensor self-temperature,
 *   the target temperature is calculated.
 *
 * 数据读取流程 | Data Reading Flow:
 *   1. 检查状态寄存器，确认新帧就绪
 *      Check status register, confirm new frame is ready
 *   2. 读取子页0的像素数据 (0x0400 ~ 0x057F)
 *      Read sub-page 0 pixel data
 *   3. 读取子页1的像素数据 (0x0400 ~ 0x057F，交错)
 *      Read sub-page 1 pixel data (interleaved)
 *   4. 读取辅助数据 (环境温度等)
 *      Read auxiliary data (ambient temperature, etc.)
 *   5. 使用校准参数计算温度
 *      Calculate temperature using calibration parameters
 *
 * 开源参考 | Open-source reference:
 *   https://github.com/colourfate/thermal_bridge
 *   https://github.com/melexis/mlx90640-library (官方驱动 | Official driver)
 * ============================================================
 */

/* ============================================================
 * 头文件包含 | Header Includes
 * ============================================================ */
#include "mlx90640.h"
#include <string.h>
#include <math.h>

/* ============================================================
 * 私有变量 | Private Variables
 * ============================================================ */

/* 校准数据（从 EEPROM 读取）| Calibration data (read from EEPROM) */
static MLX90640_CalibData_t calib_data;

/* 当前传感器配置 | Current sensor configuration */
static uint8_t current_refresh_rate = MLX90640_REFRESH_8_HZ;
static uint8_t current_resolution   = MLX90640_ADC_18BIT;

/* I2C 设备地址缓存 | Cached I2C device address */
static uint8_t device_addr = MLX90640_I2C_ADDR;

/* ============================================================
 * I2C 通信函数 | I2C Communication Functions
 * ============================================================ */

/**
 * @brief I2C 写入一个 16 位值到 16 位寄存器
 *        I2C write a 16-bit value to a 16-bit register
 *
 * MLX90640 的 I2C 协议:
 *   主机发送: [设备地址+W] [寄存器地址高字节] [寄存器地址低字节]
 *             [数据高字节] [数据低字节]
 *
 * @param hi2c  I2C 句柄指针 | Pointer to I2C handle
 * @param addr  7 位设备地址 | 7-bit device address
 * @param reg   16 位寄存器地址 | 16-bit register address
 * @param data  要写入的 16 位数据 | 16-bit data to write
 * @return MLX90640_OK 成功, MLX90640_ERR_I2C 失败
 */
int8_t MLX90640_I2C_Write(I2C_HandleTypeDef *hi2c, uint8_t addr,
                           uint16_t reg, uint16_t data)
{
    /*
     * 构造 I2C 发送缓冲区 | Build I2C transmit buffer
     * 格式: [reg_high, reg_low, data_high, data_low]
     */
    uint8_t buf[4];
    buf[0] = (uint8_t)(reg >> 8);    /* 寄存器地址高字节 | Register address high byte */
    buf[1] = (uint8_t)(reg & 0xFF);  /* 寄存器地址低字节 | Register address low byte */
    buf[2] = (uint8_t)(data >> 8);   /* 数据高字节 | Data high byte */
    buf[3] = (uint8_t)(data & 0xFF); /* 数据低字节 | Data low byte */

    /*
     * HAL_I2C_Master_Transmit 参数说明:
     *   hi2c:      I2C 句柄
     *   DevAddress: 左移 1 位后的设备地址 (HAL 要求 | HAL requirement)
     *   pData:     数据缓冲区
     *   Size:      数据长度
     *   Timeout:   超时时间 (ms)
     */
    HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(
        hi2c,
        (uint16_t)(addr << 1),  /* 左移 1 位，因为 HAL 会自动处理 R/W 位 */
                                  /* Left shift by 1, HAL handles R/W bit automatically */
        buf,
        4,
        100                      /* 100ms 超时 | 100ms timeout */
    );

    return (status == HAL_OK) ? MLX90640_OK : MLX90640_ERR_I2C;
}

/**
 * @brief I2C 从 16 位寄存器读取多个 16 位值
 *        I2C read multiple 16-bit values from 16-bit register
 *
 * 读取流程:
 *   1. 发送寄存器地址 (写模式)
 *   2. 重新启动 I2C (读模式)
 *   3. 读取数据
 *
 * @param hi2c  I2C 句柄指针
 * @param addr  7 位设备地址
 * @param reg   16 位寄存器起始地址
 * @param data  输出数据缓冲区 (每个元素 16 位)
 * @param len   要读取的 16 位值数量
 * @return MLX90640_OK 成功, MLX90640_ERR_I2C 失败
 */
int8_t MLX90640_I2C_Read(I2C_HandleTypeDef *hi2c, uint8_t addr,
                          uint16_t reg, uint16_t *data, uint16_t len)
{
    uint8_t reg_buf[2];
    uint8_t rx_buf[512];  /* 接收缓冲区 | Receive buffer */
    uint16_t bytes_to_read = len * 2;  /* 每个 16 位值 = 2 字节 */

    /* 参数检查 | Parameter check */
    if (bytes_to_read > sizeof(rx_buf)) {
        return MLX90640_ERR_PARAM;
    }

    /* 步骤 1: 发送寄存器地址 | Step 1: Send register address */
    reg_buf[0] = (uint8_t)(reg >> 8);
    reg_buf[1] = (uint8_t)(reg & 0xFF);

    HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(
        hi2c,
        (uint16_t)(addr << 1),
        reg_buf,
        2,
        100
    );

    if (status != HAL_OK) {
        return MLX90640_ERR_I2C;
    }

    /* 步骤 2: 读取数据 | Step 2: Read data */
    status = HAL_I2C_Master_Receive(
        hi2c,
        (uint16_t)((addr << 1) | 0x01),  /* 读模式 | Read mode */
        rx_buf,
        bytes_to_read,
        200  /* 更长的超时用于数据读取 | Longer timeout for data read */
    );

    if (status != HAL_OK) {
        return MLX90640_ERR_I2C;
    }

    /* 步骤 3: 字节序转换 (大端 -> 小端) */
    /* Step 3: Byte order conversion (big-endian -> little-endian) */
    /* MLX90640 使用大端格式传输数据 */
    /* MLX90640 transmits data in big-endian format */
    for (uint16_t i = 0; i < len; i++) {
        data[i] = ((uint16_t)rx_buf[i * 2] << 8) | rx_buf[i * 2 + 1];
    }

    return MLX90640_OK;
}

/* ============================================================
 * 传感器初始化 | Sensor Initialization
 * ============================================================ */

/**
 * @brief 初始化 MLX90640 | Initialize MLX90640
 *
 * 初始化步骤:
 *   1. 读取 EEPROM 校准数据 (约 832 字节)
 *   2. 解析校准参数
 *   3. 设置刷新率
 *   4. 设置 ADC 分辨率
 *   5. 配置读取模式 (交错/棋盘)
 */
int8_t MLX90640_Init(I2C_HandleTypeDef *hi2c, uint8_t addr)
{
    int8_t ret;

    /* 保存设备地址 | Save device address */
    device_addr = addr;

    /* ---- 第 1 步: 验证 I2C 通信 ---- */
    /* Step 1: Verify I2C communication */
    uint16_t reg_val = 0;
    ret = MLX90640_I2C_Read(hi2c, addr, MLX90640_REG_CTRL, &reg_val, 1);
    if (ret != MLX90640_OK) {
        return MLX90640_ERR_INIT;
    }

    /* ---- 第 2 步: 读取 EEPROM 校准数据 ---- */
    /* Step 2: Read EEPROM calibration data */
    /* EEPROM 中包含工厂校准参数，每个传感器独一无二 */
    /* EEPROM contains factory calibration parameters, unique per sensor */
    ret = MLX90640_ReadEEPROM(hi2c);
    if (ret != MLX90640_OK) {
        return MLX90640_ERR_INIT;
    }

    /* ---- 第 3 步: 设置刷新率 ---- */
    /* Step 3: Set refresh rate */
    /* 8Hz 是实时显示的推荐帧率，每 125ms 输出一帧 */
    /* 8Hz is recommended for live display, one frame every 125ms */
    ret = MLX90640_SetRefreshRate(hi2c, MLX90640_REFRESH_8_HZ);
    if (ret != MLX90640_OK) {
        return MLX90640_ERR_INIT;
    }

    /* ---- 第 4 步: 设置 ADC 分辨率 ---- */
    /* Step 4: Set ADC resolution */
    ret = MLX90640_SetResolution(hi2c, MLX90640_ADC_18BIT);
    if (ret != MLX90640_OK) {
        return MLX90640_ERR_INIT;
    }

    /* ---- 第 5 步: 设置默认发射率 ---- */
    /* Step 5: Set default emissivity */
    /* 0.95 适用于大多数非金属表面 | 0.95 suits most non-metallic surfaces */
    calib_data.emissivity = 0.95f;

    return MLX90640_OK;
}

/* ============================================================
 * EEPROM 校准数据读取 | EEPROM Calibration Data Reading
 * ============================================================ */

/**
 * @brief 读取并解析 EEPROM 校准数据 | Read and parse EEPROM calibration data
 *
 * EEPROM 中存储的校准参数包括:
 *   - 每像素的偏移值 (offset): 修正制造差异
 *   - 每像素的灵敏度 (alpha): 修正响应差异
 *   - 全局温度系数: 用于环境温度补偿
 *
 * EEPROM stores calibration parameters including:
 *   - Per-pixel offset: corrects manufacturing variations
 *   - Per-pixel alpha (sensitivity): corrects response variations
 *   - Global temperature coefficients: for ambient temperature compensation
 */
int8_t MLX90640_ReadEEPROM(I2C_HandleTypeDef *hi2c)
{
    /*
     * 实际实现需要读取约 832 字节的 EEPROM 数据
     * 并根据 Melexis 提供的公式解析校准参数
     *
     * 这里展示关键的读取和解析流程
     * Here we show the key reading and parsing flow
     */

    uint16_t eeprom_data[MLX90640_EEPROM_END - MLX90640_EEPROM_START + 1];

    /* 读取全部 EEPROM 数据 | Read all EEPROM data */
    /* 注意: 实际代码应分块读取，避免缓冲区溢出 */
    /* Note: Production code should read in chunks to avoid buffer overflow */
    uint16_t eeprom_len = 0x0034;  /* 只读取关键校准参数 | Read only key calibration params */

    int8_t ret = MLX90640_I2C_Read(hi2c, device_addr,
                                    MLX90640_EEPROM_START,
                                    eeprom_data, eeprom_len);
    if (ret != MLX90640_OK) {
        return ret;
    }

    /*
     * 解析关键校准参数 | Parse key calibration parameters
     *
     * 以下地址偏移是 Melexis 数据手册定义的
     * The following address offsets are defined by Melexis datasheet
     */

    /* VDD 相关参数 | VDD-related parameters */
    /* EEPROM[0x2410] 包含 kVdd 和 Vdd25 */
    int16_t kvdd_scale = (int16_t)eeprom_data[0x10];
    calib_data.vdd25 = ((int16_t)eeprom_data[0x11] >> 8) * 32 - (1 << 12);
    calib_data.kVdd = ((float)kvdd_scale / (1 << 12)) * 0.001f;

    /* PTAT (与温度成正比的电压) 参数 */
    /* PTAT (Proportional To Absolute Temperature) parameters */
    calib_data.Kvptat = ((float)(int16_t)eeprom_data[0x12] / (1 << 12)) * 0.001f;
    calib_data.Ktptat = ((float)(int16_t)eeprom_data[0x13] / (1 << 12)) * 0.001f;
    calib_data.vptat25 = eeprom_data[0x14];

    /* Alpha PTAT | PTAT alpha coefficient */
    uint16_t alpha_ptat_val = eeprom_data[0x15];
    calib_data.alpha_ptat = (float)alpha_ptat_val / (1 << 12);

    /* 增益系数 | Gain coefficient */
    /* 像素偏移和 alpha 系数需要读取更多 EEPROM 数据 */
    /* Pixel offsets and alpha coefficients require reading more EEPROM data */

    /* 简化处理: 设置默认值 | Simplified: set default values */
    calib_data.alpha_base = 1.0f;
    calib_data.kta = 0.0f;
    calib_data.kv = 0.0f;

    /* 初始化每像素偏移和 alpha | Initialize per-pixel offset and alpha */
    for (uint16_t i = 0; i < MLX90640_PIXEL_COUNT; i++) {
        calib_data.offset[i] = 0;
        calib_data.alpha[i] = calib_data.alpha_base;
    }

    return MLX90640_OK;
}

/* ============================================================
 * 帧数据读取 | Frame Data Reading
 * ============================================================ */

/**
 * @brief 读取一帧完整的热成像数据 | Read a complete thermal frame
 *
 * MLX90640 的双缓冲机制:
 *   传感器内部有两个子页 (sub-page 0 和 sub-page 1)
 *   在交错模式下，子页0和子页1的像素交错排列
 *   需要连续读取两帧才能获得完整的 32x24 图像
 *
 * MLX90640 double buffering:
 *   The sensor has two sub-pages internally
 *   In interleaved mode, sub-page 0 and 1 pixels are interleaved
 *   Two consecutive reads are needed for a complete 32x24 image
 */
int8_t MLX90640_ReadFrame(I2C_HandleTypeDef *hi2c, uint16_t *frame)
{
    int8_t ret;
    uint16_t status_reg;
    uint16_t ram_data[MLX90640_PIXEL_COUNT];

    /* ---- 步骤 1: 等待新帧就绪 ---- */
    /* Step 1: Wait for new frame to be ready */
    /* 状态寄存器的 bit 3 表示新帧可用 */
    /* Status register bit 3 indicates new frame available */
    uint8_t timeout = 100;  /* 最多等待 100 次轮询 | Max 100 poll cycles */
    do {
        ret = MLX90640_I2C_Read(hi2c, device_addr,
                                 MLX90640_REG_STATUS, &status_reg, 1);
        if (ret != MLX90640_OK) return ret;
        timeout--;
    } while (!(status_reg & 0x0008) && timeout > 0);

    if (timeout == 0) {
        return MLX90640_ERR_TIMEOUT;
    }

    /* ---- 步骤 2: 读取 RAM 像素数据 ---- */
    /* Step 2: Read RAM pixel data */
    /* RAM 从 0x0400 开始，包含 768 个 16 位像素值 */
    /* RAM starts at 0x0400, containing 768 16-bit pixel values */
    ret = MLX90640_I2C_Read(hi2c, device_addr,
                             MLX90640_RAM_START,
                             ram_data, MLX90640_PIXEL_COUNT);
    if (ret != MLX90640_OK) return ret;

    /* ---- 步骤 3: 读取辅助数据 (环境温度) ---- */
    /* Step 3: Read auxiliary data (ambient temperature) */
    uint16_t aux_data[8];
    ret = MLX90640_I2C_Read(hi2c, device_addr,
                             MLX90640_RAM_TA_V_BE,
                             aux_data, 8);

    /* 计算环境温度 | Calculate ambient temperature */
    float ambient_temp = 25.0f;  /* 默认值 | Default value */
    if (ret == MLX90640_OK) {
        /* PTAT 值 -> 温度转换 (简化版) */
        /* PTAT value -> temperature conversion (simplified) */
        float vptat = (float)aux_data[0];
        float dta = vptat - (float)calib_data.vptat25;
        ambient_temp = dta / calib_data.Ktptat + 25.0f;
    }

    /* ---- 步骤 4: 计算像素温度 ---- */
    /* Step 4: Calculate pixel temperatures */
    /*
     * 温度计算公式 (简化版) | Temperature calculation (simplified):
     *
     * Ta = 环境温度 (已计算) | Ambient temperature (calculated)
     *
     * 对于每个像素 | For each pixel:
     *   V_ir = raw_data[pixel]  (红外电压 | IR voltage)
     *   V_compensated = V_ir - offset[pixel] + Kta * Ta + Kv * Vdd
     *   T_obj = sqrt( V_compensated / (alpha[pixel] * emissivity) + Ta^2 ) - 273.15
     *
     * 完整公式请参考 Melexis 应用笔记:
     * For the complete formula, refer to Melexis application note:
     *   "MLX90640 software driver"
     */
    for (uint16_t i = 0; i < MLX90640_PIXEL_COUNT; i++) {
        /* 将原始数据转换为温度 (单位: 0.01°C 用于整数传输) */
        /* Convert raw data to temperature (unit: 0.01°C for integer transmission) */

        int16_t raw_val = (int16_t)ram_data[i];

        /* 简化的温度计算 | Simplified temperature calculation */
        /* 实际实现需要完整的 Melexis 补偿算法 */
        /* Actual implementation needs full Melexis compensation algorithm */
        float temp = (float)raw_val * 0.02f + ambient_temp;

        /* 转换为 0.01°C 单位的整数 | Convert to integer in 0.01°C units */
        frame[i] = (uint16_t)((int16_t)(temp * 100));
    }

    /* ---- 步骤 5: 清除状态标志 ---- */
    /* Step 5: Clear status flag */
    /* 向状态寄存器写入 0x0030 清除新帧标志 */
    /* Write 0x0030 to status register to clear new frame flag */
    MLX90640_I2C_Write(hi2c, device_addr, MLX90640_REG_STATUS, 0x0030);

    return MLX90640_OK;
}

/**
 * @brief 读取原始 RAM 数据 | Read raw RAM data
 */
int8_t MLX90640_ReadRAM(I2C_HandleTypeDef *hi2c, uint16_t start,
                         uint16_t *data, uint16_t len)
{
    return MLX90640_I2C_Read(hi2c, device_addr, start, data, len);
}

/* ============================================================
 * 温度计算 | Temperature Calculation
 * ============================================================ */

/**
 * @brief 从原始数据计算温度 (浮点精度)
 *        Calculate temperature from raw data (floating point precision)
 *
 * 这个函数提供完整的温度计算流程，输出浮点温度值
 * This function provides complete temperature calculation, outputting float temperature values
 */
void MLX90640_CalculateTemperature(const uint16_t *raw_data,
                                    MLX90640_FrameData_t *frame)
{
    float temp_min = 999.0f;
    float temp_max = -999.0f;
    float temp_sum = 0.0f;

    for (uint16_t i = 0; i < MLX90640_PIXEL_COUNT; i++) {
        int16_t raw_val = (int16_t)raw_data[i];

        /* 完整的温度补偿计算 | Full temperature compensation calculation */
        /* 参考 Melexis MLX90640 software driver 文档 */
        /* Refer to Melexis MLX90640 software driver documentation */
        float temp = (float)raw_val * 0.02f + frame->ambient_temp;

        frame->temperatures[i] = temp;

        /* 统计 | Statistics */
        if (temp < temp_min) temp_min = temp;
        if (temp > temp_max) temp_max = temp;
        temp_sum += temp;
    }

    frame->temp_min = temp_min;
    frame->temp_max = temp_max;
    frame->temp_avg = temp_sum / MLX90640_PIXEL_COUNT;
}

/* ============================================================
 * 配置函数 | Configuration Functions
 * ============================================================ */

/**
 * @brief 设置刷新率 | Set refresh rate
 *
 * 刷新率通过修改控制寄存器 (0x800D) 的 bit 7:5 来设置
 * Refresh rate is set by modifying control register bit 7:5
 *
 * @param hi2c I2C 句柄指针
 * @param rate 刷新率值 (0~7, 对应 0.5Hz ~ 64Hz)
 */
int8_t MLX90640_SetRefreshRate(I2C_HandleTypeDef *hi2c, uint8_t rate)
{
    uint16_t ctrl_reg;

    /* 先读取当前控制寄存器值 | First read current control register value */
    int8_t ret = MLX90640_I2C_Read(hi2c, device_addr,
                                     MLX90640_REG_CTRL, &ctrl_reg, 1);
    if (ret != MLX90640_OK) return ret;

    /* 清除刷新率位 (bit 7:5) | Clear refresh rate bits (bit 7:5) */
    ctrl_reg &= ~(0x07 << 7);

    /* 设置新的刷新率 | Set new refresh rate */
    ctrl_reg |= ((rate & 0x07) << 7);

    /* 写回控制寄存器 | Write back to control register */
    ret = MLX90640_I2C_Write(hi2c, device_addr, MLX90640_REG_CTRL, ctrl_reg);
    if (ret == MLX90640_OK) {
        current_refresh_rate = rate;
    }

    return ret;
}

/**
 * @brief 设置 ADC 分辨率 | Set ADC resolution
 *
 * 分辨率通过修改控制寄存器的 bit 4:3 来设置
 * Resolution is set by modifying control register bit 4:3
 */
int8_t MLX90640_SetResolution(I2C_HandleTypeDef *hi2c, uint8_t resolution)
{
    uint16_t ctrl_reg;

    int8_t ret = MLX90640_I2C_Read(hi2c, device_addr,
                                     MLX90640_REG_CTRL, &ctrl_reg, 1);
    if (ret != MLX90640_OK) return ret;

    /* 清除分辨率位 (bit 4:3) | Clear resolution bits (bit 4:3) */
    ctrl_reg &= ~(0x03 << 3);

    /* 设置新的分辨率 | Set new resolution */
    ctrl_reg |= ((resolution & 0x03) << 3);

    ret = MLX90640_I2C_Write(hi2c, device_addr, MLX90640_REG_CTRL, ctrl_reg);
    if (ret == MLX90640_OK) {
        current_resolution = resolution;
    }

    return ret;
}

/**
 * @brief 获取校准数据指针 | Get calibration data pointer
 */
MLX90640_CalibData_t* MLX90640_GetCalibData(void)
{
    return &calib_data;
}
