/**
 * ============================================================
 * ThermalEyes - Native C++ 图像处理库
 * ThermalEyes - Native C++ Image Processing Library
 * ============================================================
 * 功能概述 | Function Overview:
 *   1. 伪彩色映射 (Pseudo-color mapping): 将温度数据映射为彩色图像
 *      Map temperature data to color images
 *   2. 高斯模糊频率分解 (Gaussian blur frequency decomposition)
 *      分离图像的高频和低频成分
 *      Separate high and low frequency components of images
 *   3. 图像融合 (Image fusion): 合并热成像和可见光图像
 *      Merge thermal and visible light images
 *
 * 编译要求 | Build Requirements:
 *   - Android NDK r25+
 *   - OpenCV Android SDK 4.x
 *   - CMake 3.22+
 *
 * CMakeLists.txt 配置示例 | CMakeLists.txt configuration example:
 *   cmake_minimum_required(VERSION 3.22)
 *   project(native-lib)
 *
 *   find_package(OpenCV REQUIRED)
 *   add_library(native-lib SHARED native-lib.cpp)
 *   target_link_libraries(native-lib ${OpenCV_LIBS} log)
 *
 * 开源参考 | Open-source reference:
 *   https://github.com/colourfate/ThermalEyes
 * ============================================================
 */

#include <jni.h>
#include <android/log.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <vector>
#include <cmath>
#include <algorithm>

/* 日志宏 | Logging macros */
#define LOG_TAG    "ThermalEyes-Native"
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

/* ============================================================
 * 伪彩色映射 | Pseudo-color Mapping
 * ============================================================ */

/**
 * @brief 颜色映射节点 | Color map node
 *
 * 定义温度到颜色的映射点
 * Defines a mapping point from temperature to color
 */
struct ColorNode {
    float position;     /* 归一化位置 [0.0, 1.0] | Normalized position */
    cv::Vec3b color;    /* BGR 颜色值 | BGR color value */
};

/**
 * @brief 生成铁虹 (Ironbow) 配色方案 | Generate Ironbow color palette
 *
 * 铁虹是热成像中最经典的配色方案:
 * Ironbow is the most classic color scheme in thermal imaging:
 *   黑色 -> 蓝色 -> 紫色 -> 红色 -> 橙色 -> 黄色 -> 白色
 *   Black -> Blue -> Purple -> Red -> Orange -> Yellow -> White
 *
 * @param normalized_value 归一化温度值 [0.0, 1.0]
 * @return BGR 颜色值 (8位) | BGR color value (8-bit)
 */
static cv::Vec3b ironbowColor(float normalized_value)
{
    /* 确保值在 [0, 1] 范围内 | Clamp value to [0, 1] range */
    normalized_value = std::max(0.0f, std::min(1.0f, normalized_value));

    /* 铁虹颜色映射表 | Ironbow color mapping table */
    /* 每个节点定义一个归一化位置和对应的颜色 */
    /* Each node defines a normalized position and corresponding color */
    static const ColorNode nodes[] = {
        {0.00f, cv::Vec3b(0,    0,    0)},       /* 黑色 | Black */
        {0.10f, cv::Vec3b(20,   0,   80)},       /* 深蓝 | Dark blue */
        {0.25f, cv::Vec3b(50,   0,  180)},       /* 蓝色 | Blue */
        {0.40f, cv::Vec3b(140,  0,  160)},       /* 紫色 | Purple */
        {0.55f, cv::Vec3b(200, 30,   30)},       /* 红色 | Red */
        {0.70f, cv::Vec3b(230, 130,   0)},       /* 橙色 | Orange */
        {0.85f, cv::Vec3b(255, 220,  50)},       /* 黄色 | Yellow */
        {1.00f, cv::Vec3b(255, 255, 255)},       /* 白色 | White */
    };

    /* 线性插值查找 | Linear interpolation lookup */
    for (int i = 0; i < 7; i++) {
        if (normalized_value <= nodes[i + 1].position) {
            float t = (normalized_value - nodes[i].position) /
                      (nodes[i + 1].position - nodes[i].position);
            return nodes[i].color + t * (nodes[i + 1].color - nodes[i].color);
        }
    }
    return nodes[7].color;
}

/**
 * @brief 生成彩虹配色方案 | Generate Rainbow color palette
 *
 * 彩虹: 蓝 -> 青 -> 绿 -> 黄 -> 红
 * Rainbow: Blue -> Cyan -> Green -> Yellow -> Red
 */
