#ifndef _big5_H_
#define _big5_H_

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum big5_zone_t
{
    BIG5_ZONE_NONE = 0,
    BIG5_ZONE_ASCII,     // ASCII characters.
    BIG5_ZONE_UDC1,      // User-defined characters section 1.
    BIG5_ZONE_GRAPH,     // Graphical characters.
    BIG5_ZONE_RESERVED,  // Reserved, not for user-defined characters.
    BIG5_ZONE_FREQMOST,  // Frequently used characters.
    BIG5_ZONE_UDC2,      // User-defined characters section 2.
    BIG5_ZONE_FREQLESS,  // Less frequently used characters.
    BIG5_ZONE_UDC3,      // User-defined characters section 3.
} big5_zone_t;

bool        big5_code_is_big5(uint16_t code);
big5_zone_t big5_code_get_zone(uint16_t code);
uint16_t    big5_code_get_min(big5_zone_t zone);
uint16_t    big5_code_get_max(big5_zone_t zone);
size_t      big5_code_from_str(uint16_t *code, const char *str);
uint32_t    big5_code_to_unicode(uint16_t code);

size_t big5_to_utf8(char *dest, size_t destsz, const char *src);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif
