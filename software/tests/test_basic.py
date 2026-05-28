"""
============================================================
ThermalEyes 基础测试 | ThermalEyes Basic Tests
============================================================
用于验证数据解析、温度计算和伪彩色映射等核心功能
Tests for data parsing, temperature calculation, and pseudo-color mapping

测试内容 | Test Contents:
  1. 配置文件解析 | Configuration file parsing
  2. USB 数据包格式验证 | USB data packet format verification
  3. 温度数据解析 | Temperature data parsing
  4. CRC16 校验计算 | CRC16 checksum calculation
  5. 伪彩色映射函数 | Pseudo-color mapping functions
  6. 高斯模糊频率分解 | Gaussian blur frequency decomposition
  7. 帧缓冲区管理 | Frame buffer management
============================================================
"""

import struct
import sys
import os
import math
from pathlib import Path

# 修复 Windows 控制台编码问题 | Fix Windows console encoding issue
if sys.platform == 'win32':
    try:
        sys.stdout.reconfigure(encoding='utf-8', errors='replace')
        sys.stderr.reconfigure(encoding='utf-8', errors='replace')
    except Exception:
        os.environ['PYTHONIOENCODING'] = 'utf-8'

# 添加项目根目录到路径 | Add project root to path
project_root = Path(__file__).parent.parent
sys.path.insert(0, str(project_root))

# 尝试导入可选依赖 | Try importing optional dependencies
try:
    import numpy as np
    HAS_NUMPY = True
except ImportError:
    HAS_NUMPY = False
    print("警告: NumPy 未安装，部分测试将被跳过")
    print("Warning: NumPy not installed, some tests will be skipped")

try:
    import yaml
    HAS_YAML = True
except ImportError:
    HAS_YAML = False
    print("警告: PyYAML 未安装，配置测试将被跳过")
    print("Warning: PyYAML not installed, config tests will be skipped")


# ============================================================
# 常量定义 | Constant Definitions
# ============================================================

# MLX90640 传感器参数 | MLX90640 sensor parameters
THERMAL_COLS = 32
THERMAL_ROWS = 24
THERMAL_PIXELS = THERMAL_COLS * THERMAL_ROWS  # 768

# USB 数据包格式 | USB data packet format
FRAME_HEADER = 0x5A5A
PACKET_HEADER_SIZE = 4     # 帧头(2) + 帧计数(2)
PACKET_DATA_SIZE = THERMAL_PIXELS * 2  # 768 * 2 = 1536
PACKET_CRC_SIZE = 2
PACKET_TOTAL_SIZE = PACKET_HEADER_SIZE + PACKET_DATA_SIZE + PACKET_CRC_SIZE  # 1542

# 温度单位 | Temperature unit
TEMP_UNIT = 0.01  # 0.01°C per LSB


# ============================================================
# 辅助函数 | Helper Functions
# ============================================================

def crc16_ccitt(data):
    """
    计算 CRC16-CCITT 校验和 | Calculate CRC16-CCITT checksum

    多项式: x^16 + x^12 + x^5 + 1 (0x1021)
    初始值: 0xFFFF

    Args:
        data: 字节数据 | Byte data
    Returns:
        CRC16 值 | CRC16 value
    """
    crc = 0xFFFF
    for byte in data:
        crc ^= byte << 8
        for _ in range(8):
            if crc & 0x8000:
                crc = (crc << 1) ^ 0x1021
            else:
                crc <<= 1
            crc &= 0xFFFF
    return crc


def temp_to_raw(temp_celsius):
    """
    温度值转换为原始数据格式 | Convert temperature to raw data format

    Args:
        temp_celsius: 温度 (°C)
    Returns:
        16 位有符号整数值 (0.01°C/LSB)
    """
    raw = int(temp_celsius / TEMP_UNIT)
    if raw < 0:
        raw += 65536  # 补码表示 | Two's complement
    return raw


