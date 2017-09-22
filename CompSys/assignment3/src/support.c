#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include "support.h"
#include "trace_read.h"

void error(const char* message) {
    fprintf(stderr, "%s\n", message);
    exit(-1);
}

struct trace_reader {
    FILE* f;
    int current_insn_number;
    int input_failed;
    int entries_valid;
    Trace_Entry entries[4];
};

trace_reader_p create_trace_reader(const char* filename) {
    trace_reader_p res = (trace_reader_p) malloc(sizeof(struct trace_reader));
    res->f = fopen(filename,"r");
    if (res->f == NULL)
        error("Unable to open trace file");
    res->current_insn_number = 0;
    res->input_failed = false;
    res->entries_valid = 0;
    return res;
}

void delete_trace_reader(trace_reader_p tracer) {
    fclose(tracer->f);
    free(tracer);
}

void validate_magic(trace_reader_p tracer, int magic, int instruction_number,
                    uint64_t addr, uint64_t value) {

    // first fill in as many entries as we can possibly need (and get)
    while (!tracer->input_failed & (tracer->entries_valid < 4)) {
        size_t num_read = 
            Trace_read(tracer->f, 
                       &tracer->entries[tracer->entries_valid], 1);

        if (num_read == 1)
            tracer->entries_valid++;
        else
            tracer->input_failed = true;
    }
    // search among available entries for a match
    int match = 0;
    while (match < tracer->entries_valid) {
        Trace_Entry* entry = &tracer->entries[match];
        if (entry->type == magic && entry->destination == addr
            && entry->value == value && entry->counter == instruction_number) {
            // match! remove it from buffer
            while (match+1 < tracer->entries_valid) {
                tracer->entries[match] = tracer->entries[match+1];
                ++match;
            }
            --tracer->entries_valid;
            return; // yup! we're fine
        }
        ++match;
    }
    // we didn't match
    error("Trace validation error");
}

void validate_reg_wr(trace_reader_p tracer, int instruction_number, val addr, val value) {
    validate_magic(tracer, Trace_Type_reg, instruction_number, addr.val, value.val);
}

void validate_mem_wr(trace_reader_p tracer, int instruction_number, val addr, val value) {
    validate_magic(tracer, Trace_Type_mem, instruction_number, addr.val, value.val);
}

void validate_pc_wr(trace_reader_p tracer, int instruction_number, val pc) {
    validate_magic(tracer, Trace_Type_pc, instruction_number, 0, pc.val);
}
