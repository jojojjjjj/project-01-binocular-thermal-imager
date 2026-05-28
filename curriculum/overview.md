# 课程总纲 | Course Overview

## 课程信息 | Course Information

| 项目 | 内容 |
|------|------|
| **课程名称** | 双目手机热成像仪 "ThermalEyes" |
| **英文代号** | Binocular Thermal Imager "ThermalEyes" |
| **课程周期** | 15 天（每天 6-8 小时） |
| **适合年级** | 高中一年级+（有基础编程经验） |
| **先修要求** | 基础 C 语言或 Python 知识，了解简单电路概念 |
| **教学方式** | 项目驱动学习（笨鸟先飞原则） |
| **硬件成本** | 约 200-400 元/套 |

## 项目简介 | Project Description

本项目将带领学生从零开始构建一个**双目手机热成像仪**，设备连接手机后可同时显示可见光画面与红外热成像画面，并进行图像融合。项目涉及嵌入式硬件、固件开发、Android 应用开发三大技术领域。

> This project guides students to build a **binocular thermal imager** from scratch. The device connects to a phone and simultaneously displays visible light and infrared thermal images with image fusion. It covers three major technical domains: embedded hardware, firmware development, and Android app development.

### 系统架构 | System Architecture

```
┌─────────────────────────────────────────────────┐
│                 Android 手机 APP                  │
│  ┌──────────┐  ┌──────────┐  ┌───────────────┐  │
│  │ UVC 可见光│  │USB 串口  │  │ OpenCV 图像   │  │
│  │  摄像头   │  │ 热数据   │  │ 融合+伪彩色   │  │
│  └─────┬────┘  └────┬─────┘  └───────────────┘  │
└────────┼────────────┼───────────────────────────┘
         │ USB        │ USB CDC
┌────────┴────────────┴───────────────────────────┐
│              STM32F411 固件 (C语言)               │
│  ┌──────────────┐    ┌──────────────────────┐    │
│  │ I2C 读取     │    │ USB CDC 数据传输     │    │
│  │ MLX90640     │───>│ 32x24 温度帧 → 手机  │    │
│  └──────┬───────┘    └──────────────────────┘    │
└─────────┼───────────────────────────────────────┘
          │ I2C (400kHz)
┌─────────┴───────────────────────────────────────┐
│          MLX90640 红外热成像传感器                 │
│          32x24 像素红外阵列                        │
│          测温范围: -40C ~ 300C                    │
└──────────────────────────────────────────────────┘
```

## 开源资源 | Open Source Resources

| 资源 | 链接 | 说明 |
|------|------|------|
| **PCB 工程** | https://oshwhub.com/colourfate/binocular_thermal_imager | 立创开源硬件平台 |
| **固件代码** | https://github.com/colourfate/thermal_bridge | STM32 固件 |
| **APP 代码** | https://github.com/colourfate/ThermalEyes | Android 应用 |
| **演示视频** | https://www.bilibili.com/video/BV1M64y177ig | B站项目演示 |

## 15天课程安排 | 15-Day Schedule

### 第一阶段：环境搭建与 STM32 基础 | Phase 1: Setup & STM32 Basics (Days 1-3)

| 日期 | 主题 | 核心内容 |
|------|------|----------|
| **Day 1** | 项目介绍 + 环境搭建 | 项目背景、工具链安装（Keil/STM32CubeIDE）、硬件认识 |
| **Day 2** | STM32 基础与 GPIO | STM32 架构理解、GPIO 配置、LED 闪烁实验 |
| **Day 3** | I2C 通信原理与实践 | I2C 协议原理、逻辑分析仪抓包、STM32 I2C 配置 |

### 第二阶段：热成像传感器驱动 | Phase 2: Thermal Sensor Driver (Days 4-7)

| 日期 | 主题 | 核心内容 |
|------|------|----------|
| **Day 4** | MLX90640 传感器原理 | 红外热成像原理、传感器寄存器地图、数据手册阅读 |
| **Day 5** | MLX90640 基础驱动 | I2C 读取传感器数据、EEPROM 配置提取 |
| **Day 6** | 温度数据解析 | 原始数据到温度值的转换算法、补偿参数应用 |
| **Day 7** | 传感器校验与优化 | 数据校验、刷新率调整、噪声滤波初步 |

### 第三阶段：USB 通信与固件集成 | Phase 3: USB Communication & Firmware Integration (Days 8-10)

| 日期 | 主题 | 核心内容 |
|------|------|----------|
| **Day 8** | USB CDC 原理与配置 | USB 协议基础、CDC 类理解、CubeMX USB 配置 |
| **Day 9** | USB 数据传输实现 | 温度帧数据打包、USB 发送、串口调试 |
| **Day 10** | 固件集成与测试 | 完整固件整合：I2C 读取 + USB 发送、联调测试 |

### 第四阶段：Android 应用开发 | Phase 4: Android App Development (Days 11-13)