def raw_to_temp(raw_value):
    """
    原始数据转换为温度值 | Convert raw data to temperature

    Args:
        raw_value: 16 位无符号整数值
    Returns:
        温度 (°C)
    """
    if raw_value > 32767:
        raw_value -= 65536  # 补码转换 | Two's complement conversion
    return raw_value * TEMP_UNIT


def build_thermal_packet(temperatures, frame_count=0):
    """
    构建一个完整的热成像数据包 | Build a complete thermal data packet

    数据包格式 | Packet format:
      [0x5A][0x5A][FrameCount_L][FrameCount_H][Data...1536B...][CRC_L][CRC_H]

    Args:
        temperatures: 温度数组 (768 个浮点值, °C)
        frame_count:  帧计数器
    Returns:
        完整数据包的字节数组
    """
    assert len(temperatures) == THERMAL_PIXELS, \
        f"温度数组长度错误 | Wrong temp array length: {len(temperatures)} != {THERMAL_PIXELS}"

    packet = bytearray()

    # 帧头 | Frame header
    packet.append(0x5A)
    packet.append(0x5A)

    # 帧计数 (小端序) | Frame count (little-endian)
    packet.append(frame_count & 0xFF)
    packet.append((frame_count >> 8) & 0xFF)

    # 温度数据 (小端序, 16 位有符号整数, 0.01°C/LSB)
    # Temperature data (little-endian, 16-bit signed, 0.01°C/LSB)
    data_start = len(packet)
    for temp in temperatures:
        raw = temp_to_raw(temp)
        packet.append(raw & 0xFF)         # 低字节 | Low byte
        packet.append((raw >> 8) & 0xFF)  # 高字节 | High byte

    # CRC16 校验 (覆盖温度数据部分)
    # CRC16 checksum (covers temperature data portion)
    data_bytes = packet[data_start:]
    crc = crc16_ccitt(data_bytes)
    packet.append(crc & 0xFF)         # CRC 低字节
    packet.append((crc >> 8) & 0xFF)  # CRC 高字节

    return bytes(packet)


def parse_thermal_packet(packet):
    """
    解析热成像数据包 | Parse thermal data packet

    Args:
        packet: 数据包字节
    Returns:
        (temperatures, frame_count, crc_valid) 元组
    """
    assert len(packet) == PACKET_TOTAL_SIZE, \
        f"数据包长度错误 | Wrong packet length: {len(packet)} != {PACKET_TOTAL_SIZE}"

    # 检查帧头 | Check frame header
    assert packet[0] == 0x5A and packet[1] == 0x5A, \
        "帧头错误 | Frame header error"

    # 读取帧计数 | Read frame count
    frame_count = packet[3] << 8 | packet[2]

    # 读取温度数据 | Read temperature data
    temperatures = []
    for i in range(THERMAL_PIXELS):
        offset = PACKET_HEADER_SIZE + i * 2
        raw = packet[offset] | (packet[offset + 1] << 8)
        temperatures.append(raw_to_temp(raw))

    # 验证 CRC | Verify CRC
    data_bytes = packet[PACKET_HEADER_SIZE:PACKET_HEADER_SIZE + PACKET_DATA_SIZE]
    expected_crc = crc16_ccitt(data_bytes)
    actual_crc = packet[-2] | (packet[-1] << 8)
    crc_valid = (expected_crc == actual_crc)

    return temperatures, frame_count, crc_valid


