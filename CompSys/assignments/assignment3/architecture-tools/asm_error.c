//
// Nis Wegmann, 2017
//
// asm_error.c
//

#include "asm_error.h"

#include "asm_std.h"

void asm_error(char const * file, int line, int column, char const * message, ...)
{
    (void) column;
    
    va_list args;
    
    va_start(args, message);
    
    fprintf(stderr, "[%s, line %d] ", file, line);
    vfprintf(stderr, message, args);
    fprintf(stderr, "\n");
    
    va_end(args);
}
