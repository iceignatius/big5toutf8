#include <string.h>
#include <stdio.h>
#include <gen/jmpbk.h>
#include <gen/memobj.h>
#include <gen/utf.h>
#include "big5.h"
#include "cmdopt.h"
#include "cmddata.h"

void print_help(void)
{
    printf("BIG-5 to UTF-8 converter.\n");
    printf("Usage: big5toutf8 [options] [file]\n");
    printf("Options:\n");
    printf("  -h, --help              Print help message.\n");
    printf("  -o, --output FILE       Set the output file, or the output will be\n");
    printf("                          printed to standard output by default.\n");
    printf("  -r, --replace           Replace the source file by the output.\n");
    printf("                          This option will overwrite the output file option.\n");
    printf("  -f, --force             Ignore errors.\n");
    printf("  -b, --bom               Add BOM mark to the result.\n");
    printf("      --to-unix           Convert line end to UNIX format.\n");
}

bool add_null_terminator(mem_t *mem)
{
    char ch = 0;
    return mem_append(mem, &ch, sizeof(ch));
}

bool add_bom_mark(mem_t *mem)
{
    static const char bom[3] = { 0xEF,0xBB,0xBF };

    mem_t temp;
    mem_init(&temp, 0);

    bool succeed = false;
    do
    {
        if( !mem_append(&temp, bom, sizeof(bom)) ) break;
        if( !mem_append(&temp, mem->buf, mem->size) ) break;
        mem_move_from(mem, &temp);

        succeed = true;
    } while(false);

    mem_deinit(&temp);

    return succeed;
}

int translate_big5_to_utf8(mem_t *dest, const char *big5str)
{
    int errbytes = 0;

    mem_clear(dest);

    const char *readpos = big5str;
    while( *readpos )
    {
        uint16_t big5code;
        size_t   readsz  = big5_code_from_str(&big5code, readpos);
        uint32_t unicode = big5_code_to_unicode(big5code);

        if( !unicode )
        {
            fprintf(stderr, "ERROR : Translate error at position : %u\n", (unsigned)(readpos-big5str));
            unicode = big5code;
            ++errbytes;
        }

        readpos += readsz;

        char utf8str[6+1] = {0};
        ch_utf32_to_utf8(utf8str, unicode);
        if( !mem_append(dest, utf8str, strlen(utf8str)) )
        {
            fprintf(stderr, "ERROR : Cannot allocate more memory!");
            errbytes = -1;
            break;
        }
    }

    if( !add_null_terminator(dest) )
        errbytes = -1;

    return errbytes;
}

const char* detect_lineend(const mem_t *mem)
{
    static const char crlf[] = "\x0D\x0A";
    static const char cr  [] = "\x0D";
    static const char lf  [] = "\x0A";

    if( memfind(mem->buf, mem->size, crlf, strlen(crlf)) )
        return crlf;
    else if( memfind(mem->buf, mem->size, cr, strlen(cr)) )
        return cr;
    else if( memfind(mem->buf, mem->size, lf, strlen(lf)) )
        return lf;
    else
        return NULL;
}

bool convert_lineend_to_lf(mem_t *mem)
{
    const char *ln = detect_lineend(mem);
    if( !ln ) return true;

    mem_t pattern;
    mem_init(&pattern, 0);

    mem_t target;
    mem_init(&target, 0);

    bool res = false;
    do
    {
        static const char lf[] = "\x0A";
        if( !mem_import(&pattern, ln, strlen(ln)) ) break;
        if( !mem_import(&target , lf, strlen(lf)) ) break;

        if( !mem_find_replace(mem, &pattern, &target) ) break;

        res = true;
    } while(false);

    mem_deinit(&target);
    mem_deinit(&pattern);

    return res;
}

int translate_data(mem_t *dest, const mem_t *src, const cmdopts_t *opts)
{
    int errbytes = 0;

    mem_clear(dest);

    bool already_utf8 = is_utf8_encoding(src->buf, src->size);
    if( already_utf8 )
    {
        fprintf(stderr, "WARNING : The source is already an UTF-8 file.\n");
        errbytes = -1;
    }

    if( !already_utf8 || opts->force )
    {
        errbytes = translate_big5_to_utf8(dest, (const char*)src->buf);

        if( opts->addbom && !add_bom_mark(dest) )
        {
            fprintf(stderr, "ERROR : Cannot allocate more memory!\n");
            errbytes = -1;
        }

    }

    if( !errbytes && opts->tounix && !convert_lineend_to_lf(dest) )
    {
        fprintf(stderr, "ERROR : Line end convert failed!\n");
        errbytes = -1;
    }

    return errbytes;
}

int main(int argc, char *argv[])
{
    cmdopts_t opts;
    cmdopts_load_defaults(&opts);
    cmdopts_load_args(&opts, argc, argv);
    if( opts.need_help )
    {
        print_help();
        return 0;
    }

    mem_t src;
    mem_init(&src, 0);

    mem_t dest;
    mem_init(&dest, 0);

    int errbytes;
    JMPBK_BEGIN
    {
        if( !cmddata_read(&src, true, opts.srcfile) )
            JMPBK_THROW(-1);

        errbytes = translate_data(&dest, &src, &opts);
        if( errbytes && !opts.force )
            JMPBK_THROW(errbytes);

        const char *outfile = opts.replace ? opts.srcfile : opts.destfile;
        if( !cmddata_write(&dest, true, outfile) )
            JMPBK_THROW(-1);
    }
    JMPBK_CATCH_ALL
    {
        errbytes = JMPBK_ERRCODE;
    }
    JMPBK_END;

    mem_deinit(&dest);
    mem_deinit(&src);

    return errbytes;
}
