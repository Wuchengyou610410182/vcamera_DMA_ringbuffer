#include <linux/module.h>
#include <linux/string.h>
#include "pattern_generator.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("YourName");
MODULE_DESCRIPTION("Simple pattern generator for vcam");

void fill_frame_pattern(void *buf, int width, int height)
{
    u16 *pixel = (u16 *)buf;
    int x, y;

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            u16 val = 0x0000;

            if ((y % 2 == 0) && (x % 2 == 0))
                val = 0xFFFF;
            else if ((y % 2 == 0) && (x % 2 == 1))
                val = 0x8000;
            else if ((y % 2 == 1) && (x % 2 == 0))
                val = 0x4000;

            pixel[y * width + x] = val;
        }
    }
}

EXPORT_SYMBOL(fill_frame_pattern);



