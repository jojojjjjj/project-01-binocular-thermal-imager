# 最终展示要求 | Final Presentation Requirements

## 概述 | Overview

最终展示是 ThermalEyes 双目手机热成像仪项目的重要环节。学员将向老师和同学展示完整的端到端系统，包括硬件组装、STM32 固件、Android 应用以及实时热成像演示。

> The final presentation is a key component of the ThermalEyes binocular mobile thermal imager project. Students will present the complete end-to-end system to teachers and classmates, including hardware assembly, STM32 firmware, Android application, and live thermal imaging demonstration.

---

## 展示时间 | Presentation Time

**Day 15 | Day 15**

- **每人展示时间：** 10-15 分钟
- **演示 + 讲解 + 问答**
- **Demonstration + Presentation + Q&A**

---

## 一、展示格式 | Presentation Format

### 时间分配 | Time Allocation

| 环节 | 时长 | 内容 |
|------|------|------|
| 项目概述 | 2 分钟 | 项目背景、功能介绍、技术栈 |
| 技术架构讲解 | 3 分钟 | 系统架构图、核心模块、数据流 |
| 实时演示 | 5 分钟 | 端到端热成像系统实时展示 |
| 挑战与解决方案 | 2 分钟 | 遇到的技术难点和解决方案 |
| 经验总结 | 1-2 分钟 | 学习收获和未来改进方向 |
| 问答环节 | 2 分钟 | 回答老师和同学的提问 |

---

## 二、展示要求章节 | Required Presentation Sections

### 1. 项目概述 | Project Overview（2 分钟）

**内容要求 | Required Content:**

- 项目名称：ThermalEyes - 双目手机热成像仪
- 项目目标：使用 STM32 + MLX90640 + Android 构建便携式双目热成像系统
- 核心功能：
  - 实时红外热成像
  - 双目视角覆盖更大范围
  - 手机端伪彩色渲染和温度测量
- 技术栈概览：
  - 硬件：STM32F4/F1, MLX90640 x2, USB 数据线
  - 固件：C (HAL 库), I2C, USB CDC
  - 应用：Android (Java/Kotlin), OpenCV
- 个人贡献说明

**PPT 页数：** 2-3 页

---

### 2. 技术架构 | Technical Architecture（3 分钟）

**必须包含 | Must Include:**

- **系统架构图**：展示 STM32 -> USB -> Android 的数据流
- **硬件连接图**：展示 STM32 与两个 MLX90640 的 I2C 连接
- **软件模块图**：展示固件端各模块（I2C 驱动、温度计算、USB 通信）和 Android 端各模块（USB 接收、数据解析、OpenCV 渲染、UI）
- **数据流图**：从传感器原始数据 -> 温度计算 -> USB 传输 -> 协议解析 -> 伪彩色映射 -> 屏幕显示的完整数据流
- **通信协议说明**：帧格式定义（帧头、传感器 ID、数据、校验）

**讲解重点 | Key Points to Explain:**

1. I2C 总线上双传感器的地址管理
2. 温度数据的计算和补偿过程
3. USB CDC 数据传输协议设计
4. OpenCV 伪彩色映射的实现方法
5. 双目数据的时间同步策略

**PPT 页数：** 3-4 页

---

### 3. 挑战与解决方案 | Challenges & Solutions（2 分钟）

**要求列举 2-3 个实际遇到的技术挑战，以及解决过程。**

**常见挑战示例 | Example Challenges:**

| 挑战 | 可能的解决方案 |
|------|---------------|
| I2C 通信不稳定，偶尔读取失败 | 添加上拉电阻、降低时钟频率、增加重试机制 |
| MLX90640 温度计算不准确 | 仔细核对数据手册公式、正确读取 EEPROM 校准参数 |
| USB CDC 在 Android 上无法识别 | 检查 VID/PID 配置、使用正确的 USB Host API |
| 热成像画面卡顿 | 优化数据解析流程、使用双缓冲、降低渲染分辨率 |
| 双传感器数据不同步 | 独立管理每个传感器的读取时机、添加时间戳 |

**PPT 页数：** 1-2 页

