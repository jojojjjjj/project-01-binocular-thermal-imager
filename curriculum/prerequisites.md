# 前置知识清单 | Prerequisites Checklist

## 概述 | Overview

本课程假设学员具备基础编程能力。以下是开始课程前需要掌握的知识点和相应的学习资源。不需要嵌入式系统经验，课程会从零开始教授。

> This course assumes students have basic programming ability. Below are the knowledge points and learning resources needed before starting the course. No embedded systems experience is required; the course teaches from scratch.

---

## 1. C 语言基础 | C Programming Fundamentals

### 需要掌握的概念 | Concepts to Master

- [ ] **基本语法** (Basic Syntax)
  - 变量声明与数据类型（int, float, char, uint8_t）
  - 算术运算符（+, -, *, /, %）
  - 比较运算符（==, !=, <, >）
  - 位运算符（&, |, ^, ~, <<, >>）-- 嵌入式开发中极其重要
- [ ] **控制结构** (Control Structures)
  - if/else 条件语句
  - for 循环、while 循环
  - switch/case 语句
- [ ] **函数** (Functions)
  - 函数定义、参数传递、返回值
  - 函数声明（头文件）
- [ ] **数组与指针** (Arrays & Pointers)
  - 一维数组的声明与使用
  - 指针的基本概念（& 取地址，* 解引用）
  - 数组与指针的关系
- [ ] **结构体** (Structs)
  - struct 定义和使用
  - 通过结构体组织相关数据

### 学习资源 | Learning Resources

- **C 语言入门教程**（中文）：https://www.runoob.com/cprogramming/c-tutorial.html
- **翁恺 C 语言**（浙江大学，中国大学 MOOC）：https://www.icourse163.org/course/ZJU-9001
- **C 语言位运算详解**：https://www.runoob.com/cprogramming/c-bit-fields.html

### 自检练习 | Self-Check Exercises

```c
// 1. 定义一个结构体存储传感器数据
typedef struct {
    float temperature;
    uint8_t sensor_id;
    uint8_t valid;
} SensorData;

// 2. 位运算练习：提取一个字节中的低4位
uint8_t value = 0xAB;
uint8_t low_nibble = value & 0x0F;  // 结果: 0x0B

// 3. 将两个字节合并为一个16位整数
uint8_t high_byte = 0x12;
uint8_t low_byte = 0x34;
uint16_t combined = (high_byte << 8) | low_byte;  // 结果: 0x1234
```

---

## 2. 基础电路概念 | Basic Electronics Concepts

### 需要理解的概念 | Concepts to Understand

- [ ] **电压、电流、电阻** (Voltage, Current, Resistance)
  - 欧姆定律：V = I x R
  - 常见电压：3.3V、5V、USB 5V
- [ ] **数字电路基础** (Digital Logic)
  - 高电平（1）与低电平（0）
  - 上拉电阻与下拉电阻的概念
- [ ] **通信协议概念** (Communication Protocols)
  - 串行通信 vs 并行通信
  - 主从设备（Master/Slave）的概念
- [ ] **基本焊接** (Basic Soldering)
  - 烙铁的基本使用
  - 焊锡丝的使用
  - 不需要精通，但需要能完成简单焊接

### 学习资源 | Learning Resources

- **电子学基础**（中文视频）：B站搜索"电子学基础入门"
- **Arduino 入门教程**（帮助理解硬件概念）：https://www.arduino.cc/en/Tutorial/BuiltInExamples
- **焊接入门**：B站搜索"焊接教程入门"

---

## 3. 计算机基础概念 | Basic Computer Science Concepts

### 需要理解的概念 | Concepts to Understand

- [ ] **二进制与十六进制** (Binary & Hexadecimal)
  - 二进制表示：0b1010
  - 十六进制表示：0x0A
  - 不同进制之间的转换
