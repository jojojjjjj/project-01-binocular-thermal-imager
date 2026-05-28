# 接线指南 | Wiring Guide

本文档详细说明 STM32F411CEU6 与所有外设的接线方法，包括 MLX90640 热成像传感器、OV2640 摄像头模块、USB Type-C 接口和电源电路。

> This document details the wiring connections between the STM32F411CEU6 MCU and all peripherals, including the MLX90640 thermal sensor, OV2640 camera module, USB Type-C interface, and power supply circuit.

---

## 概述 | Overview

本项目的硬件核心是一颗 STM32F411CEU6 微控制器，通过 I2C 总线连接 MLX90640 热成像传感器，通过 DCMI（数字摄像头接口）连接 OV2640 可见光摄像头，通过 USB 2.0 FS 接口连接手机。所有芯片和模块工作在 3.3V 电压下，由 USB 5V 经 LDO 降压供电。

> The hardware core is an STM32F411CEU6 MCU, connected to the MLX90640 via I2C, the OV2640 camera via DCMI, and the phone via USB 2.0 FS. All chips and modules operate at 3.3V, derived from USB 5V through an LDO regulator.

---

## STM32F411CEU6 引脚分配总表 | Pin Assignment Table

STM32F411CEU6 采用 LQFP48 封装，共 48 个引脚。以下是本项目使用的全部引脚分配。

> The STM32F411CEU6 uses an LQFP48 package with 48 pins. Below is the complete pin assignment for this project.

### 电源引脚 | Power Pins

| 引脚号 | 引脚名 | 连接 | 说明 |
|--------|--------|------|------|
| 1 | VBAT | 3.3V | 备用电源，直接接 VDD |
| 8 | VSSA / VREF- | GND | ADC 参考地 |
| 9 | VDDA / VREF+ | 3.3V | ADC 参考电源，接 100nF 去耦电容 |
| 23 | VDD | 3.3V | 主电源 |
| 24 | VSS | GND | 主地 |
| 35 | VDD | 3.3V | 主电源 |
| 36 | VSS | GND | 主地 |
| 47 | VDD | 3.3V | 主电源 |
| 48 | VSS | GND | 主地 |

> 每对 VDD/VSS 旁边应放置一个 100nF 陶瓷电容（共 4 个），电容尽量靠近引脚放置。

### MLX90640 I2C 接口 | MLX90640 I2C Interface

| 引脚号 | 引脚名 | 功能 | 连接目标 | 说明 |
|--------|--------|------|----------|------|
| 30 | PB6 | I2C1_SCL | MLX90640 SCL (Pin 3) | I2C 时钟线，需 4.7K 上拉到 3.3V |
| 31 | PB7 | I2C1_SDA | MLX90640 SDA (Pin 4) | I2C 数据线，需 4.7K 上拉到 3.3V |

> MLX90640 的 I2C 地址默认为 0x33（7位地址）。如果需要连接两个 MLX90640，可以修改传感器上的地址配置。

### OV2640 摄像头 DCMI 接口 | OV2640 Camera DCMI Interface

| 引脚号 | 引脚名 | DCMI功能 | 连接目标 | 说明 |
|--------|--------|----------|----------|------|
| 14 | PA6 | DCMI_D0 | OV2640 D0 (Pin 8) | 数据位 0 |
| 15 | PA7 | DCMI_D1 | OV2640 D1 (Pin 9) | 数据位 1 |
| 16 | PC4 | DCMI_D2 | OV2640 D2 (Pin 10) | 数据位 2 |
| 17 | PC5 | DCMI_D3 | OV2640 D3 (Pin 11) | 数据位 3 |
| 18 | PB0 | DCMI_D4 | OV2640 D4 (Pin 12) | 数据位 4 |
| 19 | PB1 | DCMI_D5 | OV2640 D5 (Pin 13) | 数据位 5 |
| 20 | PA10 | DCMI_D6 | OV2640 D6 (Pin 14) | 数据位 6 |
| 21 | PA11 | DCMI_D7 | OV2640 D7 (Pin 15) | 数据位 7 |
| 32 | PA4 | DCMI_HSYNC | OV2640 HREF (Pin 4) | 行同步信号 |
| 34 | PA6* | DCMI_VSYNC | OV2640 VSYNC (Pin 3) | 帧同步信号 |
| 22 | PA3 | DCMI_PIXCLK | OV2640 PCLK (Pin 5) | 像素时钟 |
| 39 | PA8 | SCCB_SCL | OV2640 SCL (Pin 7) | SCCB(I2C) 时钟，控制摄像头寄存器 |
| 40 | PA9 | SCCB_SDA | OV2640 SDA (Pin 6) | SCCB(I2C) 数据 |

