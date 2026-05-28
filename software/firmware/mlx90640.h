/**
 * ============================================================
 * MLX90640 热成像传感器驱动 - 头文件
 * MLX90640 Thermal Sensor Driver - Header File
 * ============================================================
 * 传感器概述 | Sensor Overview:
 *   - 制造商 | Manufacturer: Melexis
 *   - 分辨率 | Resolution: 32x24 像素 (768 pixels)
 *   - 接口 | Interface: I2C (最高 1MHz)
 *   - 帧率 | Frame rate: 0.5Hz ~ 64Hz
 *   - 测温范围 | Temperature range: -40°C ~ 300°C
 *   - 精度 | Accuracy: ±1.0°C (典型值)
 *
 * 数据手册 | Datasheet:
 *   https://www.melexis.com/en/documents/documentation/datasheets/datasheet-mlx90640
 *
 * 开源参考 | Open-source reference:
 *   https://github.com/colourfate/thermal_bridge
 *   https://github.com/melexis/mlx90640-library
 * ============================================================
 */

#ifndef MLX90640_H
#define MLX90640_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
 * 包含文件 | Includes
 * ============================================================ */
#include "stm32f4xx_hal.h"
#include <stdint.h>

/* ============================================================
 * 常量定义 | Constant Definitions
 * ============================================================ */

/* MLX90640 I2C 地址 | MLX90640 I2C addresses */
#define MLX90640_I2C_ADDR           0x33   /* 默认地址 | Default address */
#define MLX90640_I2C_ADDR_ALT       0x33   /* 备用地址 | Alternate address */

/* 传感器分辨率 | Sensor resolution */
#define MLX90640_COLS               32     /* 列数 | Number of columns */
#define MLX90640_ROWS               24     /* 行数 | Number of rows */
#define MLX90640_PIXEL_COUNT        (MLX90640_COLS * MLX90640_ROWS)  /* 768 像素 */

/* MLX90640 寄存器地址 | MLX90640 Register Addresses */
/* 控制寄存器 | Control registers */
#define MLX90640_REG_CTRL           0x800D  /* 控制寄存器 | Control register */

/* 状态寄存器 | Status register */
#define MLX90640_REG_STATUS         0x8000  /* 状态寄存器 | Status register */

/* RAM 起始地址 | RAM start address */
/* MLX90640 的像素数据存储在 RAM 的 0x0400 ~ 0x06FF 区域 */
/* MLX90640 pixel data is stored in RAM at 0x0400 ~ 0x06FF */
#define MLX90640_RAM_START          0x0400  /* RAM 起始地址 | RAM start address */
#define MLX90640_RAM_END            0x06FF  /* RAM 结束地址 | RAM end address */

/* EEPROM 起始地址 | EEPROM start address */
#define MLX90640_EEPROM_START       0x2400  /* EEPROM 起始 | EEPROM start */
#define MLX90640_EEPROM_END         0x273F  /* EEPROM 结束 | EEPROM end */

/* 辅助数据地址 | Auxiliary data addresses */
#define MLX90640_RAM_TA_V_BE       0x0700  /* 环境温度和 V_BE | Ambient temp and V_BE */
#define MLX90640_RAM_GLOBAL_OFF_CP 0x0708  /* 补偿像素偏移 | Compensation pixel offset */

/* 刷新率设置 | Refresh rate settings */
/* 这些值写入控制寄存器的帧速率位 */
/* These values are written to the frame rate bits of the control register */
#define MLX90640_REFRESH_0_5_HZ    0x00   /* 0.5 Hz */
#define MLX90640_REFRESH_1_HZ      0x01   /* 1 Hz */
#define MLX90640_REFRESH_2_HZ      0x02   /* 2 Hz */
#define MLX90640_REFRESH_4_HZ      0x03   /* 4 Hz */
#define MLX90640_REFRESH_8_HZ      0x04   /* 8 Hz (推荐 | recommended) */
#define MLX90640_REFRESH_16_HZ     0x05   /* 16 Hz */
#define MLX90640_REFRESH_32_HZ     0x06   /* 32 Hz */
#define MLX90640_REFRESH_64_HZ     0x07   /* 64 Hz */

