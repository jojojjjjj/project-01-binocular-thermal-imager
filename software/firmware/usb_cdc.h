/**
 * ============================================================
 * USB CDC 通信模块 - 头文件
 * USB CDC Communication Module - Header File
 * ============================================================
 * 功能概述 | Function Overview:
 *   将 MLX90640 的热成像帧数据通过 USB CDC (虚拟串口) 发送到 Android 手机
 *   Send MLX90640 thermal frame data to Android phone via USB CDC (virtual serial port)
 *
 * 数据包格式 | Data Packet Format:
 *   +--------+--------+--------+--------+------------------+--------+--------+
 *   | Header | Header | Frame# | Frame# | Temperature Data | CRC16  | CRC16  |
 *   | 0x5A   | 0x5A   | High   | Low    | 768 x 2 bytes    | High   | Low    |
 *   +--------+--------+--------+--------+------------------+--------+--------+
 *   |<--- 2 bytes --->|<- 2B ->|<- 2B ->|<--- 1536 bytes ->|<--- 2 bytes --->|
 *   |                           总计: 1542 bytes                               |
 *   |                           Total: 1542 bytes                              |
 *
 * 温度数据编码 | Temperature Data Encoding:
 *   每个像素的温度以 16 位有符号整数传输
 *   单位: 0.01°C (例如: 2547 = 25.47°C, -512 = -5.12°C)
 *
 *   Each pixel temperature is transmitted as a 16-bit signed integer
 *   Unit: 0.01°C (e.g., 2547 = 25.47°C, -512 = -5.12°C)
 *
 * 开源参考 | Open-source reference:
 *   https://github.com/colourfate/thermal_bridge
 * ============================================================
 */

#ifndef USB_CDC_H
#define USB_CDC_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
 * 常量定义 | Constant Definitions
 * ============================================================ */

/* 数据包帧头 | Packet frame header */
#define USB_FRAME_HEADER         0x5A5A   /* 魔数标识 | Magic number identifier */

/* 缓冲区大小 | Buffer sizes */
/* 2(帧头) + 2(帧计数) + 768*2(温度数据) + 2(CRC) = 1542 */
/* 2(header) + 2(frame#) + 768*2(temp data) + 2(CRC) = 1542 */
#define USB_TX_BUFFER_SIZE       1542

/* USB 端点最大包大小 | USB endpoint max packet size */
/* FS (Full Speed) = 64 bytes, HS (High Speed) = 512 bytes */
#define USB_FS_MAX_PACKET_SIZE   64

/* USB 超时 | USB timeout */
#define USB_SEND_TIMEOUT_MS      100

/* 命令字节 (从手机接收) | Command bytes (received from phone) */
#define CMD_SET_REFRESH_RATE     0x01     /* 设置刷新率 | Set refresh rate */
#define CMD_GET_STATUS           0x02     /* 获取状态 | Get status */
#define CMD_RESET_SENSOR         0x03     /* 重置传感器 | Reset sensor */
#define CMD_SET_EMISSIVITY       0x04     /* 设置发射率 | Set emissivity */

/* ============================================================
 * 函数声明 | Function Declarations
 * ============================================================ */

/**
 * @brief 初始化 USB CDC 通信 | Initialize USB CDC communication
 *
 * 配置 USB 设备为 CDC (虚拟串口) 模式
 * Configure USB device as CDC (virtual serial port) mode
 *
 * 注意: 实际的 USB 初始化在 main.c 中完成
 * Note: Actual USB initialization is done in main.c
 * 此函数用于应用层初始化
 */
void USB_CDC_Init(void);

/**
 * @brief 发送一帧热成像数据 | Send one frame of thermal data
 *
 * 将 768 个像素的温度数据打包成指定格式并发送
 * Pack 768 pixel temperature values into the specified format and send
 *
 * @param thermal_frame 温度数据数组 (768 个 uint16_t, 单位 0.01°C)
 *                      Temperature data array (768 uint16_t, unit 0.01°C)
 * @param frame_count   帧计数器 | Frame counter
 * @return 0 成功, -1 失败
 */
int8_t USB_CDC_SendThermalFrame(const uint16_t *thermal_frame, uint16_t frame_count);

/**
 * @brief 发送原始数据 | Send raw data
 *
 * @param data   数据缓冲区 | Data buffer
 * @param length 数据长度 | Data length
 * @return 实际发送的字节数 | Actually sent bytes
 */
uint16_t USB_CDC_SendData(const uint8_t *data, uint16_t length);

/**
 * @brief 接收来自手机的数据/命令 | Receive data/commands from phone
 *
 * @param buffer 接收缓冲区 | Receive buffer
 * @param max_len 缓冲区最大长度 | Buffer max length
 * @return 实际接收的字节数 (0 = 无数据) | Received bytes (0 = no data)
 */
uint16_t USB_CDC_ReceiveData(uint8_t *buffer, uint16_t max_len);

/**
 * @brief 计算 CRC16 校验和 | Calculate CRC16 checksum
 *
 * 使用 CRC16-CCITT 算法
 * Uses CRC16-CCITT algorithm
 *
 * @param data 数据缓冲区 | Data buffer
 * @param len  数据长度 | Data length
 * @return CRC16 值 | CRC16 value
 */
uint16_t USB_CDC_CRC16(const uint8_t *data, uint16_t len);

/**
 * @brief 处理接收到的命令 | Process received command
 *
 * @param cmd   命令字节 | Command byte
 * @param param 命令参数 | Command parameter
 */
void USB_CDC_ProcessCommand(uint8_t cmd, uint8_t param);

#ifdef __cplusplus
}
#endif

#endif /* USB_CDC_H */
