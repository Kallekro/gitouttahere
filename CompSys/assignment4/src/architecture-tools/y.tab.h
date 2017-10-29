/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     REGISTER = 258,
     MNM_HLT = 259,
     MNM_NOP = 260,
     MNM_MOVQ = 261,
     MNM_ADDQ = 262,
     MNM_SUBQ = 263,
     MNM_MULQ = 264,
     MNM_ANDQ = 265,
     MNM_XORQ = 266,
     MNM_CMPQ = 267,
     MNM_SHRQ = 268,
     MNM_JMP = 269,
     MNM_JLE = 270,
     MNM_JL = 271,
     MNM_JE = 272,
     MNM_JNE = 273,
     MNM_JGE = 274,
     MNM_JG = 275,
     MNM_CMOVLE = 276,
     MNM_CMOVL = 277,
     MNM_CMOVE = 278,
     MNM_CMOVNE = 279,
     MNM_CMOVGE = 280,
     MNM_CMOVG = 281,
     MNM_CALL = 282,
     MNM_RET = 283,
     MNM_PUSHQ = 284,
     MNM_POPQ = 285,
     ALIGN = 286,
     POSITION = 287,
     QUAD = 288,
     RANDOM = 289,
     NEWLINE = 290,
     PERIOD = 291,
     COMMA = 292,
     COLON = 293,
     SEMICOLON = 294,
     LPAREN = 295,
     RPAREN = 296,
     DOLLAR = 297,
     INT_LITERAL = 298,
     IDENTIFIER = 299,
     DIRECTIVE = 300,
     DATA = 301,
     STRING_LITERAL = 302,
     UNKNOWN_REGISTER = 303,
     UNKNOWN_DIRECTIVE = 304,
     UNKNOWN_TOKEN = 305
   };
#endif
/* Tokens.  */
#define REGISTER 258
#define MNM_HLT 259
#define MNM_NOP 260
#define MNM_MOVQ 261
#define MNM_ADDQ 262
#define MNM_SUBQ 263
#define MNM_MULQ 264
#define MNM_ANDQ 265
#define MNM_XORQ 266
#define MNM_CMPQ 267
#define MNM_SHRQ 268
#define MNM_JMP 269
#define MNM_JLE 270
#define MNM_JL 271
#define MNM_JE 272
#define MNM_JNE 273
#define MNM_JGE 274
#define MNM_JG 275
#define MNM_CMOVLE 276
#define MNM_CMOVL 277
#define MNM_CMOVE 278
#define MNM_CMOVNE 279
#define MNM_CMOVGE 280
#define MNM_CMOVG 281
#define MNM_CALL 282
#define MNM_RET 283
#define MNM_PUSHQ 284
#define MNM_POPQ 285
#define ALIGN 286
#define POSITION 287
#define QUAD 288
#define RANDOM 289
#define NEWLINE 290
#define PERIOD 291
#define COMMA 292
#define COLON 293
#define SEMICOLON 294
#define LPAREN 295
#define RPAREN 296
#define DOLLAR 297
#define INT_LITERAL 298
#define IDENTIFIER 299
#define DIRECTIVE 300
#define DATA 301
#define STRING_LITERAL 302
#define UNKNOWN_REGISTER 303
#define UNKNOWN_DIRECTIVE 304
#define UNKNOWN_TOKEN 305




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 26 "asm_parser.y"
{
    unsigned char reg_val;
    isa_Long int_val;
    char str_val [256];
}
/* Line 1529 of yacc.c.  */
#line 155 "y.tab.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} YYLTYPE;
# define yyltype YYLTYPE /* obsolescent; will be withdrawn */
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif

extern YYLTYPE yylloc;
