# Day 1: 项目介绍 + 开发环境搭建 | Project Introduction + Environment Setup

## 学习目标 | Learning Objectives

- 理解双目热成像仪项目的整体架构和各模块的作用
- 了解红外热成像的基本原理和实际应用
- 完成 STM32CubeIDE 的安装和配置
- 完成第一个 STM32 工程的创建和编译
- 理解硬件连接方式，完成开发板基本测试

## 前置准备 | Prerequisites

- [ ] 已安装 STM32CubeIDE（如未安装，本课将指导）
- [ ] 已准备好 STM32F411 开发板和 USB 线
- [ ] 已阅读 prerequisites.md 中的前置知识
- [ ] 已准备好电脑（Windows 10/11，至少 10GB 可用空间）

## 为什么学这个？| Why This Matters

热成像技术在现实生活中应用极为广泛：疫情期间的体温筛查、工业设备的故障检测、消防救援中的搜救定位、建筑节能检测等，都依赖红外热成像技术。你将要构建的这个小设备，其核心原理与价值数万元的专业热像仪完全相同。通过这个项目，你将掌握从硬件到软件的完整嵌入式系统开发流程，这是进入物联网、智能制造、机器人等前沿领域的基础技能。

## 今日任务 | Today's Tasks

### Task 1: 了解项目全貌 (estimated 45 minutes)

**目标：** 理解双目热成像仪的系统架构，明确 15 天的学习路线

**步骤：**

1. **观看项目演示视频**
   - 打开 B站视频：https://www.bilibili.com/video/BV1M64y177ig
   - 观察最终产品的功能：热图像显示、可见光显示、图像融合
   - 记录你最感兴趣的 2-3 个功能点

2. **理解系统架构**
   - 画出系统框图：传感器 -> MCU -> USB -> 手机APP
   - 理解数据流向：MLX90640 采集温度 -> STM32 通过 I2C 读取 -> USB CDC 发送 -> APP 接收并可视化
   - 了解三个核心技术领域：
     * **硬件**：PCB 设计、元件焊接、电路连接
     * **固件**：C 语言、STM32 HAL 库、I2C/USB 协议
     * **软件**：Android 开发、OpenCV 图像处理、JNI

3. **浏览开源代码库**
   - 固件仓库：https://github.com/colourfate/thermal_bridge
   - APP 仓库：https://github.com/colourfate/ThermalEyes
   - PCB 工程：https://oshwhub.com/colourfate/binocular_thermal_imager
   - 不需要看懂所有代码，先有个整体印象

**预期结果：**
- 能画出系统的简单框图
- 能用自己的话解释热成像仪的工作流程
- 对 15 天的学习路线有清晰认识

**常见问题：**

**Q: 为什么选择 STM32F411 而不是 Arduino？**
A: STM32F411 有硬件 USB 支持（可以模拟 USB 串口设备），处理速度快（100MHz），是工业级 MCU。Arduino 通常没有硬件 USB，且性能较低。不过 Arduino 更适合入门，如果你完全没有嵌入式经验，也不用担心，课程会从零教起。

**Q: MLX90640 的 32x24 像素够用吗？**
A: 32x24 = 768 个像素点确实分辨率不高，但对于学习和演示完全足够。专业热像仪通常使用更高分辨率的传感器（如 160x120 或 320x240），但价格也贵得多（数千元级别）。我们的方案通过插值算法在 APP 端放大图像，视觉效果不错。

---

### Task 2: 安装 STM32CubeIDE (estimated 60 minutes)

**目标：** 完成开发环境的安装，确保能创建和编译 STM32 工程

**步骤：**

1. **下载 STM32CubeIDE**
   - 访问：https://www.st.com/en/development-tools/stm32cubeide.html
   - 点击 "Get Software" 按钮
   - 需要注册一个 ST 账号（免费）
   - 下载 Windows 安装包（约 800MB）

