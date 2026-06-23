# 组装步骤 | Assembly Steps

本文档提供"ThermalEyes"双目手机热成像仪的详细硬件组装指导。

**主线方案**：使用现成的 WeAct Black Pill 开发板 + MLX90640 模块 + OV2640 模块 + 杜邦线接线，零焊接即可跑通整套系统（对齐 `prerequisites.md` 第 135-183 行的 FAQ）。想挑战硬件焊接的同学，可走文末「可选进阶：自制 PCB」路线（⭐⭐⭐ 难度，含 LQFP48 拖焊）。

> This document provides detailed hardware assembly instructions for the "ThermalEyes" binocular thermal imager. The main path uses an off-the-shelf WeAct Black Pill dev board + MLX90640 module + OV2640 module + Dupont wires — no soldering required. A self-fabricated PCB path (with LQFP48 drag-soldering) is available as an optional advanced track at the end.

---

## 准备工作 | Preparation

### 工具清单 | Tools Needed

| 工具 | 用途 | 必备/可选 |
|------|------|-----------|
| USB-C 数据线（2 条） | 烧录/通信/连摄像头 | 必备 |
| 杜邦线（母对母，≥6 根） | 连接开发板与模块 | 必备 |
| 数字万用表 | 测电压、导通 | 必备 |
| ST-Link V2 下载器 | SWD 烧录（也可用板载 USB DFU 替代） | 可选 |
| 串口调试工具（PuTTY/Tera Term/Arduino IDE 串口监视器） | 查看 USB CDC 输出 | 必备 |
| 恒温电烙铁 + 焊锡丝 + 助焊剂 + 镊子 + 吸锡带 | 仅「可选进阶：自制 PCB」路线需要 | 进阶 |

### 元件检查 | Component Check

拆开所有包装后，按 BOM 清单逐项核对。建议用一个小盒子或分区托盘将元件分类放置：

```
□ WeAct Black Pill 开发板（STM32F411CEU6）  — 板载 USB-C 接口、BOOT0 按键、复位按键
□ MLX90640 热成像模块（BAA，I2C 接口）        — 注意保护镜头，多数模块自带上拉电阻
□ OV2640 摄像头模块（带排针或排母）           — 用于可见光路（融合时用）
□ USB Type-C 数据线（2 条）                   — 一条连开发板烧录/通信，一条连摄像头（如用 USB 摄像头）
□ 杜邦线（母对母，至少 6 根）                 — 连接开发板和 MLX90640/OV2640
□ ST-Link V2 下载器（可选）                   — 也可用板载 USB 直接烧录 DFU
□ 万用表                                      — 测电压、导通排查
```

> 如果你走的是文末「可选进阶：自制 PCB」路线（⭐⭐⭐），则还需要 STM32F411CEU6 (LQFP48) 裸片、LDO、晶振、阻容、USB Type-C 母座、24pin FPC 座子等散件，见该章节的元件清单。

---

## 第一步：开发板接线 | Step 1: Dev-Board Wiring

主线用现成模块 + 杜邦线，无需焊接。引脚映射详见 `wiring-guide.md` 的 48 脚映射表（开发板和自制板通用）。

### 1.1 接 MLX90640 热成像模块（I2C）

| MLX90640 模块 | WeAct Black Pill | 说明 |
|---------------|------------------|------|
| VIN / VDD     | 3.3V             | **注意是 3.3V，不是 5V**，接错可能烧模块 |
| GND           | GND              | 共地 |
| SCL           | PB6              | I2C1_SCL |
| SDA           | PB7              | I2C1_SDA |

> 部分模块已自带 4.7kΩ 上拉电阻；若你的模块没有，在 SCL、SDA 各加一个 4.7kΩ 上拉到 3.3V。

### 1.2 接 OV2640 可见光摄像头（DCMI，融合时才需要）

OV2640 模块通常走 DCMI 并行接口，引脚较多（D0-D7、PCLK、VSYNC、HSYNC、SCCB）。Day 11-13 做图像融合时再接；先跑通热成像主链路时可以不接。具体引脚映射见 `wiring-guide.md` 的 DCMI 接线图。

> 如果你的 OV2640 是带排针的模块，直接用杜邦线按表接好即可；如果是 FPC 排线款，需要转接板，新手建议选排针模块。

### 1.3 通电与基础检查（迁移自原焊接版，开发板同样适用）

