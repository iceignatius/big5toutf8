#include <string.h>
#include <stdio.h>
#include <getopt.h>
#include "cmdopt.h"

//------------------------------------------------------------------------------
void cmdopts_load_defaults(cmdopts_t *obj)
{
    memset(obj, 0, sizeof(*obj));
}
//------------------------------------------------------------------------------
void cmdopts_load_args(cmdopts_t *obj, int argc, char *argv[])
{
    struct option longopts[] =
    {
        { "help"    , no_argument      , NULL, 'h' },
        { "output"  , required_argument, NULL, 'o' },
        { "replace" , no_argument      , NULL, 'r' },
        { "force"   , no_argument      , NULL, 'f' },
        { "bom"     , no_argument      , NULL, 'b' },
        { "to-unix" , no_argument      , NULL, 'U' },
        { NULL      , 0                , NULL,  0  }
    };

    int opt;
    int index;
    while( 0 <= ( opt = getopt_long(argc, argv, "ho:rfb", longopts, &index) ) )
    {
        switch( opt )
        {
        case 'o':
            obj->destfile = optarg;
            break;

        case 'r':
            obj->replace = true;
            break;

        case 'f':
            obj->force = true;
            break;

        case 'b':
            obj->addbom = true;
            break;

        case 'U':
            obj->tounix = true;
            break;

        case 'h':
        case '?':
            obj->need_help = true;
            break;

        }
    }

    if( optind < argc )
        obj->srcfile = argv[optind++];

    if( optind < argc )
    {
        obj->need_help = true;
        fprintf(stderr, "Unknown argument: %s\n", argv[optind]);
    }
}
//------------------------------------------------------------------------------