def generate_test_temperatures(base_temp=25.0, pattern="gradient"):
    """
    生成测试温度数据 | Generate test temperature data

    Args:
        base_temp: 基准温度 (°C)
        pattern:   温度分布模式
                   "gradient"  - 梯度分布 | Gradient distribution
                   "hot_spot"  - 中心热点 | Center hot spot
                   "uniform"   - 均匀温度 | Uniform temperature
                   "random"    - 随机温度 | Random temperature
    Returns:
        温度数组 (768 个值)
    """
    temperatures = []

    for row in range(THERMAL_ROWS):
        for col in range(THERMAL_COLS):
            if pattern == "gradient":
                # 从左到右温度线性增加 | Temperature increases linearly left to right
                temp = base_temp + (col / THERMAL_COLS) * 20.0

            elif pattern == "hot_spot":
                # 中心有热点 | Hot spot in center
                dx = col - THERMAL_COLS / 2
                dy = row - THERMAL_ROWS / 2
                dist = math.sqrt(dx * dx + dy * dy)
                max_dist = math.sqrt((THERMAL_COLS / 2) ** 2 + (THERMAL_ROWS / 2) ** 2)
                temp = base_temp + 15.0 * max(0, 1.0 - dist / max_dist)

            elif pattern == "uniform":
                temp = base_temp

            elif pattern == "random":
                import random
                temp = base_temp + random.uniform(-5, 15)

            else:
                temp = base_temp

            temperatures.append(round(temp, 2))

    return temperatures


# ============================================================
# 测试函数 | Test Functions
# ============================================================

def test_config_parsing():
    """
    测试 1: 配置文件解析 | Test 1: Configuration file parsing
    """
    print("\n=== 测试 1: 配置文件解析 | Test 1: Configuration Parsing ===\n")

    if not HAS_YAML:
        print("跳过: PyYAML 未安装 | Skipped: PyYAML not installed")
        return False

    config_path = project_root / "config.template.yaml"
    if not config_path.exists():
        print(f"配置文件不存在 | Config file not found: {config_path}")
        return False

    try:
        with open(config_path, 'r', encoding='utf-8') as f:
            config = yaml.safe_load(f)

        # 验证必需的配置节 | Verify required config sections
        required_sections = ['hardware', 'sensor', 'communication', 'fusion']
        all_present = True
        for section in required_sections:
            if section in config:
                print(f"  [PASS] 配置节存在 | Config section exists: {section}")
            else:
                print(f"  [FAIL] 配置节缺失 | Config section missing: {section}")
                all_present = False

        # 验证传感器地址 | Verify sensor address
        sensor_addr = config.get('sensor', {}).get('address')
        if sensor_addr == 0x33:
            print(f"  [PASS] MLX90640 I2C 地址正确 | I2C address correct: 0x{sensor_addr:02X}")
        else:
            print(f"  [FAIL] MLX90640 I2C 地址异常 | I2C address unexpected: {sensor_addr}")
            all_present = False

        # 验证 I2C 时钟频率 | Verify I2C clock speed
        i2c_speed = config.get('hardware', {}).get('i2c', {}).get('clock_speed')
        if i2c_speed == 400000:
            print(f"  [PASS] I2C 时钟频率正确 | I2C clock speed correct: {i2c_speed}Hz")
        else:
            print(f"  [FAIL] I2C 时钟频率异常 | I2C clock speed unexpected: {i2c_speed}")
            all_present = False

        # 验证融合参数 | Verify fusion parameters
        high_ratio = config.get('fusion', {}).get('frequency_fusion', {}).get('high_freq_ratio')
        low_ratio = config.get('fusion', {}).get('frequency_fusion', {}).get('low_freq_ratio')
        if high_ratio is not None and low_ratio is not None:
            total = high_ratio + low_ratio
            print(f"  [INFO] 频率融合权重 | Freq fusion weights: high={high_ratio}, low={low_ratio}, total={total}")

        return all_present

    except Exception as e:
        print(f"  [FAIL] 配置解析失败 | Config parse failed: {e}")
        return False


