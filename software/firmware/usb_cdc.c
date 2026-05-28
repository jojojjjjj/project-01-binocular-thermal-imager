/**
 * ============================================================
 * USB CDC 通信模块 - 实现文件
 * USB CDC Communication Module - Implementation File
 * ============================================================
 *
 * USB CDC (Communications Device Class) 原理:
 *   USB CDC 是 USB 组织定义的一类设备，可以使 USB 设备模拟串口。
 *   在 Android 端，通过 UsbSerial 库与 CDC 设备通信。
 *   对于 STM32，HAL 库提供了 USBD_CDC 类。
 *
 * USB CDC (Communications Device Class) principle:
 *   USB CDC is a device class defined by USB-IF that makes a USB device
 *   emulate a serial port. On Android, the UsbSerial library communicates
 *   with CDC devices. For STM32, the HAL library provides USBD_CDC class.
 *
 * 数据传输策略 | Data Transfer Strategy:
 *   由于 USB Full-Speed 单次最大传输 64 字节，
 *   一帧 1542 字节的数据需要分多次传输。
 *   我们使用 CDC 的批量传输 (bulk transfer) 自动处理分包。
 *
 *   Since USB Full-Speed max packet is 64 bytes,
 *   one 1542-byte frame requires multiple transfers.
 *   We use CDC bulk transfer which handles packet splitting automatically.
 *
 * 开源参考 | Open-source reference:
 *   https://github.com/colourfate/thermal_bridge
 * ============================================================
 */

/* ============================================================
 * 头文件包含 | Header Includes
 * ============================================================ */
#include "usb_cdc.h"
#include "usbd_cdc_if.h"  /* STM32 USB CDC 接口 | STM32 USB CDC interface */
#include <string.h>

/* ============================================================
 * 私有变量 | Private Variables
 * ============================================================ */

/* 发送状态标志 | Transmit status flag */
static volatile uint8_t tx_busy = 0;

/* 统计信息 | Statistics */
static uint32_t total_frames_sent = 0;
static uint32_t total_bytes_sent = 0;

/* ============================================================
 * 初始化 | Initialization
 * ============================================================ */

/**
 * @brief 初始化 USB CDC 应用层 | Initialize USB CDC application layer
 *
 * USB 硬件初始化已在 main.c 中完成
 * 此函数初始化应用层状态
 *
 * USB hardware init is done in main.c
 * This function initializes application layer state
 */
void USB_CDC_Init(void)
{
    tx_busy = 0;
    total_frames_sent = 0;
    total_bytes_sent = 0;
}

/* ============================================================
 * 数据打包与发送 | Data Packaging and Sending
 * ============================================================ */

/**
 * @brief 发送一帧热成像数据 | Send one frame of thermal data
 *
 * 数据包结构 | Packet structure:
 *   Byte 0-1:   帧头 0x5A5A (魔数，用于同步)
 *               Frame header 0x5A5A (magic number for synchronization)
 *   Byte 2-3:   帧计数器 (递增的序列号)
 *               Frame counter (incrementing sequence number)
 *   Byte 4-1539: 温度数据 (768 个 16 位值, 小端序)
 *               Temperature data (768 16-bit values, little-endian)
 *   Byte 1540-1541: CRC16 校验 (覆盖 Byte 4 到 Byte 1539)
 *                   CRC16 checksum (covers Byte 4 to Byte 1539)
 *
 * @param thermal_frame 温度数据 (768 个 uint16_t, 0.01°C/LSB)
 * @param frame_count   帧编号
 * @return 0 成功, -1 失败
 */
