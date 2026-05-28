# Day 2: STM32 基础与 GPIO | STM32 Basics & GPIO

## 学习目标 | Learning Objectives

- 理解 STM32 微控制器的基本架构（内核、总线、外设）
- 掌握 STM32CubeMX 配置 GPIO 的方法
- 实现第一个硬件控制程序：LED 闪烁
- 理解嵌入式程序的执行流程（启动文件 -> main -> while循环）
- 学会使用 HAL 库的基本函数

## 前置准备 | Prerequisites

- [ ] STM32CubeIDE 已安装并能正常启动
- [ ] Day 1 的工程已创建并成功编译
- [ ] STM32F411 开发板已通过 USB 连接到电脑
- [ ] 已阅读 Day 1 的项目架构说明

## 为什么学这个？| Why This Matters

GPIO（通用输入输出）是微控制器最基础的外设，也是所有硬件控制的起点。无论是点亮 LED、读取按键、还是控制传感器，都从 GPIO 开始。理解 GPIO 的工作原理，就像学会了一个城市的交通系统 -- 后续学习 I2C、USB 等更复杂的通信协议就有了坚实的基础。今天写的 LED 闪烁程序，在嵌入式开发中的地位，等同于软件开发的 "Hello World"。

## 今日任务 | Today's Tasks

### Task 1: 理解 STM32 架构 (estimated 45 minutes)

**目标：** 建立 STM32 内部结构的认知框架，理解程序是如何运行的

**步骤：**

1. **学习 STM32 基本架构**
   - 打开 STM32F411 数据手册的 Block Diagram（框图）部分
   - 理解以下关键组件：
     * **Cortex-M4 内核**：CPU 大脑，执行指令，100MHz 主频
     * **Flash**：存储程序代码（512KB），掉电不丢失
     * **SRAM**：运行时内存（128KB），掉电丢失
     * **AHB/APB 总线**：连接内核与外设的"高速公路"
     * **GPIO**：通用输入输出端口（PA, PB, PC 等）
     * **I2C**：我们将用来连接 MLX90640 的通信接口
     * **USB OTG**：USB 通信控制器

2. **理解程序执行流程**
   - 上电后，CPU 从 Flash 的起始地址开始执行
   - 启动文件（startup_stm32f411xe.s）设置堆栈、初始化系统
   - 调用 `SystemInit()` 配置时钟
   - 调用 `main()` 函数
   - 在 `main()` 中进入 `while(1)` 无限循环，不断执行任务
   - 画出流程图：上电 -> 启动代码 -> SystemClock_Config() -> 外设初始化 -> while(1) 主循环

3. **理解时钟系统（简化版）**
   - STM32F411 使用外部晶振（HSE，通常 25MHz）
   - 通过 PLL 倍频到 100MHz 系统时钟
   - CubeMX 会自动配置时钟树，暂时不需要深入理解
   - 只需知道：外设（GPIO, I2C, USB）都需要时钟才能工作

**预期结果：**
- 能画出 STM32 的简化框图（内核、存储、总线、外设）
- 能描述程序从上电到运行 main() 的过程
- 理解为什么外设需要时钟

**常见问题：**

**Q: 时钟树配置太复杂了看不懂？**
A: 没关系。CubeMX 会自动计算和配置时钟树。你现在只需要知道 STM32 需要时钟信号来驱动外设即可。随着项目深入，你会逐步理解。

---

### Task 2: 配置 GPIO 点亮 LED (estimated 60 minutes)

**目标：** 使用 CubeMX 配置 GPIO，编写代码让 LED 闪烁

**步骤：**

1. **创建新工程**
   - File -> New -> STM32 Project
   - 选择 STM32F411CEU6
   - 工程名：`ThermalEyes_LED`
   - 语言：C

2. **在 CubeMX 中配置 GPIO**
   - 在芯片引脚图中找到 **PC13**（WeAct Black Pill 的 LED 引脚）
   - 点击 PC13，选择 **GPIO_Output**
   - PC13 会变成绿色，表示已配置为输出
   - 在左侧 GPIO 配置面板中：
     * GPIO output level: High（LED 默认灭）
     * GPIO mode: Output Push Pull（推挽输出）
     * GPIO Pull-up/Pull-down: No pull-up and no pull-down
     * Maximum output speed: Low
     * User Label: `LED`（给引脚取个好记的名字）