- [ ] **字节序** (Byte Order / Endianness)
  - 大端序（Big-Endian）
  - 小端序（Little-Endian）
  - STM32 使用小端序
- [ ] **内存概念** (Memory)
  - RAM（运行时内存）vs Flash（程序存储）
  - 寄存器（Register）的概念

### 学习资源 | Learning Resources

- **进制转换教程**：https://www.runoob.com/number/number-converter.html
- **计算机组成原理入门**：B站搜索"计算机组成原理"

---

## 4. 开发工具准备 | Development Tools

### 需要安装的软件 | Software to Install

- [ ] **STM32CubeIDE**（STM32 官方 IDE，免费）
  - 下载地址：https://www.st.com/en/development-tools/stm32cubeide.html
  - 包含代码编辑器、编译器、调试器、CubeMX 配置工具
  - 安装教程详见 Day 1

- [ ] **串口调试工具**
  - Windows: PuTTY 或 Tera Term
  - 也可使用 Arduino IDE 的串口监视器

- [ ] **Android Studio**（Day 11 开始使用）
  - 下载地址：https://developer.android.com/studio
  - 提前下载即可，不需要提前学习

- [ ] **Git**（版本控制）
  - 下载地址：https://git-scm.com/
  - 基本命令：clone, add, commit, push

### 需要准备的硬件 | Hardware to Prepare

- [ ] **STM32F411 开发板**（WeAct Black Pill 或类似）
  - 确保板子有 Micro-USB 或 Type-C 接口
  - 确保有 BOOT0 按钮（用于进入烧录模式）
- [ ] **MLX90640 红外传感器模块**
  - 确认是 BAA 版本（I2C 接口）
  - 有些模块已经焊接了排针
- [ ] **USB 数据线**（2条）
  - 一条连接 STM32 到电脑（用于烧录和 USB CDC）
  - 一条连接可见光摄像头
- [ ] **杜邦线**（母对母，至少 4 根）
  - 用于连接 STM32 和 MLX90640
- [ ] **电脑**（Windows 10/11）
  - 至少 10GB 可用磁盘空间
  - USB 接口可用

---

## 准备工作检查清单 | Preparation Checklist

开始课程前，请确保：

- [ ] 已安装 STM32CubeIDE
- [ ] 已安装串口调试工具
- [ ] 已安装 Git
- [ ] 已准备好 STM32F411 开发板
- [ ] 已准备好 MLX90640 传感器模块
- [ ] 已准备好 USB 线和杜邦线
- [ ] 已了解 C 语言基础（变量、函数、数组、指针）
- [ ] 已了解基本电路概念（电压、电流、电阻）
- [ ] 已注册 GitHub 账号

---

## 常见问题 | FAQ

**Q: 如果我 C 语言基础不够怎么办？**
A: 建议先花 2-3 天学习 C 语言基础（变量、函数、数组、指针），推荐翁恺老师的 C 语言课程。嵌入式用到的 C 语言概念比较集中，不需要学完全部。

**Q: 我没有电路基础，可以学习吗？**
A: 可以。本课程使用的硬件模块都是现成的，只需要用杜邦线连接，不需要自己设计电路。Day 1 会详细讲解硬件连接。

**Q: 需要提前学习 Android 开发吗？**
A: 不需要。Day 11-13 会从零开始教学 Android 开发所需的知识。但如果你有 Java 或 Python 基础会更容易上手。

**Q: STM32CubeIDE 和 Keil 哪个更好？**
A: STM32CubeIDE 是免费的，官方推荐，跨平台。Keil 是商业软件，Windows 专用的。本课程使用 STM32CubeIDE。

**Q: 我需要自己制作 PCB 吗？**
A: 不需要。课程前期使用开发板+杜邦线连接。如果你感兴趣，可以参考开源 PCB 工程自己打板。

---

*如有疑问，请在课程开始前咨询助教。*
*If you have questions, please consult the teaching assistant before the course starts.*