| 日期 | 主题 | 核心内容 |
|------|------|----------|
| **Day 11** | Android 开发环境 + OpenCV | Android Studio 安装、NDK 配置、OpenCV 集成 |
| **Day 12** | 伪彩色映射与热图像渲染 | 温度数据可视化、伪彩色映射算法、热图像绘制 |
| **Day 13** | 图像融合与 UI 完善 | 可见光+热图像融合、UI 布局、交互功能 |

### 第五阶段：系统集成与展示 | Phase 5: System Integration & Presentation (Days 14-15)

| 日期 | 主题 | 核心内容 |
|------|------|----------|
| **Day 14** | 全系统联调与优化 | 硬件-固件-APP 联调、性能优化、Bug 修复 |
| **Day 15** | 项目展示与总结 | 最终演示准备、PPT 制作、成果展示 |

## 学习路径 | Learning Path

```
Phase 1 (Day 1-3): 基础搭建
    └── 工具安装 → STM32 入门 → I2C 通信

Phase 2 (Day 4-7): 传感器驱动
    └── 传感器原理 → 驱动开发 → 数据解析 → 校准优化

Phase 3 (Day 8-10): USB 通信
    └── USB 原理 → CDC 实现 → 固件集成

Phase 4 (Day 11-13): Android APP
    └── 环境搭建 → 热图像渲染 → 图像融合

Phase 5 (Day 14-15): 系统整合
    └── 联调优化 → 最终展示
```

## 每日课程结构 | Daily Class Structure

每个每日课程文件（day-01.md 到 day-15.md）都包含：

1. **学习目标** (Learning Objectives) - 中英双语
2. **前置准备** (Prerequisites) - 需要完成的准备工作
3. **为什么学这个** (Why This Matters) - 真实世界应用解释
4. **今日任务** (Today's Tasks) - 3-5 个实际操作任务，含步骤、预期结果、常见问题
5. **今日作业** (Homework) - 课后作业
6. **明日预告** (Tomorrow's Preview) - 简要预告
7. **参考资源** (References) - 相关链接

## 评估节点 | Assessment Milestones

| 节点 | 时间 | 内容 |
|------|------|------|
| **Phase 1 Check-in** | Day 3 结束 | 工具链安装完成、LED 闪烁 + I2C 扫描成功 |
| **Phase 2 Check-in** | Day 7 结束 | 能读取并解析 MLX90640 温度数据 |
| **Phase 3 Check-in** | Day 10 结束 | USB CDC 稳定传输温度帧到电脑串口 |
| **Phase 4 Check-in** | Day 13 结束 | APP 能显示热图像并完成图像融合 |
| **Final Presentation** | Day 15 | 完整演示 + PPT 展示 |

## 关键里程碑 | Key Milestones

- **Day 2:** 第一个嵌入式程序运行（LED 闪烁）
- **Day 3:** I2C 总线扫描成功，发现传感器
- **Day 5:** 第一次从传感器读到原始数据
- **Day 6:** 温度数据正确解析，得到合理的温度值
- **Day 9:** USB CDC 成功向电脑发送数据
- **Day 12:** APP 上第一次看到热图像
- **Day 14:** 完整系统端到端工作
- **Day 15:** 最终演示成功

## 硬件清单 | Hardware Bill of Materials

| 组件 | 型号/规格 | 数量 | 参考价格 |
|------|----------|------|----------|
| **MCU** | STM32F411CEU6 (WeAct Black Pill) | 1 | ~15 元 |
| **红外传感器** | MLX90640-BAA (I2C, 32x24) | 1 | ~150 元 |
| **可见光摄像头** | USB UVC 摄像头模块 | 1 | ~20 元 |
| **USB 连接器** | Micro-USB 或 Type-C 线缆 | 2 | ~5 元 |
| **PCB** | 立创EDA 导出的 PCB | 1 | ~10 元 |
| **外壳/支架** | 3D打印或手工制作 | 1 | ~15 元 |
| **其他** | 排线、焊锡、杜邦线等 | 若干 | ~10 元 |
| **总计** | | | **约 200-250 元** |

## 预期成果 | Expected Outcomes

完成课程后，学员将能够：

1. 理解嵌入式系统的工作原理（MCU + 传感器 + 通信）
2. 阅读芯片数据手册并编写驱动代码
3. 使用 I2C、USB 等常见通信协议
4. 开发包含 OpenCV 图像处理的 Android 应用
5. 完成从硬件到软件的完整系统集成
6. 进行技术演示和文档编写

After completing the course, students will be able to:

1. Understand how embedded systems work (MCU + sensor + communication)
2. Read chip datasheets and write driver code
3. Use common communication protocols like I2C, USB
4. Develop Android apps with OpenCV image processing
5. Complete full system integration from hardware to software
6. Conduct technical presentations and documentation

---

*详细每日课程见 day-01.md 到 day-15.md*
*See day-01.md through day-15.md for detailed daily lessons*
