# 双目手机热成像仪 "ThermalEyes" | Binocular Mobile Phone Thermal Imager

## 项目简介 | Project Overview

"ThermalEyes" 是一个基于可见光和热成像传感器的双目手机热成像仪开源项目。设备同时具有普通可见光摄像头和红外热成像传感器，通过 MCU（STM32F411）读取传感器数据后经 USB 传输到手机端，借助手机算力进行图像处理和融合，最终在手机屏幕上显示高质量的热成像叠加图像。

> "ThermalEyes" is an open-source binocular thermal imager for smartphones. The device features both a visible-light camera and an infrared thermal sensor. Data is read by an STM32F411 MCU and transmitted to the phone via USB, where image fusion and processing are performed using OpenCV, producing high-quality thermal overlay images on the phone screen.

- **作者**：灰质君（colourfate）
- **B站视频**：https://www.bilibili.com/video/BV1M64y177ig
- **开源协议**：MIT License

## 最终效果 | Final Result

完成本项目后，你将拥有一个能够：
- 通过 USB 连接手机的热成像设备
- 实时采集可见光和热成像双路数据
- 在手机上实时显示图像融合结果（可见光 + 热成像叠加）
- 跟踪显示最高/最低温度点
- 切换多种伪彩色色彩映射方案
- 调整融合算法参数（视差补偿、高频比例）

After completing this project, you will have a device that can:
- Connect to a phone via USB for thermal imaging
- Acquire both visible-light and thermal data in real time
- Display real-time image fusion results on the phone
- Track and display min/max temperature points
- Switch between multiple pseudo-color palettes
- Adjust fusion algorithm parameters (disparity compensation, high-frequency ratio)

## 核心技术 | Core Technologies

| 技术 | 用途 | 说明 |
|------|------|------|
| **STM32F411** | MCU 主控 | ARM Cortex-M4，512KB Flash，128KB RAM，UFQFPN48 封装 |
| **MLX90640** | 热成像传感器 | 32×24 像素红外热成像阵列，I2C 接口 |
| **USB CDC/HS** | 数据传输 | MCU 与手机之间的高速 USB 通信 |
| **UVC** | 视频传输 | USB Video Class 协议传输可见光画面 |
| **Android (Java/C++)** | 手机 APP | 接收数据、图像处理、界面展示 |
| **OpenCV** | 图像处理 | 高斯模糊图像融合、伪彩色映射 |

## 开源资源 | Open Source Resources

| 资源 | 链接 |
|------|------|
| PCB 设计 | https://oshwhub.com/colourfate/binocular_thermal_imager |
| 固件代码 | https://github.com/colourfate/thermal_bridge |
| Android APP | https://github.com/colourfate/ThermalEyes |
| B站演示视频 | https://www.bilibili.com/video/BV1M64y177ig |

## 硬件清单 | Hardware List

| 部件 | 规格 | 参考价格 | 说明 |
|------|------|----------|------|
| STM32F411CEU6 | Cortex-M4 MCU | ¥15 | 主控芯片 |
| MLX90640 | 32×24 热成像传感器 | ¥80-120 | 红外热成像模块 |
| OV2640/OV5640 | 可见光摄像头模块 | ¥15-25 | 用于可见光采集 |
| USB Type-C 连接器 | - | ¥2 | 与手机连接 |
| PCB | 嘉立创打样 | ¥5-20 | 参考OSHWhub设计 |
| 其他被动元件 | 电容电阻等 | ¥10-20 | 按BOM采购 |

**总预算：约 ¥150-200**

详细清单见 [hardware/BOM.md](hardware/BOM.md)

## 软件环境 | Software Environment

### 固件开发
- **IDE**：STM32CubeIDE
- **框架**：STM32 HAL 库
- **语言**：C

### APP 开发
- **IDE**：Android Studio
- **最低 SDK**：Android 5.0 (API 21)
- **目标 SDK**：Android 13 (API 33)
- **语言**：Java + C++ (NDK/OpenCV)
- **核心依赖**：OpenCV、UVCAndroid、UsbSerial

## 快速开始 | Quick Start

### 固件
```bash
# 1. 克隆固件仓库
git clone https://github.com/colourfate/thermal_bridge.git

# 2. 用 STM32CubeIDE 打开工程

# 3. 编译并烧录到 STM32F411
```

