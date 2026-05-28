# 评分细则 | Detailed Grading Rubric

## 总体说明 | Overall Description

本文档提供 ThermalEyes 双目手机热成像仪项目的详细评分标准，用于评估学员在 15 天项目中的综合表现。总分 100 分，另有最高 10 分的加分项。

> This document provides detailed grading criteria for the ThermalEyes binocular mobile thermal imager project, evaluating student performance across the 15-day project. Total score is 100 points with up to 10 bonus points.

---

## 评分维度总览 | Grading Dimensions Overview

| 维度 | 权重 | 分值 | 说明 |
|------|------|------|------|
| 技术实现 | 40% | 40 分 | 固件质量、应用质量、硬件组装、系统集成 |
| 文档质量 | 20% | 20 分 | README、代码注释、接线文档 |
| 演示展示 | 20% | 20 分 | 演示效果、讲解深度、视觉辅助 |
| 进度汇报 | 10% | 10 分 | 每周检查点质量、问题跟踪 |
| 团队协作 | 10% | 10 分 | Git 提交记录、任务分配 |

---

## 一、技术实现 (40 分) | Technical Implementation (40 pts)

### 1.1 固件质量 (15 分) | Firmware Quality (15 pts)

#### 1.1.1 I2C 驱动实现 (5 分)

| 分数 | 标准 | Criteria |
|------|------|----------|
| 5 分 | I2C 通信稳定可靠；有完善的错误处理和重试机制；支持 DMA 传输；能正确扫描和识别总线设备 | I2C communication is stable and reliable; comprehensive error handling and retry mechanisms; DMA transfer support; correct bus scanning and device identification |
| 4 分 | I2C 通信基本稳定；有错误处理但不够完善 | I2C communication mostly stable; has error handling but could be improved |
| 3 分 | I2C 通信可用但偶有失败；基本错误处理 | I2C works but occasional failures; basic error handling |
| 2 分 | I2C 通信不稳定；缺乏错误处理 | I2C communication unstable; lacks error handling |
| 0-1 分 | I2C 通信无法正常工作 | I2C communication does not work properly |

#### 1.1.2 MLX90640 驱动实现 (5 分)

| 分数 | 标准 | Criteria |
|------|------|----------|
| 5 分 | 完整的帧数据读取（分页机制正确）；温度计算准确（误差 < 1 度 C）；正确读取和运用 EEPROM 校准参数；支持双传感器 | Complete frame data reading (correct page mechanism); accurate temperature calculation (error < 1 degree C); correct EEPROM calibration parameter usage; dual sensor support |
| 4 分 | 帧数据读取完整；温度计算较准确（误差 < 2 度 C）；有校准参数运用 | Complete frame reading; fairly accurate temperature (error < 2 degrees C); uses calibration parameters |
| 3 分 | 能读取帧数据；温度计算基本正确（误差 < 5 度 C） | Can read frame data; temperature calculation roughly correct (error < 5 degrees C) |
| 2 分 | 帧数据读取不完整；温度计算有较大误差 | Incomplete frame reading; significant temperature calculation errors |
| 0-1 分 | 无法正确读取 MLX90640 数据 | Cannot correctly read MLX90640 data |

#### 1.1.3 USB CDC 通信与协议 (5 分)

| 分数 | 标准 | Criteria |
|------|------|----------|
| 5 分 | USB CDC 稳定枚举；数据传输可靠无丢包；有完善的帧协议（帧头+序号+校验）；支持双传感器数据区分；传输速率满足实时需求 | USB CDC enumerates reliably; data transfer without packet loss; complete frame protocol (header+sequence+checksum); dual sensor data distinction; transfer rate meets real-time requirements |
| 4 分 | USB CDC 正常工作；数据传输基本可靠；有帧协议 | USB CDC works; data transfer mostly reliable; has frame protocol |
| 3 分 | USB CDC 能通信；偶尔丢包；协议简单 | USB CDC communicates; occasional packet loss; simple protocol |
| 2 分 | USB CDC 通信不稳定；数据经常丢失 | USB CDC communication unstable; frequent data loss |
| 0-1 分 | USB CDC 无法正常工作 | USB CDC does not work |