2. **安装 STM32CubeIDE**
   - 双击安装包运行
   - 保持默认安装路径（建议不要有中文路径）
   - 安装过程中会提示安装驱动，全部允许
   - 安装时间约 10-15 分钟

3. **首次启动与验证**
   - 打开 STM32CubeIDE
   - 选择工作空间（Workspace）路径，建议：`C:\Users\你的用户名\STM32Proj`
   - 等待欢迎页面加载完成
   - 关闭欢迎页面，看到空的 IDE 界面

4. **验证工具链**
   - 点击菜单 Window -> Preferences -> STM32Cube
   - 确认 CubeMX 路径已自动配置
   - 确认 Toolchain 路径已自动检测到

**预期结果：**
- STM32CubeIDE 成功安装并能启动
- 工作空间已创建
- 工具链配置正确

**常见问题：**

**Q: 安装时杀毒软件报警？**
A: STM32CubeIDE 安装的 USB 驱动可能触发杀毒软件报警。添加信任即可。确保从 ST 官网下载的安装包是安全的。

**Q: 启动很慢？**
A: 首次启动需要初始化组件，可能需要 1-2 分钟。后续启动会快一些。

**Q: 提示需要 Java？**
A: STM32CubeIDE 自带 JRE，不需要额外安装 Java。如果报错，尝试重新安装。

---

### Task 3: 创建第一个 STM32 工程 (estimated 60 minutes)

**目标：** 使用 CubeMX 创建一个 STM32F411 的基础工程，编译并烧录

**步骤：**

1. **创建新工程**
   - 点击 File -> New -> STM32 Project
   - 在弹出的芯片选择器中搜索：`STM32F411CEU6`
   - 选择 LQFP48 封装的芯片
   - 点击 Next

2. **配置工程信息**
   - Project Name: `ThermalEyes_Day1`
   - Target Language: C
   - Target Binary Type: Executable
   - 点击 Finish，等待 CubeMX 生成代码

3. **查看工程结构**
   - 在左侧 Project Explorer 中展开工程
   - 查看 `Core/Src/main.c` -- 这是主程序文件
   - 查看 `Core/Inc/main.h` -- 这是头文件
   - 理解 CubeMX 生成的代码结构：
     * `main.c` 中有 `MX_GPIO_Init()` 等初始化函数
     * `SystemClock_Config()` 配置系统时钟
     * `main()` 函数中有无限循环 `while (1)`

4. **编译工程**
   - 右键工程名 -> Build Project（或按 Ctrl+B）
   - 等待编译完成
   - 在 Console 窗口中查看编译输出
   - 确认没有错误（0 errors），可能有警告（warnings）可以先忽略

5. **连接开发板并烧录**
   - 用 USB 线连接 STM32F411 开发板到电脑
   - 如果是 WeAct Black Pill 板，直接通过 USB 连接即可
   - 右键工程 -> Run As -> STM32 C/C++ Application
   - 如果提示配置调试器，选择 "ST-LINK (ST-LINK GDBSERVER)"
   - 等待程序烧录完成
   - **注意：** 此时程序还没有可见的效果，因为还没有配置 GPIO

**预期结果：**
- 工程创建成功
- 编译无错误
- 程序成功烧录到开发板

**常见问题：**

**Q: 找不到 STM32F411CEU6？**
A: 确认你的开发板型号。WeAct Black Pill 通常使用 STM32F411CEU6。如果搜索不到，尝试只搜索 `STM32F411` 然后在结果中找。

**Q: 编译报错 "No rule to make target"？**
A: 尝试右键工程 -> Clean Project，然后重新 Build。

**Q: 烧录失败 "ST-LINK not found"？**
A:
1. 检查 USB 线是否连接好
2. 确认驱动已安装（CubeIDE 安装时会自动安装）
3. 尝试按开发板上的 RESET 按钮
4. 如果使用的是新版 Black Pill，可能需要先按住 BOOT0 再按 RESET 进入烧录模式

---

### Task 4: 认识硬件并完成连线 (estimated 45 minutes)

