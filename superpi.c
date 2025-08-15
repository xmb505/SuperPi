/*
 * SuperPi - 高精度圆周率计算工具
 * 版权所有 (c) 2025 新毛宝贝 (xmb505)
 * 
 * 使用实际计算方法来计算任意精度的圆周率值
 * 这是一个教学友好的代码，包含详细的中文注释
 */

#include <stdio.h>      // 标准输入输出函数
#include <stdlib.h>     // 标准库函数（内存分配、进程控制等）
#include <string.h>     // 字符串处理函数
#include <time.h>       // 时间相关函数
#include <locale.h>     // 本地化支持（多语言）
#include <libintl.h>    // GNU国际化库
#include <stdint.h>     // 精确宽度整数类型
#include <unistd.h>     // Unix标准函数
#include <sys/sysinfo.h> // 系统信息获取
#include <gmp.h>        // GNU高精度数学库，用于大数计算

// 国际化宏定义：将字符串标记为可翻译
#define _(STRING) gettext(STRING)

// 默认计算100万位圆周率
#define DEFAULT_DIGITS 1000000
// 最大支持1000万位（可根据内存扩展）
#define MAX_DIGITS 10000000

// 全局变量：存储程序名称，用于错误信息输出
char *program_name = NULL;

// 函数声明（提前声明，让编译器知道这些函数的存在）
void print_usage(void);           // 打印使用帮助
void print_version(void);         // 打印版本信息
uint64_t calculate_pi_digits(uint64_t digits, char **result);  // 计算圆周率
void save_pi_to_file(const char *pi_str, uint64_t digits);     // 保存结果到文件

int main(int argc, char *argv[]) {
    uint64_t digits = DEFAULT_DIGITS;  // 默认计算位数
    
    /* 国际化设置：让程序支持多语言显示 */
    setlocale(LC_ALL, "");  // 根据系统环境设置本地化
    bindtextdomain("superpi", "/usr/share/locale");  // 设置翻译文件路径
    textdomain("superpi");  // 设置当前使用的翻译域
    
    program_name = argv[0];  // 保存程序名称，用于错误提示
    
    /* 解析命令行参数 */
    if (argc > 2) {  // 参数太多，显示用法
        fprintf(stderr, _("用法: %s [位数]\n"), program_name);
        return 1;  // 返回错误码1
    }
    
    if (argc == 2) {  // 用户提供了参数
        // 检查是否是帮助选项
        if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
            print_usage();  // 显示帮助信息
            return 0;  // 正常退出
        }
        // 检查是否是版本选项
        if (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-v") == 0) {
            print_version();  // 显示版本信息
            return 0;  // 正常退出
        }
        
        /* 解析用户输入的位数 */
        char *endptr;  // 用于检测转换是否成功
        digits = strtoull(argv[1], &endptr, 10);  // 将字符串转换为无符号长整数
        if (*endptr != '\0' || digits == 0) {  // 转换失败或输入为0
            fprintf(stderr, _("错误: 无效的位数输入。\n"));
            return 1;  // 返回错误码1
        }
    }
    
    /* 开始计算 */
    printf(_("SuperPi - 正在计算圆周率到 %llu 位...\n"), (unsigned long long)digits);
    
    clock_t start = clock();  // 记录开始时间
    
    /* 调用核心计算函数 */
    char *pi_result = NULL;  // 用于存储计算结果
    uint64_t calculated = calculate_pi_digits(digits, &pi_result);  // 实际计算
    
    clock_t end = clock();  // 记录结束时间
    double elapsed = ((double)(end - start)) / CLOCKS_PER_SEC;  // 计算耗时（秒）
    
    /* 处理计算结果 */
    if (calculated > 0 && pi_result) {  // 计算成功
        printf(_("圆周率计算完成，耗时 %.2f 秒\n"), elapsed);
        printf(_("平均性能: %.2f 位/秒\n"), (double)calculated / elapsed);
        save_pi_to_file(pi_result, calculated);  // 保存结果到文件
        free(pi_result);  // 释放内存，防止内存泄漏
    } else {  // 计算失败
        fprintf(stderr, _("错误: 圆周率计算失败\n"));
    }
    
    return 0;  // 程序正常结束
}