### APP
```bash
# 1. 克隆 APP 仓库
git clone https://github.com/colourfate/ThermalEyes.git

# 2. 用 Android Studio 打开工程

# 3. 编译并安装到手机（使用 WiFi ADB 调试，因为 USB 口被热成像设备占用）
```

## 课程安排 | Course Schedule

本项目为期 **15 天**，分为五个阶段：

### 阶段一：基础准备（Day 1-3）
- Day 1: 项目介绍 + 环境搭建
- Day 2: STM32 基础与 GPIO
- Day 3: I2C 通信协议

### 阶段二：传感器驱动（Day 4-7）
- Day 4: MLX90640 热成像传感器原理
- Day 5: I2C 驱动 MLX90640
- Day 6: 传感器数据读取与解析
- Day 7: 温度数据校准与验证

### 阶段三：USB 通信（Day 8-10）
- Day 8: USB 基础与 CDC 协议
- Day 9: USB 数据传输实现
- Day 10: 固件联调测试

### 阶段四：Android APP（Day 11-13）
- Day 11: Android 项目搭建 + USB 通信
- Day 12: OpenCV 图像融合算法
- Day 13: 伪彩色映射与温度跟踪

### 阶段五：集成优化（Day 14-15）
- Day 14: 全系统集成联调
- Day 15: 优化 + 演示准备

## 评分标准 | Grading Rubric

| 维度 | 权重 | 说明 |
|------|------|------|
| 技术实现 | 40% | 功能完整度、代码质量 |
| 文档质量 | 20% | README、注释、图表 |
| 演示展示 | 20% | Final Demo 清晰度与深度 |
| 进度汇报 | 10% | 每周 Check-in 质量 |
| 团队协作 | 10% | Git 提交记录、分工合理性 |

详细评分标准见 [assignments/rubric.md](assignments/rubric.md)

## 项目结构 | Project Structure

```
project-01-binocular-thermal-imager/
├── curriculum/          # 课程大纲与每日任务
│   ├── overview.md      # 课程总纲
│   ├── prerequisites.md # 前置知识
│   ├── day-01.md ~ day-15.md
│   ├── assignments.md   # 作业汇总
│   └── grading-rubric.md
├── hardware/            # 硬件相关
│   ├── BOM.md           # 物料清单
│   ├── wiring-guide.md  # 接线指南
│   ├── assembly-steps.md
│   └── troubleshooting.md
├── software/            # 软件代码框架
│   ├── firmware/        # STM32 固件参考
│   ├── android/         # Android APP 参考
│   ├── requirements.txt
│   └── config.template.yaml
└── assignments/         # 作业与评分
    ├── week-1-checkin.md
    ├── week-2-checkin.md
    ├── week-3-checkin.md
    ├── final-presentation.md
    └── rubric.md
```

## 学习资源 | Learning Resources

- [STM32CubeIDE 官方文档](https://www.st.com/en/development-tools/stm32cubeide.html)
- [MLX90640 数据手册](https://www.melexis.com/en/product/MLX90640)
- [OpenCV 官方文档](https://docs.opencv.org/)
- [Android USB Host 文档](https://developer.android.com/guide/topics/connectivity/usb-host)
- [B站项目演示](https://www.bilibili.com/video/BV1M64y177ig)

## 常见问题 | FAQ

**Q: 需要什么基础？**
A: 最好有 C 语言和基本电路知识。如果没有 STM32 经验，课程会从基础开始引导。

**Q: PCB 必须自己设计吗？**
A: 不需要。OSHWhub 上有完整的设计文件，可以直接通过嘉立创打样。如果你有能力，也可以尝试修改设计。

**Q: 可以用其他 MCU 吗？**
A: 原则上可以，但本项目针对 STM32F411 优化，教程以 STM32CubeIDE 为主。

**Q: iOS 可以用吗？**
A: 目前 APP 仅支持 Android。iOS 需要自行开发，可考虑作为进阶挑战。

## 许可证 | License

本项目基于 colourfate 的开源项目，采用 MIT 许可证。

---

*最后更新：2026-05-27*
*Last Updated: 2026-05-27*