int8_t USB_CDC_SendThermalFrame(const uint16_t *thermal_frame, uint16_t frame_count)
{
    uint16_t idx = 0;
    static uint8_t tx_buf[USB_TX_BUFFER_SIZE];

    /* ---- 第 1 部分: 帧头 (2 bytes) ---- */
    /* Part 1: Frame header (2 bytes) */
    tx_buf[idx++] = 0x5A;  /* 帧头第 1 字节 | Header byte 1 */
    tx_buf[idx++] = 0x5A;  /* 帧头第 2 字节 | Header byte 2 */

    /* ---- 第 2 部分: 帧计数器 (2 bytes, 小端序) ---- */
    /* Part 2: Frame counter (2 bytes, little-endian) */
    tx_buf[idx++] = (uint8_t)(frame_count & 0xFF);        /* 低字节 | Low byte */
    tx_buf[idx++] = (uint8_t)((frame_count >> 8) & 0xFF); /* 高字节 | High byte */

    /* ---- 第 3 部分: 温度数据 (768 x 2 = 1536 bytes) ---- */
    /* Part 3: Temperature data (768 x 2 = 1536 bytes) */
    /*
     * 每个像素温度转换为 2 字节 (小端序)
     * Each pixel temperature converted to 2 bytes (little-endian)
     *
     * 温度编码示例 | Temperature encoding examples:
     *   25.47°C -> 2547 -> [0xEB, 0x09]
     *   -5.12°C -> -512 -> [0xFE, 0xFF] (补码 | two's complement)
     */
    for (uint16_t i = 0; i < MLX90640_PIXEL_COUNT; i++) {
        tx_buf[idx++] = (uint8_t)(thermal_frame[i] & 0xFF);        /* 低字节 */
        tx_buf[idx++] = (uint8_t)((thermal_frame[i] >> 8) & 0xFF); /* 高字节 */
    }

    /* ---- 第 4 部分: CRC16 校验 (2 bytes) ---- */
    /* Part 4: CRC16 checksum (2 bytes) */
    /* CRC 覆盖温度数据部分 (从 byte 4 到 byte 1539) */
    /* CRC covers temperature data portion (from byte 4 to byte 1539) */
    uint16_t crc = USB_CDC_CRC16(&tx_buf[4], MLX90640_PIXEL_COUNT * 2);
    tx_buf[idx++] = (uint8_t)(crc & 0xFF);        /* CRC 低字节 | CRC low byte */
    tx_buf[idx++] = (uint8_t)((crc >> 8) & 0xFF); /* CRC 高字节 | CRC high byte */

    /* ---- 发送数据 ---- */
    /* Send data */
    /*
     * CDC_Transmit_FS 是 STM32 HAL 提供的 USB CDC 发送函数
     * 它会自动将数据分成多个 USB 包 (每包最大 64 字节) 发送
     *
     * CDC_Transmit_FS is the STM32 HAL USB CDC send function
     * It automatically splits data into multiple USB packets (max 64 bytes each)
     */
    uint8_t result = CDC_Transmit_FS(tx_buf, idx);

    if (result == USBD_OK) {
        total_frames_sent++;
        total_bytes_sent += idx;
        return 0;
    }

    return -1;
}

/**
 * @brief 发送原始数据 | Send raw data
 *
 * @param data   数据缓冲区
 * @param length 数据长度
 * @return 实际发送的字节数
 */
uint16_t USB_CDC_SendData(const uint8_t *data, uint16_t length)
{
    if (CDC_Transmit_FS((uint8_t *)data, length) == USBD_OK) {
        total_bytes_sent += length;
        return length;
    }
    return 0;
}

/* ============================================================
 * 数据接收 | Data Reception
 * ============================================================ */

/**
 * @brief 接收来自手机的数据/命令 | Receive data/commands from phone
 *
 * Android APP 通过 USB CDC 发送控制命令，例如:
 *   - 设置刷新率
 *   - 请求传感器状态
 *   - 设置发射率
 *
 * Android APP sends control commands via USB CDC, such as:
 *   - Set refresh rate
 *   - Request sensor status
 *   - Set emissivity
 *
 * @param buffer  接收缓冲区
 * @param max_len 缓冲区最大长度
 * @return 实际接收的字节数
 */
uint16_t USB_CDC_ReceiveData(uint8_t *buffer, uint16_t max_len)
{
    /*
     * 在 STM32 HAL USB CDC 中，接收通过回调函数实现:
     *   void CDC_Receive_FS(uint8_t *Buf, uint32_t *Len)
     *
     * 这个回调在 usbd_cdc_if.c 中定义 (STM32CubeMX 生成)
     * 实际项目中应在此回调中解析命令并设置标志
     *
     * In STM32 HAL USB CDC, reception is implemented via callback:
     * This callback is defined in usbd_cdc_if.c (generated by STM32CubeMX)
     * In production, parse commands and set flags in this callback
     */
    return 0;
}

/* ============================================================
 * CRC16 校验 | CRC16 Checksum
 * ============================================================ */