> **注意**：OV2640 使用 SCCB 协议（类似 I2C）进行寄存器配置，使用 DCMI 并行接口传输图像数据。SCCB 的 SCL/SDA 需要 4.7K 上拉电阻。

### USB 接口 | USB Interface

| 引脚号 | 引脚名 | 功能 | 连接目标 | 说明 |
|--------|--------|------|----------|------|
| 32 | PA12 | USB_DP | USB Type-C D+ (DP) | USB 数据正，串联 22R 电阻 |
| 33 | PA11 | USB_DM | USB Type-C D- (DM) | USB 数据负，串联 22R 电阻 |

> **USB D+ 上拉电阻**：在 PA12 (USB_DP) 与 3.3V 之间接一个 1.5K 电阻，用于告知主机这是全速设备 (Full-Speed)。这是 USB 枚举的关键元件。

### 时钟与复位 | Clock & Reset

| 引脚号 | 引脚名 | 功能 | 连接 | 说明 |
|--------|--------|------|------|------|
| 5 | PH0-OSC_IN | HSE输入 | 8MHz 晶振 Pin 1 | 外部高速时钟输入 |
| 6 | PH1-OSC_OUT | HSE输出 | 8MHz 晶振 Pin 2 | 外部高速时钟输出 |
| 7 | NRST | 复位 | 复位按键 → GND | 低电平复位，接 10K 上拉到 3.3V |
| 44 | BOOT0 | 启动模式 | 10K 下拉到 GND | 正常运行模式（从 Flash 启动） |

> 晶振的两个引脚各接一个 20pF 电容到 GND。8MHz HSE 通过 PLL 倍频到 100MHz 作为系统时钟。

### 调试接口 | Debug Interface (SWD)

| 引脚号 | 引脚名 | 功能 | 连接目标 | 说明 |
|--------|--------|------|----------|------|
| 37 | PA13 | SWDIO | ST-Link SWDIO | 调试数据 |
| 38 | PA14 | SWCLK | ST-Link SWCLK | 调试时钟 |

> 使用 ST-Link V2 连接这 4 根线（SWDIO、SWCLK、3.3V、GND）即可下载和调试固件。

### LED 指示灯 | LED Indicators

| 引脚号 | 引脚名 | 功能 | 连接 | 说明 |
|--------|--------|------|------|------|
| 25 | PC13 | 电源/状态 LED | LED → 1K电阻 → GND | 红色 LED，上电常亮表示有电 |
| 26 | PC14 | 数据传输 LED | LED → 1K电阻 → GND | 绿色 LED，闪烁表示正在传输数据 |

---

## 详细接线图 | Detailed Wiring Diagrams

### 1. 电源电路 | Power Supply Circuit

```
USB Type-C                              STM32F411 系统
┌─────────┐                            ┌──────────────┐
│         │                            │              │
│  VBUS ──┼───────┬────────────────────┼── 5V (USB)   │
│  (5V)   │       │                    │              │
│         │       │  ┌──────────────┐  │              │
│         │       └──┤ AMS1117-3.3  ├──┼── 3.3V       │
│         │          │              │  │              │
│         │       ┌──┤ GND     VIN ├──┘  (给 MCU、   │
│         │       │  │              │     MLX90640、  │
│         │       │  └──────────────┘     OV2640 供电)│
│  GND ───┼───────┼───────────────────────── GND      │
│         │       │                               │   │
└─────────┘       │                               │   │
                  │                               │   │
        输入电容  │  输出电容                      │   │
    10uF ┤├──────┘    10uF ┤├────────────────────┘   │
         GND              3.3V                        │
```

**电源电路元件清单**：
- AMS1117-3.3: 输入 5V (USB VBUS)，输出 3.3V
- 输入电容: 10uF 陶瓷电容，接在 VIN 和 GND 之间
- 输出电容: 10uF 陶瓷电容，接在 VOUT 和 GND 之间
- 每个 VDD 引脚旁放 100nF 去耦电容（共 4 个）

### 2. MLX90640 接线 | MLX90640 Connection

```
STM32F411                              MLX90640 模块
┌──────────────┐                       ┌──────────────┐
│              │                       │              │
│  PB6 (SCL) ──┼───────────────────────┼── SCL (Pin3) │
│              │          4.7K ┤├      │              │
│              │           3.3V        │              │
│              │                       │              │
│  PB7 (SDA) ──┼───────────────────────┼── SDA (Pin4) │
│              │          4.7K ┤├      │              │
│              │           3.3V        │              │
│              │                       │              │
│       3.3V ──┼───────────────────────┼── VDD (Pin1) │
│              │                       │              │
│        GND ──┼───────────────────────┼── GND (Pin2) │
│              │                       │              │
└──────────────┘                       └──────────────┘
```

