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
     MNM_ANDQ = 264,
     MNM_XORQ = 265,
     MNM_CMPQ = 266,
     MNM_SHRQ = 267,
     MNM_JMP = 268,
     MNM_JLE = 269,
     MNM_JL = 270,
     MNM_JE = 271,
     MNM_JNE = 272,
     MNM_JGE = 273,
     MNM_JG = 274,
     MNM_CMOVLE = 275,
     MNM_CMOVL = 276,
     MNM_CMOVE = 277,
     MNM_CMOVNE = 278,
     MNM_CMOVGE = 279,
     MNM_CMOVG = 280,
     MNM_CALL = 281,
     MNM_RET = 282,
     MNM_PUSHQ = 283,
     MNM_POPQ = 284,
     ALIGN = 285,
     POSITION = 286,
     QUAD = 287,
     RANDOM = 288,
     NEWLINE = 289,
     PERIOD = 290,
     COMMA = 291,
     COLON = 292,
     SEMICOLON = 293,
     LPAREN = 294,
     RPAREN = 295,
     DOLLAR = 296,
     INT_LITERAL = 297,
     IDENTIFIER = 298,
     DIRECTIVE = 299,
     DATA = 300,
     STRING_LITERAL = 301,
     UNKNOWN_REGISTER = 302,
     UNKNOWN_DIRECTIVE = 303,
     UNKNOWN_TOKEN = 304
   };
#endif
/* Tokens.  */
#define REGISTER 258
#define MNM_HLT 259
#define MNM_NOP 260
#define MNM_MOVQ 261
#define MNM_ADDQ 262
#define MNM_SUBQ 263
#define MNM_ANDQ 264
#define MNM_XORQ 265
#define MNM_CMPQ 266
#define MNM_SHRQ 267
#define MNM_JMP 268
#define MNM_JLE 269
#define MNM_JL 270
#define MNM_JE 271
#define MNM_JNE 272
#define MNM_JGE 273
#define MNM_JG 274
#define MNM_CMOVLE 275
#define MNM_CMOVL 276
#define MNM_CMOVE 277
#define MNM_CMOVNE 278
#define MNM_CMOVGE 279
#define MNM_CMOVG 280
#define MNM_CALL 281
#define MNM_RET 282
#define MNM_PUSHQ 283
#define MNM_POPQ 284
#define ALIGN 285
#define POSITION 286
#define QUAD 287
#define RANDOM 288
#define NEWLINE 289
#define PERIOD 290
#define COMMA 291
#define COLON 292
#define SEMICOLON 293
#define LPAREN 294
#define RPAREN 295
#define DOLLAR 296
#define INT_LITERAL 297
#define IDENTIFIER 298
#define DIRECTIVE 299
#define DATA 300
#define STRING_LITERAL 301
#define UNKNOWN_REGISTER 302
#define UNKNOWN_DIRECTIVE 303
#define UNKNOWN_TOKEN 304




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 26 "asm_parser.y"
{
    unsigned char reg_val;
    isa_Long int_val;
    char str_val [256];
}
/* Line 1529 of yacc.c.  */
#line 153 "y.tab.h"
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
