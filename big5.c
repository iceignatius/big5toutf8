#include <gen/utf.h>
#include "big5_utbl.h"
#include "big5.h"

//------------------------------------------------------------------------------
bool big5_code_is_big5(uint16_t code)
{
    /**
     * Check if a code is an available BIG-5 code.
     */
    if( code < 0x7F ) return true;

    unsigned byte1 = code >> 8;
    unsigned byte2 = code & 0xFF;
    bool byte1_in_range = ( 0x81 <= byte1 && byte1 <= 0xFE );
    bool byte2_in_range = ( 0x40 <= byte2 && byte2 <= 0x7E ) || ( 0xA1 <= byte2 && byte2 <= 0xFE );
    return byte1_in_range && byte2_in_range;
}
//------------------------------------------------------------------------------
big5_zone_t big5_code_get_zone(uint16_t code)
{
    /**
     * Get which code zone the code in.
     */
    if     ( code <=   0x7F           )  return BIG5_ZONE_ASCII;
    else if( !big5_code_is_big5(code) )  return BIG5_ZONE_NONE;
    else if( code <= 0xA0FE           )  return BIG5_ZONE_UDC1;
    else if( code <= 0xA3BF           )  return BIG5_ZONE_GRAPH;
    else if( code <= 0xA3FE           )  return BIG5_ZONE_RESERVED;
    else if( code <= 0xC67E           )  return BIG5_ZONE_FREQMOST;
    else if( code <= 0xC8FE           )  return BIG5_ZONE_UDC2;
    else if( code <= 0xF9D5           )  return BIG5_ZONE_FREQLESS;
    else if( code <= 0xFEFE           )  return BIG5_ZONE_UDC3;
    else                                 return BIG5_ZONE_NONE;
}
//------------------------------------------------------------------------------
uint16_t big5_code_get_min(big5_zone_t zone)
{
    /**
     * Get the minimum code of a specific code zone.
     */
    switch( zone )
    {
    case BIG5_ZONE_NONE     :  return 0;
    case BIG5_ZONE_ASCII    :  return 0;
    case BIG5_ZONE_UDC1     :  return 0x8140;
    case BIG5_ZONE_GRAPH    :  return 0xA140;
    case BIG5_ZONE_RESERVED :  return 0xA3C0;
    case BIG5_ZONE_FREQMOST :  return 0xA440;
    case BIG5_ZONE_UDC2     :  return 0xC6A1;
    case BIG5_ZONE_FREQLESS :  return 0xC940;
    case BIG5_ZONE_UDC3     :  return 0xF9D6;
    default                 :  return 0;
    }
}
//------------------------------------------------------------------------------
uint16_t big5_code_get_max(big5_zone_t zone)
{
    /**
     * Get the maximum code of a specific code zone.
     */
    switch( zone )
    {
    case BIG5_ZONE_NONE     :  return 0;
    case BIG5_ZONE_ASCII    :  return 0x7F;
    case BIG5_ZONE_UDC1     :  return 0xA0FE;
    case BIG5_ZONE_GRAPH    :  return 0xA3BF;
    case BIG5_ZONE_RESERVED :  return 0xA3FE;
    case BIG5_ZONE_FREQMOST :  return 0xC67E;
    case BIG5_ZONE_UDC2     :  return 0xC8FE;
    case BIG5_ZONE_FREQLESS :  return 0xF9D5;
    case BIG5_ZONE_UDC3     :  return 0xFEFE;
    default                 :  return 0;
    }
}
//------------------------------------------------------------------------------
size_t big5_code_from_str(uint16_t *code, const char *str)
{
    /**
     * Get a BIG-5 code from a string.
     *
     * @param code It will return the result code of the first word of the string,
     *             and that may be a BIG-5 code.
     * @param str  The string to be parsed.
     * @return Size of data read from the string.
     *
     * @remarks A code that is not in range of BIG-5 will be present as it is
     *          (and it will be a single byte).
     */
    if( !str ) return 0;

    const uint8_t *src       = (const uint8_t*)str;
    unsigned       byte1     = src[0];
    unsigned       byte2     = src[1];
    uint16_t       big5code  = ( byte1 << 8 ) | byte2;
    bool           use_2byte = ( byte1 > 0x7F && big5_code_is_big5(big5code) );

    if( code ) *code = use_2byte ? big5code : byte1;
    return use_2byte ? 2 : 1;
}
//------------------------------------------------------------------------------
uint32_t big5_code_to_unicode(uint16_t code)
{
    /**
     * Translate a BIG-5 code to an Unicode code.
     * And note that an invalid code will be translated to ZERO.
     */
    if( code <= 0x7F ) return code;  // Return the code directly if the code is an ASCII code.

    unsigned         idx_l = 0;
    unsigned         idx_u = sizeof(big5_utbl)/sizeof(big5_utbl[0]) - 1;
    big5_utbl_item_t res_l = big5_utbl[idx_l];
    big5_utbl_item_t res_u = big5_utbl[idx_u];

    if( code < res_l.big5 || res_u.big5 < code ) return 0;
    if( code == res_l.big5 ) return res_l.unicode;
    if( code == res_u.big5 ) return res_u.unicode;

    unsigned         idx;
    big5_utbl_item_t res;
    while( idx_u - idx_l > 1 )
    {
        idx = ( idx_l + idx_u ) >> 1;
        res = big5_utbl[idx];
        if( code > res.big5 )
        {
            idx_l = idx;
            res_l = res;
        }
        else if( code < res.big5 )
        {
            idx_u = idx;
            res_u = res;
        }
        else
        {
            break;
        }
    }

    return code == res.big5 ? res.unicode : 0;
}
//------------------------------------------------------------------------------
size_t big5_to_utf8(char *dest, size_t destsz, const char *src)
{
    /**
     * Translate BIG-5 string to UTF-8 format.
     *
     * @param dest   A buffer to receive the UTF-8 result,
     *               and can be NULL to calculate to calculate buffer size only.
     * @param destsz Size of the output buffer.
     * @param src    The BIG-5 string source.
     * @return Size of data write to the output buffer if succeed (including the null-terminator);
     *         and ZERO if failed.
     *         Or if @a dest is NULL, the return will be the minimum size of output buffer needed
     *         (including the null-terminator).
     *
     * @remarks Characters that cannot be translated will be present as it is.
     */
    size_t recsz = 0;

    if( !src ) return 0;

    if( !dest )
    {
        // Calculate buffer size needed only.

        while( *src )
        {
            uint16_t big5code;
            src   += big5_code_from_str(&big5code, src);
            recsz += ch_utf32_to_utf8(NULL, big5code);
        }

        ++recsz;  // Don't forget the null-terminator
    }
    else
    {
        // Translate characters and write to output buffer.

        while( *src )
        {
            uint16_t big5code;
            size_t big5size = big5_code_from_str(&big5code, src);
            src += big5size;

            char utf8str[6];
            size_t utf8size = ch_utf32_to_utf8(utf8str, big5code);
            if( destsz <= utf8size ) return 0;

            destsz -= utf8size;
            recsz  += utf8size;

            char *pos = utf8str;
            while( utf8size-- )
                *dest++ = *pos++;
        }

        // Write the null-terminator.
        *dest = 0;
        ++recsz;
    }

    return recsz;
}
//------------------------------------------------------------------------------
