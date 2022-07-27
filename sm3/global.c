#include <stdio.h>
#include "global.h"

void print_bin_info(const char *name, uint8_t *data, uint32_t len)
{
    uint32_t j = 0;
    uint32_t i = 0;

    if (!data || len <= 0) {
        return;
    }

    printf("name:%s size:%d\n", (!name || !name[0]) ? "" : name, len);
    printf("------------------------+------------------------\n");

    for (i = 0; i < len; i++) {
        if (i > 0) {
            if (i % 16 == 0) {
                printf("\n");
                j++;
                if (j % 8 == 0) {
                    printf("------------------------+------------------------\n");
                }
            }
            else if (i % 8 == 0) {
                printf("| ");
            }
        }

        if ((i + 1) % 16 == 0) {
            printf("%02x", data[i]);
        }
        else {
            printf("%02x ", data[i]);
        }
    }

    printf("\n");
    printf("------------------------+------------------------\n");
}

void error_locate(
    const char *info, 
    const char *file, 
    const char *func, 
    int         line)
{
    printf("Error:%s file:%s function:%s line:%d\n", info, file, func, line);
}