**MLX90640 引脚定义（标准模块）**：
| 引脚 | 名称 | 连接到 | 说明 |
|------|------|--------|------|
| 1 | VDD | 3.3V | 电源（3.3V，不可接 5V） |
| 2 | GND | GND | 地 |
| 3 | SCL | PB6 (I2C1_SCL) | I2C 时钟 |
| 4 | SDA | PB7 (I2C1_SDA) | I2C 数据 |

> **重要**：I2C 总线的 SCL 和 SDA 各需要一个 4.7K 上拉电阻接到 3.3V。部分 MLX90640 模块已自带上拉电阻，请确认后再决定是否需要额外添加。

### 3. OV2640 摄像头接线 | OV2640 Camera Connection

```
STM32F411 (DCMI)                    OV2640 模块 (24pin FPC)
┌───────────────────┐                ┌──────────────────┐
│                   │                │                  │
│  PA6  (D0)   ─────┼────────────────┼── D0  (Pin 8)    │
│  PA7  (D1)   ─────┼────────────────┼── D1  (Pin 9)    │
│  PC4  (D2)   ─────┼────────────────┼── D2  (Pin 10)   │
│  PC5  (D3)   ─────┼────────────────┼── D3  (Pin 11)   │
│  PB0  (D4)   ─────┼────────────────┼── D4  (Pin 12)   │
│  PB1  (D5)   ─────┼────────────────┼── D5  (Pin 13)   │
│  PA10 (D6)   ─────┼────────────────┼── D6  (Pin 14)   │
│  PA11 (D7)   ─────┼────────────────┼── D7  (Pin 15)   │
│                   │                │                  │
│  PA4  (HSYNC) ────┼────────────────┼── HREF (Pin 4)   │
│  PA6* (VSYNC) ────┼────────────────┼── VSYNC (Pin 3)  │
│  PA3  (PCLK)  ────┼────────────────┼── PCLK (Pin 5)   │
│                   │                │                  │
│  PA8  (SCCB_SCL)──┼────────────────┼── SCL  (Pin 7)   │
│  PA9  (SCCB_SDA)──┼────────────────┼── SDA  (Pin 6)   │
│                   │                │                  │
│  3.3V         ─────┼────────────────┼── VDD  (Pin 1/2) │
│  GND          ─────┼────────────────┼── GND  (Pin 16)  │
│                   │                │                  │
│  (PWDN GPIO)  ─────┼────────────────┼── PWDN (Pin 2)  │
│                   │                │                  │
└───────────────────┘                └──────────────────┘
```

**OV2640 标准引脚定义（24pin FPC 排线）**：
| Pin | 名称 | 方向 | 连接到 | 说明 |
|-----|------|------|--------|------|
| 1,2 | VDD (3.3V) | 电源 | 3.3V | 摄像头供电 |
| 3 | VSYNC | 输出 | DCMI_VSYNC | 帧同步 |
| 4 | HREF | 输出 | DCMI_HSYNC | 行同步 |
| 5 | PCLK | 输出 | DCMI_PIXCLK | 像素时钟 |
| 6 | SDA | 双向 | SCCB_SDA | SCCB 数据 |
| 7 | SCL | 输入 | SCCB_SCL | SCCB 时钟 |
| 8-15 | D0-D7 | 输出 | DCMI_D0-D7 | 8位并行数据 |
| 16 | GND | 电源 | GND | 地 |
| 17 | PWDN | 输入 | GPIO (可选) | 断电控制，高电平断电 |

> **SCCB 上拉**：OV2640 的 SCCB 总线（SCL/SDA）需要 4.7K 上拉电阻到 3.3V。注意 SCCB 和 MLX90640 的 I2C 使用不同的 I2C 外设，不会冲突。

### 4. USB 接口接线 | USB Connection

```
STM32F411                              USB Type-C 母座
┌──────────────┐                       ┌──────────────┐
│              │                       │              │
│  PA12 (DP) ──┼───[22R]───────────────┼── D+ (A6/B6) │
│              │                       │              │
│  PA11 (DM) ──┼───[22R]───────────────┼── D- (A7/B7) │
│              │                       │              │
│       3.3V ──┼───[1.5K]──┬──────────┤              │
│              │            │          │              │
│              │            └──────────┼── D+ (DP)    │
│              │                       │              │
│       5V    ─┼───────────────────────┼── VBUS       │
│              │                       │              │
│       GND   ─┼───────────────────────┼── GND        │
│              │                       │              │
└──────────────┘                       └──────────────┘
```