---

### 1.2 Android 应用质量 (15 分) | Android App Quality (15 pts)

#### 1.2.1 USB 数据接收与解析 (5 分)

| 分数 | 标准 | Criteria |
|------|------|----------|
| 5 分 | USB Host API 使用正确；后台线程接收数据；协议解析健壮；处理了连接断开和重连；双传感器数据正确区分 | Correct USB Host API usage; background thread data reception; robust protocol parsing; handles disconnect/reconnect; correct dual sensor data distinction |
| 4 分 | USB 接收正常工作；基本协议解析；处理了主要异常 | USB reception works; basic protocol parsing; handles main exceptions |
| 3 分 | 能接收 USB 数据；解析功能基本可用 | Can receive USB data; parsing basically functional |
| 2 分 | USB 接收有问题；数据解析不完整 | USB reception has issues; incomplete data parsing |
| 0-1 分 | 无法接收 USB 数据 | Cannot receive USB data |

#### 1.2.2 OpenCV 热成像渲染 (5 分)

| 分数 | 标准 | Criteria |
|------|------|----------|
| 5 分 | 伪彩色映射效果优秀；支持多种色标方案；图像插值质量好；有温度标尺；渲染帧率 >= 8 FPS；双目画面正确显示 | Excellent pseudo-color mapping; supports multiple color schemes; good image interpolation; has temperature scale bar; rendering >= 8 FPS; correct dual-view display |
| 4 分 | 伪彩色渲染正确；至少一种色标；画面基本流畅 | Correct pseudo-color rendering; at least one color scheme; display mostly smooth |
| 3 分 | 能显示热成像画面；色标映射基本正确 | Can display thermal image; color mapping roughly correct |
| 2 分 | 热成像显示有问题；色标映射不准确 | Thermal display has issues; color mapping inaccurate |
| 0-1 分 | 无法显示热成像画面 | Cannot display thermal image |

#### 1.2.3 UI/UX 与功能完整度 (5 分)

| 分数 | 标准 | Criteria |
|------|------|----------|
| 5 分 | 界面美观专业；触摸测温功能完善；最高/最低温度标注清晰；有温度报警功能；应用稳定无崩溃 | Professional and attractive UI; touch temperature measurement works well; max/min temperature labels are clear; temperature alarm feature; app is stable with no crashes |
| 4 分 | 界面清晰；触摸测温可用；有温度标注 | Clear UI; touch measurement works; has temperature labels |
| 3 分 | 界面基本可用；有主要功能 | UI is functional; has main features |
| 2 分 | 界面简陋；功能不完整 | Basic UI; incomplete features |
| 0-1 分 | 应用无法运行或无功能 | App cannot run or has no features |

---

### 1.3 硬件组装 (5 分) | Hardware Assembly (5 pts)

| 分数 | 标准 | Criteria |
|------|------|----------|
| 5 分 | 两个 MLX90640 正确连接；I2C 上拉电阻正确；USB 接口可靠；线缆整洁有管理；如有外壳则结构稳固美观 | Both MLX90640 correctly connected; I2C pull-up resistors correct; USB interface reliable; cables well managed; if enclosure present, structure is sturdy and attractive |
| 4 分 | 硬件连接正确；线缆基本整齐 | Hardware connections correct; cables reasonably tidy |
| 3 分 | 硬件连接基本可用；线缆稍乱 | Hardware basically functional; cables somewhat messy |
| 2 分 | 硬件连接有问题；线缆混乱 | Hardware connection issues; messy cables |
| 0-1 分 | 硬件无法正常工作 | Hardware does not work properly |

---

### 1.4 系统集成 (5 分) | System Integration (5 pts)

