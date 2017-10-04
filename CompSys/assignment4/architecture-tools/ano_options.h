//
// (C) Finn Schiermer Andersen, 2016
//
// ano_options.h
//
// Generic option handling.
//

#ifndef __OPTIONS_H__
#define __OPTIONS_H__

int has_flag(char * flag, int argc, char const * argv []);

int arguments_ok(int argc, char const * argv []);

int option_with_default(char * spec, int def, int argc, char const * argv []);

#endif // __OPTIONS_H__
