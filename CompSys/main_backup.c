#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
 
#include "support.h"
#include "wires.h"
#include "arithmetic.h"
#include "memories.h"
#include "alu.h"

#define HLT 0x0
#define NOP 0x1
#define MOV_RtoR 0x2
#define MOV_ItoR 0x3
#define MOV_RtoM 0x4
#define MOV_MtoR 0x5
#define ARITHMETIC 0x6
#define JCC 0x7
#define CALL 0x8
#define RET 0x9
#define PUSH 0xA
#define POP 0xB

#define REG_SP from_int(4)

int main(int argc, char* argv[]) {

    if (argc < 2)
        error("missing name of programfile to simulate");

    trace_reader_p tracer = NULL;
    if (argc == 3) {
        tracer = create_trace_reader(argv[2]);
    }
    // Setup global state. 
    // Each cycle ends by capturing computations in global state.

    // Registerfile:
    // - we use separate read and write ports for sp updates. This can be optimized away.
    mem_p regs = memory_create(16,3,2); // 8-byte cells, 3 readports, 2 writeport

    // Memory:
    // - Shared instruction and data memory. Two read ports for instruction fetch,
    //   one read and one write for data.
    mem_p mem = memory_create(1024,3,1); // 8-byte cells, 3 readports, 1 writeport
    memory_read_from_file(mem, argv[1]);
    // special registers
    val pc = from_int(0);
    conditions cc; cc.of = cc.sf = cc.zf = false;

    // a stop signal for stopping the simulation.
    bool stop = false;

    // We need the instruction number for trace validation
    int instruction_number = 0;

    while (!stop) { // for each cycle:

        ++instruction_number;

        // fetch next instruction
        val inst_word = memory_read_unaligned(mem, 0, 1, pc, true);

        // decode
        val major_op = pick_bits(4,4, inst_word);
        val minor_op = pick_bits(0,4, inst_word);

	bool is_RtoRmove = is(MOV_RtoR, major_op);
        bool is_ItoRmove = is(MOV_ItoR, major_op);
        bool is_move = is_RtoRmove || is_ItoRmove;
        bool has_double_regs = is_RtoRmove;
        bool has_reg = is_ItoRmove;

        val size = or( use_if(!has_double_regs && !has_reg, from_int(1)),
                       or( 
                         use_if(has_double_regs, from_int(2)),
                         use_if(has_reg, from_int(6))));

                
        val reg_a = pick_bits(12,4, inst_word);
        val reg_b = pick_bits(8,4, inst_word);
        val imm_bytes = or( use_if(!has_double_regs && !has_reg, pick_bits(8, 32, inst_word)),
                            use_if(has_double_regs || has_reg, pick_bits(16, 32, inst_word)));
        val imm = imm_bytes;
        val sign_extended_imm = sign_extend(31,imm);
        val next_inst_pc = add(pc, size);
        stop = is(HLT, major_op);
        
        val op_a;
        // execute
        if (has_double_regs) {
          op_a = memory_read(regs, 0, reg_a, true);
        }
        else {
          op_a = imm;
        }
        val op_b = memory_read(regs, 1, reg_b, true);

        // select result for register update
        val datapath_result = op_a;

        // pick result value and target register
        val target_reg = reg_b;
        bool reg_wr_enable = is_move;

        // determine PC for next cycle
        val next_pc = next_inst_pc;

        // potentially pretty-print something to show progress before
        // ending cycle and overwriting state from start of cycle:
        printf("%lx : ", pc.val);
        for (int j=0; j<size.val; ++j) {
          unsigned int byte = (inst_word.val >> (8*j)) & 0xff;
            printf("%x ", byte);
        }
        if (reg_wr_enable)
            printf("\t\tr%ld = %lx\n", target_reg.val, datapath_result.val);
        else
            printf("\n");

        if ((tracer != NULL) & !stop) {
            // Validate values written to registers and memory against trace from
            // reference simulator. We do this *before* updating registers in order
            // to have the old register content available in case trace validation
            // fails. If so, the error function is called, and by setting a breakpoint
            // on it, all values are easy to inspect with a debugger.
            validate_pc_wr(tracer, instruction_number, next_pc);
            if (reg_wr_enable)
                validate_reg_wr(tracer, instruction_number, target_reg, datapath_result);
        }

        // store results at end of cycle
        pc = next_pc;
        memory_write(regs, 1, target_reg, datapath_result, reg_wr_enable);
    }
    printf("Done\n");
}
