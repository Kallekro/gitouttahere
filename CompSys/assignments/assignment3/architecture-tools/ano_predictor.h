//
// (C) Finn Schiermer Andersen, 2016
//
// ano_predictor.h
//
// Simple branch predictor models.
//

#ifndef __PREDICTOR_H__
#define __PREDICTOR_H__

struct predictor_t;
typedef struct predictor_t* predictor_ptr;

/*
  Create specific type of branch predictor. The following are are supported:
  - predict taken
  - predict backward taken, forward not taken
  - predict like an oracle - correct all the time
  - predict based on local history (history of individual branch)
    Uses least significant bits of PC to index a table of
    two-bit predictors.
  - predict based on a mix of local and global history.
    often called a "gshare" predictor in the research litterature.
    This predictor uses a bitvector to encode the last branch
    decisions. For prediction it is xor'ed with the PC of the 
    branch, the result used to index the predictor table.
*/
predictor_ptr create_taken_predictor(void);
predictor_ptr create_btfnt_predictor(void);
predictor_ptr create_oracle_predictor(void);
predictor_ptr create_local_predictor(int history_size);
predictor_ptr create_gshare_predictor(int history_size);

void free_predictor(predictor_ptr);
void print_predictor(predictor_ptr);

int train_predictor(predictor_ptr predictor, unsigned long pc, unsigned long target, int is_taken);

/*
  Create a return predictor. This predictor just has a small
  stack into which we push and pop return addresses.
 */
predictor_ptr create_return_predictor(int stack_size);
void note_call(predictor_ptr predictor, unsigned long return_pc);
int predict_return(predictor_ptr predictor, unsigned long target_pc);

#endif // __PREDICTOR_H__
