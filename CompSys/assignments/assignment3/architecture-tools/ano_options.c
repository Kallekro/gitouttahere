//
// (C) Finn Schiermer Andersen, 2016
//
// ano_options.c
//
// Generic option handling.
//

#include <string.h>
#include <stdio.h>

#include "ano_options.h"

int has_flag(char * flag, int argc, char const * argv [])
{
    for (int j = 0; j < argc; ++j)
    {
        if (argv[j] != 0 && strcmp(flag, argv[j]) == 0)
        {
            argv[j] = 0; // option recognized
            
            return 1;
        }
    }
    
    return 0;
}

int arguments_ok(int argc, char const * argv [])
{
    int errors = 0;
    
    for (int j = 0; j < argc; ++j)
    {
        if (argv[j])
        {
            printf("Argument error. Unknown option: '%s'\n", argv[j]);
            
            ++errors;
        }
    }
    
    return errors == 0;
}

int option_with_default(char * spec, int def, int argc, char const * argv [])
{
    for (int j = 0; j < argc; ++j)
    {
        int res;
        
        if (argv[j] != 0 && sscanf(argv[j], spec, &res) == 1)
        {
            argv[j] = 0; // option recognized
            
            return res;
        }
    }
    
    return def;
}
