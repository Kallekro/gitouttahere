//
// (C) Finn Schiermer Andersen, 2016
//
// ano.h
//
// Annotation support.
//

#ifndef __ANNOTATION_H__
#define __ANNOTATION_H__

#include "isa_register.h"
#include "isa_word.h"

// -- Old types from book-code. TODO: Use types from 'isa.h'. -- //

typedef unsigned long reg_id_t;

typedef unsigned char byte_t;

typedef long long int word_t;

typedef long long unsigned uword_t;

#define REG_RSP isa_Register_rsp

// ---- //

// All the annotation-relevant data. opaque type.

struct annotation_t;

typedef struct annotation_t * annotation_ptr;

typedef struct annotation_t Annotation;

// startup & shutdown:

annotation_ptr create_annotation(int argc, char const * argv[]);

void annotation_usage(void);

void print_annotation(annotation_ptr annotation);
void free_annotation(annotation_ptr annotation);

// make note related to instruction, always call note_insn first:

void note_insn(annotation_ptr annotation, char* insn, word_t pc);

void note_done(annotation_ptr annotation);
void note_done_reg(annotation_ptr annotation, char* reg);
void note_done_cc(annotation_ptr annotation, char* cc);
void note_done_reg_cc(annotation_ptr annotation, char* reg, char* cc);
void note_done_mem(annotation_ptr annotation, char* mem);
void note_done_reg_reg(annotation_ptr annotation, char* reg1, char* reg2);
void note_done_reg_mem(annotation_ptr annotation, char* reg, char* mem);

// determine timing for a specific instruction:

void model_nop(annotation_ptr annotation);
void model_alu(annotation_ptr annotation, reg_id_t src, reg_id_t dest);
void model_ialu(annotation_ptr annotation, reg_id_t dest);
void model_jmp(annotation_ptr annotation, word_t next_pc, int is_conditional, int is_taken);
void model_call(annotation_ptr annotation, word_t return_pc, word_t target, word_t push_addr);
void model_ret(annotation_ptr annotation, word_t next_pc, word_t pop_addr);
void model_rrmov(annotation_ptr annotation, reg_id_t src, reg_id_t dest, int is_conditional);
void model_irmov(annotation_ptr annotation, reg_id_t dest);
void model_mrmov(annotation_ptr annotation, reg_id_t base, reg_id_t dest, word_t addr);
void model_rmmov(annotation_ptr annotation, reg_id_t base, reg_id_t src, word_t addr);
void model_push(annotation_ptr annotation, reg_id_t src, word_t addr);
void model_pop(annotation_ptr annotation, reg_id_t dest, word_t addr);

#endif // __ANNOTATION_H__