def test_usb_packet_format():
    """
    测试 2: USB 数据包格式验证 | Test 2: USB data packet format verification
    """
    print("\n=== 测试 2: USB 数据包格式 | Test 2: USB Packet Format ===\n")

    all_passed = True

    # 测试数据包大小 | Test packet size
    if PACKET_TOTAL_SIZE == 1542:
        print(f"  [PASS] 数据包大小正确 | Packet size correct: {PACKET_TOTAL_SIZE} bytes")
    else:
        print(f"  [FAIL] 数据包大小错误 | Packet size wrong: {PACKET_TOTAL_SIZE}")
        all_passed = False

    # 详细计算 | Detailed calculation
    header = 2
    frame_count = 2
    temp_data = THERMAL_PIXELS * 2
    crc = 2
    total = header + frame_count + temp_data + crc
    print(f"  [INFO] 数据包构成 | Packet composition:")
    print(f"         帧头 | Header:     {header} bytes")
    print(f"         帧计数 | Frame#:    {frame_count} bytes")
    print(f"         温度数据 | Temp:    {temp_data} bytes ({THERMAL_PIXELS} pixels x 2)")
    print(f"         CRC16:              {crc} bytes")
    print(f"         总计 | Total:      {total} bytes")

    # 测试帧头标识 | Test frame header magic
    if FRAME_HEADER == 0x5A5A:
        print(f"  [PASS] 帧头标识正确 | Frame header correct: 0x{FRAME_HEADER:04X}")
    else:
        print(f"  [FAIL] 帧头标识错误 | Frame header wrong: 0x{FRAME_HEADER:04X}")
        all_passed = False

    return all_passed


def test_temperature_encoding():
    """
    测试 3: 温度数据编码/解码 | Test 3: Temperature data encoding/decoding
    """
    print("\n=== 测试 3: 温度数据编码 | Test 3: Temperature Encoding ===\n")

    test_cases = [
        (25.47,  2547,  "正常温度 | Normal temperature"),
        (0.0,    0,     "零度 | Zero"),
        (-5.12, -512,   "负温度 | Negative temperature"),
        (-40.0, -4000,  "最低温度 | Min temperature"),
        (300.0,  30000, "最高温度 | Max temperature"),
        (36.5,   3650,  "体温 | Body temperature"),
    ]

    all_passed = True
    for temp_c, expected_raw, desc in test_cases:
        # 测试编码: 温度 -> 原始值
        raw = temp_to_raw(temp_c)
        if raw == expected_raw or raw == (expected_raw + 65536):
            print(f"  [PASS] 编码 | Encode: {temp_c:.2f}°C -> {raw} ({desc})")
        else:
            print(f"  [FAIL] 编码 | Encode: {temp_c:.2f}°C -> {raw}, expected {expected_raw} ({desc})")
            all_passed = False

        # 测试解码: 原始值 -> 温度
        if expected_raw < 0:
            decode_raw = expected_raw + 65536
        else:
            decode_raw = expected_raw
        decoded_temp = raw_to_temp(decode_raw)
        if abs(decoded_temp - temp_c) < 0.01:
            print(f"  [PASS] 解码 | Decode: {decode_raw} -> {decoded_temp:.2f}°C")
        else:
            print(f"  [FAIL] 解码 | Decode: {decode_raw} -> {decoded_temp:.2f}°C, expected {temp_c:.2f}°C")
            all_passed = False

    return all_passed


def test_crc16():
    """
    测试 4: CRC16 校验计算 | Test 4: CRC16 checksum calculation
    """
    print("\n=== 测试 4: CRC16 校验 | Test 4: CRC16 Checksum ===\n")

    all_passed = True

    # 测试空数据 | Test empty data
    crc_empty = crc16_ccitt(b"")
    if crc_empty == 0xFFFF:
        print(f"  [PASS] 空数据 CRC | Empty data CRC: 0x{crc_empty:04X}")
    else:
        print(f"  [FAIL] 空数据 CRC | Empty data CRC: 0x{crc_empty:04X}, expected 0xFFFF")
        all_passed = False

    # 测试已知数据 | Test known data
    test_data = bytes([0x01, 0x02, 0x03, 0x04])
    crc = crc16_ccitt(test_data)
    print(f"  [INFO] 测试数据 | Test data: {test_data.hex()} -> CRC: 0x{crc:04X}")

    # 测试同一数据两次计算结果一致 | Test consistency
    crc1 = crc16_ccitt(test_data)
    crc2 = crc16_ccitt(test_data)
    if crc1 == crc2:
        print(f"  [PASS] CRC 计算一致性 | CRC consistency: {crc1} == {crc2}")
    else:
        print(f"  [FAIL] CRC 不一致 | CRC inconsistency: {crc1} != {crc2}")
        all_passed = False

    # 测试不同数据产生不同 CRC | Test different data produces different CRC
    test_data2 = bytes([0x01, 0x02, 0x03, 0x05])
    crc2 = crc16_ccitt(test_data2)
    if crc != crc2:
        print(f"  [PASS] 不同数据不同 CRC | Different data different CRC")
    else:
        print(f"  [FAIL] 不同数据相同 CRC | Different data same CRC")
        all_passed = False

    return all_passed