**阶段测试：电源与连接**
```
1. 用 USB-C 线连接 Black Pill 和电脑，板载电源灯应常亮
2. 万用表测量板上 3.3V 引脚：应为 3.3V ± 0.1V
3. 测量 3.3V 和 GND 之间：应有一定电阻（kΩ 级别），不应短路（0 欧）
4. 拔掉 USB，确认 MLX90640 接线无误后再上电
5. 如果电源灯不亮：换一条 USB 数据线（很多 USB-C 线是纯充电线）
```

### 1.4 I2C 扫描确认 MLX90640 在线

烧录一个简单的 I2C 扫描程序（HW1 任务 3 提供），扫描 I2C1 总线，应在地址 0x33 发现 MLX90640。扫不到时按 troubleshooting.md 的「I2C 扫描不到设备」排查（接线/上拉/3.3V/速度）。

---

## 第二步：固件烧录 | Step 2: Firmware Flashing

### 安装 STM32CubeIDE

1. 下载 STM32CubeIDE：https://www.st.com/en/development-tools/stm32cubeide.html
2. 安装时选择默认选项
3. 安装 ST-Link 驱动

### 烧录固件

1. **克隆固件仓库**：
   ```bash
   git clone https://github.com/colourfate/thermal_bridge.git
   ```

2. **打开工程**：
   - 启动 STM32CubeIDE
   - File → Open Projects from File System → 选择 thermal_bridge 文件夹

3. **编译工程**：
   - Project → Build All
   - 确认无编译错误

4. **连接 ST-Link**：
   - ST-Link 连接到 Black Pill 的 SWD 调试接口（SWDIO→PA13, SWCLK→PA14, GND→GND）
   - 确认 ST-Link 指示灯亮起
   - 也可以不接 ST-Link，直接用板载 USB-C 走 DFU 模式烧录

5. **烧录**：
   - Run → Debug (F11) 或 Run → Flash
   - 等待烧录完成
   - MCU 将自动复位并开始运行

### 首次测试

烧录固件后，进行以下测试：

1. **LED 测试**：
   - 板载 LED（PC13）应常亮或闪烁（表示有电、初始化或数据传输）

2. **USB 连接测试**：
   - 用 USB-C 线连接 Black Pill 和电脑
   - 电脑设备管理器应出现新的 COM 端口（USB CDC 虚拟串口）
   - 打开串口工具应能收到温度帧数据

3. **MLX90640 数据测试**：
   - 用手指靠近 MLX90640 镜头
   - PC 端可视化脚本（或串口工具）上应能看到对应像素温度上升

4. **OV2640 图像测试**（接了摄像头时）：
   - 融合脚本应能读到可见光画面
   - 画面应清晰，无花屏或条纹

---

## 第三步：最终组装 | Step 3: Final Assembly

### 方案 A：裸板/模块直接使用（最简单）

直接使用开发板和模块，无需外壳。注意以下几点：

1. MLX90640 镜头避免触摸和刮擦
2. OV2640 镜头避免触摸
3. 杜邦线接口较多，搬运时注意别松脱；可用扎带或胶带固定线束
4. 开发板背面和模块背面避免短路，可贴一层绝缘胶带

### 方案 B：3D 打印外壳（推荐）

OSHWhub 项目提供了 3D 打印外壳的 STL 文件：

1. **下载 STL 文件**：
   从 https://oshwhub.com/colourfate/binocular_thermal_imager 的"附件"中下载 `3d_printed_casing.zip`

2. **3D 打印**：
   - 使用 PLA 或 PETG 材料
   - 层高 0.2mm
   - 无需支撑（根据具体 STL 设计）
   - 可以使用学校/图书馆的 3D 打印机
   - 也可以找淘宝 3D 打印代工，约 ¥10-20

3. **组装外壳**：
   - 将 PCB 放入外壳底座
   - 对准 USB Type-C 接口和镜头开孔
   - 用螺丝或卡扣固定
   - 盖上外壳顶部

### 方案 C：纸盒外壳（应急方案）

如果没有 3D 打印条件：

1. 找一个合适大小的硬纸盒
2. 在对应位置开孔：
   - USB Type-C 接口孔
   - MLX90640 镜头孔（约 10mm 直径）
   - OV2640 镜头孔（约 8mm 直径）
3. 用双面胶或热熔胶固定 PCB
4. 可以用黑色电工胶布包裹外观

---

## 组装检查清单 | Assembly Checklist

完成所有组装后，逐项确认（开发板主线）：