---

### 4. 实时演示 | Live Demonstration（5 分钟）

**演示要求详见下方"演示要求"章节。**

**See "Demo Requirements" section below for details.**

---

### 5. 经验总结 | Lessons Learned（1-2 分钟）

**内容要求 | Required Content:**

- 项目中最大的技术收获
- 对嵌入式系统开发的新理解
- 对移动应用开发的体验
- 对红外热成像技术的认识
- 如果重新做这个项目，会有什么不同的做法
- 可能的改进方向（更高分辨率传感器、WiFi 传输、温度报警等）

**PPT 页数：** 1-2 页

---

## 三、演示要求 | Demo Requirements

### 实时热成像演示 | Live Thermal Imaging Demonstration

**必须展示的功能 | Required Demonstrations:**

1. **系统启动和连接**（1 分钟）
   - 展示硬件组装：STM32 + 两个 MLX90640 传感器
   - 连接 USB 数据线到 Android 手机
   - 启动 Android 应用，展示自动连接过程

2. **实时热成像画面**（2 分钟）
   - 展示实时伪彩色热成像画面
   - 用手掌在传感器前移动，展示实时响应
   - 展示双目并排画面

3. **温度测量功能**（1 分钟）
   - 展示触摸测温：点击画面上的点显示温度值
   - 展示最高/最低温度标注
   - 如有温度报警功能，进行演示

4. **不同物体演示**（1 分钟）
   - 准备热水杯（约 50-60 度）展示高温区域
   - 准备冰块或冷水展示低温区域
   - 展示人体手部的热成像效果

### 演示注意事项 | Demo Notes

- **提前 30 分钟**到达演示场地进行设备调试
- **准备备用方案**：如果 USB 连接失败，使用预先录制的演示视频
- **准备多个测试物体**：热水杯、冰块、室温物体
- **确保手机电量充足**：连接 USB 同时充电
- **控制演示节奏**：不要在一个功能上花太多时间

---

## 四、问答准备指南 | Q&A Preparation Guide

### 技术问题 | Technical Questions

**Q1: 为什么选择 MLX90640 而不是其他热成像传感器？**
> Why choose MLX90640 over other thermal imaging sensors?

参考回答要点：
- 成本低（约 200-300 元），适合学生项目
- 32x24 分辨率足以展示热成像原理
- I2C 接口简单，易于与 STM32 通信
- 有丰富的开源社区支持和文档

**Q2: 系统的最大测温范围和精度是多少？**
> What is the maximum temperature measurement range and accuracy of the system?

参考回答要点：
- MLX90640 测温范围：-40 度 C 到 300 度 C（物体温度）
- 精度：约 +/- 1.5 度 C（在正常环境温度范围内）
- 分辨率：0.1 度 C

**Q3: 为什么使用 USB 而不是蓝牙或 WiFi 传输数据？**
> Why use USB instead of Bluetooth or WiFi for data transmission?

参考回答要点：
- USB 传输速率高（Full Speed 12 Mbps），能满足双目实时数据传输需求
- USB 连接同时可以给 STM32 供电
- USB CDC 协议简单，开发门槛低
- 延迟低，适合实时应用

**Q4: 双目热成像有什么实际应用场景？**
> What are the practical application scenarios for binocular thermal imaging?

参考回答要点：
- 建筑物能耗检测：检测墙体隔热问题
- 电气设备巡检：发现过热的电气连接
- 人体体温筛查：大范围快速检测
- 消防搜救：在烟雾中定位人员
- 工业质量控制：检测产品温度分布

**Q5: 如何提高热成像的画面质量？**
> How can thermal imaging image quality be improved?

参考回答要点：
- 使用更高分辨率的传感器（如 MLX90655 64x48 或 FLIR Lepton 80x60）
- 优化插值算法（从双线性改为双三次）
- 添加降噪处理（时间域平均）
- 优化伪彩色方案选择
- 改善传感器的光学设计（使用红外透镜）

### 项目问题 | Project Questions

**Q6: 项目中遇到的最大技术挑战是什么？你是如何解决的？**

