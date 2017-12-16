//
// (C) Finn Schiermer Andersen, 2016
//
// ano_predictor.c
//
// Implementation of simple predictor models.
//

#include <stdlib.h>
#include <stdio.h>

#include "ano_predictor.h"

typedef enum { pred_taken, pred_btfnt, pred_local, pred_gshare, pred_oracle, pred_return } pred_kind_t;

struct predictor_t {
    pred_kind_t kind;
    unsigned long num_predictions;
    unsigned long num_hits;
    int history_size;
    int history;
    int stackp;
    // only one of the following used:
    char* state_machines;
    unsigned long* return_stack;
};

typedef enum { p_snt, p_wnt, p_wt, p_st } pred_state_t;

static predictor_ptr create_predictor(int history_size) {
    predictor_ptr res = malloc(sizeof(struct predictor_t));
    res->num_predictions = 0;
    res->num_hits = 0;
    res->history_size = history_size;
    res->history = 0;
    res->return_stack = 0;
    if (history_size) {
	res->state_machines = malloc(1<<history_size);
	// init to weakly taken:
	for (int i=0; i<(1<<history_size); ++i)
	    res->state_machines[i] = p_wt;
    } else {
	res->state_machines = 0;
    }
    return res;
}

predictor_ptr create_taken_predictor() {
    predictor_ptr res = create_predictor(0);
    res->kind = pred_taken;
    return res;
}

predictor_ptr create_oracle_predictor() {
    predictor_ptr res = create_predictor(0);
    res->kind = pred_oracle;
    return res;
}

predictor_ptr create_btfnt_predictor() {
    predictor_ptr res = create_predictor(0);
    res->kind = pred_btfnt;
    return res;
}

predictor_ptr create_gshare_predictor(int history_size) {
    predictor_ptr res = create_predictor(history_size);
    res->kind = pred_gshare;
    return res;
}

predictor_ptr create_local_predictor(int history_size) {
    predictor_ptr res = create_predictor(history_size);
    res->kind = pred_local;
    return res;
}

predictor_ptr create_return_predictor(int history_size) {
    predictor_ptr res = create_predictor(0);
    res->kind = pred_return;
    res->history_size = history_size;
    res->stackp = 0;
    res->return_stack = malloc(sizeof(unsigned long) * history_size);
    return res;
}

void free_predictor(predictor_ptr predictor) {
    if (predictor->state_machines)
	free(predictor->state_machines);
    if (predictor->return_stack)
	free(predictor->return_stack);
    free(predictor);
}

void print_predictor(predictor_ptr predictor) {
    if (predictor->num_predictions) {
	printf("    Predictions: %ld   hits: %ld   hitrate: %f\n",
	       predictor->num_predictions,
	       predictor->num_hits,
	       predictor->num_hits*1.0 / predictor->num_predictions);
    } else {
	printf("    No predictions\n");
    }
}

int train_predictor(predictor_ptr predictor, unsigned long pc, unsigned long target, int is_taken) {
    predictor->num_predictions++;
    switch (predictor->kind) {
    case pred_taken:
	predictor->num_hits += is_taken != 0;
	return is_taken;
    case pred_oracle:
	predictor->num_hits++;
	return 1;
    case pred_btfnt: {
	int backward = target < pc;
	int hit = (is_taken == backward);
	predictor->num_hits += hit;
	return hit;
    }
    case pred_local:
    case pred_gshare: 
    {
	int index = ((1<<predictor->history_size)-1) & pc;
	if (predictor->kind == pred_gshare)
	    index ^= predictor->history;
	int predict_taken = predictor->state_machines[index] >= p_wt;
	// now adjust predictor:
	if (is_taken && predictor->state_machines[index] < p_st)
	    predictor->state_machines[index]++;
	else if (!is_taken && predictor->state_machines[index] > p_snt)
	    predictor->state_machines[index]--;
	// update history
	predictor->history |= is_taken << predictor->history_size;
	predictor->history >>= 1;
	int hit = (is_taken == predict_taken);
	predictor->num_hits += hit;
	return hit;
    }
    case pred_return:
	printf("Error - trying to use the return predictor as branch predictor\n");
	exit(1);
    }
    return 0;
}

void note_call(predictor_ptr predictor, unsigned long return_pc) {
    predictor->stackp++;
    if (predictor->stackp == predictor->history_size)
	predictor->stackp = 0;
    predictor->return_stack[predictor->stackp] = return_pc;
}

int predict_return(predictor_ptr predictor, unsigned long target_pc) {
    if (predictor->kind != pred_return) {
	printf("Error - trying to use a branch predictor to predict return\n");
	exit(1);
    }
    unsigned long prediction = predictor->return_stack[predictor->stackp];
    if (predictor->stackp == 0)
	predictor->stackp = predictor->history_size;
    predictor->stackp--;
    int hit = prediction == target_pc;
    predictor->num_hits += hit;
    predictor->num_predictions += 1;
    return hit;
}
