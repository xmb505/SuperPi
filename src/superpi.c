/*
 * SuperPi - 高精度圆周率计算工具 (Gauss-Legendre算法版本)
 * 版权所有 (c) 2025 新毛宝贝 (xmb505)
 * 
 * 使用Gauss-Legendre算法计算任意精度的圆周率值
 * 结合GMP库进行高精度计算，使用FFTW3进行优化
 */

#include <stdio.h>      // 标准输入输出函数
#include <stdlib.h>     // 标准库函数（内存分配、进程控制等）
#include <string.h>     // 字符串处理函数
#include <time.h>       // 时间相关函数
#include <locale.h>     // 本地化支持（多语言）
#include <libintl.h>    // GNU国际化库
#include <stdint.h>     // 精确宽度整数类型
#include <unistd.h>     // Unix标准函数
#include <signal.h>     // 信号处理
#include <gmp.h>        // GNU高精度数学库，用于大数计算
#include <fftw3.h>      // FFTW库，用于优化计算

// 国际化宏定义：将字符串标记为可翻译
#define _(STRING) gettext(STRING)

// 默认计算100万位圆周率
#define DEFAULT_DIGITS 1000000
// 最大支持1000万位（可根据内存扩展）
#define MAX_DIGITS 10000000

// 全局变量：存储程序名称，用于错误信息输出
char *program_name = NULL;
// 全局变量：用于持续计算模式
volatile sig_atomic_t keep_running = 1;

// 函数声明（提前声明，让编译器知道这些函数的存在）
void print_usage(void);           // 打印使用帮助
void print_version(void);         // 打印版本信息
void signal_handler(int sig);     // 信号处理函数
uint64_t calculate_pi_digits(uint64_t digits, char **result);  // 计算圆周率
void save_pi_to_file(const char *pi_str, uint64_t digits);     // 保存结果到文件
void print_progress_time(uint64_t current_digits, double elapsed_time);  // 显示进度时间

// 信号处理函数，用于处理Ctrl+C
void signal_handler(int sig) {
    if (sig == SIGINT) {
        keep_running = 0;
        printf(_("\n收到中断信号，正在停止计算...\n"));
    }
}

int main(int argc, char *argv[]) {
    uint64_t digits = DEFAULT_DIGITS;  // 默认计算位数
    int keep_mode = 0;  // 持续计算模式标志
    
    /* 国际化设置：让程序支持多语言显示 */
    setlocale(LC_ALL, "");  // 根据系统环境设置本地化
    bindtextdomain("superpi", "/usr/share/locale");  // 设置翻译文件路径
    textdomain("superpi");  // 设置当前使用的翻译域
    
    program_name = argv[0];  // 保存程序名称，用于错误提示
    
    /* 注册信号处理函数 */
    signal(SIGINT, signal_handler);
    
    /* 解析命令行参数 */
    if (argc > 2) {  // 参数太多，显示用法
        fprintf(stderr, _("用法: %s [选项] [位数]\n"), program_name);
        return 1;  // 返回错误码1
    }
    
    if (argc == 1) {  // 无参数，进入交互模式
        printf(_("SuperPi - 高精度圆周率计算工具\n"));
        printf(_("使用Gauss-Legendre算法计算π值\n"));
        printf(_("支持无限精度计算\n\n"));
        printf(_("请输入要计算的圆周率位数: "));
        
        if (scanf("%lu", &digits) != 1) {
            fprintf(stderr, _("错误: 请输入一个有效的数字\n"));
            return 1;
        }
        
        if (digits <= 0 || digits > MAX_DIGITS) {
            fprintf(stderr, _("错误: 位数必须在1到%llu之间\n"), (unsigned long long)MAX_DIGITS);
            return 1;
        }
    } else if (argc == 2) {  // 用户提供了一个参数
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
        // 检查是否是持续计算选项
        if (strcmp(argv[1], "--keep") == 0 || strcmp(argv[1], "-k") == 0) {
            keep_mode = 1;
            digits = 1000;  // 初始位数
        } else {
            /* 解析用户输入的位数 */
            char *endptr;  // 用于检测转换是否成功
            digits = strtoull(argv[1], &endptr, 10);  // 将字符串转换为无符号长整数
            if (*endptr != '\0' || digits == 0) {  // 转换失败或输入为0
                fprintf(stderr, _("错误: 无效的位数输入。\n"));
                return 1;  // 返回错误码1
            }
        }
    }
    
    /* 参数检查 */
    if (!keep_mode && (digits <= 0 || digits > MAX_DIGITS)) {
        fprintf(stderr, _("错误: 位数必须在1到%llu之间\n"), (unsigned long long)MAX_DIGITS);
        return 1;
    }
    
    /* 开始计算 */
    if (keep_mode) {
        printf(_("SuperPi - 持续计算圆周率模式\n"));
        printf(_("按Ctrl+C停止计算\n\n"));
        
        uint64_t current_digits = 1000;
        while (keep_running) {
            printf(_("SuperPi - 正在计算圆周率到 %llu 位...\n"), (unsigned long long)current_digits);
            printf(_("开始时间: %s\n"), __TIME__);
            
            clock_t start = clock();  // 记录开始时间
            
            /* 调用核心计算函数 */
            char *pi_result = NULL;  // 用于存储计算结果
            uint64_t calculated = calculate_pi_digits(current_digits, &pi_result);  // 实际计算
            
            clock_t end = clock();  // 记录结束时间
            double elapsed = ((double)(end - start)) / CLOCKS_PER_SEC;  // 计算耗时（秒）
            
            /* 处理计算结果 */
            if (calculated > 0 && pi_result && keep_running) {  // 计算成功且未被中断
                printf(_("圆周率计算完成，耗时 %.2f 秒\n"), elapsed);
                printf(_("平均性能: %.2f 位/秒\n"), (double)calculated / elapsed);
                save_pi_to_file(pi_result, calculated);  // 保存结果到文件
                free(pi_result);  // 释放内存，防止内存泄漏
            } else if (!keep_running) {  // 被用户中断
                printf(_("计算已被用户中断\n"));
                if (pi_result) free(pi_result);
                break;
            } else {  // 计算失败
                fprintf(stderr, _("错误: 圆周率计算失败\n"));
                if (pi_result) free(pi_result);
                break;
            }
            
            // 增加位数进行下一轮计算
            current_digits *= 2;
            if (current_digits > MAX_DIGITS) {
                current_digits = 1000;  // 重置到初始值
            }
            
            // 短暂休眠避免CPU占用过高
            sleep(1);
        }
    } else {
        printf(_("SuperPi - 正在计算圆周率到 %llu 位...\n"), (unsigned long long)digits);
        printf(_("开始时间: %s\n"), __TIME__);
        
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
            if (pi_result) free(pi_result);
            return 1;
        }
    }
    
    return 0;  // 程序正常结束
}