static cv::Vec3b rainbowColor(float v)
{
    v = std::max(0.0f, std::min(1.0f, v));
    /* 使用 OpenCV 的 HSV 到 BGR 转换 */
    /* Use OpenCV's HSV to BGR conversion */
    /* H: 0=红, 60=黄, 120=绿, 180=青, 240=蓝 */
    /* 反转: 低温=蓝(240), 高温=红(0) */
    /* Inverted: low temp=blue(240), high temp=red(0) */
    float hue = (1.0f - v) * 240.0f;
    cv::Mat hsv(1, 1, CV_8UC3, cv::Scalar(hue / 2, 255, 255));  /* OpenCV H 范围 0-180 */
    cv::Mat bgr;
    cv::cvtColor(hsv, bgr, cv::COLOR_HSV2BGR);
    return bgr.at<cv::Vec3b>(0, 0);
}

/**
 * @brief 生成灰度配色方案 | Generate Grayscale color palette
 */
static cv::Vec3b grayscaleColor(float v)
{
    v = std::max(0.0f, std::min(1.0f, v));
    uint8_t gray = (uint8_t)(v * 255);
    return cv::Vec3b(gray, gray, gray);
}

/**
 * @brief 生成热力配色方案 | Generate Hot color palette
 *
 * 黑色 -> 红色 -> 黄色 -> 白色
 * Black -> Red -> Yellow -> White
 */
static cv::Vec3b hotColor(float v)
{
    v = std::max(0.0f, std::min(1.0f, v));
    static const ColorNode nodes[] = {
        {0.00f, cv::Vec3b(0,   0,   0)},
        {0.35f, cv::Vec3b(0,   0, 200)},
        {0.65f, cv::Vec3b(0,  200, 255)},
        {1.00f, cv::Vec3b(255, 255, 255)},
    };
    for (int i = 0; i < 3; i++) {
        if (v <= nodes[i + 1].position) {
            float t = (v - nodes[i].position) / (nodes[i + 1].position - nodes[i].position);
            return nodes[i].color + t * (nodes[i + 1].color - nodes[i].color);
        }
    }
    return nodes[3].color;
}

/**
 * @brief 生成冷色配色方案 | Generate Cool color palette
 *
 * 深蓝 -> 青 -> 绿
 * Dark blue -> Cyan -> Green
 */
static cv::Vec3b coolColor(float v)
{
    v = std::max(0.0f, std::min(1.0f, v));
    uint8_t b = (uint8_t)((1.0f - v) * 255);
    uint8_t g = (uint8_t)(v * 255);
    uint8_t r = 0;
    return cv::Vec3b(b, g, r);
}

/* ============================================================
 * JNI 函数: 伪彩色映射 | JNI Function: Pseudo-color Mapping
 * ============================================================ */

/**
 * @brief 将温度数据映射为伪彩色图像 | Map temperature data to pseudo-color image
 *
 * 处理流程 | Processing flow:
 *   1. 归一化温度数据到 [0, 1] 范围
 *      Normalize temperature data to [0, 1] range
 *   2. 根据选择的配色方案映射为颜色
 *      Map to colors based on selected palette
 *   3. 将 32x24 的小图放大到显示分辨率 (双线性插值)
 *      Upscale 32x24 image to display resolution (bilinear interpolation)
 *
 * @param env          JNI 环境
 * @param thiz         Java 对象
 * @param thermalAddr  OpenCV Mat native 地址 (32x24, CV_32FC1, 温度值)
 * @param minTemp      显示范围最低温度
 * @param maxTemp      显示范围最高温度
 * @param paletteType  配色方案 (0=Ironbow, 1=Rainbow, 2=Grayscale, 3=Hot, 4=Cool)
 * @return             着色后的 Mat native 地址 (CV_8UC3, BGR)
 */
