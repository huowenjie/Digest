#include "sm3.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// 定义常量 Ti
#define DEF_TI(nm, t1, t2) static uint32_t nm[64] = { \
        t1, t1, t1, t1, t1, t1, t1, t1,\
        t1, t1, t1, t1, t1, t1, t1, t1,\
        t2, t2, t2, t2, t2, t2, t2, t2,\
        t2, t2, t2, t2, t2, t2, t2, t2,\
        t2, t2, t2, t2, t2, t2, t2, t2,\
        t2, t2, t2, t2, t2, t2, t2, t2,\
        t2, t2, t2, t2, t2, t2, t2, t2,\
        t2, t2, t2, t2, t2, t2, t2, t2 \
    };

//DEF_TI(t, 0x1945cc79, 0x8a9d877a)
DEF_TI(t, 0x79cc4519, 0x7a879d8a)

// 循环左移
static inline uint32_t shift_left_loop(uint32_t data, int num)
{
    uint32_t ret = 0;

    num = num % 32;
    ret = (data << num) | (data >> (32 - num));
    return ret;
}

// 转换64位整数的字节序
static inline uint64_t change_byte_order64(uint64_t num)
{
    // 0x1234567812345678
    uint64_t ret = 0;

    ret |= ((num & (uint64_t)0x00000000000000ff) << 56);
    ret |= ((num & (uint64_t)0x000000000000ff00) << 40);
    ret |= ((num & (uint64_t)0x0000000000ff0000) << 24);
    ret |= ((num & (uint64_t)0x00000000ff000000) << 8);
    ret |= ((num & (uint64_t)0xff00000000000000) >> 56);
    ret |= ((num & (uint64_t)0x00ff000000000000) >> 40);
    ret |= ((num & (uint64_t)0x0000ff0000000000) >> 24);
    ret |= ((num & (uint64_t)0x000000ff00000000) >> 8);

    return  ret;
}

// 转换32位整数的字节序
static inline uint32_t change_byte_order32(uint32_t num)
{
    // 0x12345678
    uint32_t ret = 0;

    ret |= ((num & (uint32_t)0x000000ff) << 24);
    ret |= ((num & (uint32_t)0x0000ff00) << 8);
    ret |= ((num & (uint32_t)0x00ff0000) >> 8);
    ret |= ((num & (uint32_t)0xff000000) >> 24);

    return  ret;
}

// 判断当前计算机字节序是否是小端法
static inline int is_little_endian()
{
    uint16_t num = 0x0001;
    return ((uint8_t *)(&num))[0] ? 1 : 0;
}

// 4 个布尔函数

// ff 0~15
static inline uint32_t sm3_calc_ff1(uint32_t x, uint32_t y, uint32_t z)
{
    return x ^ y ^ z;
}

// ff 16~63
static inline uint32_t sm3_calc_ff2(uint32_t x, uint32_t y, uint32_t z)
{
    return (x & y) | (x & z) | (y & z);
}

// gg 0~15
static inline uint32_t sm3_calc_gg1(uint32_t x, uint32_t y, uint32_t z)
{
    return x ^ y ^ z;
}

// gg 16~63
static inline uint32_t sm3_calc_gg2(uint32_t x, uint32_t y, uint32_t z)
{
    return (x & y) | ((~x) & z);
}

// 2个置换函数
static inline uint32_t sm3_calc_rep0(uint32_t x)
{
    return x ^ shift_left_loop(x, 9) ^ shift_left_loop(x, 17);
}

static inline uint32_t sm3_calc_rep1(uint32_t x)
{
    return x ^ shift_left_loop(x, 15) ^ shift_left_loop(x, 23);
}