| 分数 | 标准 | Criteria |
|------|------|----------|
| 5 分 | STM32 + USB + Android 端到端稳定运行超过 10 分钟；延迟 < 300ms；双目画面同步良好；有错误恢复机制 | STM32 + USB + Android runs stably for over 10 minutes end-to-end; latency < 300ms; dual-view well synchronized; has error recovery mechanism |
| 4 分 | 系统稳定运行 5 分钟以上；延迟可接受；基本功能正常 | System runs stably for 5+ minutes; acceptable latency; basic features work |
| 3 分 | 系统能运行演示；偶有不稳定 | System can run demo; occasionally unstable |
| 2 分 | 系统部分功能可用；集成不完整 | System partially functional; incomplete integration |
| 0-1 分 | 系统无法端到端运行 | System cannot run end-to-end |

---

## 二、文档质量 (20 分) | Documentation Quality (20 pts)

### 2.1 README 质量 (8 分) | README Quality (8 pts)

| 分数 | 标准 | Criteria |
|------|------|----------|
| 7-8 分 | README 包含：项目简介、功能列表、硬件清单（含购买链接）、接线图/引脚定义表、编译和下载步骤、Android 应用安装说明、使用指南、常见问题排查。结构清晰，图文并茂。 | README includes: project overview, feature list, hardware BOM (with purchase links), wiring diagram/pin table, compile and flash steps, Android app installation guide, usage guide, troubleshooting. Well structured with images. |
| 5-6 分 | README 内容较完整；有硬件清单和使用说明；缺少部分细节 | README fairly complete; has hardware list and usage guide; missing some details |
| 3-4 分 | README 基本内容存在；但不完整或缺乏细节 | README has basic content but is incomplete or lacks detail |
| 1-2 分 | README 存在但内容很少 | README exists but has minimal content |
| 0 分 | 无 README | No README |

### 2.2 代码注释 (7 分) | Code Comments (7 pts)

| 分数 | 标准 | Criteria |
|------|------|----------|
| 7 分 | 每个源文件头部有模块说明；每个函数有参数和返回值注释；关键算法（温度计算、伪彩色映射）有详细注释；数据结构有说明；注释覆盖率 > 70%；中英双语注释 | Each source file has module description; each function has parameter and return value comments; key algorithms (temperature calculation, pseudo-color mapping) are thoroughly commented; data structures documented; comment coverage > 70%; bilingual comments |
| 5-6 分 | 大部分函数有注释；关键算法有说明；注释覆盖率 40-70% | Most functions have comments; key algorithms explained; 40-70% comment coverage |
| 3-4 分 | 部分代码有注释；关键函数缺少说明 | Some code has comments; key functions lack explanation |
| 1-2 分 | 注释极少 | Very few comments |
| 0 分 | 无注释 | No comments |

### 2.3 接线文档 (5 分) | Wiring Documentation (5 pts)

| 分数 | 标准 | Criteria |
|------|------|----------|
| 5 分 | 有完整的接线图（可手绘或使用 Fritzing/KiCad）；包含所有引脚连接的对照表；标注了上拉电阻位置和阻值；标注了电源连接 | Complete wiring diagram (hand-drawn or Fritzing/KiCad); pin connection table for all connections; pull-up resistor positions and values marked; power connections marked |
| 4 分 | 有接线图和引脚表；基本完整 | Has wiring diagram and pin table; mostly complete |
| 3 分 | 有简单的接线说明或示意图 | Has simple wiring description or sketch |
| 1-2 分 | 接线文档不完整 | Incomplete wiring documentation |
| 0 分 | 无接线文档 | No wiring documentation |

---

## 三、演示展示 (20 分) | Presentation (20 pts)

### 3.1 实时演示效果 (10 分) | Live Demo Effect (10 pts)

