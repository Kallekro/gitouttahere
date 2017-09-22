/*
// Nis Wegmann, 2017
//
// asm_lexer.l
*/

%{

#include "isa.h"
#include "y.tab.h"

%}

%option yylineno
%option caseless
%option noyywrap

%%

"%"[ \t]*"rax"                  { yylval.reg_val = isa_Register_rax; return REGISTER; }
"%"[ \t]*"rcx"                  { yylval.reg_val = isa_Register_rcx; return REGISTER; }
"%"[ \t]*"rdx"                  { yylval.reg_val = isa_Register_rdx; return REGISTER; }
"%"[ \t]*"rbx"                  { yylval.reg_val = isa_Register_rbx; return REGISTER; }
"%"[ \t]*"rsp"                  { yylval.reg_val = isa_Register_rsp; return REGISTER; }
"%"[ \t]*"rbp"                  { yylval.reg_val = isa_Register_rbp; return REGISTER; }
"%"[ \t]*"rsi"                  { yylval.reg_val = isa_Register_rsi; return REGISTER; }
"%"[ \t]*"rdi"                  { yylval.reg_val = isa_Register_rdi; return REGISTER; }
"%"[ \t]*"r8"                   { yylval.reg_val = isa_Register_r8 ; return REGISTER; }
"%"[ \t]*"r9"                   { yylval.reg_val = isa_Register_r9 ; return REGISTER; }
"%"[ \t]*"r10"                  { yylval.reg_val = isa_Register_r10; return REGISTER; }
"%"[ \t]*"r11"                  { yylval.reg_val = isa_Register_r11; return REGISTER; }
"%"[ \t]*"r12"                  { yylval.reg_val = isa_Register_r12; return REGISTER; }
"%"[ \t]*"r13"                  { yylval.reg_val = isa_Register_r13; return REGISTER; }
"%"[ \t]*"r14"                  { yylval.reg_val = isa_Register_r14; return REGISTER; }
"%"[ \t]*"r15"                  { yylval.reg_val = isa_Register_r15; return REGISTER; }

"%"[ \t]*[_a-zA-Z][_a-zA-Z0-9]* { strcpy(yylval.str_val, yytext); return UNKNOWN_REGISTER; }

"hlt"                           { return MNM_HLT; }
"halt"                          { return MNM_HLT; }
"nop"                           { return MNM_NOP; }
"movq"                          { return MNM_MOVQ; }
"addq"                          { return MNM_ADDQ; }
"subq"                          { return MNM_SUBQ; }
"andq"                          { return MNM_ANDQ; }
"xorq"                          { return MNM_XORQ; }
"cmpq"                          { return MNM_CMPQ; }
"shrq"                          { return MNM_SHRQ; }
"jmp"                           { return MNM_JMP; }
"jle"                           { return MNM_JLE; }
"jl"                            { return MNM_JL; }
"je"                            { return MNM_JE; }
"jne"                           { return MNM_JNE; }
"jge"                           { return MNM_JGE; }
"jg"                            { return MNM_JG; }
"cmovle"                        { return MNM_CMOVLE; }
"cmovl"                         { return MNM_CMOVL; }
"cmove"                         { return MNM_CMOVE; }
"cmovne"                        { return MNM_CMOVNE; }
"cmovge"                        { return MNM_CMOVGE; }
"cmovg"                         { return MNM_CMOVG; }
"call"                          { return MNM_CALL; }
"ret"                           { return MNM_RET; }
"pushq"                         { return MNM_PUSHQ; }
"popq"                          { return MNM_POPQ; }

"."[ \t]*"align"                { return ALIGN; }
"."[ \t]*"pos"                  { return POSITION; }
"."[ \t]*"quad"                 { return QUAD; }
"."[ \t]*"rand"                 { return RANDOM; }

"."[ \t]*[_a-zA-Z][_a-zA-Z0-9]* { strcpy(yylval.str_val, yytext); return UNKNOWN_DIRECTIVE; }

"//".*                          { /* Comments. */ }
[ \t]                           { /* White space. */ }
"\n"                            { return NEWLINE; }

[_a-zA-Z][_a-zA-Z0-9]*          { strcpy(yylval.str_val, yytext); return IDENTIFIER; }

[0-9][0-9]*                     { yylval.int_val = _isa_signedLong((int32_t) atoi(yytext)); return INT_LITERAL; }
0b[0-9a-fA-F][0-9a-fA-F]*       { yylval.int_val = _isa_signedLong((int32_t) strtoul(yytext + 2, NULL,  2)); return INT_LITERAL; }
0o[0-9a-fA-F][0-9a-fA-F]*       { yylval.int_val = _isa_signedLong((int32_t) strtoul(yytext + 2, NULL,  8)); return INT_LITERAL; }
0x[0-9a-fA-F][0-9a-fA-F]*       { yylval.int_val = _isa_signedLong((int32_t) strtoul(yytext + 2, NULL, 16)); return INT_LITERAL; }

","                             { return COMMA; }
":"                             { return COLON; }
";"                             { return SEMICOLON; }
"("                             { return LPAREN; }
")"                             { return RPAREN; }
"$"                             { return DOLLAR; }

.                               { strcpy(yylval.str_val, yytext); return UNKNOWN_TOKEN; }

%%