**USB 接线要点**：
1. **D+/D- 串联电阻**：22R 电阻串联在信号线上，用于阻抗匹配，减少信号反射
2. **D+ 上拉电阻**：1.5K 电阻接在 PA12 (D+) 和 3.3V 之间，告诉主机这是 USB 全速设备
3. **VBUS**：直接连接 USB 5V 到 LDO 输入
4. **GND**：USB 地与板子地连接

> **USB Type-C 注意事项**：如果不使用 CC 芯片，需要在 Type-C 的 CC1 和 CC2 引脚各接一个 5.1K 下拉电阻到 GND，让手机识别这是一个 UFP（上行端口，即设备端）。这样手机才会供电并尝试通信。

### 5. 调试接口接线 | Debug Interface (SWD)

```
ST-Link V2                             STM32F411
┌──────────────┐                       ┌──────────────┐
│              │                       │              │
│  SWDIO   ────┼───────────────────────┼── PA13       │
│  SWCLK   ────┼───────────────────────┼── PA14       │
│  3.3V    ────┼───────────────────────┼── 3.3V       │
│  GND     ────┼───────────────────────┼── GND        │
│              │                       │              │
└──────────────┘                       └──────────────┘
```

> ST-Link 的 3.3V 是参考电压，不要用来给整个板子供电。板子应由 USB 供电。

---

## 完整引脚映射速查表 | Quick Pin Mapping Reference

| Pin # | Pin Name | 功能 | 连接目标 |
|-------|----------|------|----------|
| 1 | VBAT | 电源 | 3.3V |
| 2 | PC13 | GPIO | LED 红色 (电源指示) |
| 3 | PC14 | GPIO | LED 绿色 (数据指示) |
| 4 | PC15 | GPIO | 未用 |
| 5 | PH0 | HSE IN | 8MHz 晶振 |
| 6 | PH1 | HSE OUT | 8MHz 晶振 |
| 7 | NRST | 复位 | 复位按键 + 10K 上拉 |
| 8 | VSSA | 模拟地 | GND |
| 9 | VDDA | 模拟电源 | 3.3V |
| 10 | PA0 | GPIO | 未用 / 预留 |
| 11 | PA1 | GPIO | 未用 / 预留 |
| 12 | PA2 | GPIO | 未用 / 预留 |
| 13 | PA3 | DCMI_PIXCLK | OV2640 PCLK |
| 14 | PA4 | DCMI_HSYNC | OV2640 HREF |
| 15 | PA5 | GPIO | 未用 |
| 16 | PA6 | DCMI_D0 | OV2640 D0 |
| 17 | PA7 | DCMI_D1 | OV2640 D1 |
| 18 | PB0 | DCMI_D4 | OV2640 D4 |
| 19 | PB1 | DCMI_D5 | OV2640 D5 |
| 20 | PB2 | GPIO | 未用 |
| 21 | PB3 | GPIO | 未用 |
| 22 | PB4 | GPIO | 未用 |
| 23 | VDD | 电源 | 3.3V |
| 24 | VSS | 地 | GND |
| 25 | PB5 | GPIO | 未用 |
| 26 | PB6 | I2C1_SCL | MLX90640 SCL |
| 27 | PB7 | I2C1_SDA | MLX90640 SDA |
| 28 | BOOT0 | 启动选择 | 10K 下拉 GND |
| 29 | PB8 | GPIO | 未用 / 预留 |
| 30 | PB9 | GPIO | 未用 / 预留 |
| 31 | PB10 | GPIO | 未用 / 预留 |
| 32 | PA8 | SCCB_SCL | OV2640 SCL |
| 33 | PA9 | SCCB_SDA | OV2640 SDA |
| 34 | PA10 | DCMI_D6 | OV2640 D6 |
| 35 | VDD | 电源 | 3.3V |
| 36 | VSS | 地 | GND |
| 37 | PA11 | USB_DM | USB D- |
| 38 | PA12 | USB_DP | USB D+ |
| 39 | PA13 | SWDIO | ST-Link SWDIO |
| 40 | PA14 | SWCLK | ST-Link SWCLK |
| 41 | PA15 | GPIO | 未用 / 预留 |
| 42 | PC10 | GPIO | 未用 / 预留 |
| 43 | PC11 | GPIO | 未用 / 预留 |
| 44 | PC12 | GPIO | 未用 / 预留 |
| 45 | PD2 | GPIO | 未用 / 预留 |
| 46 | PC4 | DCMI_D2 | OV2640 D2 |
| 47 | PC5 | DCMI_D3 | OV2640 D3 |
| 48 | VDD | 电源 | 3.3V |

---

## 接线注意事项 | Wiring Notes & Precautions

### 电源相关 | Power Notes

1. **LDO 散热**：AMS1117-3.3 压降 1.7V (5V→3.3V)，如果总电流 200mA，功耗约 0.34W。正常工作不需额外散热。
2. **去耦电容**：每个 VDD/VSS 引脚对旁边必须放 100nF 电容，距离越近越好。
3. **模拟电源**：VDDA (Pin 9) 通过一个 10R 电阻 + 100nF 电容的 RC 滤波器从 3.3V 取电，可以减少模拟噪声。
4. **不要用 5V 给 MLX90640 供电**：MLX90640 的工作电压是 3.3V，接 5V 会烧毁传感器。

### I2C 相关 | I2C Notes

1. **上拉电阻**：I2C 总线必须接上拉电阻。如果 MLX90640 模块已带上拉，检查阻值是否合适（4.7K 是标准值）。
2. **总线速度**：MLX90640 支持 I2C 标准模式 (100kHz) 和快速模式 (400kHz)。本项目使用 400kHz 以获得更高的帧率（最高可达 32fps）。
3. **线缆长度**：I2C 总线不宜过长，PCB 上的走线控制在 5cm 以内最佳。

### USB 相关 | USB Notes

1. **D+ 上拉电阻**：这是 USB 枚举的关键。1.5K 上拉电阻告诉主机（手机）这是一个全速 USB 设备。没有这个电阻，手机不会识别设备。
2. **USB 走线**：D+ 和 D- 走线应平行等长，间距尽量一致，避免过长的走线。
3. **Type-C CC 下拉**：5.1K 下拉电阻是必需的，否则手机不会供电。
4. **不要使用 USB Hub**：调试时直接连接手机，USB Hub 可能导致供电不足或通信不稳定。

### 焊接相关 | Soldering Notes

1. **MCU 焊接方向**：LQFP48 封装的第 1 脚旁边有一个小圆点（或缺口），不要焊反。
2. **MLX90640 镜头保护**：焊接时注意不要加热过度，避免热传导到镜头。焊接前可以用胶带临时保护镜头表面。
3. **FPC 排线**：OV2640 的 FPC 排线很脆弱，不要反复弯折。插入 FPC 座子时先打开翻盖，插入排线后扣紧翻盖。
4. **检查短路**：焊接完成后，用万用表测量 3.3V 和 GND 之间是否有短路，确认无误后再上电。

---

## 接线检查清单 | Wiring Checklist

完成接线后，按以下清单逐项检查：

```
电源部分：
□ USB Type-C VBUS → LDO VIN → 3.3V 输出
□ LDO 输入电容 (10uF) 已安装
□ LDO 输出电容 (10uF) 已安装
□ 每个 VDD 引脚旁有 100nF 去耦电容
□ 3.3V 和 GND 之间无短路（万用表蜂鸣档）

MCU 基础：
□ 8MHz 晶振已连接 PH0/PH1
□ 20pF 负载电容 x2 已安装
□ NRST 接 10K 上拉 + 复位按键
□ BOOT0 接 10K 下拉到 GND
□ VBAT 接 3.3V

MLX90640：
□ PB6 → SCL, PB7 → SDA
□ 4.7K 上拉电阻 x2 已安装（或模块自带）
□ VDD 接 3.3V（不是 5V！）
□ GND 已连接

OV2640：
□ DCMI 数据线 D0-D7 已连接
□ HSYNC/VSYNC/PCLK 已连接
□ SCCB SCL/SDA 已连接
□ 4.7K SCCB 上拉电阻已安装
□ VDD 接 3.3V, GND 已连接

USB：
□ PA12(D+) → 22R → USB D+
□ PA11(D-) → 22R → USB D-
□ 1.5K 上拉电阻在 D+ 和 3.3V 之间
□ Type-C CC1/CC2 各接 5.1K 下拉到 GND

调试接口：
□ PA13(SWDIO) → ST-Link
□ PA14(SWCLK) → ST-Link
□ 3.3V 和 GND 连接到 ST-Link

LED：
□ PC13 → 1K → 红色 LED → GND
□ PC14 → 1K → 绿色 LED → GND
```

---

*最后更新：2026-05-27 | Last Updated: 2026-05-27*
