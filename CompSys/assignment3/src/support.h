#include <stddef.h>
#include "wires.h"

struct trace_reader;
typedef struct trace_reader *trace_reader_p;

void error(const char* message);

trace_reader_p create_trace_reader(const char* filename);
void delete_trace_reader(trace_reader_p);
void validate_reg_wr(trace_reader_p, int instruction_number, val addr, val value);
void validate_mem_wr(trace_reader_p, int instruction_number, val addr, val value);
void validate_pc_wr(trace_reader_p, int instruction_number, val pc);