| 分数 | 标准 | Criteria |
|------|------|----------|
| 9-10 分 | 热成像系统实时流畅运行；双目画面清晰；成功演示多种温度物体（热水、冰块、人体）；触摸测温功能正常；演示引人入胜 | Thermal imaging system runs smoothly in real-time; dual-view is clear; successfully demonstrates multiple temperature objects (hot water, ice, human body); touch measurement works; engaging demo |
| 7-8 分 | 系统实时运行；主要功能都能演示；画面基本流畅 | System runs in real-time; main features demonstrated; display mostly smooth |
| 5-6 分 | 系统能运行；部分功能可演示；画面有卡顿 | System runs; some features demonstrated; display has lag |
| 3-4 分 | 系统勉强运行；使用备用视频演示 | System barely runs; uses backup video |
| 0-2 分 | 系统无法运行演示 | System cannot run demo |

### 3.2 讲解深度 (5 分) | Explanation Depth (5 pts)

| 分数 | 标准 | Criteria |
|------|------|----------|
| 5 分 | 能清晰解释系统架构和每个模块的原理；能深入讲解 I2C 协议、温度计算、USB CDC、OpenCV 渲染等核心技术；逻辑清晰，讲解有层次 | Can clearly explain system architecture and module principles; deep explanation of I2C protocol, temperature calculation, USB CDC, OpenCV rendering; clear logic and structured presentation |
| 4 分 | 能解释主要技术原理；理解基本正确 | Can explain main technical principles; basically correct understanding |
| 3 分 | 能解释部分技术内容；有些概念模糊 | Can explain some technical content; some concepts unclear |
| 1-2 分 | 技术讲解表面；概念理解有误 | Superficial technical explanation; incorrect concept understanding |
| 0 分 | 无法进行技术讲解 | Cannot give technical explanation |

### 3.3 视觉辅助与 PPT (5 分) | Visual Aids and PPT (5 pts)

| 分数 | 标准 | Criteria |
|------|------|----------|
| 5 分 | PPT 专业美观；有系统架构图、数据流图、硬件连接图；有热成像效果截图；图文并茂；排版清晰 | Professional and attractive PPT; has system architecture diagram, data flow diagram, hardware connection diagram; thermal imaging screenshots; well illustrated; clear layout |
| 4 分 | PPT 内容完整；有必要的图表 | PPT content complete; has necessary diagrams |
| 3 分 | PPT 基本可用；图表较少 | PPT is functional; few diagrams |
| 1-2 分 | PPT 内容不完整或质量差 | PPT incomplete or poor quality |
| 0 分 | 无 PPT | No PPT |

---

## 四、进度汇报 (10 分) | Progress Reports (10 pts)

### 4.1 Week 1 检查点 (3 分) | Week 1 Check-in (3 pts)

| 分数 | 标准 | Criteria |
|------|------|----------|
| 3 分 | 按时完成检查；STM32 环境搭建完成；I2C 通信实验成功；MLX90640 原始数据可读取；能回答技术问题 | Completed on time; STM32 environment set up; I2C communication experiment successful; MLX90640 raw data readable; can answer technical questions |
| 2 分 | 完成大部分检查项；有少量未完成 | Completed most check items; a few incomplete |
| 1 分 | 完成部分检查项；进度落后 | Completed some check items; behind schedule |
| 0 分 | 未参加检查或未完成任何检查项 | Did not attend check-in or completed nothing |

### 4.2 Week 2 检查点 (4 分) | Week 2 Check-in (4 pts)

| 分数 | 标准 | Criteria |
|------|------|----------|
| 4 分 | 按时完成检查；MLX90640 完整帧数据读取正常；温度计算基本准确；USB CDC 能传输数据；至少一个传感器端到端联通 | Completed on time; full MLX90640 frame reading works; temperature calculation mostly accurate; USB CDC can transmit data; at least one sensor connected end-to-end |
| 3 分 | 完成大部分检查项；部分功能未完全稳定 | Completed most items; some features not fully stable |
| 2 分 | 完成部分检查项；USB 通信尚未完全实现 | Completed some items; USB communication not fully implemented |
| 1 分 | 进度明显落后 | Significantly behind schedule |
| 0 分 | 未参加检查 | Did not attend check-in |

### 4.3 Week 3 检查点 / 最终交付 (3 分) | Week 3 Check-in / Final Delivery (3 pts)

| 分数 | 标准 | Criteria |
|------|------|----------|
| 3 分 | 系统端到端集成完成；Android 应用能显示热成像；文档提交完整；演示准备充分 | End-to-end integration complete; Android app displays thermal imaging; documentation submitted; demo preparation thorough |
| 2 分 | 系统基本集成；部分功能待完善 | System basically integrated; some features need improvement |
| 1 分 | 集成不完整 | Incomplete integration |
| 0 分 | 未完成最终交付 | Did not complete final delivery |

---

## 五、团队协作 (10 分) | Collaboration (10 pts)

### 5.1 Git 提交记录 (5 分) | Git Commit History (5 pts)

| 分数 | 标准 | Criteria |
|------|------|----------|
| 5 分 | 提交频率合理（每天至少 1 次有意义的提交）；commit message 清晰有意义（如 "feat: implement MLX90640 temperature compensation" 而非 "update"）；使用了分支管理；有代码审查记录 | Reasonable commit frequency (at least 1 meaningful commit per day); clear commit messages (e.g., "feat: implement MLX90640 temperature compensation" not "update"); uses branching; has code review records |
| 4 分 | 提交频率正常；commit message 基本清晰 | Normal commit frequency; commit messages mostly clear |
| 3 分 | 有提交记录；commit message 不够规范 | Has commits; commit messages not well formatted |
| 1-2 分 | 提交很少或 commit message 无意义 | Very few commits or meaningless messages |
| 0 分 | 无 Git 提交记录 | No Git commit history |

### 5.2 任务分配与执行 (5 分) | Task Distribution and Execution (5 pts)

| 分数 | 标准 | Criteria |
|------|------|----------|
| 5 分 | 有明确的任务分工记录（如使用 GitHub Issues 或看板）；各成员任务量均衡；能展示协作过程；问题沟通及时有效 | Clear task distribution records (e.g., GitHub Issues or kanban); balanced workload among members; can demonstrate collaboration process; timely and effective communication |
| 4 分 | 有基本任务分工；各成员有明确职责 | Basic task distribution; members have clear responsibilities |
| 3 分 | 有分工但不明确；部分成员贡献不均 | Some distribution but unclear; uneven contributions |
| 1-2 分 | 缺乏分工；个别成员承担绝大部分工作 | Lack of distribution; one member does most of the work |
| 0 分 | 无协作记录 | No collaboration records |

---

## 六、加分项 | Bonus Points

| 项目 | 加分 | 条件 | Conditions |
|------|------|------|------------|
| 自定义 3D 打印外壳 | +3 分 | 外壳设计合理、美观、实用 | Custom 3D-printed enclosure that is well-designed, attractive, and practical |
| 多种伪彩色方案切换 | +2 分 | 至少 3 种色标方案可在应用中切换 | At least 3 color schemes switchable in the app |
| 温度数据导出功能 | +2 分 | 能将温度数据导出为 CSV 或图片 | Temperature data export to CSV or image |
| 温度报警阈值设置 | +1 分 | 用户可设置报警温度，超温时画面变色提醒 | User-configurable alarm threshold with visual alert |
| 使用 FreeRTOS | +1 分 | 固件使用 FreeRTOS 进行任务调度 | Firmware uses FreeRTOS for task scheduling |
| OTA 固件升级 | +1 分 | 通过 USB 或蓝牙实现固件远程升级 | Over-the-air firmware upgrade via USB or Bluetooth |

**加分上限：10 分 | Bonus cap: 10 points**

---

## 七、扣分项 | Deductions

| 项目 | 扣分 | 条件 | Conditions |
|------|------|------|------------|
| 延迟提交 | -10%/天 | 作业或项目延迟提交（每天扣总分的 10%） | Late submission of assignments or project |
| 代码抄袭 | -100% | 抄袭他人代码，直接复制粘贴不加标注 | Plagiarized code, copying without attribution |
| 演示缺席 | -20% | 无故缺席最终展示 | Unexcused absence from final presentation |
| 文档缺失 | -5%/项 | 缺少必要的文档（README、注释、接线图） | Missing required documentation |