/* 分辨率设置 | Resolution settings */
#define MLX90640_ADC_16BIT         0x00   /* 16 位 ADC | 16-bit ADC */
#define MLX90640_ADC_17BIT         0x01   /* 17 位 ADC | 17-bit ADC */
#define MLX90640_ADC_18BIT         0x02   /* 18 位 ADC | 18-bit ADC */
#define MLX90640_ADC_19BIT         0x03   /* 19 位 ADC | 19-bit ADC */

/* 读取模式 | Reading pattern */
#define MLX90640_INTERLEAVED       0x00   /* 交错模式 | Interleaved mode */
#define MLX90640_CHESS             0x01   /* 棋盘模式 | Chess pattern */

/* 返回状态码 | Return status codes */
#define MLX90640_OK                0      /* 成功 | Success */
#define MLX90640_ERR_I2C           -1     /* I2C 通信错误 | I2C communication error */
#define MLX90640_ERR_TIMEOUT       -2     /* 超时 | Timeout */
#define MLX90640_ERR_INIT          -3     /* 初始化失败 | Initialization failed */
#define MLX90640_ERR_PARAM         -4     /* 参数错误 | Parameter error */

/* ============================================================
 * 数据结构 | Data Structures
 * ============================================================ */

/**
 * @brief MLX90640 校准参数 | MLX90640 calibration parameters
 *
 * 这些参数存储在传感器的 EEPROM 中，用于温度计算
 * These parameters are stored in sensor EEPROM for temperature calculation
 */
typedef struct {
    /* 增益系数 | Gain coefficient */
    float kVdd;                /* VDD 电压系数 | VDD voltage coefficient */
    int16_t vdd25;             /* 25°C 时的 VDD | VDD at 25°C */

    /* 偏置参数 | Offset parameters */
    float Kvptat;              /* PTAT 电压温度系数 | PTAT voltage temperature coefficient */
    float Ktptat;              /* PTAT 温度系数 | PTAT temperature coefficient */
    int16_t vptat25;           /* 25°C 时的 PTAT 电压 | PTAT voltage at 25°C */
    float alpha_ptat;          /* PTAT alpha 系数 | PTAT alpha coefficient */

    /* 像素灵敏度 | Pixel sensitivity */
    float alpha_base;          /* 基准 alpha | Base alpha */
    float kta;                 /* 热电堆系数 | Thermopile coefficient */
    float kv;                  /* 电压系数 | Voltage coefficient */

    /* 分辨率修正 | Resolution correction */
    float resolution_corr;     /* 分辨率修正因子 | Resolution correction factor */

    /* 发射率 | Emissivity */
    float emissivity;          /* 默认 0.95 | Default 0.95 */

    /* 像素偏移 | Pixel offsets */
    int16_t offset[MLX90640_PIXEL_COUNT];    /* 每像素偏移 | Per-pixel offset */
    float alpha[MLX90640_PIXEL_COUNT];        /* 每像素 alpha | Per-pixel alpha */
} MLX90640_CalibData_t;

/**
 * @brief MLX90640 帧数据 | MLX90640 frame data
 */
typedef struct {
    /* 原始数据 | Raw data */
    uint16_t raw_data[MLX90640_PIXEL_COUNT * 2];  /* 包含两个子页 | Including two sub-pages */

    /* 计算后的温度数据 (单位: °C) | Calculated temperature data (unit: °C) */
    float temperatures[MLX90640_PIXEL_COUNT];

    /* 环境温度 | Ambient temperature */
    float ambient_temp;

    /* 统计信息 | Statistics */
    float temp_min;            /* 帧内最低温度 | Minimum temperature in frame */
    float temp_max;            /* 帧内最高温度 | Maximum temperature in frame */
    float temp_avg;            /* 帧内平均温度 | Average temperature in frame */

    /* 帧信息 | Frame info */
    uint16_t frame_number;     /* 帧编号 | Frame number */
    uint8_t  sub_page;         /* 当前子页 (0 或 1) | Current sub-page (0 or 1) */
} MLX90640_FrameData_t;

/* ============================================================
 * 函数声明 | Function Declarations
 * ============================================================ */

/**
 * @brief 初始化 MLX90640 传感器 | Initialize MLX90640 sensor
 *
 * 执行以下操作:
 *   1. 读取 EEPROM 校准数据
 *   2. 设置刷新率
 *   3. 设置分辨率
 *   4. 配置读取模式
 *
 * @param hi2c    I2C 句柄指针 | Pointer to I2C handle
 * @param addr    I2C 设备地址 | I2C device address
 * @return MLX90640_OK 成功, 其他值表示错误
 */