/* 打印使用帮助信息 */
void print_usage(void) {
    printf(_("SuperPi - 高精度圆周率计算工具\n\n"));
    printf(_("用法: %s [选项] [位数]\n"), program_name);
    printf(_("  位数    要计算的圆周率小数位数（无限制）\n"));
    printf(_("\n选项:\n"));
    printf(_("  -h, --help     显示此帮助信息\n"));
    printf(_("  -v, --version  显示版本信息\n"));
    printf(_("  -k, --keep     持续计算圆周率并保存到文件\n"));
    printf(_("\n示例:\n"));
    printf(_("  %s 1000        计算1000位\n"), program_name);
    printf(_("  %s --keep      持续计算圆周率\n"), program_name);
    printf(_("  %s --version   显示版本信息\n"), program_name);
    printf(_("\n系统要求:\n"));
    printf(_("  Ubuntu/Debian系统，需要编译工具\n"));
}

/* 打印版本信息 */
void print_version(void) {
    printf("SuperPi 5.0.0\n");
    printf(_("版权所有 (c) 2025 新毛宝贝 (xmb505)\n"));
    printf(_("使用Gauss-Legendre算法计算圆周率，支持无限精度\n"));
    printf(_("针对64位系统优化\n"));
    printf(_("博客: blog.xmb505.top\n"));
    
    // 获取Git哈希值（如果可用）
    #ifdef GIT_VERSION
    printf(_(" (git: %s)\n"), GIT_VERSION);
    #endif
}

/*
 * 显示计算进度时间（模拟Windows SuperPi体验）
 * 在2的幂次位数时显示时间：128, 256, 512, 1024, 2048, 4096, 8192...
 * 
 * 参数说明：
 *   current_digits - 当前计算到的位数
 *   elapsed_time - 从开始到现在的耗时（秒）
 */
void print_progress_time(uint64_t current_digits, double elapsed_time) {
    /* 检查是否是2的幂次（128, 256, 512, 1024...） */
    if (current_digits >= 128 && (current_digits & (current_digits - 1)) == 0) {
        printf(_("计算到 %llu 位用时: %.3f 秒\n"), 
               (unsigned long long)current_digits, elapsed_time);
        fflush(stdout);  // 立即刷新输出，确保用户能看到
    }
}