// sm3 主循环, 512 位数据位一组, 对数据进行扩展压缩
static int sm3_loop(SM3_CTX *ctx, const uint32_t *sub_msg)
{
    uint32_t a, b, c, d, e, f, g, h;
    uint32_t w[16]  = { 0 };
    uint32_t i = 0;
    uint32_t j = 0;
    uint32_t wi = 0;
    uint32_t wj = 0;
    uint32_t ss1 = 0;
    uint32_t ss2 = 0;
    uint32_t tt1 = 0;
    uint32_t tt2 = 0;

    if (!ctx || !sub_msg) {
        ERROR_LOCATE("INVALID ARGS!");
        return FAILED;
    }

    a = ctx->A;
    b = ctx->B;
    c = ctx->C;
    d = ctx->D;
    e = ctx->E;
    f = ctx->F;
    g = ctx->G;
    h = ctx->H;

    // 前 16 字数据的扩展分组赋值
    for (i = 0; i < 16; ++i) {
        w[i] = change_byte_order32(sub_msg[i]);
    }

    for (i = 0; i < 16; ++i) {
        // 数据的压缩
        ss1 = shift_left_loop(
            (shift_left_loop(a, 12) + e + shift_left_loop(t[i], i % 32)), 7);
        ss2 = ss1 ^ shift_left_loop(a, 12);
        tt1 = sm3_calc_ff1(a, b, c) + d + ss2 + (w[i % 16] ^ w[(i + 4) % 16]);
        tt2 = sm3_calc_gg1(e, f, g) + h + ss1 + w[i % 16];
        d = c;
        c = shift_left_loop(b, 9);
        b = a;
        a = tt1;
        h = g;
        g = shift_left_loop(f, 19);
        f = e;
        e = sm3_calc_rep0(tt2);

        // 数据的扩展
        w[i % 16] = sm3_calc_rep1(w[i % 16] ^ w[(i + 7) % 16] ^ shift_left_loop(w[(i + 13) % 16], 15)) ^ 
            shift_left_loop(w[(i + 3) % 16], 7) ^ w[(i + 10) % 16];
    }

    // 后48字采用ff2, gg2
    for (i = 16; i < 64; ++i) {
        ss1 = shift_left_loop(
            (shift_left_loop(a, 12) + e + shift_left_loop(t[i], i % 32)), 7);
        ss2 = ss1 ^ shift_left_loop(a, 12);
        tt1 = sm3_calc_ff2(a, b, c) + d + ss2 + (w[i % 16] ^ w[(i + 4) % 16]);
        tt2 = sm3_calc_gg2(e, f, g) + h + ss1 + w[i % 16];
        d = c;
        c = shift_left_loop(b, 9);
        b = a;
        a = tt1;
        h = g;
        g = shift_left_loop(f, 19);
        f = e;
        e = sm3_calc_rep0(tt2);

        w[i % 16] = sm3_calc_rep1(w[i % 16] ^ w[(i + 7) % 16] ^ shift_left_loop(w[(i + 13) % 16], 15)) ^ 
            shift_left_loop(w[(i + 3) % 16], 7) ^ w[(i + 10) % 16];
    }

    // --------------------------------------------------------------------

    ctx->A ^= a;
    ctx->B ^= b;
    ctx->C ^= c;
    ctx->D ^= d;
    ctx->E ^= e;
    ctx->F ^= f;
    ctx->G ^= g;
    ctx->H ^= h;

    return SUCCESS;
}

int sm3_init(SM3_CTX *ctx)
{
    if (!ctx) {
        ERROR_LOCATE("NULL_POINTER SM3_CTX!");
        return FAILED;
    }

    memset(ctx, 0, sizeof(SM3_CTX));

    // 初始化链接变量, 大端法字节序
    ctx->A = 0x7380166fUL;
    ctx->B = 0x4914b2b9UL;
    ctx->C = 0x172442d7UL;
    ctx->D = 0xda8a0600UL;
    ctx->E = 0xa96f30bcUL;
    ctx->F = 0x163138aaUL;
    ctx->G = 0xe38dee4dUL;
    ctx->H = 0xb0fb0e4eUL;

    return SUCCESS;
}

int sm3_update(SM3_CTX *ctx, const uint8_t *msg, uint32_t len)
{
    if (!ctx) { ERROR_LOCATE("NULL_POINTER SM3_CTX!"); return FAILED; }
    if (!msg) { ERROR_LOCATE("NULL_POINTER SM3 MSG!"); return FAILED; }
    if (!len) { ERROR_LOCATE("NULL_POINTER MSG LEN!"); return FAILED; }

    uint32_t msg_len = 0;   // 待处理数据长度
    uint32_t res_len = 0;   // 缓冲区剩余数据长度
    uint32_t buf_len = 0;   // 缓冲区可填充长度

    const uint8_t *mp = NULL;
    uint8_t *bp = NULL;

    /*
     * 1.比较消息长度与分组长度，如果消息长度小于分组长度则直接拷贝至缓冲区
     *  然后在 final 阶段进行最终填充和运算；
     * 2.如果消息长度大于分组长度，则首先计算整数倍分组长度的数据，剩余的数据
     *  拷贝至缓冲区等待 final 阶段运算；
     * 3.如果是多次调用，每次需要累计缓冲区中的数据，然后最终进行填充和运算；
     * 
     * 总之，仅填充一次
     */

    // 累计消息摘要的总长度，是否会溢出？
    ctx->msg_len += len;
    msg_len = len;

    res_len = ctx->len;

    mp = msg;
    bp = ctx->buff;

    // 调用时有缓冲区剩余数据，则优先处理剩余数据
    if (res_len > 0) {
        // 如果当前剩余数据和新计算的数据长度之和小于缓冲区则将数据拷贝至缓冲区
        if (res_len + msg_len < SM3_BLOCK) {
            // 拷贝数据到缓冲区
            memcpy(bp + res_len, msg, msg_len);

            res_len += msg_len;
            msg_len = 0;

            ctx->len = res_len;
        } else {
            buf_len = SM3_BLOCK - res_len;

            // 先拷贝一部分到缓冲区，让缓冲区填满
            memcpy(bp + res_len, mp, buf_len);

            // 缓冲区填满后，直接进行一次运算
            sm3_loop(ctx, (const uint32_t *)bp);

            // 更新上下文
            mp += buf_len;
            msg_len -= buf_len;

            // 运算结束之后，清空缓冲区
            res_len = 0;
            ctx->len = res_len;
            memset(ctx->buff, 0, sizeof(SM3_BLOCK));
        }
    }

    // 缓冲区没有剩余数据的情况
    if (res_len == 0) {
        // 首先数据可以分几组
        uint32_t n = msg_len / SM3_BLOCK;

        if (n > 0) {
            // 直接进行运算，然后统计大小
            uint32_t num = n * SM3_BLOCK / 4;
            uint32_t *p = (uint32_t *)mp;

            // 对每个子分组进行运算
            for (uint32_t i = 0; i < num; i += 16) {
                sm3_loop(ctx, p + i);
            }

            // 运算完毕后统计剩余数据的大小
            res_len = msg_len - n * SM3_BLOCK;
        } else {
            // 剩余数据大小
            res_len = msg_len;
        }
        ctx->len = res_len;
    }

    // 将剩余数据拷贝至缓冲区
    if (msg_len > 0) {
        memcpy(bp, mp + (msg_len - res_len), res_len);
    }

    return SUCCESS;
}

