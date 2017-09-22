//
// Nis Wegmann, 2017
//
// asm_parser.y
//

%{

#include "asm_error.h"
#include "asm_ir.h"
#include "asm_std.h"
#include "isa.h"

extern int yylex(void);
extern int yylineno;
extern char const * yyfilename;
extern asm_IR * yyIR;

static int yyerror(char const *);

%}

%locations

%union
{
    unsigned char reg_val;
    isa_Long int_val;
    char str_val [256];
}

// Registers.
%token <reg_val> REGISTER

// Mnemonics.
%token MNM_HLT;
%token MNM_NOP;
%token MNM_MOVQ;
%token MNM_ADDQ;
%token MNM_SUBQ;
%token MNM_ANDQ;
%token MNM_XORQ;
%token MNM_CMPQ;
%token MNM_SHRQ;
%token MNM_JMP;
%token MNM_JLE;
%token MNM_JL;
%token MNM_JE;
%token MNM_JNE;
%token MNM_JGE;
%token MNM_JG;
%token MNM_CMOVLE;
%token MNM_CMOVL;
%token MNM_CMOVE;
%token MNM_CMOVNE;
%token MNM_CMOVGE;
%token MNM_CMOVG;
%token MNM_CALL;
%token MNM_RET;
%token MNM_PUSHQ;
%token MNM_POPQ;

// Directives.
%token ALIGN
%token POSITION
%token QUAD
%token RANDOM

// Other tokens.
%token NEWLINE PERIOD COMMA COLON SEMICOLON LPAREN RPAREN DOLLAR

%token <int_val> INT_LITERAL

%token <str_val> IDENTIFIER DIRECTIVE DATA STRING_LITERAL

// Error tokens.
%token <str_val> UNKNOWN_REGISTER UNKNOWN_DIRECTIVE UNKNOWN_TOKEN

%type <reg_val> register

%start program

%%

program

    : lines
    ;

lines

    : line lines
    |
    ;

line

    : MNMuction NEWLINE
    | label
    | directive NEWLINE
    | NEWLINE
    | unknown
    ;

MNMuction

    : MNM_HLT
    {
        asm_IR_addInstruction(yyIR, ISA_HLT(), NULL);
    }
    
    | MNM_NOP
    {
        asm_IR_addInstruction(yyIR, ISA_NOP(), NULL);
    }
    
    | MNM_MOVQ register COMMA register
    {
        asm_IR_addInstruction(yyIR, ISA_MOVQ_RR($2, $4), NULL);
    }
    
    | MNM_MOVQ DOLLAR INT_LITERAL COMMA register
    {
        asm_IR_addInstruction(yyIR, ISA_MOVQ_IR($3, $5), NULL);
    }
    
    | MNM_MOVQ IDENTIFIER COMMA register
    {
        asm_IR_addInstruction(yyIR, ISA_MOVQ_IR(0, $4), $2);
    }
    
    | MNM_MOVQ LPAREN register RPAREN COMMA register
    {
        asm_IR_addInstruction(yyIR, ISA_MOVQ_MR(0, $3, $6), NULL);
    }
    
    | MNM_MOVQ register COMMA LPAREN register RPAREN
    {
        asm_IR_addInstruction(yyIR, ISA_MOVQ_RM($2, $5, 0), NULL);
    }
    
    | MNM_MOVQ IDENTIFIER LPAREN register RPAREN COMMA register
    {
        asm_IR_addInstruction(yyIR, ISA_MOVQ_MR(0, $4, $7), $2);
    }
    
    | MNM_MOVQ INT_LITERAL LPAREN register RPAREN COMMA register
    {
        asm_IR_addInstruction(yyIR, ISA_MOVQ_MR($2, $4, $7), NULL);
    }
    
