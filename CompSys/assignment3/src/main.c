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

// Macro for parsing condition codes
#define CCParser(cur, is_arith, result) (is_arith && result) || ((is_arith ^ cur) && !is_arith);


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
        
        // booleans for different move instructions
        bool is_RtoRmove = is(MOV_RtoR, major_op);
        bool is_ItoRmove = is(MOV_ItoR, major_op);
        bool is_RtoMmove = is(MOV_RtoM, major_op);
        bool is_MtoRmove = is(MOV_MtoR, major_op);
	
        // boolean for arithmetic operations
        bool is_Arithmetic = is(ARITHMETIC, major_op);

        // booleans for stack manipulation operations
        bool is_Push = is(PUSH, major_op);
        bool is_Pop = is(POP, major_op);

        // booleans for call and return
        bool is_Call = is(CALL, major_op);     
        bool is_Return = is(RET, major_op);
	
        // boolean for jump instruction 
        bool is_Jump = is(JCC, major_op);
        // if program should actually jump depends on current condition codes
        bool do_Jump = is_Jump && eval_condition(cc, minor_op); 	
        
        // booleans for nop and halt instructions
        bool is_Nop = is(NOP, major_op);
        bool is_Halt = is(HLT, major_op);

        // these three instructions have size 1
        bool is_Small = is_Nop || is_Halt || is_Return;

        // these instructions are move instructions
        bool is_move = is_RtoRmove || is_ItoRmove || is_RtoMmove || is_MtoRmove;
        // these instructions contain registers
      	bool has_regs = is_move || is_Arithmetic || is_Push || is_Pop;
        // these instructions contain immediate values
        bool has_imm = is_ItoRmove || is_RtoMmove || is_MtoRmove || is_Jump || is_Call;
        // these instructions are stack operations
	      bool stack_op = is_Push || is_Pop || is_Call || is_Return;	

        // Find size
      	val size = or( use_if(is_Small, from_int(1)),
                       use_if(!is_Small, or(use_if(has_imm, from_int(6)),
                                            use_if(!has_imm, from_int(2)))));
	
        size = or(use_if(is_Jump || is_Call, from_int(5)),
                  use_if(!is_Jump && !is_Call, size));	

        bool flipped = is_MtoRmove || (is_Arithmetic && is(0x4, minor_op));
        // find register a in instruction
        // the registers are flipped in memory to register move
        val reg_a = or(use_if(flipped, pick_bits(8,4, inst_word)),
                       use_if(!flippeD, pick_bits(12,4, inst_word)));

        // find instruction b in instruction
        // the registers are flipped in memory to register move
        val reg_b = or(use_if(is_MtoRmove, pick_bits(12,4, inst_word)),
                       use_if(!is_MtoRmove, pick_bits(8,4, inst_word)));

        // if the operation is a stack operation the target register aka. register b is the stack pointer register
	      reg_b = or(use_if(stack_op, REG_SP),
		               use_if(!stack_op, reg_b));	

        // find the immediate bytes in the instruction
	      val imm_bytes = or( use_if(!has_regs, pick_bits(8, 32, inst_word)),
                            use_if(has_regs, pick_bits(16, 32, inst_word)));

        // sign extend the immediate bytes
        // currently the value is only occupying half of the 8 bytes in val
        // so occupy the rest with the correct sign
        val sign_extended_imm = sign_extend(31,imm_bytes);

        val next_inst_pc = add(pc, size); 

        stop = is_Halt;
        
        val op_a;
        val op_b;
        // execute
	
        op_a = memory_read(regs, 0, reg_a, true);
        op_b = memory_read(regs, 1, reg_b, true);
        // store the value in the register
        // because pop needs it after operand a has been altered
        val reg_a_val = op_a;

        // if imm to reg operand operand a is the immediates
       	op_a = or(use_if(is_ItoRmove, sign_extended_imm),
		              use_if(!is_ItoRmove, op_a));

        // if push or call, operand a is -8
        // if pop or return, operand a is 8
        // this is the value used to increment or decrement the stack pointer
        op_a = or(use_if(!stack_op, op_a),
                  use_if(stack_op, or(use_if(is_Push || is_Call,   from_int(-8)),
                                      use_if(!is_Push && !is_Call, from_int(8)))));
	
        // if arithmentic use the correct alu method, specified in minor_op
        // else be sure to use add (to decrement/increment the stack pointer) 	
        val alu_method = or(use_if(is_Arithmetic, minor_op),
                            use_if(!is_Arithmetic, from_int(0))); // 0 = add

        // execute the alu and get both the result and the condition codes
        alu_execute_result alu_res = alu_execute(alu_method, op_a, op_b);

        // Set condition codes
        // the macro we use will set the codes to be the codes from the alu result if the instruction is an arithmetic operation
        // and if the instruction is not an arithmetic operation the condition codes will remain as they were
        cc.of = CCParser(cc.of, is_Arithmetic, alu_res.cc.of); 
        cc.sf = CCParser(cc.sf, is_Arithmetic, alu_res.cc.sf);
        cc.zf = CCParser(cc.zf, is_Arithmetic, alu_res.cc.zf);
        
	      // select result for register update
        // if either arithmetic or stack instruction we use the result from the alu execution
        // this is either the decremented pointer or the result of an arithmetic operation
        // if neither arithmetic or stack instruction the result is either the immediate or the register value being moved
        val datapath_result_reg = or(use_if(is_Arithmetic || stack_op, alu_res.result),
			                               use_if(!is_Arithmetic && !stack_op, op_a));

        // if memory to register the memory needs to be read and stored in the register update result
        datapath_result_reg = or(use_if(is_MtoRmove, memory_read(mem, 2, add(op_a, sign_extended_imm), is_MtoRmove)),
                                 use_if(!is_MtoRmove,datapath_result_reg));

        // select result for memory update
        // there are three different scenarios for memory update, 
        // register to memory uses the value in the register
        // push also uses this value, but has overwritten op_a
	      val datapath_result_mem = or(use_if(is_RtoMmove, op_a),
                                     use_if(!is_RtoMmove, or(use_if(is_Push, reg_a_val), 
                                                             use_if(!is_Push, sign_extended_imm))));
        // call writes the next instruction so return can return to here 
        datapath_result_mem = or(use_if(is_Call, next_inst_pc),
                                 use_if(!is_Call, datapath_result_mem));
        
        // set register write enable      
        // if move to register and the conditions are met (for movq the minor_op is 0 therefor always move)
        bool reg_wr_enable =((is_move && !is_RtoMmove) && eval_condition(cc, minor_op));
        // if arithmetic operation and not compare or stack operation
        reg_wr_enable = reg_wr_enable || (is_Arithmetic && !is(0x4, minor_op)) || stack_op;
        
        // set memory write enable
        // only these three instructions need to write to memory  
        bool mem_wr_enable = is_RtoMmove || is_Push || is_Call;

        // we chose the standard that register b is the target register
        val target_reg = reg_b;

        // target memory is either a displaced address or the next free space on the stack
        val target_mem = or(use_if(is_RtoMmove, add(op_b, sign_extended_imm)), 
                            use_if(!is_RtoMmove, add(op_b, from_int(-8))));

        // if pop or return read value at the top of the stack 
        val top_stack = memory_read(mem, 2, memory_read(regs, 2, REG_SP, is_Pop || is_Return), is_Pop || is_Return);
	
        // determine PC for next cycle
        val next_pc = or( use_if(do_Jump || is_Call, sign_extended_imm),
                          use_if(!do_Jump && !is_Call, or( use_if(is_Return, top_stack), 
                                                           use_if(!is_Return, next_inst_pc))));

        // potentially pretty-print something to show progress before
        // ending cycle and overwriting state from start of cycle:
        printf("%lx : ", pc.val);
        for (int j=0; j<size.val; ++j) {
          unsigned int byte = (inst_word.val >> (8*j)) & 0xff;
            printf("%x ", byte);
        }
        if (reg_wr_enable) {
            if (size.val != 6 || size.val != 5) printf("\t");
            printf("\t\tr%ld = %lx\n", target_reg.val, datapath_result_reg.val);
        }
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
        // pop needs to write to two registers, the target from instruction and the stack pointer
        memory_write(regs, 1, reg_a, top_stack, is_Pop);
        memory_write(mem, 0, target_mem, datapath_result_mem, mem_wr_enable);
    }
    printf("Done\n");
}
