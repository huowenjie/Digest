#ifndef __SM3_H__
#define __SM3_H__

#include "global.h"

// 摘要分组长度
#define SM3_BLOCK 64

typedef struct MD5_CTX_st {
    // 8 个 32 位链接变量(或寄存器值)
    uint32_t A;
    uint32_t B;
    uint32_t C;
    uint32_t D;
    uint32_t E;
    uint32_t F;
    uint32_t G;
    uint32_t H;

    uint64_t msg_len;           // 消息摘要总长度
    uint32_t len;               // 每一次 update 之后 buff 缓冲区里的剩余数据长度
    uint8_t  buff[SM3_BLOCK];   // 运算缓冲区
} SM3_CTX;

/*
 * 单次运算
 * 
 * msg[in]:         消息摘要原文
 * in_len[in]:      消息摘要原文长度
 * result[out/in]:  消息摘要
 * len[out/in]:     消息摘要长度
 * 
 * 如果 result 为空，函数将消息摘要长度赋给 *len
 * 成功，返回0；失败返回 -1
 */
int sm3_msg(const uint8_t *msg, uint32_t in_len, uint8_t *result, uint32_t *len);

/*
 * 分组摘要运算初始化
 * 
 * ctx[in]:         消息摘要上下文
 * 成功，返回0；失败返回 -1
 */
int sm3_init(SM3_CTX *ctx);

/*
 * 分组摘要运算
 * 
 * ctx[in]: 消息摘要上下文
 * msg[in]: 消息摘要原文
 * len[in]: 消息摘要原文长度
 * 
 * 成功，返回0；失败返回 -1
 */
int sm3_update(SM3_CTX *ctx, const uint8_t *msg, uint32_t len);

/*
 * 分组摘要运算最终处理
 * 
 * ctx[in]:         消息摘要上下文
 * result[out/in]:  消息摘要
 * len[out/in]:     消息摘要长度
 * 
 * 如果 result 为空，函数将消息摘要长度赋给 *len
 * 成功，返回0；失败返回 -1
 */
int sm3_final(SM3_CTX *ctx, uint8_t *result, uint32_t *len);

#endif // __SM3_H__