int8_t MLX90640_Init(I2C_HandleTypeDef *hi2c, uint8_t addr);

/**
 * @brief 读取一帧完整的热成像数据 | Read a complete thermal frame
 *
 * MLX90640 使用双缓冲机制，需要读取两个子页 (sub-page 0 和 1) 组成一帧
 * MLX90640 uses double buffering, requires reading two sub-pages for one frame
 *
 * @param hi2c    I2C 句柄指针 | Pointer to I2C handle
 * @param frame   输出温度数据数组 (768 个 uint16_t, 单位 0.01°C)
 *                Output temperature array (768 uint16_t, unit 0.01°C)
 * @return MLX90640_OK 成功, 其他值表示错误
 */
int8_t MLX90640_ReadFrame(I2C_HandleTypeDef *hi2c, uint16_t *frame);

/**
 * @brief 读取原始 RAM 数据 | Read raw RAM data
 *
 * @param hi2c    I2C 句柄指针 | Pointer to I2C handle
 * @param start   起始地址 | Start address
 * @param data    数据缓冲区 | Data buffer
 * @param len     数据长度 | Data length
 * @return MLX90640_OK 成功, 其他值表示错误
 */
int8_t MLX90640_ReadRAM(I2C_HandleTypeDef *hi2c, uint16_t start,
                         uint16_t *data, uint16_t len);

/**
 * @brief 读取 EEPROM 校准数据 | Read EEPROM calibration data
 *
 * @param hi2c I2C 句柄指针 | Pointer to I2C handle
 * @return MLX90640_OK 成功, 其他值表示错误
 */
int8_t MLX90640_ReadEEPROM(I2C_HandleTypeDef *hi2c);

/**
 * @brief 从原始数据计算温度 | Calculate temperature from raw data
 *
 * 使用 Melexis 提供的补偿算法将原始 ADC 值转换为温度 (°C)
 * Uses Melexis compensation algorithm to convert raw ADC values to temperature (°C)
 *
 * @param raw_data 原始 RAM 数据 | Raw RAM data
 * @param frame    输出帧数据结构 | Output frame data structure
 */
void MLX90640_CalculateTemperature(const uint16_t *raw_data,
                                    MLX90640_FrameData_t *frame);

/**
 * @brief 设置刷新率 | Set refresh rate
 *
 * @param hi2c  I2C 句柄指针 | Pointer to I2C handle
 * @param rate  刷新率设置值 (MLX90640_REFRESH_x_HZ)
 *              Refresh rate setting value
 * @return MLX90640_OK 成功, 其他值表示错误
 */
int8_t MLX90640_SetRefreshRate(I2C_HandleTypeDef *hi2c, uint8_t rate);

/**
 * @brief 设置分辨率 | Set resolution
 *
 * @param hi2c       I2C 句柄指针 | Pointer to I2C handle
 * @param resolution 分辨率设置值 (MLX90640_ADC_xxBIT)
 *                   Resolution setting value
 * @return MLX90640_OK 成功, 其他值表示错误
 */
int8_t MLX90640_SetResolution(I2C_HandleTypeDef *hi2c, uint8_t resolution);

/**
 * @brief I2C 写入寄存器 | I2C write register
 *
 * @param hi2c  I2C 句柄指针
 * @param addr  设备地址
 * @param reg   寄存器地址 (16位)
 * @param data  要写入的数据
 * @return MLX90640_OK 成功, 其他值表示错误
 */
int8_t MLX90640_I2C_Write(I2C_HandleTypeDef *hi2c, uint8_t addr,
                           uint16_t reg, uint16_t data);

/**
 * @brief I2C 读取寄存器 | I2C read register
 *
 * @param hi2c  I2C 句柄指针
 * @param addr  设备地址
 * @param reg   寄存器地址 (16位)
 * @param data  读取的数据缓冲区
 * @param len   读取长度
 * @return MLX90640_OK 成功, 其他值表示错误
 */
int8_t MLX90640_I2C_Read(I2C_HandleTypeDef *hi2c, uint8_t addr,
                          uint16_t reg, uint16_t *data, uint16_t len);

/**
 * @brief 获取校准数据指针 | Get calibration data pointer
 *
 * @return 校准数据结构指针 | Pointer to calibration data structure
 */
MLX90640_CalibData_t* MLX90640_GetCalibData(void);

#ifdef __cplusplus
}
#endif

#endif /* MLX90640_H */