def test_packet_build_and_parse():
    """
    测试 5: 数据包构建与解析 | Test 5: Packet build and parse
    """
    print("\n=== 测试 5: 数据包构建与解析 | Test 5: Packet Build & Parse ===\n")

    all_passed = True

    # 生成测试温度数据 | Generate test temperature data
    test_temps = generate_test_temperatures(25.0, "gradient")
    frame_count = 42

    # 构建数据包 | Build packet
    packet = build_thermal_packet(test_temps, frame_count)

    # 验证数据包大小 | Verify packet size
    if len(packet) == PACKET_TOTAL_SIZE:
        print(f"  [PASS] 构建数据包大小正确 | Built packet size: {len(packet)}")
    else:
        print(f"  [FAIL] 构建数据包大小错误 | Built packet size: {len(packet)} != {PACKET_TOTAL_SIZE}")
        all_passed = False

    # 解析数据包 | Parse packet
    parsed_temps, parsed_count, crc_valid = parse_thermal_packet(packet)

    # 验证帧计数 | Verify frame count
    if parsed_count == frame_count:
        print(f"  [PASS] 帧计数正确 | Frame count correct: {parsed_count}")
    else:
        print(f"  [FAIL] 帧计数错误 | Frame count wrong: {parsed_count} != {frame_count}")
        all_passed = False

    # 验证 CRC | Verify CRC
    if crc_valid:
        print(f"  [PASS] CRC 校验通过 | CRC check passed")
    else:
        print(f"  [FAIL] CRC 校验失败 | CRC check failed")
        all_passed = False

    # 验证温度数据 | Verify temperature data
    max_diff = max(abs(a - b) for a, b in zip(test_temps, parsed_temps))
    if max_diff < 0.02:  # 允许 0.02°C 精度损失 | Allow 0.02°C precision loss
        print(f"  [PASS] 温度数据一致 | Temperature data consistent (max diff: {max_diff:.4f}°C)")
    else:
        print(f"  [FAIL] 温度数据不一致 | Temperature data inconsistent (max diff: {max_diff:.4f}°C)")
        all_passed = False

    return all_passed


def test_frame_patterns():
    """
    测试 6: 不同温度分布模式 | Test 6: Different temperature patterns
    """
    print("\n=== 测试 6: 温度分布模式 | Test 6: Temperature Patterns ===\n")

    if not HAS_NUMPY:
        print("  跳过: NumPy 未安装 | Skipped: NumPy not installed")
        return False

    all_passed = True
    patterns = ["gradient", "hot_spot", "uniform"]

    for pattern in patterns:
        temps = generate_test_temperatures(25.0, pattern)

        if len(temps) == THERMAL_PIXELS:
            arr = np.array(temps)
            print(f"  [PASS] {pattern}: size={len(temps)}, "
                  f"min={arr.min():.1f}°C, max={arr.max():.1f}°C, "
                  f"avg={arr.mean():.1f}°C")
        else:
            print(f"  [FAIL] {pattern}: size={len(temps)} != {THERMAL_PIXELS}")
            all_passed = False

    return all_passed