```
接线与电源：
□ USB-C 数据线（非纯充电线），板载电源灯常亮
□ 万用表测得板上 3.3V 引脚 = 3.3V ± 0.1V
□ 3.3V 与 GND 之间不短路
□ MLX90640 模块 VIN 接 3.3V（不是 5V）
□ 杜邦线 SCL/SDA 方向正确，接触良好

MCU 系统：
□ 开发板能进入烧录模式（BOOT0 按键可用）
□ ST-Link 能连接并读取芯片 ID（或 DFU 模式可烧录）
□ 固件烧录成功

传感器系统：
□ MLX90640 模块连接正确，I2C 通信正常（扫描到 0x33）
□ OV2640 摄像头接线正确（接了的话），SCCB 通信正常
□ 两个传感器镜头无遮挡

USB 通信：
□ USB 线是数据线（非纯充电线）
□ 电脑设备管理器出现新 COM 端口
□ PC 端可视化脚本能正常接收温度帧

外观：
□ 线束固定，搬运不易松脱
□ 外壳安装（如有）
□ 标签或标记
```

> 走「可选进阶：自制 PCB」路线的同学，额外检查焊点质量：所有焊点光亮、无虚焊、无焊锡桥接、助焊剂残留已用酒精清洗、PCB 正反面无明显短路、STM32 芯片第 1 脚方向正确。

---

## 常见组装问题 | Common Assembly Issues

### Q1: 开发板 ST-Link 连不上 / 烧录失败

**可能原因**：
- USB 线是纯充电线（无数据功能）
- 开发板未进入烧录模式（BOOT0 未按）
- ST-Link 驱动未安装
- SWD 引脚（PA13/PA14）被外设占用

**解决方法**：
1. 换一条确认能传数据的 USB-C 线
2. 按 BOOT0 按键再上电，进入 DFU/烧录模式
3. 安装 ST-Link 驱动；或改用板载 USB DFU 烧录
4. 确认 SWDIO/SWCLK 接线正确（PA13/PA14）
5. 走「可选进阶：自制 PCB」的同学，还要检查 MCU 第 1 脚方向、引脚是否桥接、NRST 和 BOOT0 电阻、VDD 电压

### Q2: MLX90640 扫描不到

**可能原因**：
- I2C 接线错误
- 上拉电阻缺失
- MLX90640 供电异常
- I2C 速度设置过快

**解决方法**：
1. 确认 SCL 和 SDA 未接反
2. 测量 SCL/SDA 线上的电压，空闲时应为 3.3V
3. 确认 MLX90640 VDD 为 3.3V
4. 降低 I2C 速度到标准模式 (100kHz) 重新尝试

### Q3: OV2640 画面全黑或花屏

**可能原因**：
- FPC 排线方向错误
- FPC 排线未插紧
- SCCB 通信异常
- PWDN 引脚处于高电平（断电状态）

**解决方法**：
1. 取出 FPC 排线，确认金手指方向正确
2. 重新插入排线，确保推到底
3. 检查 SCCB 上拉电阻
4. 确认 PWDN 引脚（如果有使用）为低电平

### Q4: USB 连电脑无反应（设备管理器没出现 COM 口）

**可能原因**：
- 使用了纯充电线（无数据功能）
- 固件没正确配置 USB CDC
- 开发板未上电或接触不良

**解决方法**：
1. 换一条确认能传数据的 USB-C 线（最常见原因）
2. 确认固件里 USB CDC 已启用并正确初始化
3. 换一个 USB 口或换一台电脑试试
4. 走「可选进阶：自制 PCB」的同学，还要检查 D+ 上拉电阻（1.5K）、Type-C CC 下拉电阻（5.1K x2）是否安装

---

## 安全注意事项 | Safety Notes

1. **防静电**：触摸开发板和模块前，先用手摸一下金属物体（如桌子金属腿）释放静电。有条件的话佩戴防静电手环。
2. **MLX90640 保护**：红外传感器的镜头表面有镀膜，不要用手指触摸，不要用酒精或溶剂擦拭。如有灰尘用气吹清理。
3. **USB 供电**：本项目仅从 USB 取电，电流很小（约 200mA），安全。但不要同时连接 ST-Link 供电和 USB 供电，避免电源冲突。
4. **（仅自制 PCB 路线）烙铁安全**：电烙铁温度高达 350°C，切勿触摸烙铁头。不用时放回烙铁架。离开工位时断电。焊接时保持通风（助焊剂挥发有烟尘），焊后用无水乙醇清洗残留助焊剂。

---

## 可选进阶：自制 PCB | Optional Advanced: Self-Fabricated PCB