3. **配置系统时钟**
   - 点击左侧 "Clock Configuration" 标签
   - CubeMX 通常会自动配置，确认 HCLK = 100MHz
   - 如果不是 100MHz，按照以下步骤手动配置：
     * Input frequency (HSE): 25MHz
     * PLL Source Mux: HSE
     * PLLM: 25, PLLN: 200, PLLP: 4, PLLQ: 8
     * System Clock Mux: PLLCLK
     * 确认 HCLK = 100MHz

4. **生成代码**
   - File -> Save（或 Ctrl+S）
   - CubeMX 会提示是否生成代码，点击 Yes
   - 等待代码生成完成

5. **编写 LED 闪烁代码**
   - 打开 `Core/Src/main.c`
   - 在 `while (1)` 循环中添加代码：

   ```c
   /* USER CODE BEGIN 3 */
   HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);  // 翻转 LED 状态
   HAL_Delay(500);  // 延时 500ms
   /* USER CODE END 3 */
   ```

   **重要提示：** 所有你自己写的代码必须写在 `/* USER CODE BEGIN xxx */` 和 `/* USER CODE END xxx */` 之间。否则下次 CubeMX 重新生成代码时会被覆盖！

6. **编译并烧录**
   - 按 Ctrl+B 编译
   - 右键工程 -> Run As -> STM32 Application 烧录
   - 观察开发板上的 LED 是否以 1 秒为周期闪烁（500ms 亮 + 500ms 灭）

**预期结果：**
- LED 以 1 秒周期稳定闪烁
- 理解 GPIO 输出模式的配置过程
- 理解 HAL_GPIO_TogglePin() 和 HAL_Delay() 的用法

**常见问题：**

**Q: LED 不闪？**
A: 逐步排查：
1. 确认编译无错误
2. 确认烧录成功（Console 中显示成功信息）
3. 确认 PC13 是你的开发板 LED 引脚（不同板子可能不同）
4. 有些板子 LED 是低电平点亮，试试改为 `HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET)`

**Q: HAL_Delay 的精度如何？**
A: HAL_Delay 基于 SysTick 定时器，精度约 1ms。对于 LED 闪烁足够精确，但不适合精确计时。

**Q: 为什么代码要写在 USER CODE 区域？**
A: CubeMX 生成的代码区域会在下次重新生成时被覆盖。USER CODE 区域的内容会被保留。这是 CubeMX 的代码保护机制。

---

### Task 3: 按键输入与程序控制 (estimated 45 minutes)

**目标：** 配置 GPIO 输入模式，读取按键状态，用按键控制 LED

**步骤：**

1. **配置按键 GPIO**
   - 在 CubeMX 引脚图中找到按键引脚
   - WeAct Black Pill 通常没有板载按键，我们使用跳线模拟
   - 找到 **PA0** 引脚，配置为 **GPIO_Input**
   - 配置为 Pull-up（上拉），默认高电平
   - User Label: `BUTTON`

2. **重新生成代码**
   - Ctrl+S 保存并重新生成代码
   - 注意 USER CODE 区域的代码会被保留

3. **编写按键检测代码**
   - 在 `while(1)` 中修改代码：

   ```c
   /* USER CODE BEGIN 3 */
   // 读取按键状态（低电平表示按下）
   if (HAL_GPIO_ReadPin(BUTTON_GPIO_Port, BUTTON_Pin) == GPIO_PIN_RESET) {
       HAL_Delay(20);  // 简单消抖
       if (HAL_GPIO_ReadPin(BUTTON_GPIO_Port, BUTTON_Pin) == GPIO_PIN_RESET) {
           HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
           // 等待按键释放
           while (HAL_GPIO_ReadPin(BUTTON_GPIO_Port, BUTTON_Pin) == GPIO_PIN_RESET);
       }
   }
   /* USER CODE END 3 */
   ```

4. **硬件连接模拟按键**
   - 用杜邦线连接 PA0 到 GND 模拟按键按下
   - 断开杜邦线模拟按键释放
   - 观察 LED 是否在"按下"时切换状态

