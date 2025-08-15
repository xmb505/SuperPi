# SuperPi

一个简洁高效的Linux平台圆周率计算工具，专为CPU稳定性测试设计。

## 简介

SuperPi是一个用C语言编写的轻量级圆周率计算程序，旨在为Linux用户提供类似Windows平台Super PI的CPU稳定性测试体验。该软件使用Gauss-Legendre算法计算指定精度的圆周率值，并将结果保存到文本文件中，方便用户验证计算结果和评估CPU性能。

## 特性

- 📦 **易安装**：通过星火商店一键安装
- 📄 **简洁输出**：结果直接保存为文本文件
- ⚡ **高性能算法**：采用Gauss-Legendre算法，计算速度快
- 🧮 **高精度计算**：使用GMP库进行任意精度计算
- 🔧 **多种计算模式**：支持单次计算和持续计算模式

## 安装

### 通过星火商店安装（推荐）

在星火商店中搜索"SuperPi"并点击安装，或在终端中执行：

```bash
sudo aptss install superpi
```

### 手动编译安装

```bash
git clone https://github.com/xmb505/SuperPi
cd superpi
make all
```

## 使用方法

基本用法：
```bash
superpi [选项] [计算位数]
```

选项：
- `-h, --help`：显示帮助信息
- `-v, --version`：显示版本信息
- `-k, --keep`：持续计算模式

示例：
```bash
# 计算100万位圆周率
superpi 1000000

# 计算1000万位圆周率
superpi 10000000

# 持续计算模式（按Ctrl+C停止）
superpi --keep
```

执行后，程序将在当前工作目录下生成一个名为`圆周率_位数.text`的文件，包含计算得到的圆周率值。

## 系统要求

- Linux操作系统（Ubuntu 18.04+、Debian 10+、或其他兼容发行版）
- GCC编译器
- GMP库（用于高精度计算）
- FFTW3库（用于计算优化）
- 最低512MB内存（推荐2GB以上用于大位数计算）

## 性能测试

SuperPi的计算时间主要取决于：
- CPU性能（核心数、频率、架构）
- 内存带宽和延迟
- 计算位数

典型性能参考（Intel i7-12700K）：
- 100万位：约0.5-1秒
- 1000万位：约5-10秒
- 1亿位：约1-2分钟

## 文件说明

计算完成后生成的文件格式：
```
圆周率_1000000位.text
3.14159265358979323846264338327950288419716939937510...
```

## 开发

### 构建依赖

```bash
sudo apt-get update
sudo apt-get install build-essential libgmp-dev libfftw3-dev
```

### 编译

```bash
make all
```