int sm3_final(SM3_CTX *ctx, uint8_t *result, uint32_t *len)
{
    if (!ctx) { 
        ERROR_LOCATE("NULL_POINTER SM3_CTX!"); 
        return FAILED; 
    }

    if (!result) {
        *len = 8 * sizeof(uint32_t);
        return SUCCESS;
    }

    if (*len < 8 * sizeof(uint32_t)) {
        ERROR_LOCATE("BUFF NOT ENOUGH!"); 
        return FAILED; 
    }

    // 填充消息长度为 64 字节(512 位)的整数倍小 8 字节的值
    uint8_t *buff = (uint8_t *)ctx->buff;
    uint32_t num = ctx->len;

    // num 必定小于 64 所以有
    buff[num] = 0x80;
    num++;

    // 如果当前剩余长度不足以存储一个64位的数，则需要全部填充 0 且再做一次摘要运算
    if (num > (SM3_BLOCK - 8)) {
        memset(buff + num, 0, SM3_BLOCK - num);
        
        // 计算倒数第二个分组
        sm3_loop(ctx, (const uint32_t *)buff);

        // 剩余数据清 0
        num = 0;
    }

    // 填充最后一个分组
    memset(buff + num, 0, SM3_BLOCK - num - 8);

    // 最后 8 字节填充数据长度 (单位：bit)
    uint64_t *len_pt = (uint64_t *)(buff + SM3_BLOCK - 8);

    // 这里要是否会溢出？
    *len_pt = ctx->msg_len << 3;

    // 小端法机器要转换字节序
    if (is_little_endian()) {
        *len_pt = change_byte_order64(*len_pt);
    }

    // 计算最后一个分组数据
    sm3_loop(ctx, (const uint32_t *)buff);
    num = 0;
    buff = NULL;

    uint32_t *p = (uint32_t *)result;

    if (is_little_endian()) {
        p[0] = change_byte_order32(ctx->A);
        p[1] = change_byte_order32(ctx->B);
        p[2] = change_byte_order32(ctx->C);
        p[3] = change_byte_order32(ctx->D);
        p[4] = change_byte_order32(ctx->E);
        p[5] = change_byte_order32(ctx->F);
        p[6] = change_byte_order32(ctx->G);
        p[7] = change_byte_order32(ctx->H);
    } else {
        p[0] = ctx->A;
        p[1] = ctx->B;
        p[2] = ctx->C;
        p[3] = ctx->D;
        p[4] = ctx->E;
        p[5] = ctx->F;
        p[6] = ctx->G;
        p[7] = ctx->H;
    }

    // 清理数据
    memset(ctx, 0, sizeof(SM3_CTX));
    return SUCCESS;
}

int sm3_msg(const uint8_t *in_msg, uint32_t in_len, uint8_t *out_msg, uint32_t *out_len)
{
    SM3_CTX ctx;
    int ret = SUCCESS;

    if (!out_len) {
        ERROR_LOCATE("OUT LEN IS NULL!");
        return FAILED;
    }

    if (!out_msg) {
        *out_len = 4 * sizeof(uint32_t);
        return SUCCESS;
    }

    if (*out_len < 4 * sizeof(uint32_t)) {
        ERROR_LOCATE("BUFF NOT ENOUGH!"); 
        return FAILED; 
    }

    ret = sm3_init(&ctx);
    if (ret != SUCCESS) {
        ERROR_LOCATE("sm3_init ERROR!"); 
        return ret;
    }

    ret = sm3_update(&ctx, in_msg, in_len);
    if (ret != SUCCESS) {
        ERROR_LOCATE("sm3_update ERROR!"); 
        return ret;
    }

    ret = sm3_final(&ctx, out_msg, out_len);
    if (ret != SUCCESS) {
        ERROR_LOCATE("sm3_final ERROR!"); 
    }

    return ret;
}