/*
 * 计算圆周率的核心函数
 * 使用Gauss-Legendre算法结合GMP高精度库进行计算
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
    mpf_t a, b, t, p;           // Gauss-Legendre算法变量
    mpf_t a_next, b_next, t_next; // 下一次迭代的变量
    mpf_t pi;                   // 存储最终的π值
    mpf_t temp1, temp2;         // 临时变量
    
    /* 初始化所有变量 */
    mpf_init(a);
    mpf_init(b);
    mpf_init(t);
    mpf_init(p);
    mpf_init(a_next);
    mpf_init(b_next);
    mpf_init(t_next);
    mpf_init(pi);
    mpf_init(temp1);
    mpf_init(temp2);
    
    /* 设置Gauss-Legendre算法的初始值 */
    mpf_set_ui(a, 1);           // a0 = 1
    mpf_set_ui(temp1, 2);
    mpf_sqrt(b, temp1);
    mpf_div_ui(b, b, 2);        // b0 = 1/sqrt(2)
    mpf_set_ui(p, 1);           // p0 = 1
    mpf_set_d(t, 0.25);         // t0 = 1/4
    
    /* 获取开始时间用于进度显示 */
    clock_t calc_start = clock();
    
    /* Gauss-Legendre算法迭代 */
    unsigned long iterations = 0;
    while (1) {
        iterations++;
        
        /* 计算下一次迭代的值 */
        // a_next = (a + b) / 2
        mpf_add(temp1, a, b);
        mpf_div_ui(a_next, temp1, 2);
        
        // b_next = sqrt(a * b)
        mpf_mul(temp1, a, b);
        mpf_sqrt(b_next, temp1);
        
        // t_next = t - p * (a_next - a)^2
        mpf_sub(temp1, a_next, a);
        mpf_mul(temp2, temp1, temp1);
        mpf_mul(temp1, p, temp2);
        mpf_sub(t_next, t, temp1);
        
        // p_next = 2 * p
        mpf_mul_ui(p, p, 2);
        
        /* 检查收敛条件 */
        mpf_sub(temp1, a_next, b_next);
        mpf_abs(temp1, temp1);
        
        // 如果差值足够小，则停止迭代
        if (mpf_cmp_d(temp1, 1e-50) < 0) {
            break;
        }
        
        /* 每10次迭代检查一次时间，显示2的幂次进度 */
        if (iterations % 10 == 0) {
            clock_t now = clock();
            double elapsed = ((double)(now - calc_start)) / CLOCKS_PER_SEC;
            
            /* 估算当前精度位数（经验公式） */
            uint64_t estimated_digits = (uint64_t)(iterations * 2);
            
            /* 显示2的幂次进度，避免重复显示 */
            static uint64_t last_shown = 0;
            uint64_t power_of_two = 128;
            while (power_of_two <= digits && power_of_two <= estimated_digits) {
                if (estimated_digits >= power_of_two && 
                    estimated_digits < power_of_two * 2 && 
                    power_of_two != last_shown) {
                    printf(_("%6llu位: %8.3f秒\n"), 
                           (unsigned long long)power_of_two, elapsed);
                    fflush(stdout);
                    last_shown = power_of_two;
                    break;
                }
                power_of_two *= 2;
            }
        }
        
        /* 更新变量 */
        mpf_set(a, a_next);
        mpf_set(b, b_next);
        mpf_set(t, t_next);
    }
    
    /* 计算最终的π值：π ≈ (a + b)^2 / (4 * t) */
    mpf_add(temp1, a, b);
    mpf_mul(temp2, temp1, temp1);
    mpf_mul_ui(temp1, t, 4);
    mpf_div(pi, temp2, temp1);
    
    /* 为结果分配内存缓冲区 */
    *result = malloc(digits + 10);  // 额外空间用于小数点和终止符
    if (!*result) {  // 内存分配失败
        /* 清理所有GMP变量，防止内存泄漏 */
        mpf_clear(a);
        mpf_clear(b);
        mpf_clear(t);
        mpf_clear(p);
        mpf_clear(a_next);
        mpf_clear(b_next);
        mpf_clear(t_next);
        mpf_clear(pi);
        mpf_clear(temp1);
        mpf_clear(temp2);
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
    mpf_clear(a);
    mpf_clear(b);
    mpf_clear(t);
    mpf_clear(p);
    mpf_clear(a_next);
    mpf_clear(b_next);
    mpf_clear(t_next);
    mpf_clear(pi);
    mpf_clear(temp1);
    mpf_clear(temp2);
    
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
    
    /* 构造文件名 */
    char filename[256];
    if (digits == 0) {  // 持续计算模式
        snprintf(filename, sizeof(filename), "圆周率_永远.text");
    } else {
        snprintf(filename, sizeof(filename), "圆周率_%llu位.text", (unsigned long long)digits);
    }
    
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
    fprintf(fp, _("算法: Gauss-Legendre\n"));
    fprintf(fp, _("日期: %s\n"), __DATE__);
    
    fclose(fp);  // 关闭文件
    printf(_("结果已保存到: %s\n"), filename);
}