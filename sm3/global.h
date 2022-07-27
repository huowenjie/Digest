#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <stdint.h>

#define SUCCESS 0
#define FAILED  -1

#define ERROR_LOCATE(str) \
    error_locate((str),  __FILE__, __FUNCTION__, __LINE__);

// 打印二进制数据
void print_bin_info(const char *name, uint8_t *data, uint32_t len);

// 定位错误
void error_locate(
    const char *info, 
    const char *file, 
    const char *func, 
    int         line);

#endif // __GLOBAL_H__
