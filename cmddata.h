/*
 * Command data reader and writer.
 */
#ifndef _CMDDATA_H_
#define _CMDDATA_H_

#include <gen/memobj.h>

#ifdef __cplusplus
extern "C" {
#endif

bool cmddata_read(mem_t *mem, bool isstr, const char *filename);
bool cmddata_write(const mem_t *mem, bool isstr, const char *filename);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif
