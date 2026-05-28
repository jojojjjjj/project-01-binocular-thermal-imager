# Day 14: 全系统联调与优化 | Full System Integration & Optimization

## 学习目标 | Learning Objectives

- 完成硬件 + 固件 + APP 的端到端联调
- 识别并解决系统集成中的常见问题
- 优化系统性能（帧率、延迟、稳定性）
- 添加错误处理和用户反馈机制
- 准备最终演示

## 前置准备 | Prerequisites

- [ ] 固件稳定运行，USB 发送温度数据正常
- [ ] APP 基本功能完成（热图像显示、USB 通信）
- [ ] 硬件连接正确
- [ ] 已准备演示用的测试场景

## 为什么学这个？| Why This Matters

系统集成是整个项目最关键的阶段。单独的模块可能都工作正常，但组合在一起时往往会暴露各种问题：时序不匹配、数据丢失、性能瓶颈等。这是工程实践的真实写照 -- 在工业界，系统联调通常占项目总时间的 30-40%。掌握联调的方法论，是成为优秀工程师的必备技能。

## 今日任务 | Today's Tasks

### Task 1: 端到端联调 (estimated 90 minutes)

**目标：** 确保从传感器到手机屏幕的完整数据流畅通

**步骤：**

1. **建立联调检查清单**
   ```
   联调检查项：
   Phase A: 硬件连接
   [ ] STM32 供电正常（LED 亮）
   [ ] MLX90640 连接正确（SDA/SCL/VCC/GND）
   [ ] USB 线连接 STM32 到手机（通过 OTG）
   [ ] 可见光摄像头连接（如有）

   Phase B: 固件验证
   [ ] 固件正常启动（串口输出启动信息）
   [ ] I2C 通信正常（能读到传感器数据）
   [ ] USB CDC 枚举成功（手机识别到设备）
   [ ] 温度数据发送正常

   Phase C: APP 验证
   [ ] APP 启动正常
   [ ] USB 权限获取成功
   [ ] USB 连接建立成功
   [ ] 帧数据解析正确
   [ ] 热图像渲染正常
   [ ] 温度显示正确

   Phase D: 功能验证
   [ ] 手掌测试：靠近传感器时温度升高
   [ ] 热水测试：热水杯在传感器前方显示高温区域
   [ ] 多人测试：不同人的手温不同
   ```

2. **逐步验证每个环节**
   - 按 A -> B -> C -> D 的顺序验证
   - 每个环节出问题时，停下来排查
   - 记录每个环节的状态和问题

3. **常见联调问题排查**

   **问题 1: 手机无法识别 USB 设备**
   - 检查 OTG 线是否正常
   - 确认手机支持 USB Host 模式
   - 尝试在另一台手机上测试
   - 检查固件 USB 配置是否正确

   **问题 2: APP 收不到数据**
   - 确认 USB CDC 枚举成功
   - 检查 APP 的 USB 权限
   - 使用 USB 抓包工具查看数据
   - 确认帧格式匹配

   **问题 3: 热图像显示异常**
   - 检查温度数据是否合理
   - 确认 minTemp/maxTemp 范围正确
   - 检查字节序（大小端）
   - 验证帧解析逻辑

   **问题 4: 帧率太低**
   - 测量每个环节的耗时
   - 确认 I2C 读取速度
   - 检查 USB 传输瓶颈
   - 优化图像渲染代码

**预期结果：**
- 端到端数据流畅通
- 能在手机上看到实时的热图像
- 手掌测试显示温度变化

---

### Task 2: 性能优化 (estimated 60 minutes)

**目标：** 提升系统帧率和响应速度

**步骤：**

1. **测量各环节耗时**
   - 在固件中添加计时代码：
   ```c
   uint32_t t0 = HAL_GetTick();
   MLX90640_ReadBothSubpages();
   uint32_t t1 = HAL_GetTick();
   MLX90640_CalculateTemperatures(0.95f);
   uint32_t t2 = HAL_GetTick();
   PackThermalFrame();
   uint32_t t3 = HAL_GetTick();
   SendThermalFrame();
   uint32_t t4 = HAL_GetTick();
   printf("I2C:%lu Calc:%lu Pack:%lu USB:%lu Total:%lu ms\r\n",
          t1-t0, t2-t1, t3-t2, t4-t3, t4-t0);
   ```

2. **识别性能瓶颈**
   - 通常 I2C 读取是最慢的环节
   - USB 传输通常足够快
   - 图像渲染可能是 APP 端的瓶颈