> ⭐⭐⭐ 难度。主线已用开发板+模块跑通整条链路，想挑战硬件焊接的同学可走这条进阶路线：参考开源工程自制 PCB，手工拖焊 STM32F411 LQFP48。需要焊接基础（LQFP48 0.5mm 间距 + 0402 贴片 + 24pin FPC 0.5mm 座子），零焊接经验建议先练手再上。

### 进阶元件清单

```
□ STM32F411CEU6 (LQFP48)           — 放在防静电袋中
□ USB Type-C 母座
□ 24pin FPC 座子
□ AMS1117-3.3 (LDO)
□ 8MHz 晶振 + 20pF 电容 x2
□ 电阻: 10K x5, 1.5K x1, 22R x2, 1K x2, 4.7K x4, 5.1K x2
□ 电容: 100nF x4, 10uF x2
□ LED: 红色 x1, 绿色 x1
□ 轻触按键 x1
□ PCB (5片)
□ 排针 (可选，调试用)
□ 恒温电烙铁、焊锡丝、助焊剂、镊子、放大镜、吸锡带、酒精+棉签
```

### 进阶 A：PCB 打样

1. 访问 https://oshwhub.com/colourfate/binocular_thermal_imager，克隆工程到自己的账号
2. 在嘉立创 EDA 中打开克隆的工程，检查走线、封装、板子尺寸（约 50x30mm）
3. 一键下单到 https://www.jlc.com/ ，参数：2 层、1.6mm、1oz、沉金（ENIG，对贴片更友好）、最小 5 片起订，约 ¥5
4. 嘉立创打样通常 2-3 天生产，快递 1-3 天

> 想自己设计 PCB 的同学，最小系统板设计要点：MCU 居中、USB Type-C 与传感器分列两端；去耦电容紧贴 VDD；D+/D- 平行等长、差分阻抗 90Ω；I2C 走线短直；底层大面积铺地；丝印标注每个接口名称。

### 进阶 B：元件焊接

**焊接顺序**：从矮到高、从小到大，先贴片后插件。

1. **电源电路**：USB Type-C 母座 → AMS1117-3.3 (LDO，注意 SOT-223 tab 面是 VOUT) → 电源滤波电容（输入/输出各 10uF）。阶段测试：USB 上电后万用表测 LDO 输出 = 3.3V ± 0.1V。
2. **MCU 最小系统**：
   - **拖焊 STM32F411 LQFP48**（最难）：涂薄助焊剂 → 对齐第 1 脚（芯片小圆点对 PCB 丝印 Pin 1）→ 先焊对角两脚固定 → 涂助焊剂 → 烙铁带少量锡沿引脚拖焊 → 放大镜查连锡 → 有连锡涂助焊剂重拖或吸锡带吸走 → 四边重复。烙铁 320-350°C，助焊剂是关键，别急。
   - 去耦电容（100nF x4）放 MCU 每对 VDD/VSS 旁；8MHz 晶振 + 两个 20pF 接 PH0/PH1；NRST 接 10K 上拉 + 复位按键到 GND；BOOT0 (Pin 44) 接 10K 到 GND。
   - 阶段测试：上电测 VDD=3.3V、NRST=3.3V；接 ST-Link（SWDIO→PA13, SWCLK→PA14, GND→GND，不接 3.3V）读芯片 ID，读不到就查方向/桥接/电阻/供电。
3. **MLX90640 接口**：I2C 上拉（4.7K x2，PB6/PB7→3.3V，模块自带可跳过）→ 焊 4pin 排母插模块（或直接焊模块）→ VDD→3.3V/GND/SCL→PB6/SDA→PB7。
4. **OV2640 接口**：焊 24pin FPC 座子（细间距，涂助焊剂、对齐、先焊两端固定再焊全排）→ SCCB 上拉（4.7K x2，PA8/PA9→3.3V）→ 插 FPC 排线（金手指朝下、推到底、压翻盖，排线脆弱别反复折）。
5. **USB 外围**：USB 阻抗电阻（22R x2，PA12/PA11→Type-C D+/D-）→ D+ 上拉（1.5K，PA12→3.3V，告诉主机这是全速设备）→ Type-C CC 下拉（5.1K x2，CC1/CC2→GND）→ LED（红 PC13、绿 PC14，各串 1K→GND）。

### 进阶 C：测试与外壳

焊接完成后，用主线第三步的「组装检查清单」逐项核对（额外核对焊点质量：光亮、无虚焊、无桥接、助焊剂已清洗）。外壳可参考主线第三步的方案 B（3D 打印，OSHWhub 提供 STL）或方案 C（纸盒应急）。

---

*最后更新：2026-06-23 | Last Updated: 2026-06-23*
