#include <stdio.h>
#include <string.h>

#include "sm3.h"

void file_test();

int main(int argc, char *argv[])
{
    SM3_CTX ctx;

    uint8_t ret[32] = { 0 };
    uint32_t len = 32;

    uint32_t size  = 0;
    uint32_t size1 = 0;
    uint32_t size2 = 0;

    static uint8_t origin[] = "abc";
    static uint8_t origin1[] = "a";
    static uint8_t origin2[] = "bc";

    size  = strlen((const char *)origin);
    size1 = strlen((const char *)origin1);
    size2 = strlen((const char *)origin2);

    sm3_msg(origin, size, ret, &len);

    sm3_init(&ctx);
    sm3_update(&ctx, origin1, size1);
    sm3_update(&ctx, origin2, size2);

    print_bin_info("sm3 result1", ret, len);

    memset(ret, 0, len);
    sm3_final(&ctx, ret, &len);
    print_bin_info("sm3 result2", ret, len);

    file_test();
    return 0;
}

void file_test()
{
    int  test = 0;
    char file[64] = { 0 };
    char buff[64] = { 0 };
    
    uint8_t sm3buf[32] = { 0 };
    uint32_t sm3len = 32;

    size_t blen = sizeof(buff);
    size_t rlen = 0;

    FILE *fp = NULL;
    SM3_CTX ctx;

    printf("File test? 1.yes, 0.no\n");
    scanf("%d", &test);

    test = (test != 1) ? 0 : 1;
    if (!test) {
        return;
    }

    printf("file path:\n");
    scanf("%s", file);

    fp = fopen(file, "rb");
    if (!fp) {
        ERROR_LOCATE("Open file failed, errno!\n");
        return;
    }

    sm3_init(&ctx);

    while ((rlen = fread(buff, sizeof(char), blen, fp)) == blen) {
       sm3_update(&ctx, (const uint8_t *)buff, blen);
    }

    // 计算剩余的数据
    if (rlen > 0) {
        sm3_update(&ctx, (const uint8_t *)buff, rlen);
    }

    sm3_final(&ctx, sm3buf, &sm3len);
    print_bin_info("the file sm3 result", sm3buf, sm3len);
    fclose(fp);
}