准备思路：回顾整个开发过程，选择一个印象最深的 bug 或技术难点，说明问题现象、排查过程和最终解决方案。

**Q7: 如果给你更多时间，你会如何改进这个项目？**

准备思路：可以从硬件（更好传感器、3D 打印外壳）、固件（FreeRTOS、OTA 升级）、应用（WiFi 传输、数据记录、云端分析）等角度回答。

**Q8: 这个项目让你对嵌入式系统开发有什么新的认识？**

准备思路：从软硬件协同、实时性要求、调试方法、工程思维等角度回答。

---

## 五、PPT 要求 | PPT Requirements

### 整体要求 | Overall Requirements

- **总页数：** 8-15 页
- **风格：** 简洁、技术导向
- **语言：** 中英双语
- **字体大小：** 标题 >= 28pt，正文 >= 18pt

### 必含页面 | Required Pages

1. **封面页**（1 页）
   - 项目名称：ThermalEyes - 双目手机热成像仪
   - 学员姓名、日期

2. **项目概述**（2-3 页）
   - 项目简介和目标
   - 功能列表
   - 技术栈

3. **系统架构**（2-3 页）
   - 硬件架构图（STM32 + MLX90640 + USB + Android）
   - 软件模块图
   - 数据流图
   - 通信协议说明

4. **核心实现**（2-3 页）
   - I2C 驱动和温度计算
   - USB CDC 数据传输
   - OpenCV 热成像渲染

5. **演示效果**（1-2 页）
   - 热成像效果截图
   - 不同场景的对比图

6. **挑战与解决**（1-2 页）
   - 技术难点
   - 解决方案
   - 调试过程

7. **总结展望**（1 页）
   - 项目收获
   - 改进方向

---

## 六、评分标准 | Grading Criteria for Presentation

| 评分项 | 分值 | 评分标准 |
|--------|------|----------|
| **演示效果** | 30 分 | 热成像系统实时运行，画面流畅，功能完整 |
| **技术讲解** | 25 分 | 架构清晰，原理正确，深度适当 |
| **PPT 质量** | 20 分 | 内容完整，视觉清晰，有图表支撑 |
| **问答表现** | 15 分 | 回答准确，思路清晰，能深入讨论 |
| **时间控制** | 10 分 | 在规定时间内完成，节奏合理 |
| **总分** | **100 分** | |

### 评分细则 | Detailed Scoring

**演示效果 (30 分) | Demo Effect (30 pts):**
- 25-30 分：系统流畅运行，所有功能正常，演示引人入胜
- 20-24 分：系统基本运行，主要功能正常，偶有小问题
- 15-19 分：系统部分运行，有关键功能缺失
- 0-14 分：系统无法运行或演示失败

**技术讲解 (25 分) | Technical Explanation (25 pts):**
- 21-25 分：讲解深入，能解释底层原理，逻辑清晰
- 16-20 分：讲解清楚，基本原理正确
- 11-15 分：讲解表面，部分概念模糊
- 0-10 分：讲解混乱或概念错误

**PPT 质量 (20 分) | PPT Quality (20 pts):**
- 17-20 分：美观专业，有系统架构图和数据流图
- 13-16 分：内容完整，基本清晰
- 9-12 分：内容有缺失，排版一般
- 0-8 分：内容不完整或质量差

---

## 七、演示准备时间线 | Demo Preparation Timeline

### Day 14 下午 | Day 14 Afternoon

- [ ] 完成系统集成最终调试
- [ ] 录制备用演示视频
- [ ] 完成 PPT 初稿

### Day 15 上午 | Day 15 Morning

- [ ] PPT 最终修改
- [ ] 演示排练第一遍（计时）
- [ ] 根据排练调整内容
- [ ] 演示排练第二遍
- [ ] 准备演示道具（热水杯、冰块等）

### Day 15 下午 | Day 15 Afternoon

- [ ] 提前 30 分钟到场调试设备
- [ ] 测试 USB 连接和应用启动
- [ ] 正式展示

---

*祝展示成功！Good luck with your presentation!*
*最后更新：2026-05-27 | Last updated: 2026-05-27*