---

## 八、评分流程 | Grading Process

### Step 1: 自我评估（Day 15 上午）| Self-Assessment (Day 15 Morning)

学员填写自评表，包含：
- 各功能模块完成情况
- 代码质量自我评价
- 文档完整性检查
- 遇到的主要问题和解决方案

### Step 2: 功能验证（Day 15 下午）| Function Verification (Day 15 Afternoon)

验证清单：
1. STM32 固件编译下载，通过串口验证 MLX90640 数据读取
2. USB CDC 连接 PC，使用 Python 脚本验证数据传输
3. Android 应用连接 STM32，验证热成像画面显示
4. 双目画面同时显示
5. 触摸测温功能验证
6. 连续运行 5 分钟稳定性测试

### Step 3: 代码审查（Day 15 下午）| Code Review (Day 15 Afternoon)

审查维度：
- 固件代码结构和质量
- Android 代码结构和质量
- 注释完整性
- 错误处理覆盖率
- 模块化程度

### Step 4: 演示评分（Day 15 下午）| Demo Scoring (Day 15 Afternoon)

根据演示效果现场评分：
- 实时演示流畅度
- 技术讲解深度
- PPT 质量
- 问答表现

### Step 5: 综合评分（Day 15 下午）| Final Scoring (Day 15 Afternoon)

汇总各项得分，计算最终成绩。

---

## 九、成绩等级 | Grade Levels

| 分数 | 等级 | 说明 | Description |
|------|------|------|-------------|
| 95-100 | A+ | 卓越 | Exceptional work, beyond expectations |
| 90-94 | A | 优秀 | Excellent work in all dimensions |
| 85-89 | A- | 优秀 | Strong work with minor gaps |
| 80-84 | B+ | 良好 | Good work, solid understanding |
| 75-79 | B | 良好 | Competent work, meets expectations |
| 70-74 | B- | 良好 | Adequate work, some areas need improvement |
| 65-69 | C+ | 中等 | Acceptable work, noticeable gaps |
| 60-64 | C | 及格 | Minimum acceptable standard |
| <60 | F | 不及格 | Does not meet minimum requirements |

---

## 十、评分表模板 | Scoring Sheet Template

### 学员信息 | Student Information

| 字段 | 值 |
|------|------|
| 学员姓名 | |
| 项目名称 | ThermalEyes - 双目手机热成像仪 |
| 评分日期 | |

### 评分记录 | Scoring Record

| 评分维度 | 子项 | 满分 | 得分 | 备注 |
|----------|------|------|------|------|
| **技术实现 (40)** | I2C 驱动 | 5 | | |
| | MLX90640 驱动 | 5 | | |
| | USB CDC 通信 | 5 | | |
| | USB 数据接收解析 | 5 | | |
| | OpenCV 渲染 | 5 | | |
| | UI/UX 功能 | 5 | | |
| | 硬件组装 | 5 | | |
| | 系统集成 | 5 | | |
| **文档质量 (20)** | README | 8 | | |
| | 代码注释 | 7 | | |
| | 接线文档 | 5 | | |
| **演示展示 (20)** | 实时演示 | 10 | | |
| | 讲解深度 | 5 | | |
| | 视觉辅助 | 5 | | |
| **进度汇报 (10)** | Week 1 | 3 | | |
| | Week 2 | 4 | | |
| | Week 3 | 3 | | |
| **团队协作 (10)** | Git 记录 | 5 | | |
| | 任务分配 | 5 | | |
| **加分项** | | /10 | | |
| **扣分项** | | | | |
| **总分** | | **/100** | | |

### 综合评语 | Overall Comments

**优点 | Strengths:**

**待改进 | Areas for Improvement:**

**建议 | Suggestions:**

---

*最后更新：2026-05-27 | Last updated: 2026-05-27*
