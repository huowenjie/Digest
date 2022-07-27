#include <stdio.h>
#include <string.h>

#include "md5.h"

void file_test();

int main(int argc, char *argv[])
{
    MD5_CTX ctx;

    uint8_t ret[16] = { 0 };
    uint32_t len = 16;

    uint32_t size  = 0;
    uint32_t size1 = 0;
    uint32_t size2 = 0;

    static uint8_t origin[] = "12345678901234567890123456789012345678901234567890123456789012341234567890123456789012345678901234567890123456789012345678901234";
    static uint8_t origin1[] = "123456789012345678901234567890123456789012345678901234567890123412345678901234567890123456789012345678901234567890123456789";
    static uint8_t origin2[] = "01234";

    size = strlen((const char *)origin);
    size1 = strlen((const char *)origin1);
    size2 = strlen((const char *)origin2);

    md5_msg(origin, size, ret, &len);

    md5_init(&ctx);
    md5_update(&ctx, origin1, size1);
    md5_update(&ctx, origin2, size2);

    print_bin_info("MD5 result1", ret, len);

    memset(ret, 0, len);
    md5_final(&ctx, ret, &len);
    print_bin_info("MD5 result2", ret, len);

    file_test();
    return 0;
}

void file_test()
{
    int  test = 0;
    char file[64] = { 0 };
    char buff[64] = { 0 };
    
    uint8_t md5buf[16] = { 0 };
    uint32_t md5len = 16;

    size_t blen = sizeof(buff);
    size_t rlen = 0;

    FILE *fp = NULL;
    MD5_CTX ctx;

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
    
    md5_init(&ctx);

    while ((rlen = fread(buff, sizeof(char), blen, fp)) == blen) {
       md5_update(&ctx, (const uint8_t *)buff, blen);
    }

    // 计算剩余的数据
    if (rlen > 0) {
        md5_update(&ctx, (const uint8_t *)buff, rlen);
    }

    md5_final(&ctx, md5buf, &md5len);
    print_bin_info("the file MD5 result", md5buf, md5len);
    fclose(fp);
}