3. **优化策略**
   - **I2C 优化**：使用 DMA 传输，减少 CPU 等待
   - **温度计算优化**：使用查表法代替复杂计算
   - **USB 发送优化**：批量发送，减少函数调用次数
   - **APP 渲染优化**：避免不必要的内存分配，复用 Bitmap

4. **设置合理的帧率目标**
   - 最低可接受：2 FPS
   - 目标帧率：5-10 FPS
   - 理想帧率：10-15 FPS

**预期结果：**
- 系统帧率至少达到 5 FPS
- 各环节耗时清楚
- 无明显性能瓶颈

---

### Task 3: 错误处理和稳定性 (estimated 45 minutes)

**目标：** 添加完善的错误处理，确保系统在各种情况下稳定运行

**步骤：**

1. **固件端错误处理**
   ```c
   // 添加 I2C 错误恢复
   if (MLX90640_ReadSubpage(0) != HAL_OK) {
       error_count++;
       if (error_count > 5) {
           // 尝试重新初始化 I2C
           HAL_I2C_DeInit(&hi2c1);
           HAL_Delay(10);
           HAL_I2C_Init(&hi2c1);
           error_count = 0;
       }
       continue;  // 跳过本次循环
   }
   ```

2. **APP 端错误处理**
   ```java
   // USB 断开重连
   private void handleUsbDisconnect() {
       runOnUiThread(() -> {
           connectionStatus.setText("连接断开");
           connectionStatus.setTextColor(Color.RED);
       });

       // 尝试重连
       new Thread(() -> {
           while (!usbManager.connect()) {
               try { Thread.sleep(2000); } catch (Exception e) {}
           }
           runOnUiThread(() -> {
               connectionStatus.setText("已连接");
               connectionStatus.setTextColor(Color.GREEN);
           });
       }).start();
   }
   ```

3. **压力测试**
   - 连续运行 30 分钟，观察稳定性
   - 反复插拔 USB，测试重连
   - 覆盖传感器，测试极端温度
   - 快速切换 APP 前后台

**预期结果：**
- 系统运行 30 分钟无崩溃
- USB 断开后能自动重连
- 错误有明确的提示

---

### Task 4: 最终功能验证 (estimated 45 minutes)

**目标：** 验证所有核心功能，准备演示

**步骤：**

1. **核心功能验证清单**
   - [ ] 温度测量精度（与温度计对比，偏差 < 2C）
   - [ ] 热图像显示清晰，颜色映射正确
   - [ ] 可见光+热图像融合效果（如已实现）
   - [ ] 帧率满足最低要求（> 2 FPS）
   - [ ] 手掌测试效果明显
   - [ ] 热水杯测试效果明显

2. **准备演示场景**
   - 场景 1：手掌温度检测
     * 将手掌放在传感器前方 10-30cm
     * 展示热图像中手掌的温度分布
   - 场景 2：热水杯检测
     * 将热水杯放在传感器前方
     * 展示高温区域
   - 场景 3：室温检测
     * 展示室温下的热图像
     * 指出不同物体的温度差异

3. **准备演示材料**
   - 演示用的热水杯
   - 冰块或冷水（展示低温）
   - 手电筒或其他热源（可选）
   - 确保 USB 线足够长，方便演示

**预期结果：**
- 所有核心功能验证通过
- 演示场景准备就绪
- 系统稳定可演示

---

## 今日作业 | Homework

1. **联调完成**（必须）
   - 端到端数据流畅通
   - 热图像实时显示
   - 提交联调过程记录

2. **性能报告**（必须）
   - 记录各环节耗时
   - 最终帧率
   - 已知的限制和问题

3. **演示准备**（必须）
   - 完成功能验证清单
   - 准备演示场景
   - 至少预演一次

4. **代码整理**（推荐）
   - 为关键代码添加注释
   - 清理调试代码
   - 提交最终版本到 Git

## 明日预告 | Tomorrow's Preview

明天是课程的最后一天！你将准备演示 PPT，进行最终展示，回顾 15 天的学习历程。恭喜你完成了这个从硬件到软件的完整嵌入式系统项目！

## 参考资源 | References

- **系统集成方法论**：https://www.embedded.com/system-integration-testing/
- **Android USB 调试**：https://developer.android.com/studio/debug/
- **STM32 调试技巧**：https://www.st.com/en/development-tools/stm32cubeide.html
- **ThermalEyes APP 源码**：https://github.com/colourfate/ThermalEyes

---

*预计完成时间：6-7 小时*
*Estimated completion time: 6-7 hours*