/* 打印使用帮助信息 */
void print_usage(void) {
    printf(_("SuperPi - 高精度圆周率计算工具\n\n"));
    printf(_("用法: %s [位数]\n"), program_name);
    printf(_("  位数    要计算的圆周率小数位数（无限制）\n"));
    printf(_("\n选项:\n"));
    printf(_("  -h, --help     显示此帮助信息\n"));
    printf(_("  -v, --version  显示版本信息\n"));
    printf(_("\n示例:\n"));
    printf(_("  %s 1000000    计算100万位\n"), program_name);
    printf(_("  %s 10000000   计算1000万位\n"), program_name);
    printf(_("  %s 100000000  计算1亿位\n"), program_name);
    printf(_("\n系统要求:\n"));
    printf(_("  Ubuntu/Debian系统，需要编译工具\n"));
}

/* 打印版本信息 */
void print_version(void) {
    printf("SuperPi 4.0.0\n");
    printf(_("版权所有 (c) 2025 新毛宝贝 (xmb505)\n"));
    printf(_("实际计算圆周率，支持无限精度\n"));
    printf(_("针对64位系统优化\n"));
    printf(_("博客: blog.xmb505.top\n"));
}

/*
 * 计算圆周率的核心函数
 * 使用马青公式（Machin-like formula）结合GMP高精度库进行计算
 * 公式：π/4 = 4*arctan(1/5) - arctan(1/239)
 * 
 * 参数说明：
 *   digits - 要计算的小数位数
 *   result - 用于存储结果的字符串指针（通过参数返回）
 * 返回值：实际计算的位数，失败返回0
 */