**目标：** 熟悉开发板和传感器模块，理解引脚定义，完成基本连接

**步骤：**

1. **认识 STM32F411 开发板**
   - 找到以下关键位置：
     * USB 接口（用于供电、烧录、USB CDC）
     * BOOT0 按钮（进入烧录模式）
     * RESET 按钮（复位）
     * LED 指示灯（通常在 PC13 引脚）
     * GPIO 引脚排针（两侧的排针座）

2. **认识 MLX90640 传感器模块**
   - 找到以下关键位置：
     * I2C 接口引脚：VIN, GND, SDA, SCL
     * 红外传感器阵列（中间的金属盖）
   - 注意模块的工作电压：通常 3.3V 或 5V 兼容
   - 记录模块上的标识信息

3. **查看原理图中的连接关系**
   - 浏览 PCB 开源工程：https://oshwhub.com/colourfate/binocular_thermal_imager
   - 记录关键引脚连接：
     * MLX90640 SDA -> STM32 PB7（I2C1_SDA）
     * MLX90640 SCL -> STM32 PB6（I2C1_SCL）
     * MLX90640 VIN -> STM32 3.3V
     * MLX90640 GND -> STM32 GND

4. **连接硬件（暂不接传感器）**
   - 暂时只连接 STM32 开发板到电脑的 USB 线
   - 传感器等 Day 3 I2C 实验时再连接
   - 确认开发板上电后 LED 灯的状态

**预期结果：**
- 理解开发板各部分的名称和功能
- 知道 MLX90640 需要哪些引脚连接
- 开发板通过 USB 连接电脑正常供电

**常见问题：**

**Q: 开发板上有两个 USB 接口？**
A: 有些 Black Pill 板有一个 Micro-USB。另一个可能是 SWD 调试接口（4 针）。USB 接口用于供电、烧录和 USB CDC 通信。

**Q: MLX90640 模块的引脚顺序不确定？**
A: 不同厂商的模块引脚顺序可能不同。查看模块上的丝印标注（通常是 VIN, GND, SDA, SCL），或参考你购买店铺的说明文档。

---

## 今日作业 | Homework

1. **环境验证**（必须）
   - 提交 STM32CubeIDE 的截图（显示成功编译的 Console 输出）
   - 提交创建的工程目录截图

2. **学习笔记**（必须）
   - 创建一个 `notes.md` 文件
   - 画出热成像仪的系统架构图（可以手绘拍照）
   - 写出你对项目的三个疑问（带着问题学习效果更好）

3. **代码阅读**（推荐）
   - 克隆固件仓库到本地：`git clone https://github.com/colourfate/thermal_bridge`
   - 浏览仓库目录结构，找到 I2C 和 USB 相关的文件
   - 不需要看懂代码，了解文件组织方式即可

4. **探索任务**（可选）
   - 查阅 MLX90640 数据手册中的"Features"部分
   - 了解红外热成像的应用场景，写 3 个你觉得最有趣的应用

## 明日预告 | Tomorrow's Preview

明天我们将深入学习 STM32 的基本架构，配置 GPIO 点亮 LED，并理解嵌入式程序的执行流程。你将写出第一个让硬件"动起来"的程序！

## 参考资源 | References

- **STM32CubeIDE 下载**：https://www.st.com/en/development-tools/stm32cubeide.html
- **项目演示视频**：https://www.bilibili.com/video/BV1M64y177ig
- **固件开源仓库**：https://github.com/colourfate/thermal_bridge
- **APP 开源仓库**：https://github.com/colourfate/ThermalEyes
- **PCB 开源工程**：https://oshwhub.com/colourfate/binocular_thermal_imager
- **STM32F411 数据手册**：https://www.st.com/en/microcontrollers/stm32f411.html
- **STM32 入门教程**：https://www.st.com/en/development-tools/stm32cubeide.html#getsoftwareredirect

---

*预计完成时间：4-5 小时*
*Estimated completion time: 4-5 hours*