/**
 * @brief 计算 CRC16-CCITT 校验和 | Calculate CRC16-CCITT checksum
 *
 * 多项式: x^16 + x^12 + x^5 + 1 (0x1021)
 * 初始值: 0xFFFF
 *
 * CRC 用于检测 USB 传输中的数据错误
 * CRC is used to detect data errors during USB transmission
 *
 * @param data 数据缓冲区
 * @param len  数据长度
 * @return CRC16 值
 */
uint16_t USB_CDC_CRC16(const uint8_t *data, uint16_t len)
{
    uint16_t crc = 0xFFFF;  /* 初始值 | Initial value */

    for (uint16_t i = 0; i < len; i++) {
        crc ^= (uint16_t)data[i] << 8;

        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x1021;  /* 多项式 | Polynomial */
            } else {
                crc <<= 1;
            }
        }
    }

    return crc;
}

/* ============================================================
 * 命令处理 | Command Processing
 * ============================================================ */

/**
 * @brief 处理来自 Android APP 的控制命令
 *        Process control commands from Android APP
 *
 * 命令格式 | Command format:
 *   [0xAA] [CMD] [PARAM] [CRC8]
 *   - 0xAA: 命令帧头 | Command header
 *   - CMD: 命令类型 | Command type
 *   - PARAM: 参数 | Parameter
 *   - CRC8: 校验 | Checksum
 *
 * @param cmd   命令字节
 * @param param 命令参数
 */
void USB_CDC_ProcessCommand(uint8_t cmd, uint8_t param)
{
    uint8_t response[8];
    uint16_t resp_len = 0;

    switch (cmd)
    {
        case CMD_SET_REFRESH_RATE:
            /* 设置 MLX90640 刷新率 | Set MLX90640 refresh rate */
            /* param: 0~7 对应 0.5Hz ~ 64Hz */
            /* 需要调用 MLX90640_SetRefreshRate() */
            response[0] = 0xAA;
            response[1] = cmd;
            response[2] = 0x00;  /* 成功 | Success */
            resp_len = 3;
            break;

        case CMD_GET_STATUS:
            /* 返回传感器状态 | Return sensor status */
            response[0] = 0xAA;
            response[1] = cmd;
            response[2] = 0x00;  /* 状态正常 | Status OK */
            response[3] = (uint8_t)(total_frames_sent & 0xFF);
            response[4] = (uint8_t)((total_frames_sent >> 8) & 0xFF);
            resp_len = 5;
            break;

        case CMD_RESET_SENSOR:
            /* 重置传感器 | Reset sensor */
            response[0] = 0xAA;
            response[1] = cmd;
            response[2] = 0x00;
            resp_len = 3;
            break;

        case CMD_SET_EMISSIVITY:
            /* 设置发射率 | Set emissivity */
            /* param: 发射率 x 100 (如 95 = 0.95) */
            response[0] = 0xAA;
            response[1] = cmd;
            response[2] = 0x00;
            resp_len = 3;
            break;

        default:
            /* 未知命令 | Unknown command */
            response[0] = 0xAA;
            response[1] = cmd;
            response[2] = 0xFF;  /* 错误: 未知命令 | Error: unknown command */
            resp_len = 3;
            break;
    }

    /* 发送响应 | Send response */
    if (resp_len > 0) {
        USB_CDC_SendData(response, resp_len);
    }
}

/* ============================================================
 * STM32 USB CDC 接收回调 (在 usbd_cdc_if.c 中调用)
 * STM32 USB CDC receive callback (called from usbd_cdc_if.c)
 * ============================================================ */

/**
 * @brief USB CDC 接收回调函数 | USB CDC receive callback
 *
 * 此函数应在 usbd_cdc_if.c 的 CDC_Receive_FS() 中被调用
 * This function should be called from CDC_Receive_FS() in usbd_cdc_if.c
 *
 * 示例 usbd_cdc_if.c 中的修改:
 * Example modification in usbd_cdc_if.c:
 *
 *   static int8_t CDC_Receive_FS(uint8_t *Buf, uint32_t *Len)
 *   {
 *       // 解析命令 | Parse command
 *       if (Buf[0] == 0xAA && *Len >= 3) {
 *           USB_CDC_ProcessCommand(Buf[1], Buf[2]);
 *       }
 *
 *       // 准备接收下一包 | Prepare for next receive
 *       USBD_CDC_SetRxBuffer(&hUsbDeviceFS, &Buf[0]);
 *       USBD_CDC_ReceivePacket(&hUsbDeviceFS);
 *       return (USBD_OK);
 *   }
 */
