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
    // Setting the rsp register to point to the stack

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
        bool is_RtoMmove = is(MOV_RtoM, major_op);
        bool is_MtoRmove = is(MOV_MtoR, major_op);
	
	bool is_Arithmetic = is(ARITHMETIC, major_op);

	bool is_Push = is(PUSH, major_op);
	bool is_Pop = is(POP, major_op);

        bool is_Call = is(CALL, major_op);     
        bool is_Return = is(RET, major_op);
	
        bool is_Jump = is(JCC, major_op);
        bool do_Jump = is_Jump && eval_condition(cc, minor_op); 	

        bool is_Nop = is(NOP, major_op);
        bool is_Halt = is(HLT, major_op);

        bool is_Small = is_Nop || is_Halt || is_Return;

        bool is_move = is_RtoRmove || is_ItoRmove || is_RtoMmove || is_MtoRmove;
	bool has_regs = is_move || is_Arithmetic || is_Push || is_Pop;
        bool mov_to_mem = is_RtoMmove;
        bool has_imm = is_ItoRmove || mov_to_mem || is_MtoRmove || is_Jump || is_Call;
	bool stack_op = is_Push || is_Pop || is_Call || is_Return;	
	val size = or( use_if(is_Small, from_int(1)),
                       use_if(!is_Small, or(use_if(has_imm, from_int(6)),
					   use_if(!has_imm, from_int(2))))
		       );
	
        size = or(use_if(is_Jump, from_int(5)),
                  use_if(!is_Jump, size));	
       // val reg_a = pick_bits(12,4,inst_word);
       // val reg_b = pick_bits(8,4,inst_word);

        val reg_a = or(use_if(is_MtoRmove, pick_bits(8,4, inst_word)),
                       use_if(!is_MtoRmove, pick_bits(12,4, inst_word)));
        val reg_b = or(use_if(is_MtoRmove, pick_bits(12,4, inst_word)),
                       use_if(!is_MtoRmove, pick_bits(8,4, inst_word)));

	reg_b = or(use_if(stack_op, REG_SP),
		   use_if(!stack_op, reg_b));	

	val imm_bytes = or( use_if(!has_regs, pick_bits(8, 32, inst_word)),
                            use_if(has_regs, pick_bits(16, 32, inst_word)));
        val imm = or(use_if(has_imm, imm_bytes),
		     use_if(!has_imm, from_int(0)));
	
        val sign_extended_imm = sign_extend(31,imm);

        val next_inst_pc = add(pc, size);

        val next_jmp_inst_pc = imm;

        stop = is_Halt;
        
        val op_a;
        val op_b;
        // execute
	
        op_a = memory_read(regs, 1, reg_a, true);
        val reg_a_val = op_a;

	op_a = or(use_if(is_ItoRmove, imm),
		  use_if(!is_ItoRmove, op_a));

        op_a = or(use_if(!stack_op, op_a),
                  use_if(stack_op, or(use_if(is_Push || is_Call,   from_int(-8)),
                                      use_if(!is_Push && !is_Call, from_int(8)))));
	
        op_b = memory_read(regs, 0, reg_b, true);
        
        val alu_method = or(use_if(is_Arithmetic, minor_op),
                            use_if(!is_Arithmetic, from_int(0))); // Set to add

	alu_execute_result alu_res = alu_execute(alu_method, op_a, op_b);
        cc.of = (is_Arithmetic && alu_res.cc.of) || (is_Arithmetic ^ cc.of);
        cc.sf = (is_Arithmetic && alu_res.cc.sf) || (is_Arithmetic ^ cc.sf);
        cc.zf = (is_Arithmetic && alu_res.cc.zf) || (is_Arithmetic ^ cc.zf);
        
	// select result for register update
        val datapath_result_reg = or(use_if(is_Arithmetic || stack_op, alu_res.result),
				     use_if(!is_Arithmetic && !stack_op, op_a));

        val mem_lookup = or(use_if(is_MtoRmove, add(op_a, imm)),
                        use_if(!is_MtoRmove, from_int(0)));

        datapath_result_reg = or(use_if(is_MtoRmove,memory_read(mem,1,mem_lookup,true)),
		                 use_if(!is_MtoRmove,datapath_result_reg));


	val datapath_result_mem = or(use_if(is_RtoMmove, op_a),
                                     use_if(!is_RtoMmove, or(use_if(is_Push, reg_a_val), 
                                                             use_if(!is_Push, imm))));

        // pick result value and target register        
	bool reg_wr_enable =((is_move && !mov_to_mem) && eval_condition(cc, minor_op));
        reg_wr_enable = reg_wr_enable || (is_Arithmetic && !is(0x4, minor_op));
	reg_wr_enable = reg_wr_enable || stack_op;

	bool mem_wr_enable = (is_move && mov_to_mem) || is_Push || is_Call;

	val target_reg = reg_b;

	val target_mem = or(use_if(is_RtoMmove, add(op_b, imm)), 
                            use_if(!is_RtoMmove, add(op_b, from_int(-8))));
	
        // determine PC for next cycle
        val next_pc = or (use_if(do_Jump, next_jmp_inst_pc),
                          use_if(!do_Jump, next_inst_pc));

        // potentially pretty-print something to show progress before
        // ending cycle and overwriting state from start of cycle:
        printf("%lx : ", pc.val);
        for (int j=0; j<size.val; ++j) {
          unsigned int byte = (inst_word.val >> (8*j)) & 0xff;
            printf("%x ", byte);
        }
        if (reg_wr_enable)
            printf("\t\tr%ld = %lx\n", target_reg.val, datapath_result_reg.val);
        else
            printf("\n");

        if ((tracer != NULL) & !stop) {
            // Validate values written to registers and memory against trace from
            // reference simulator. We do this *before* updating registers in order
            // to have the old register content available in case trace validation
            // fails. If so, the error function is called, and by setting a breakpoint
            // on it, all values are easy to inspect with a debugger.
            validate_pc_wr(tracer, instruction_number, next_pc);

	    if (reg_wr_enable) {
	      validate_reg_wr(tracer, instruction_number, target_reg, datapath_result_reg);
	    }
 
	    if (mem_wr_enable) {
	      validate_mem_wr(tracer, instruction_number, target_mem, datapath_result_mem);
	    }
        }

        // store results at end of cycle
        pc = next_pc;
        memory_write(regs, 0, target_reg, datapath_result_reg, reg_wr_enable);
        memory_write(mem, 0, target_mem, datapath_result_mem, mem_wr_enable);
    }
    printf("Done\n");
}