**预期结果：**
- 能通过杜邦线模拟按键控制 LED 开关
- 理解 GPIO 输入模式和上拉电阻的作用
- 理解按键消抖的概念

**常见问题：**

**Q: 为什么需要消抖？**
A: 机械按键在按下和释放时，触点会发生弹跳（bounce），导致在几毫秒内产生多次通断。消抖就是滤除这些抖动信号，确保只检测到一次按键动作。

---

### Task 4: 理解 HAL 库 (estimated 30 minutes)

**目标：** 掌握 HAL 库的基本使用方法，能查阅 HAL 库文档

**步骤：**

1. **了解 HAL 库的命名规则**
   - 函数命名：`HAL_外设_动作()`
     * `HAL_GPIO_TogglePin()` -- GPIO 翻转引脚
     * `HAL_GPIO_ReadPin()` -- GPIO 读取引脚
     * `HAL_GPIO_WritePin()` -- GPIO 写入引脚
     * `HAL_Delay()` -- 延时函数
   - 参数命名：`外设_参数`
     * `LED_GPIO_Port` -- LED 所在的 GPIO 端口（GPIOC）
     * `LED_Pin` -- LED 所在的引脚号（GPIO_PIN_13）

2. **查看 HAL 库源码**
   - 在工程中展开 Drivers/STM32F4xx_HAL_Driver
   - 找到 `stm32f4xx_hal_gpio.c` 和 `.h` 文件
   - 阅读 `HAL_GPIO_TogglePin` 函数的实现
   - 注意函数上方的注释说明，了解参数含义

3. **记录常用 HAL 函数**
   - 创建一个个人笔记文件，记录以下函数：
     * `HAL_GPIO_WritePin(port, pin, state)` -- 设置引脚电平
     * `HAL_GPIO_ReadPin(port, pin)` -- 读取引脚电平，返回 GPIO_PIN_SET 或 GPIO_PIN_RESET
     * `HAL_Delay(ms)` -- 毫秒级延时
   - 理解这些函数的参数类型和返回值

**预期结果：**
- 理解 HAL 库的命名规律，能根据名称猜测函数用途
- 能查阅 HAL 库文档找到需要的函数
- 掌握 GPIO 相关的常用函数

**常见问题：**

**Q: HAL 库和标准库（Standard Peripheral Library）有什么区别？**
A: HAL（Hardware Abstraction Layer）是 ST 推荐的新一代库，抽象层级更高，配合 CubeMX 使用更方便。标准库已不再更新。本课程统一使用 HAL 库。

---

## 今日作业 | Homework

1. **LED 闪烁实验**（必须）
   - 成功实现 LED 闪烁
   - 修改闪烁频率（尝试 100ms, 200ms, 1000ms），观察效果
   - 提交代码截图和运行效果照片

2. **按键实验**（必须）
   - 成功实现按键控制 LED
   - 提交代码和说明

3. **理解检查**（必须）
   - 用自己的话解释以下概念（写在笔记中）：
     * 什么是 GPIO？推挽输出是什么意思？
     * 为什么嵌入式程序需要一个 while(1) 循环？
     * CubeMX 生成的代码为什么要用 USER CODE 区域？

4. **挑战任务**（可选）
   - 实现一个 LED 呼吸灯效果（提示：使用 PWM 或者简单的亮度梯度延时）
   - 或者实现按不同次数切换不同闪烁模式

## 明日预告 | Tomorrow's Preview

明天我们将学习 I2C 通信协议！这是连接 STM32 和 MLX90640 热传感器的关键协议。你将理解 I2C 是如何通过两根线传输数据的，并编写代码扫描 I2C 总线上的设备。

## 参考资源 | References

- **STM32F411 数据手册**：https://www.st.com/en/microcontrollers/stm32f411.html
- **STM32 HAL 库文档**：在 CubeIDE 中按 F1 查看
- **STM32CubeIDE 用户指南**：Help -> Help Contents
- **GPIO 教程**：https://deepbluembedded.com/stm32-gpio-tutorial/
- **WeAct Black Pinch 原理图**：https://github.com/WeActStudio/WeActStudio.MiniF4

---

*预计完成时间：4-5 小时*
*Estimated completion time: 4-5 hours*