extern "C" JNIEXPORT jlong JNICALL
Java_com_example_thermaleyes_MainActivity_nativeApplyPseudoColor(
        JNIEnv *env, jobject thiz,
        jlong thermal_addr,
        jfloat min_temp, jfloat max_temp,
        jint palette_type)
{
    /* 获取温度数据 Mat | Get temperature data Mat */
    cv::Mat &thermal = *(cv::Mat *) thermal_addr;

    /* 创建着色图像 (与 thermal 相同尺寸) | Create color image (same size as thermal) */
    cv::Mat color_mat(thermal.rows, thermal.cols, CV_8UC3);

    /* ---- 步骤 1: 温度归一化和颜色映射 ---- */
    /* Step 1: Temperature normalization and color mapping */
    float range = max_temp - min_temp;
    if (range < 0.01f) range = 1.0f;  /* 防止除以零 | Prevent division by zero */

    for (int row = 0; row < thermal.rows; row++) {
        for (int col = 0; col < thermal.cols; col++) {
            float temp = thermal.at<float>(row, col);

            /* 归一化到 [0, 1] | Normalize to [0, 1] */
            float normalized = (temp - min_temp) / range;

            /* 根据配色方案映射颜色 | Map color based on palette */
            cv::Vec3b color;
            switch (palette_type) {
                case 0:  /* Ironbow */
                    color = ironbowColor(normalized);
                    break;
                case 1:  /* Rainbow */
                    color = rainbowColor(normalized);
                    break;
                case 2:  /* Grayscale */
                    color = grayscaleColor(normalized);
                    break;
                case 3:  /* Hot */
                    color = hotColor(normalized);
                    break;
                case 4:  /* Cool */
                    color = coolColor(normalized);
                    break;
                default:
                    color = ironbowColor(normalized);
                    break;
            }

            color_mat.at<cv::Vec3b>(row, col) = color;
        }
    }

    /* ---- 步骤 2: 放大到显示分辨率 ---- */
    /* Step 2: Upscale to display resolution */
    /*
     * MLX90640 只有 32x24 = 768 像素，直接显示会非常模糊
     * 使用双线性插值放大到 640x480 或更高分辨率
     *
     * MLX90640 has only 32x24 = 768 pixels, direct display would be very blurry
     * Use bilinear interpolation to upscale to 640x480 or higher
     */
    cv::Mat upscaled;
    cv::resize(color_mat, upscaled, cv::Size(640, 480), 0, 0, cv::INTER_LINEAR);

    /* 可选: 进一步高斯模糊使像素过渡更平滑 */
    /* Optional: Further Gaussian blur for smoother pixel transitions */
    cv::GaussianBlur(upscaled, upscaled, cv::Size(3, 3), 1.0);

    /* 创建新的 Mat 并返回地址 | Create new Mat and return address */
    cv::Mat *result = new cv::Mat(upscaled);
    return (jlong) result;
}

/* ============================================================
 * 图像融合算法 | Image Fusion Algorithm
 * ============================================================ */

/**
 * @brief 使用高斯模糊进行频率域分解融合
 *        Frequency domain decomposition fusion using Gaussian blur
 *
 * 算法原理 | Algorithm Principle:
 *
 *   1. 将图像 A (热成像) 和图像 B (可见光) 分别分解为高频和低频成分
 *      Decompose images A (thermal) and B (visible) into high and low frequency components
 *
 *   2. 分解方法: 使用高斯模糊
 *      Decomposition method: Using Gaussian blur
 *      - 低频 = 高斯模糊后的图像 (整体趋势/温度分布)
 *        Low-freq = Gaussian blurred image (overall trend/temperature distribution)
 *      - 高频 = 原图 - 低频 (细节/边缘信息)
 *        High-freq = Original - Low-freq (detail/edge information)
 *
 *   3. 融合规则:
 *      Fusion rules:
 *      - 低频: 取热成像的低频 (温度分布信息)
 *        Low-freq: Take thermal's low-freq (temperature distribution)
 *      - 高频: 取可见光的高频 (细节和边缘)
 *        High-freq: Take visible light's high-freq (detail and edges)
 *
 *   4. 融合结果 = 热成像低频 * w_low + 可见光高频 * w_high
 *      Fused result = thermal low-freq * w_low + visible high-freq * w_high
 *
 * @param thermal       热成像图像 (已着色, CV_8UC3)
 * @param visible       可见光图像 (CV_8UC3)
 * @param kernel_size   高斯核大小 (奇数)
 * @param sigma         高斯标准差
 * @param high_weight   高频权重 (可见光细节)
 * @param low_weight    低频权重 (热成像信息)
 * @return              融合后的图像
 */
static cv::Mat frequencyDecompositionFusion(
        const cv::Mat &thermal,
        const cv::Mat &visible,
        int kernel_size,
        double sigma,
        float high_weight,
        float low_weight)
{
    LOGI("频率域融合 | Frequency fusion: kernel=%d, sigma=%.1f, high_w=%.2f, low_w=%.2f",
         kernel_size, sigma, high_weight, low_weight);

    /* 确保两幅图像尺寸一致 | Ensure both images have same size */
    cv::Mat visible_resized;
    if (visible.size() != thermal.size()) {
        cv::resize(visible, visible_resized, thermal.size());
    } else {
        visible_resized = visible;
    }

    /* 转换为 float 类型进行计算 (避免精度损失) */
    /* Convert to float for calculation (avoid precision loss) */
    cv::Mat thermal_f, visible_f;
    thermal.convertTo(thermal_f, CV_32FC3);
    visible_resized.convertTo(visible_f, CV_32FC3);

    /* ---- 分解热成像图像 ---- */
    /* Decompose thermal image */

    /* 热成像低频: 高斯模糊 | Thermal low-freq: Gaussian blur */
    cv::Mat thermal_low;
    cv::GaussianBlur(thermal_f, thermal_low, cv::Size(kernel_size, kernel_size), sigma);

    /* 热成像高频: 原图 - 低频 | Thermal high-freq: original - low-freq */
    cv::Mat thermal_high = thermal_f - thermal_low;

    /* ---- 分解可见光图像 ---- */
    /* Decompose visible light image */

    /* 可见光低频 | Visible low-freq */
    cv::Mat visible_low;
    cv::GaussianBlur(visible_f, visible_low, cv::Size(kernel_size, kernel_size), sigma);

    /* 可见光高频 | Visible high-freq */
    cv::Mat visible_high = visible_f - visible_low;

    /* ---- 融合 ---- */
    /* Fusion */
    /*
     * 融合公式 | Fusion formula:
     *   fused = thermal_low * low_weight + visible_high * high_weight + thermal_high * 0.5
     *
     * 解释:
     *   - thermal_low * low_weight: 保留热成像的温度分布信息
     *     Preserve thermal temperature distribution information
     *   - visible_high * high_weight: 保留可见光的边缘和纹理细节
     *     Preserve visible light edges and texture details
     *   - thermal_high * 0.5: 部分保留热成像的细节
     *     Partially preserve thermal details
     */
    cv::Mat fused = thermal_low * low_weight + visible_high * high_weight;

    /* 钳制到 [0, 255] 范围 | Clamp to [0, 255] range */
    cv::Mat result;
    fused.convertTo(result, CV_8UC3);

    return result;
}