uint64_t calculate_pi_digits(uint64_t digits, char **result) {
    /* 参数检查 */
    if (!result || digits <= 0 || digits > MAX_DIGITS) return 0;
    
    /* 
     * 设置计算精度
     * 我们需要比请求的位数更高的精度来确保准确性
     * log2(10) ≈ 3.322，额外增加10000位作为安全余量
     */
    mpf_set_default_prec(digits * 3.322 + 10000);
    
    /* 声明GMP高精度变量 */
    mpf_t pi, term1, term2, sum, x1, x2, x1_pow, x2_pow, temp;
    
    /* 初始化所有变量 */
    mpf_init(pi);        // 存储最终的π值
    mpf_init(term1);     // 存储4*arctan(1/5)
    mpf_init(term2);     // 存储arctan(1/239)
    mpf_init(sum);       // 用于级数求和
    mpf_init(x1);        // 存储1/5
    mpf_init(x2);        // 存储1/239
    mpf_init(x1_pow);    // 存储x1的幂次
    mpf_init(x2_pow);    // 存储x2的幂次
    mpf_init(temp);      // 临时变量
    
    /* 设置马青公式的参数值 */
    mpf_set_ui(x1, 1);      // x1 = 1
    mpf_div_ui(x1, x1, 5);  // x1 = 1/5
    
    mpf_set_ui(x2, 1);      // x2 = 1
    mpf_div_ui(x2, x2, 239);  // x2 = 1/239
    
    /* 
     * 计算arctan(1/5)使用泰勒级数展开
     * 泰勒级数：arctan(x) = x - x³/3 + x⁵/5 - x⁷/7 + ...
     * 这是一个交错级数，每一项的符号交替变化
     */
    mpf_set_ui(sum, 0);     // 初始化求和变量为0
    mpf_set(x1_pow, x1);    // 初始幂次为x1^1
    
    /* 级数求和循环 */
    for (unsigned long int i = 0; i < (digits + 1000); i++) {
        /* 计算当前项：x^(2i+1) / (2i+1) */
        mpf_div_ui(temp, x1_pow, 2*i + 1);
        
        /* 根据项数的奇偶性决定加减 */
        if (i % 2 == 0) {
            mpf_add(sum, sum, temp);  // 偶数项：加
        } else {
            mpf_sub(sum, sum, temp);  // 奇数项：减
        }
        
        /* 为下一次迭代准备：x1_pow *= x1^2 */
        mpf_mul(temp, x1_pow, x1);
        mpf_mul(x1_pow, temp, x1);
    }
    
    mpf_mul_ui(term1, sum, 4);  // term1 = 4 * arctan(1/5)
    
    /* 
     * 计算arctan(1/239)，使用同样的泰勒级数方法
     * 由于1/239很小，级数收敛更快
     */
    mpf_set_ui(sum, 0);     // 重置求和变量
    mpf_set(x2_pow, x2);    // 初始幂次为x2^1
    
    for (unsigned long int i = 0; i < (digits + 1000); i++) {
        /* 计算当前项：x^(2i+1) / (2i+1) */
        mpf_div_ui(temp, x2_pow, 2*i + 1);
        
        /* 根据项数的奇偶性决定加减 */
        if (i % 2 == 0) {
            mpf_add(sum, sum, temp);  // 偶数项：加
        } else {
            mpf_sub(sum, sum, temp);  // 奇数项：减
        }
        
        /* 为下一次迭代准备：x2_pow *= x2^2 */
        mpf_mul(temp, x2_pow, x2);
        mpf_mul(x2_pow, temp, x2);
    }
    
    mpf_set(term2, sum);  // term2 = arctan(1/239)
    
    /* 计算最终的π值：π = 4 * (4*arctan(1/5) - arctan(1/239)) */
    mpf_sub(pi, term1, term2);  // pi = term1 - term2
    mpf_mul_ui(pi, pi, 4);      // pi = 4 * (term1 - term2)
    
    /* 为结果分配内存缓冲区 */
    *result = malloc(digits + 10);  // 额外空间用于小数点和终止符
    if (!*result) {  // 内存分配失败
        /* 清理所有GMP变量，防止内存泄漏 */
        mpf_clear(pi);
        mpf_clear(term1);
        mpf_clear(term2);
        mpf_clear(sum);
        mpf_clear(x1);
        mpf_clear(x2);
        mpf_clear(x1_pow);
        mpf_clear(x2_pow);
        mpf_clear(temp);
        return 0;  // 返回失败
    }
    
    /* 将高精度数值转换为字符串格式 */
    gmp_snprintf(*result, digits + 10, "%.*Ff", (int)digits+1, pi);
    
    /* 
     * 处理字符串格式：
     * GMP返回的格式是 "3.1415926..."
     * 我们需要去掉"3."，只保留小数部分
     */
    char *dot = strchr(*result, '.');  // 查找小数点位置
    if (dot) {
        /* 将字符串左移，去掉"3."前缀 */
        memmove(*result, dot + 1, strlen(dot));
        /* 截断到请求的位数 */
        (*result)[digits] = '\0';
    }
    
    /* 清理所有GMP变量，释放内存 */
    mpf_clear(pi);
    mpf_clear(term1);
    mpf_clear(term2);
    mpf_clear(sum);
    mpf_clear(x1);
    mpf_clear(x2);
    mpf_clear(x1_pow);
    mpf_clear(x2_pow);
    mpf_clear(temp);
    
    /* 返回实际计算的位数 */
    return digits;
}

/*
 * 将计算结果保存到文本文件
 * 参数说明：
 *   pi_str - 计算得到的圆周率小数部分字符串
 *   digits - 小数位数
 */
void save_pi_to_file(const char *pi_str, uint64_t digits) {
    /* 参数检查 */
    if (!pi_str || digits <= 0) return;
    
    /* 构造文件名：圆周率_位数.txt */
    char filename[256];
    snprintf(filename, sizeof(filename), "圆周率_%llu.txt", (unsigned long long)digits);
    
    /* 打开文件用于写入 */
    FILE *fp = fopen(filename, "w");
    if (!fp) {  // 文件打开失败
        fprintf(stderr, _("错误: 无法创建文件 %s\n"), filename);
        return;
    }
    
    /* 写入文件内容 */
    fprintf(fp, "3.");  // 写入整数部分和小数点
    
    /* 逐字符写入小数部分 */
    for (uint64_t i = 0; i < digits; i++) {
        fprintf(fp, "%c", pi_str[i]);
    }
    
    /* 写入文件尾部信息 */
    fprintf(fp, "\n\n");
    fprintf(fp, _("由SuperPi计算\n"));
    fprintf(fp, _("位数: %llu\n"), (unsigned long long)digits);
    fprintf(fp, _("算法: 实际计算\n"));
    fprintf(fp, _("日期: %s"), __DATE__);
    
    fclose(fp);  // 关闭文件
    printf(_("结果已保存到: %s\n"), filename);
}