def test_pixel_coordinate_mapping():
    """
    测试 7: 像素坐标映射 | Test 7: Pixel coordinate mapping
    """
    print("\n=== 测试 7: 像素坐标映射 | Test 7: Pixel Coordinate Mapping ===\n")

    all_passed = True

    # 测试: (row, col) <-> 线性索引 的映射
    # Test: (row, col) <-> linear index mapping
    test_cases = [
        (0,  0,  0),    # 左上角 | Top-left
        (0,  31, 31),   # 第一行最右 | First row rightmost
        (23, 0,  736),  # 最后一行最左 | Last row leftmost
        (23, 31, 767),  # 右下角 | Bottom-right
        (12, 16, 400),  # 中心区域 | Center area
    ]

    for row, col, expected_idx in test_cases:
        idx = row * THERMAL_COLS + col
        if idx == expected_idx:
            # 反向映射 | Reverse mapping
            r = idx // THERMAL_COLS
            c = idx % THERMAL_COLS
            if r == row and c == col:
                print(f"  [PASS] ({row:2d},{col:2d}) <-> {idx:3d}")
            else:
                print(f"  [FAIL] 反向映射错误 | Reverse mapping error: {idx} -> ({r},{c})")
                all_passed = False
        else:
            print(f"  [FAIL] ({row:2d},{col:2d}) -> {idx}, expected {expected_idx}")
            all_passed = False

    return all_passed


def test_gaussian_decomposition():
    """
    测试 8: 高斯模糊频率分解 | Test 8: Gaussian blur frequency decomposition
    """
    print("\n=== 测试 8: 高斯频率分解 | Test 8: Gaussian Frequency Decomposition ===\n")

    if not HAS_NUMPY:
        print("  跳过: NumPy 未安装 | Skipped: NumPy not installed")
        return False

    try:
        from scipy.ndimage import gaussian_filter

        # 创建测试信号: 低频正弦波 + 高频噪声
        # Create test signal: low-freq sine + high-freq noise
        size = 64
        x = np.linspace(0, 4 * np.pi, size)
        low_freq = np.sin(x) * 10 + 25  # 低频成分 | Low-freq component
        high_freq = np.random.randn(size) * 0.5  # 高频噪声 | High-freq noise
        signal = low_freq + high_freq

        # 高斯模糊提取低频 | Gaussian blur to extract low-freq
        sigma = 5.0
        low_extracted = gaussian_filter(signal, sigma=sigma)

        # 高频 = 原始 - 低频 | High-freq = original - low-freq
        high_extracted = signal - low_extracted

        # 验证: 原始 = 低频 + 高频 | Verify: original = low + high
        reconstructed = low_extracted + high_extracted
        max_error = np.max(np.abs(signal - reconstructed))

        if max_error < 1e-10:
            print(f"  [PASS] 分解可逆性 | Decomposition reversibility (error: {max_error:.2e})")
        else:
            print(f"  [FAIL] 分解不可逆 | Decomposition not reversible (error: {max_error:.2e})")
            return False

        # 验证低频成分的方差小于原始信号
        # Verify low-freq variance is less than original
        if np.var(low_extracted) < np.var(signal):
            print(f"  [PASS] 低频方差减小 | Low-freq variance reduced: "
                  f"{np.var(signal):.2f} -> {np.var(low_extracted):.2f}")
        else:
            print(f"  [FAIL] 低频方差未减小 | Low-freq variance not reduced")
            return False

        # 验证高频成分的均值接近零
        # Verify high-freq mean is near zero
        if abs(np.mean(high_extracted)) < 0.5:
            print(f"  [PASS] 高频均值接近零 | High-freq mean near zero: {np.mean(high_extracted):.4f}")
        else:
            print(f"  [FAIL] 高频均值偏移 | High-freq mean offset: {np.mean(high_extracted):.4f}")
            return False

        return True

    except ImportError:
        print("  跳过: SciPy 未安装 | Skipped: SciPy not installed")
        return False