/* ============================================================
 * JNI 函数: 图像融合 | JNI Function: Image Fusion
 * ============================================================ */

/**
 * @brief 融合热成像和可见光图像 | Fuse thermal and visible light images
 *
 * @param env          JNI 环境
 * @param thiz         Java 对象
 * @param thermalAddr  热成像 Mat 地址 (CV_8UC3, 已着色)
 * @param visibleAddr  可见光 Mat 地址 (CV_8UC3)
 * @param mode         融合模式 (0=Overlay, 1=SideBySide, 2=Blend, 3=Frequency)
 * @param highWeight   高频权重
 * @param lowWeight    低频权重
 * @return             融合后的 Mat 地址
 */
extern "C" JNIEXPORT jlong JNICALL
Java_com_example_thermaleyes_MainActivity_nativeFuseImages(
        JNIEnv *env, jobject thiz,
        jlong thermal_addr,
        jlong visible_addr,
        jint mode,
        jfloat high_weight,
        jfloat low_weight)
{
    cv::Mat &thermal = *(cv::Mat *) thermal_addr;
    cv::Mat &visible = *(cv::Mat *) visible_addr;

    cv::Mat result;

    switch (mode) {
        case 0: {
            /* ---- 模式 0: 叠加 (Overlay) ---- */
            /* Mode 0: Overlay - thermal overlaid on visible with transparency */
            cv::Mat visible_resized;
            if (visible.size() != thermal.size()) {
                cv::resize(visible, visible_resized, thermal.size());
            } else {
                visible_resized = visible.clone();
            }

            /* 使用 alpha 混合 | Use alpha blending */
            /* result = thermal * alpha + visible * (1 - alpha) */
            double alpha = 0.6;  /* 热成像透明度 | Thermal transparency */
            cv::addWeighted(thermal, alpha, visible_resized, 1.0 - alpha, 0, result);
            break;
        }

        case 1: {
            /* ---- 模式 1: 并排 (Side by Side) ---- */
            /* Mode 1: Side by side display */
            cv::Mat thermal_resized, visible_resized;
            int half_width = visible.cols / 2;
            int height = visible.rows;

            cv::resize(thermal, thermal_resized, cv::Size(half_width, height));
            visible_resized = visible.clone();

            /* 左边放热成像，右边放可见光 */
            /* Thermal on left, visible on right */
            result = visible_resized.clone();
            thermal_resized.copyTo(result(cv::Rect(0, 0, half_width, height)));
            break;
        }

        case 2: {
            /* ---- 模式 2: 加权混合 (Blend) ---- */
            /* Mode 2: Weighted blending */
            cv::Mat visible_resized;
            if (visible.size() != thermal.size()) {
                cv::resize(visible, visible_resized, thermal.size());
            } else {
                visible_resized = visible.clone();
            }

            /* 简单加权平均 | Simple weighted average */
            cv::addWeighted(thermal, 0.5, visible_resized, 0.5, 0, result);
            break;
        }

        case 3: {
            /* ---- 模式 3: 频率域分解融合 (Frequency Domain) ---- */
            /* Mode 3: Frequency domain decomposition fusion */
            result = frequencyDecompositionFusion(
                thermal, visible,
                15,           /* 高斯核大小 | Gaussian kernel size */
                5.0,          /* sigma */
                high_weight,  /* 高频权重 | High-freq weight */
                low_weight    /* 低频权重 | Low-freq weight */
            );
            break;
        }

        default:
            result = thermal.clone();
            break;
    }

    cv::Mat *output = new cv::Mat(result);
    return (jlong) output;
}