//    | MNM_MOVQ register COMMA IDENTIFIER LPAREN register RPAREN
//    {
//        asm_IR_addInstruction(yyIR, ISA_MOVQ_RM($2, $6, 0), $4);
//    }
    
    | MNM_MOVQ register COMMA INT_LITERAL LPAREN register RPAREN
    {
        asm_IR_addInstruction(yyIR, ISA_MOVQ_RM($2, $6, $4), NULL);
    }
    
    | MNM_ADDQ register COMMA register
    {
        asm_IR_addInstruction(yyIR, ISA_ADDQ_RR($2, $4), NULL);
    }
    
    | MNM_SUBQ register COMMA register
    {
        asm_IR_addInstruction(yyIR, ISA_SUBQ_RR($2, $4), NULL);
    }
    
    | MNM_ANDQ register COMMA register
    {
        asm_IR_addInstruction(yyIR, ISA_ANDQ_RR($2, $4), NULL);
    }
    
    | MNM_XORQ register COMMA register
    {
        asm_IR_addInstruction(yyIR, ISA_XORQ_RR($2, $4), NULL);
    }
    
    | MNM_CMPQ register COMMA register
    {
        asm_IR_addInstruction(yyIR, ISA_CMPQ_RR($2, $4), NULL);
    }
    
    | MNM_SHRQ register COMMA register
    {
        asm_IR_addInstruction(yyIR, ISA_SHRQ_RR($2, $4), NULL);
    }
    
    | MNM_JMP IDENTIFIER
    {
        asm_IR_addInstruction(yyIR, ISA_JMP(0), $2);
    }
    
    | MNM_JLE IDENTIFIER
    {
        asm_IR_addInstruction(yyIR, ISA_JLE(0), $2);
    }
    
    | MNM_JL IDENTIFIER
    {
        asm_IR_addInstruction(yyIR, ISA_JL(0), $2);
    }
    
    | MNM_JE IDENTIFIER
    {
        asm_IR_addInstruction(yyIR, ISA_JE(0), $2);
    }
    
    | MNM_JNE IDENTIFIER
    {
        asm_IR_addInstruction(yyIR, ISA_JNE(0), $2);
    }
    
    | MNM_JGE IDENTIFIER
    {
        asm_IR_addInstruction(yyIR, ISA_JGE(0), $2);
    }
    
    | MNM_JG IDENTIFIER
    {
        asm_IR_addInstruction(yyIR, ISA_JG(0), $2);
    }
    
    | MNM_CMOVLE register COMMA register
    {
        asm_IR_addInstruction(yyIR, ISA_CMOVLE($2, $4), NULL);
    }
    
    | MNM_CMOVL register COMMA register
    {
        asm_IR_addInstruction(yyIR, ISA_CMOVL($2, $4), NULL);
    }
    
    | MNM_CMOVE register COMMA register
    {
        asm_IR_addInstruction(yyIR, ISA_CMOVE($2, $4), NULL);
    }
    
    | MNM_CMOVNE register COMMA register
    {
        asm_IR_addInstruction(yyIR, ISA_CMOVNE($2, $4), NULL);
    }
    
    | MNM_CMOVGE register COMMA register
    {
        asm_IR_addInstruction(yyIR, ISA_CMOVGE($2, $4), NULL);
    }
    
    | MNM_CMOVG register COMMA register
    {
        asm_IR_addInstruction(yyIR, ISA_CMOVG($2, $4), NULL);
    }
    
    | MNM_CALL IDENTIFIER
    {
        asm_IR_addInstruction(yyIR, ISA_CALL(0), $2);
    }
    
    | MNM_RET
    {
        asm_IR_addInstruction(yyIR, ISA_RET(), NULL);
    }
    
    | MNM_PUSHQ register
    {
        asm_IR_addInstruction(yyIR, ISA_PUSHQ($2), NULL);
    }
    
    | MNM_POPQ register
    {
        asm_IR_addInstruction(yyIR, ISA_POPQ($2), NULL);
    }
    
    ;

register

    : REGISTER
    
    | UNKNOWN_REGISTER
    {
        asm_error(yyfilename, yylineno, 0, "Unknown register: '%s'.", $1);
        YYERROR;
    }
    
    ;

label

    : IDENTIFIER COLON
    {
        asm_IR_addLabel(yyIR, $1);
    }
    
    ;

directive

    : ALIGN INT_LITERAL
    {
        asm_IR_setAlignment(yyIR, $2);
    }
    
    | POSITION INT_LITERAL
    {
        asm_IR_setPosition(yyIR, $2);
    }
    
    | QUAD INT_LITERAL
    {
        asm_IR_addQuad(yyIR, $2);
    }
    
    | RANDOM INT_LITERAL
    {
        asm_IR_addRandom(yyIR, $2, 1, 1);
    }
    
    | RANDOM INT_LITERAL INT_LITERAL
    {
        asm_IR_addRandom(yyIR, $2, $3, 1);
    }
    
    | RANDOM INT_LITERAL INT_LITERAL INT_LITERAL
    {
        asm_IR_addRandom(yyIR, $2, $3, $4);
    }
    
    ;

unknown

    : UNKNOWN_DIRECTIVE
    {
        asm_error(yyfilename, yylineno, 0, "Unknown directive: '%s'.", $1);
        YYERROR;
    }
    
    | UNKNOWN_TOKEN
    {
        asm_error(yyfilename, yylineno, 0, "Unexpected symbol: '%s'.", $1);
        YYERROR;
    }
    
    ;

%%

char const * yyfilename;

static int yyerror(char const * message)
{
    asm_error(yyfilename, yylineno, 0, message);
    
    return 0;
}