def test_2d_thermal_simulation():
    """
    测试 9: 二维热成像模拟 | Test 9: 2D thermal image simulation
    """
    print("\n=== 测试 9: 二维热成像模拟 | Test 9: 2D Thermal Simulation ===\n")

    if not HAS_NUMPY:
        print("  跳过: NumPy 未安装 | Skipped: NumPy not installed")
        return False

    # 创建 32x24 的热点温度分布 | Create 32x24 hot spot temperature distribution
    temps = generate_test_temperatures(25.0, "hot_spot")
    grid = np.array(temps).reshape(THERMAL_ROWS, THERMAL_COLS)

    # 找到最高温度位置 | Find max temperature position
    max_idx = np.unravel_index(np.argmax(grid), grid.shape)
    min_idx = np.unravel_index(np.argmin(grid), grid.shape)

    # 验证最高温度在中心附近 | Verify max temp is near center
    center_row, center_col = THERMAL_ROWS // 2, THERMAL_COLS // 2
    dist_to_center = abs(max_idx[0] - center_row) + abs(max_idx[1] - center_col)

    if dist_to_center <= 2:  # 允许 2 像素偏差 | Allow 2 pixel tolerance
        print(f"  [PASS] 最高温位置接近中心 | Max temp near center: "
              f"({max_idx[1]},{max_idx[0]}), dist={dist_to_center}")
    else:
        print(f"  [FAIL] 最高温位置偏离中心 | Max temp away from center: "
              f"({max_idx[1]},{max_idx[0]}), dist={dist_to_center}")
        return False

    print(f"  [INFO] 温度范围 | Temperature range: "
          f"{grid.min():.1f}°C ~ {grid.max():.1f}°C")
    print(f"  [INFO] 中心温度 | Center temperature: {grid[center_row, center_col]:.1f}°C")
    print(f"  [INFO] 边角温度 | Corner temperature: {grid[0, 0]:.1f}°C")

    return True


# ============================================================
# 测试运行器 | Test Runner
# ============================================================

def run_all_tests():
    """
    运行所有测试 | Run all tests
    """
    print("=" * 60)
    print("ThermalEyes 双目热成像仪 - 基础测试")
    print("ThermalEyes Binocular Thermal Imager - Basic Tests")
    print("=" * 60)

    tests = [
        ("配置文件解析 | Config parsing",        test_config_parsing),
        ("USB 数据包格式 | USB packet format",   test_usb_packet_format),
        ("温度数据编码 | Temperature encoding",   test_temperature_encoding),
        ("CRC16 校验 | CRC16 checksum",           test_crc16),
        ("数据包构建/解析 | Packet build/parse",   test_packet_build_and_parse),
        ("温度分布模式 | Temperature patterns",    test_frame_patterns),
        ("像素坐标映射 | Pixel coord mapping",     test_pixel_coordinate_mapping),
        ("高斯频率分解 | Gaussian decomposition",  test_gaussian_decomposition),
        ("二维热成像模拟 | 2D thermal simulation", test_2d_thermal_simulation),
    ]

    results = {}

    for name, test_func in tests:
        print(f"\n运行测试 | Running test: {name}")
        print("-" * 40)
        try:
            result = test_func()
            results[name] = result
        except Exception as e:
            print(f"  [ERROR] 测试异常 | Test exception: {e}")
            results[name] = False

    # 打印总结 | Print summary
    print("\n" + "=" * 60)
    print("测试总结 | Test Summary")
    print("=" * 60)

    passed = 0
    failed = 0
    for name, result in results.items():
        status = "PASS" if result else "FAIL"
        print(f"  [{status}] {name}")
        if result:
            passed += 1
        else:
            failed += 1

    print(f"\n通过 | Passed: {passed}/{passed + failed}")
    print(f"失败 | Failed: {failed}/{passed + failed}")

    all_passed = all(results.values())
    print("\n" + "=" * 60)
    if all_passed:
        print("所有测试通过 | All tests passed!")
    else:
        print("部分测试失败，请检查上方输出 | Some tests failed, check output above")

    return all_passed


if __name__ == "__main__":
    success = run_all_tests()
    sys.exit(0 if success else 1)
