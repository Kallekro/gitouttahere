#include "alu.h"
#include "arithmetic.h"

bool bool_xor(bool a, bool b) { return a^b; }
bool bool_not(bool a) { return !a; }

bool eval_condition(conditions c, val op) { // from fig 3.15, p. 242
    bool zf = c.zf;
    bool sf = c.sf;
    bool of = c.of;
    bool res_a = is(ALWAYS, op);
    bool res_le = (is(LE, op) & (bool_xor(sf,of)|zf));
    bool res_lt = (is(LT, op) & bool_xor(sf,of));
    bool res_eq = (is(EQ, op) & zf);
    bool res_ne = (is(NE, op) & bool_not(zf));
    bool res_ge = (is(GE, op) & bool_not(bool_xor(sf,of)));
    bool res_gt = (is(GT, op) & bool_not(bool_xor(sf,of)) & bool_not(zf));
    bool res = res_a | res_le | res_lt | res_eq | res_ne | res_ge | res_gt;
    return res;
}

alu_execute_result alu_execute(val op, val op_a, val op_b) {
    alu_execute_result result;
    bool is_sub = is(SUB, op) || is(CMP, op);
    bool is_add = is(ADD, op);
    val val_a = or( use_if(!is_sub, op_a), 
		    use_if( is_sub, neg(64, op_a)));
    generic_adder_result adder_result = generic_adder(op_b, val_a, is_sub);
    bool adder_of = adder_result.of;
    val adder_res = adder_result.result;
    val res = or( use_if(is_add, adder_res),
		  or( use_if(is_sub, adder_res),
		      or( use_if(is(AND, op), and(op_a, op_b)),
			  use_if(is(XOR, op), xor(op_a, op_b)))));

    result.cc.sf = pick_one(63, res);
    result.cc.zf = !reduce_or(res);
    result.cc.of = (is_sub || is_add) && adder_of; //otherwise cleared
    result.result = res;
    return result;
}

#include "wires.h"
/*
  A simple ALU matching the subset of x86 that we'll need
 */
// Encoding of ALU operations
#define ADD 0
#define SUB 1
#define AND 2
#define XOR 3
#define CMP 4

// Encoding of conditions
#define ALWAYS 0
#define LE 1
#define LT 2
#define EQ 3
#define NE 4
#define GE 5
#define GT 6

typedef struct {
    bool of;
    bool zf;
    bool sf;
} conditions;

bool eval_condition(conditions cc, val op);

typedef struct {
    val result;
    conditions cc;
} alu_execute_result;

alu_execute_result alu_execute(val op, val op_a, val op_b);

#include "arithmetic.h"


bool same_sign(val a, val b) {
    return ! (pick_one(63,a) ^ pick_one(63, b));
}

// For demonstration purposes we'll do addition without using the '+' operator :-)
// in the following, p == propagate, g == generate.
// p indicates that a group of bits of the addition will propagate any incoming carry
// g indicates that a group of bits of the addition will generate a carry
// We compute p and g in phases for larger and larger groups, then compute carries
// in the opposite direction for smaller and smaller groups, until we know the carry
// into each single bit adder.

typedef struct { val p; val g; } pg;

pg gen_pg(pg prev) {
    hilo p_prev = unzip(prev.p);
    hilo g_prev = unzip(prev.g);
    pg next;
    next.p = and( p_prev.lo, p_prev.hi);
    next.g = or( g_prev.hi, and( g_prev.lo, p_prev.hi));
    return next;
}

val gen_c(pg prev, val c_in) {
    hilo p_prev = unzip(prev.p);
    hilo g_prev = unzip(prev.g);
    hilo c_out;
    c_out.lo = c_in;
    c_out.hi = or( and(c_in, p_prev.lo), g_prev.lo);
    return zip(c_out);
}

generic_adder_result generic_adder(val val_a, val val_b, bool carry_in) {
    // determine p and g for single bit adds:
    pg pg_1;
    pg_1.p = or( val_a, val_b);
    pg_1.g = and( val_a, val_b);
    // derive p and g for larger and larger groups
    pg pg_2 = gen_pg(pg_1);
    pg pg_4 = gen_pg(pg_2);
    pg pg_8 = gen_pg(pg_4);
    pg pg_16 = gen_pg(pg_8);
    pg pg_32 = gen_pg(pg_16);
    // then derive carries for smaller and smaller groups
    val c_64 = use_if(carry_in, from_int(1));
    val c_32 = gen_c(pg_32, c_64);
    val c_16 = gen_c(pg_16, c_32);
    val c_8 = gen_c(pg_8, c_16);
    val c_4 = gen_c(pg_4, c_8);
    val c_2 = gen_c(pg_2, c_4);
    val c_1 = gen_c(pg_1, c_2);
    // we now know all carries!
    generic_adder_result result;
    result.result = xor( xor(val_a, val_b), c_1);
    //check that result.result == val_a + val_b + carry_in;
    result.of = pick_one(63, c_1) ^ pick_one(62, c_1);
    // check that result.of = same_sign(val_a, val_b) && !same_sign(result.result, val_a);
    return result;
}

val add(val a, val b) {
    generic_adder_result tmp;
    tmp = generic_adder(a, b, 0);
    return tmp.result;
}

val use_if(bool control, val value) {
    if (control) return value;
    else return from_int(0);
}

val and(val a, val b) {
    return from_int(a.val & b.val);
}

val or(val a, val b) {
    return from_int(a.val | b.val);
}

val xor(val a, val b) {
    return from_int(a.val ^ b.val);
}

val neg(int num_bits, val a) {
    if (num_bits < 64) {
        uint64_t mask = (((uint64_t) 1) << num_bits) - 1;
        return from_int(~a.val & mask);
    } else {
        return from_int(~a.val);
    }
}

bool is(uint64_t cnst, val a) {
    return a.val == cnst;
}

bool reduce_or(val a) {
    return a.val != 0;
}

bool reduce_and(int num_bits, val a) {
    return neg(num_bits, a).val == 0;
}
/*
  Arithmetic stuff and simple gates
*/

#include "wires.h"

// mask out a value if control is false
val use_if(bool control, val value);

// bitwise and, or, xor and negate for bitvectors
val and(val a, val b);
val or(val a, val b);
val xor(val a, val b);
val neg(int num_bits, val);

// reduce a bit vector to a bool by and'ing or or'ing all elements
bool reduce_and(int num_bits, val);
bool reduce_or(val);

// 64 bit addition
val add(val a, val b);

// detect specific value
bool is(uint64_t cnst, val a);

// 64-bit adder that can also take a carry-in and deliver an overflow status.
typedef struct {
    bool of;
    val result;
} generic_adder_result;

generic_adder_result generic_adder(val val_a, val val_b, bool carry_in);


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
        bool is_move = is(MOV_RtoR, major_op);
        bool has_regs = is_move;
        val size = or( use_if(!has_regs, from_int(1)),
                       use_if(has_regs, from_int(2)));
        val reg_a = pick_bits(12,4, inst_word);
        val reg_b = pick_bits(8,4, inst_word);
        val imm_bytes = or( use_if(!has_regs, pick_bits(8, 32, inst_word)),
                            use_if(has_regs, pick_bits(16, 32, inst_word)));
        val imm = imm_bytes;
        val sign_extended_imm = sign_extend(31,imm);
        val next_inst_pc = add(pc, size);
        stop = is(HLT, major_op);

        // execute
        val op_a = memory_read(regs, 0, reg_a, true);
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
CC = gcc
CFLAGS = -Wall -O0 -std=c11 -ggdb

all: sim

../src.zip: alu.c alu.h arithmetic.c arithmetic.h main.c Makefile memories.c memories.h support.c support.h trace_read.h wires.c wires.h
	cd .. && zip src.zip src/alu.c src/alu.h src/arithmetic.c src/arithmetic.h src/main.c src/Makefile src/memories.c src/memories.h src/support.c src/support.h src/trace_read.h src/wires.c src/wires.h


sim: *.c
	$(CC) $(CFLAGS) *.c -o sim

clean:
	rm -f *.o sim
#include <stdio.h>
#include <stdlib.h>

#include "memories.h"
typedef uint64_t word;

struct memory {
    word* data;
    int num_elements;
};

mem_p memory_create(int num_elements, int rd_ports, int wr_ports) {
    mem_p res = (mem_p) malloc(sizeof(struct memory));
    res->num_elements = num_elements;
    res->data = (word*) malloc(sizeof(word) * num_elements);
    return res;
}

void memory_destroy(mem_p mem) {
    free(mem->data);
    free(mem);
}

void error(const char*);

void memory_read_from_file(mem_p mem, const char* filename) {
    FILE* f = fopen(filename, "r");
    if (f == NULL) {
	error("Failed to open file");
    }
    size_t elements_left = mem->num_elements;
    word* ptr = mem->data;
    // very naughty! ignoring check for errors..
    fread(ptr, sizeof(word), elements_left, f);
}

// return value in selected cell
val memory_read(mem_p mem, int rd_port, val address, bool enable) {
    if (enable)
        return from_int(mem->data[address.val]);
    else
        return from_int(0);
}

// helper function - pick byte from position in double word buffer
int pick_byte(int pos, val a, val b) {
    if (pos >= 8) return pick_byte(pos - 8, b, from_int(0));
    // base case, the byte we want is in 'a'
    return (a.val >> (pos*8)) & 0xFF;
}

// read word from memory. Access is unaligned and may span 2 cells, thus 
// requiring 2 read ports.
val memory_read_unaligned(mem_p mem, int rd_port_a, int rd_port_b, val address, bool enable) {
    val frst_addr = pick_bits(3,61,address);
    val next_addr = from_int(frst_addr.val + 1); // cheating here!
    int pos_in_word = address.val & 0x7;
    val word_a = memory_read(mem, rd_port_a, frst_addr, enable);
    val word_b = memory_read(mem, rd_port_b, next_addr, enable);
    uint64_t result = 0;
    // now pick 8 bytes starting with the last and shift them into result
    for (int k = 7; k >= 0; --k) {
        result <<= 8;
        result |= pick_byte(pos_in_word + k, word_a, word_b);
    }
    return from_int(result);
}



// update selected cell with new value at rising edge of clock
// there are no internal forwarding from write to read in same clock period
void memory_write(mem_p mem, int wr_port, val address, val value, bool wr_enable) {
    if (wr_enable)
        if ((address.val) < mem->num_elements)
            mem->data[address.val] = value.val;
}

/*
  Memory elements

  All memories have 64-bit words.
  Each access port supports only *aligned* access (lower bits in address ignored)
  Unaligned reads are supported using two ports. Storage is little endian.
  (although this can only be observed from the interaction of unaligned reads
  and aligned reads/writes).
  For aligned access, addresses are word addresses.
  For unaligned access, addresses are byte addresses.
*/

#include "wires.h"

struct memory;
typedef struct memory *mem_p;

mem_p memory_create(int num_elements, int rd_ports, int wr_ports);
void memory_destroy(mem_p);
void memory_read_from_file(mem_p, const char* filename);

// return value in selected cell
val memory_read(mem_p, int rd_port, val address, bool enable);

// read word from memory. Access is unaligned and may span 2 cells, thus 
// requiring 2 read ports.
val memory_read_unaligned(mem_p, int rd_port_a, int rd_port_b, val address, bool enable);


// update selected cell with new value at rising edge of clock
// there are no internal forwarding from write to read in same clock period
void memory_write(mem_p, int wr_port, val address, val value, bool wr_enable);
ELF          >    �@     @       8t          @ 8 	 @ $ !       @       @ @     @ @     �      �                   8      8@     8@                                          @       @     (      (                    .      .`     .`     x      �                    (.      (.`     (.`     �      �                   T      T@     T@     D       D              P�td   �       � @     � @     l      l             Q�td                                                  R�td   .      .`     .`     �      �             /lib64/ld-linux-x86-64.so.2          GNU                        GNU F�Zۤ��\���FK�)(*Y            �   �        �!c9�                        n                      !                      )                                            N                      9                      U                      \                      T                      s                       @                                                                       @     3       G     �0`             libc.so.6 exit fopen error puts putchar __isoc99_fscanf fclose malloc stderr fread fprintf __libc_start_main free __gmon_start__ GLIBC_2.7 GLIBC_2.2.5                               ii   �      ui	   �       �/`        
           �0`                   0`                    0`                   (0`                   00`                   80`                   @0`                   H0`                   P0`                   X0`        	           `0`                   h0`                   p0`                   H��H�u)  H��t��   H���              �5b)  �%d)  @ �%b)  h    ������%Z)  h   ������%R)  h   ������%J)  h   �����%B)  h   �����%:)  h   �����%2)  h   �����%*)  h   �p����%")  h   �`����%)  h	   �P����%)  h
   �@����%
)  h   �0����%�(  f�        1�I��^H��H���PTI���@ H��p@ H���@ �w����fD  ��0` UH-�0` H��H��v�    H��t]��0` ��f�     ]�@ f.�     ��0` UH��0` H��H��H��H��?H�H��t�    H��t]��0` �� ]�fD  �=q(   uUH���n���]�^(  ��@ � .` H�? u� �    H��t�UH����]�z���UH������U��E��E�2E�������]�UH����E��E�����������]�UH��SH��(H�}�H�u��EوE��EڈE��E؈E�H�E�H�ƿ    �  �E�H�E�H�ƿ   �  ���U��E�։��e���
E���!؅����E�H�E�H�ƿ   �d  ���U��E�։��0�����!؅����E�H�E�H�ƿ   �2  ���E�!Ѕ����E�H�E�H�ƿ   �  ���E��������!؅����E�H�E�H�ƿ   ��  ���U��E�։����������������!؅����E�H�E�H�ƿ   �  ���U��E�։��t�������������!��E���y�����!؅����E��E�
E�
E�
E�
E�
E�
E��������E��E�H��([]�UH��ATSH��   H��p���H��`���H��P���H��p���H�ƿ   �
  ��uH��p���H�ƿ   ��  ��t�   ��    �E��e�H��p���H�ƿ    ��  �E�H��`���H�ƿ@   �]  H���E�H�։��  H���E�����H��`���H�։��  H��H����  H�E��U�H�M�H��P���H��H����  ��H�ЉM�H�E��EЈE�H�E�H�E�H��P���H��`���H��H���  H��H��p���H�ƿ   �  ��H�މ��  H��H��P���H��`���H��H���"  I��H��p���H�ƿ   ��  ��L�����  H��H���  H���E�H�U�H�։��  H��H����  H���E�H�U�H�։��  H��H����  H�E�H�E�H�ƿ?   �g  �E�H�E�H���j  �������������Eɀ}� u�}� t�}� t�   ��    ���E�H�E�H�E�H�E�H�E�H�E�H�E�H�U�H�E�H��H��H��H�İ   [A\]�UH��SH��(H�}�H�u�H�E�H�ƿ?   ��  ��H�E�H�ƿ?   �  1�������������H��([]�UH��H��@H��H��H��H�E�H�U�H�E�H����  H�E�H�U�H�E�H����  H�E�H�U�H�U�H�E�H��H���  H�E�H�U�H�E�H��H���h  H��H�E�H��H���{  H�E�H�E�H�U���UH��SH��XH��H��H��H��H�M�H�]�H�U�H�E�H���a  H�E�H�U�H�E�H���M  H�E�H�U�H�E�H�E�H�U�H�E�H��H����  H��H�E�H��H����  H�E�H�U�H�E�H��H���  H��X[]�UH��SH��  H�� ���H�������Ј�����H������H�� ���H��H���  H�E�H������H�� ���H��H���\  H�E�H�U�H�E�H��H���r���H�E�H�U�H�U�H�E�H��H���W���H�E�H�U�H�U�H�E�H��H���<���H�E�H�U�H�U�H�E�H��H���!���H�E�H�U�H�U�H�E�H��H������H�E�H�Uؿ   �  H��������H�։��  H�����H�����H�M�H�E�H��H���J���H�� ���H�� ���H�M�H�E�H��H���)���H��0���H��0���H�M�H�E�H��H������H��@���H��@���H�M�H�E�H��H�������H��P���H��P���H�M�H�E�H��H�������H��`���H��`���H�M�H�E�H��H������H��p���H������H�� ���H��H���  H��H��p���H��H���  H�E�H��p���H�ƿ?   �i  ��H��p���H�ƿ>   �S  1��������E�H�E�H�U�H��H�Ӊ�H��  []�UH��H��0H�}�H�u�H�M�H�E�    H��H��������H�ЉM�H�E�H�E���UH��H����H�u��E��}� tH�E��
�    ��  ��UH��H�� H�}�H�u�H�U�H�E�H!�H����  ��UH��H�� H�}�H�u�H�U�H�E�H	�H���  ��UH��H�� H�}�H�u�H�U�H�E�H1�H���  ��UH��H�� �}�H�u��}�?-�E�   ��H��H��H��H�E�H�E�H��H#E�H���?  �H�E�H��H���.  ��UH��H�}�H�u�H�E�H;E���]�UH��H�}�H�E�H����]�UH��H���}�H�u�H�U��E�H�։��`���H������UH��SH��H  ������H������������
��@ ��  H�E�    ������uH������H��H� H����  H�Eغ   �   �   �N  H�E�   �   �   �6  H�E�H������H��H�H�E�H��H���  �    �)  H������ƅ���� ������������������������ƅ���� ǅ����    �  ������H������H�E�A�   H�Ѻ   �    H���  H������H������H�¾   �   �$  H�� ���H������H�¾   �    �  H�����H�� ���H�ƿ   �7����������������������   �F  H��������H�։�����H�ÿ   �%  H������������H�։������H��H���7���H�� ���H������H�¾   �   �f
  H��0���H������H�¾   �   �F
  H��@���H������H�¾    �   �&
  H��������H�։��u���H��H������H�¾    �   ��	  H������������H�։��?���H��H������H��P���H��P���H��`���H��`���H�ƿ   �
  H��p���H�� ���H������H��H������H�E�H�� ���H�ƿ    ����������H��0���H�E�   �    H���  H�E�H��@���H�E�   �   H���f  H�E�H�E�H�E�H��@���H�E�������������H�E�H�E�H������H�ƿ  @ �    ����ǅ����    �AH����������������H��H��%�   �������������ƿ' @ �    �i���������������Hc�H�� ���H9�r������� tH�U�H�E�H�ƿ+ @ �    �*����
�
   �����H�}� ��������������!Ѕ�t;H�UЋ�����H�E؉�H���  ������ tH�M�H�U�������H�E�H���'  H�E�H������������H�M�H�U�H�E�A��   H���~  �����������L����9 @ �K����    H��H  []�UH��H�� �}�u�U�   ����H�E�H�E��U�P�E�H�H��H���b���H��H�E�H�H�E���UH��H��H�}�H�E�H� H������H�E�H���������UH��H��0H�}�H�u�H�Eо> @ H������H�E�H�}� u
�@ @ �  H�E؋@H�H�E�H�E�H� H�E�H�M�H�U�H�E��   H���q������UH��H�� H�}��u�H�U��ȈE��}� tH�E�H� H�U�H��H�H� H���  �
�    ��  ��UH��H�� �}�H�u�H�U��}�~#�    ��  H�E��H�H�E�H�Ɖ�������H�U��E�����H��H������UH��H��pH�}��u��U�H�M�D���E�H�E�H�¾=   �   ��  H�E�H�E�H��H���^  H�E�H�E����E��M�H�U��u�H�E�H�������H�E��M�H�UЋu�H�E�H�������H�E�H�E�    �E�   �*H�e��U��E��H�U�H�E�H�Ɖ������H�H	E��m��}� y�H�E�H����  ��UH��H�}��u�H�U�H�M�D���E܀}� t+H�U�H�E��@H�H9�sH�E�H� H�U�H��H�H�E�H��]�UH��H��0H�}�H�u�H�U�H�E�    �   H�E�H��H��H�E�H�H�pH�E�H��H��H�E�H�H�HH�E�H��H��H�E�H�H�PH�E�H��H��H�E�H�H��H�E�I��I��H��H���T @ H�Ǹ    �������tH�E��H�E�H�E�H;E��i���H�E���UH��H��H�}�H��  H�U��e @ H�Ǹ    ����������+���UH��H�� H�}还   �����H�E�H�E�i @ H�������H��H�E�H�H�E�H� H��u
�k @ ����H�E��@    H�E��@    H�E��@    H�E���UH��H��H�}�H�E�H� H���>���H�E�H����������UH��H��@H�}؉uԉU�H�M�L�E��\H�E؋@H�H��H�PH�E�H�H�HH�E�H� �   H��H���$���H�E�H�}�uH�E؋@�PH�E؉P�H�E��@   H�E؋@����H�E؋@����!Є�u��E�    ��   �E�H�H��H�PH�E�H�H��H�E�H�E�H�P�E�H�H9���   H�E�H�@H;E���   H�E�H�@H;E���   H�E�H��E�H�H9�uw�Q�E�HH�E؋U�Hc�H��H�H��H�U�Hc�H��H�H��H�JH�HH�JH�HH�JH�HH�R H�P �E��E�PH�E؋@9�|�H�E؋@�P�H�E؉P��E�H�E؋@;E������� @ ������UH��H��0H�}��u�H�U�H�M�H�u�H�M��U�H�E�I��    H���)������UH��H��0H�}��u�H�U�H�M�H�u�H�M��U�H�E�I��   H����������UH��H�� H�}��u�H�U�H�M��U�H�E�I�ȹ    �   H���������UH��H��@H�}�H�E�H�E�H�E�    H�E�H�E��E�    �>H�E��H�Eԉ�H��H��H	E�H�m�H�E��H�Eԉ�H��H��H	E�H�m�E��}�~�H�E�H���+  H�E�H�E�H���  H�E�H�E�H�U���UH��H��0H��H��H��H�E�H�U�H�E�H�E�H�E�H�E�H�E�    �E�    �EH�E��H�E������H��H��H	E�H�m�H�E���H�E����H��H��H	E�H�m��E��}�~�H�E�H���|   ��UH��H�� �}�u�H�U�H�E�H�E��E��H�m��}�?H�E�   �E��H�e�H�m�H�E�H!E�H�E�H���'   ��UH��}�H�u�H�U��E���H��H�Ѓ�H����]�UH��H�}�H�E�H�E�H�E�]�UH��H�� �}�H�u�H�E�H�E��H�e�H�E���H	E�H�m��E�P��U��u�H�E�H��������UH��H�� �}�H�u�H�U��E�   ��H��H��H!�H�E�H�E�H�U�H�H)�H���a�����f.�      AWAVA��AUATL�%�  UH�-�  SI��I��L)�H��H�������H��t 1��     L��L��D��A��H��H9�u�H��[]A\A]A^A_Ðf.�     ��  H��H���         missing name of programfile to simulate %lx :  %x  		r%ld = %lx
 Done r Failed to open file %lX %lX %lX %lX
 %s
 r Unable to open trace file Trace validation error ;h  ,   ����  �����  �����  �����  ���  ����D  ���l  e����  �����  �����  ����  ���$  8���D  ]���d  �����  �����  �����  ����  '���  O���$  ���L  R���l  |����  �����  /����  �����  [���  ����,  f���L  ����l  ����  5����  �����  ����  H���  ~���,  ���L  ����l  ����  &����  <����  �����  ����  D���T         zR x�      X���*                  zR x�  $      H����    FJw� ?;*3$"       D   ����    A�CZ      d   ����    A�CX   $   �   �����   A�CE��      $   �   m���]   A�CJ��N    $   �   ����O    A�CE�E          �   �����    A�C�  $     4����    A�CE��       $   D  ����Q   A�CH�D         l  ����:    A�Cu      �  ����)    A�Cd      �  ����%    A�C`      �  ����%    A�C`      �  ����%    A�C`        ����S    A�CN     ,  .���    A�CT      L  '���    A�CO      l  ���(    A�Cc   $   �  #����   A�CH��         �  ����J    A�CE     �  ����*    A�Ce      �  ����j    A�Ce       2���I    A�CD     4  [���S    A�CN     T  �����    A�C�     t  G���M    A�CH     �  t����    A�C�     �  ���3    A�C          �  %���r    A�Cm     �  w���*    A�Ce        �����   A�C�    4  ���9    A�Ct      T  ���9    A�Ct      t  4���6    A�Cq      �  J����    A�C�     �  �����    A�C�     �  8���U    A�CP     �  m���%    A�C`        r���    A�CQ      4  h���I    A�CD     T  ����B    A�C}   D   t  ����e    B�B�E �B(�H0�H8�M@r8A0A(B BBB    �  ����                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   P@     0@                                  x@            �@            .`                          .`                   ���o    �@            @@            �@     
       �                                            0`                                        X@            (@            0       	              ���o    �@     ���o           ���o    �@                                                                                                             (.`                     �@     �@     �@     �@     �@     @     @     &@     6@     F@     V@     f@                     GCC: (Ubuntu 5.4.0-6ubuntu1~16.04.4) 5.4.0 20160609 ,             v@     <                      ,           �@     9                      ,    �       �@     �                      ,    �       �@     �                      ,    �       D@     �                      ,    �       @     I                             �      �   v@     <          H   f  int 2  F   m   Y   %   7i   T   �   val ^     val p   �   of �    zf �   sf �    �  �  �   �      �    cc �         �     of �       �    p  �   	�   �   v@            �\  
a �   �l
b �   �h 	�  �   �@            ��  
a �   �l �   �   �@     �      �\  
c �   �H
op �   �@zf �   �Usf 	�   �Vof 
�   �W`  �   �X�  �   �Y�   �   �Z?   �   �[�  �   �\.   �   �]f   �   �^res �   �_    �   U
@     ]      �
op �   ��~V  �   ��~[  �   ��~   �   ���  �   ��~F  �   ��~�   �   ��x    �@M  �   ��~5   �   ��res �   ��  _   �   �   �  �   �@     9      ,  H   f  int 2  F   m   Y   %   7i   T   �   val ^     val p   �   hi �    lo �    5  �   �   of �       �    �  p  �   	  p �    g �    pg �   	  �   �@     O       �M  
a �   �P
b �   �@ 	O  	  @     �       ��  �  	  ��x  �   �@�  �   �Pc  	  �` 	  �   �@     �       �  �  	  ��?  �   ��x  �   ���  �   �@�  �   �P 	�  $�    @     Q      �2  �   $�   ��}�  $�   ��}#  $�   ��}�  &	  ��~�  *	  ��  +	  ��  ,	  ���  -	  ��I  .	  �@l  0�   ��~�  1�   ��~a  2�   ��~c_8 3�   ��~c_4 4�   ��~c_2 5�   ��~c_1 6�   ��~   8�   �P add @�   q@     :       �z  
a @�   �P
b @�   �@tmp A�   �` 	q  F�   �@     )       ��    F�   �lf  F�   �` and K�   �@     %       ��  
a K�   �`
b K�   �P or O�   �@     %       �+  
a O�   �`
b O�   �P xor S�   @     %       �e  
a S�   �`
b S�   �P neg W�   C@     S       ��  ,  W;   �\
a W�   �PX@     -       D  Y^   �h  is `�   �@            ��  :  `^   �h
a `�   �` �  d�   �@            �*  
a d�   �` V  h�   �@     (       �,  h;   �l
a h�   �`      ;  �   ;  �   �@     �        2  T   int -  x  F   m   Y   H   f    �   O   %   74   �   val �     val �   i  �   �   	_  �  '  �   �   	�    of �    zf �   sf �    �  �   
�  ;   �@     �      �    ;   ��}d    ��}  �   �H�  (�   �Pmem -�   �Xpc 0�   ��}cc 1  ��}�  4�   ��}�  7;   ��}�@     �      �  >�   ��}  A�   ��}�  B�   ��~�  C�   ��}�  D�   ��}�  E�   ��~  G�   ��~
  H�   ��~�  I�   ��~imm K�   ��~R  L�   ��~�  M�   ��~V  Q�   ��[  R�   ��B  U�   ���  X�   ���  Y�   ��}�  \�   �@a@     b       j a;   ��}m@     :       �  b^   ��}    z    �   �  �     �   �@     �        �  �8   T   F   m   Y   H   f  int 2  �  �i   h  �i     �   O     0�     ��$    �b    �  ��     ��   r  ��   �  ��    x  ��   (�  ��   0�  ��   8%  ��   @	�   �   H	�  �   P	�  �   X	+  \  `	�  b  h		  b   p	�  b   t	E  p   x	1  F   �	B  T   �	�  h  �	�  x  �	I  !{   �	�  )�   �	�  *�   �	�  +�   �	�  ,�   �	�  .-   �	�  /b   �	�  1~  � 
  �m  �\  b  �\   �  �b  &  �b    +  �   �   x  �     $  �   �  �    �  �   -  %   78   �  val �    val �  '  �  �  �    �     �  	b    �  �    _  �  �@     J       �l  �  b   �\�  b   �XY  b   �Tres �  �h �  �@     *       ��  mem �  �h �  @     j       ��  mem �  �H�  �  �@f �  �XQ  -   �`ptr    �h �   �  &�  �@     I       �Z  mem &�  �hQ  &b   �d=  &�  �P�  &Z  �` �  �  .b   �@     S       ��  pos .b   �la .�  �`b .�  �P �  6�  @     �       ��  mem 6�  ���  6b   ���  6b   ��=  6�  ���  6Z  ���  7�  ���  8�  �@�  9b   ��4  :�  �P;  ;�  �`   <�  �h�@     9       k >b   ��  �  I�@     M       �mem I�  �h  Ib   �d=  I�  �Pf  I�  �@�  IZ  �L  f   S  �   �  �   D@     �      G  H   f  int 2  F   m   Y   %   7i   T   �  �i   �  �B   h  �B     �   O     0�     ��/    �;    �  ��     ��   r  ��   �  ��    x  ��   (�  ��   0�  ��   8%  ��   @	�   �   H	�  �   P	�  �   X	+  g  `	�  m  h		  ;   p	�  ;   t	E  {   x	1  P   �	B  -   �	�  s  �	�  �  �	I  !�   �	�  )�   �	�  *�   �	�  +�   �	�  ,�   �	�  .p   �	�  /;   �	�  1�  � 
  �m  �g  b  �g   �  �m  &  �;    6  �   �   �  �     /  �   �  �    �  �   -  x  	�  val 	^     val 	�  i  �  �  _  �*  f �   �  ;   �  ;   '  ;   �  �   �  W   M  (     �   L   �  �  ^    "  ^   @  ^   f  ^    L  M  �   �  �  �    5  'p   D@     �       �  �  '�  �X�  '  �P  'p   �HX@     �       �  )p   �h  �  �  @     3       �I  �  �  �h X  �  5@     r       ��  �  �  �Xres �  �h    �@     *       ��     �  �h 7  %�@     �      �k    %�  �H@  %;   �D�  %;   �@�  &^   ��f  &^   ���  4;   �\�@     \       J  |  *p   �` t@     �         6  �h  l  Gr@     9       ��    G�  �h�  G;   �d�  G�  �Pf  G�  �@ �  K�@     9       �    K�  �h�  K;   �d�  K�  �Pf  K�  �@   O�@     6       �^    O�  �h�  O;   �dpc O�  �P   �m   L   �  �   F  �   @     I      �  H   f  int 2  F   m   Y   %   7i   T   �   val ^     val p   �   hi �    lo �    5  �   N  �   @     �       �?  	f  �   ��
hi ^   �H
lo ^   �P
val ^   �X   �   �`>@     M       
i ;   �D  zip �   �@     �       ��  	�  �   �@
hi ^   �X
lo ^   �`   ^   �h�@     T       
i ;   �T  T   �   H@     U       �,  lsb  ;   �\sz  ;   �X	f   �   �P
v !^   �`q@            D  $^   �h  }  -j  �@     %       �j  	�  -;   �l	f  -�   �` �  j  1�   �@            ��  v 1^   �X
val 2�   �` �  7�   �@     I       �  	s  7;   �\	f  7�   �P
val 8^   �`
res 9^   �h ^  B�   !@     B       �	�  B;   �\	f  B�   �P  C^   �h  %  $ >  $ >   :;I  :;   :;I8   :;I   :;I8  	.?:;'I@�B  
 :;I  .?:;'I@�B  4 :;I  4 :;I  .?:;'I@�B   :;I   %  $ >  $ >   :;I  :;   :;I8   :;I   :;I8  	.?:;'I@�B  
 :;I   :;I  4 :;I  4 :;I  .?:;'I@�B    .?:;'I@�B  .?:;'I@�B  .?:;'I@�B   %  $ >  $ >   I   :;I  :;   :;I8   :;I  	 <  
.?:;'I@�B   :;I  4 :;I  4 :;I     %   :;I  $ >  $ >      I  :;   :;I8  	 :;I8  
 :;  I  ! I/  & I  :;   :;I8   :;I  .?:;'I@�B   :;I  4 :;I  .?:;'@�B   :;I  4 :;I    .?:;'@�B   %  $ >  $ >   :;I      I  :;   :;I8  	 :;I8  
 :;  I  ! I/  & I  :;   :;I8   :;I  I:;  (   .:;'I@�B   :;I    4 :;I  .?:;'@�B  .?:;'I@�B  4 :;I     :;I  4 :;I?<   %  $ >  $ >   :;I  :;   :;I8   :;I  .?:;'I@�B  	 :;I  
4 :;I  4 :;I    .?:;'I@�B   :;I  .?:;'I@�B  .?:;'I@�B   (   Y   �      /usr/include  alu.c    wires.h    alu.h    stdint.h   arithmetic.h      	v@     ��>uuu=52",<L�Kv# t t t X u g R & u � � 9 , : \ = � f f f t X g � � �    W   �      /usr/include  arithmetic.c    wires.h    stdint.h   arithmetic.h      	�@     7t�=>g)�0�=>�)/v"�������%������27׼�$K0 fg�0�/0�/0�/0�guL�0��0��0�g �    e   �      /usr/include  main.c    stdint.h   support.h    memories.h    wires.h    alu.h      	�@     ������yu��w�Zw-����W��u��������ˆ� � # d tN����u���B#�  �
 =   �   �      /usr/lib/gcc/x86_64-linux-gnu/5/include /usr/include/x86_64-linux-gnu/bits /usr/include  memories.c    stddef.h   types.h   stdio.h   libio.h   stdint.h   memories.h    wires.h      	�@     ן�K0��@�Ku�ɮ�?ug؟1/ f#K2��=���� � Y � Jj�4�g!� y   �   �      /usr/include /usr/lib/gcc/x86_64-linux-gnu/5/include /usr/include/x86_64-linux-gnu/bits  trace_read.h    support.c    stdint.h   stddef.h   types.h   stdio.h   libio.h   support.h    wires.h      	D@     '=�MLK* wf X�KQ.�����������K0��?�1s�@u0x��uY� X!  /MH0tJ�0g�>g�>/� �    B   �      /usr/include  wires.c    wires.h    stdint.h     	@     ��� � � K � F Jm��0���� � � K � F Jl�0!��g��Y��1�u0��K0�/Y�Uͻ0��Y alu_execute_result alu.c alu_execute uint64_t res_ge adder_res res_eq unsigned char long unsigned int res_gt short unsigned int res_lt /home/bjorn/Documents/school/compsys/assignments/assignment3/src bool_xor val_a eval_condition GNU C11 5.4.0 20160609 -mtune=generic -march=x86-64 -ggdb -O0 -std=c11 -fstack-protector-strong is_add adder_of op_a op_b res_a short int generic_adder_result res_le is_sub conditions res_ne _Bool bool_not generic_adder c_32 g_prev arithmetic.c c_out reduce_or pg_16 val_b pg_1 pg_2 pg_4 pg_8 gen_c same_sign control carry_in num_bits hilo cnst c_in mask pg_32 gen_pg reduce_and c_16 value c_64 use_if p_prev tracer instruction_number next_inst_pc inst_word size stop has_regs next_pc target_reg imm_bytes reg_wr_enable main minor_op is_move reg_a reg_b major_op argc sizetype mem_p long long int main.c datapath_result sign_extended_imm argv trace_reader_p long double memory __off_t _IO_read_ptr _chain size_t _shortbuf memory_write _IO_buf_base memory_read_from_file pos_in_word num_elements wr_port _fileno _IO_read_end _flags _IO_buf_end _cur_column address _old_offset elements_left memory_create _IO_marker _IO_write_ptr pick_byte _sbuf data _IO_save_base _lock _flags2 _mode rd_port_a rd_port_b memory_read_unaligned rd_ports filename _IO_write_end _IO_lock_t _IO_FILE memories.c _pos _markers word_a word_b _vtable_offset rd_port wr_ports _next __off64_t _IO_read_base _IO_save_end frst_addr __pad1 __pad2 __pad3 __pad4 __pad5 _unused2 memory_read _IO_backup_base next_addr memory_destroy _IO_write_base count entry delete_trace_reader entries_valid Trace_read destination Trace_Entry create_trace_reader validate_reg_wr num_read entries match validate_mem_wr message Trace_Type error counter current_insn_number support.c input_failed Trace_Type_pc index Trace_Type_mem stderr validate_pc_wr Trace_Type_reg validate_magic wires.c unzip pick_bits sign_extend from_int num_bytes pick_one reverse_bytes values sign_position                               8@                   T@                   t@                   �@                   �@                   @@                   �@                   �@                  	 (@                  
 X@                   x@                   �@                   p@                   �@                   �@                   �@                   � @                   "@                   .`                   .`                    .`                   (.`                   �/`                    0`                   x0`                   �0`                                                                                                                                                      ��                      .`                  �@                  �@             .     0@             D     �0`            S     .`             z     P@             �     .`             �    ��                �    ��                �    ��                �    ��                �    ��                �     D@     �       �    ��                    ��                �     (@             �      .`                  ��                     .`                 (.`                  .`             .     � @             A     0`             W    �@            �    �@     �       g                     y                     �    !@     B       �    C@     S       �                      �    �@     J            x0`             �    q@     :       �                     �    �@     *       �    �@     )                                                 '    �@     9       7    �@     �      F    �0`             M                     a    �@            j    @     3       p    �@            a    �@             z    5@     r       �    @     j       <                     W    �@     %       �    �@     6       �    @     �       �    @     �       �    �@     *       �    �@     %       �                         �@     �          x0`             "    �@     I       .    H@     U       8    �@            ;                     P    �@     (       [                      j   �0`             w     @     Q      �    �@            �    �@     �       �    p@     e       �                     �    �@     I       �    @     �           �0`                 �@     *       �    v@            �    @     %       �    �@            �    �0`             �    �@     �      w    �@     %       �    r@     9           �@     M                            &                      :    �@     S       D                     V   �0`             b                      |    �@     O       �    U
@     ]      �    x@             �    �0`             crtstuff.c __JCR_LIST__ deregister_tm_clones __do_global_dtors_aux completed.7585 __do_global_dtors_aux_fini_array_entry frame_dummy __frame_dummy_init_array_entry alu.c arithmetic.c main.c memories.c support.c Trace_read wires.c __FRAME_END__ __JCR_END__ __init_array_end _DYNAMIC __init_array_start __GNU_EH_FRAME_HDR _GLOBAL_OFFSET_TABLE_ __libc_csu_fini free@@GLIBC_2.2.5 putchar@@GLIBC_2.2.5 sign_extend neg _ITM_deregisterTMCloneTable memory_create add __isoc99_fscanf@@GLIBC_2.7 delete_trace_reader use_if puts@@GLIBC_2.2.5 fread@@GLIBC_2.2.5 validate_mem_wr eval_condition _edata fclose@@GLIBC_2.2.5 from_int error reduce_or create_trace_reader memory_read_from_file validate_pc_wr memory_read_unaligned unzip memory_destroy pick_one __libc_start_main@@GLIBC_2.2.5 validate_magic __data_start memory_read pick_bits is fprintf@@GLIBC_2.2.5 reduce_and __gmon_start__ __dso_handle generic_adder _IO_stdin_used gen_c __libc_csu_init malloc@@GLIBC_2.2.5 reverse_bytes gen_pg bool_xor bool_not __bss_start main validate_reg_wr memory_write fopen@@GLIBC_2.2.5 _Jv_RegisterClasses pick_byte exit@@GLIBC_2.2.5 __TMC_END__ _ITM_registerTMCloneTable same_sign alu_execute stderr@@GLIBC_2.2.5  .symtab .strtab .shstrtab .interp .note.ABI-tag .note.gnu.build-id .gnu.hash .dynsym .dynstr .gnu.version .gnu.version_r .rela.dyn .rela.plt .init .plt.got .text .fini .rodata .eh_frame_hdr .eh_frame .init_array .fini_array .jcr .dynamic .got.plt .data .bss .comment .debug_aranges .debug_info .debug_abbrev .debug_line .debug_str                                                                                    8@     8                                    #             T@     T                                     1             t@     t      $                              D   ���o       �@     �      (                             N             �@     �      �                          V             @@     @      �                              ^   ���o       �@     �                                   k   ���o       �@     �      0                            z             (@     (      0                            �      B       X@     X                                 �             x@     x                                    �             �@     �      �                             �             p@     p                                    �             �@     �      b                             �             �@     �      	                              �             �@     �      �                              �             � @     �       l                             �             "@     "                                   �             .`     .                                    �             .`     .                                    �              .`      .                                    �             (.`     (.      �                           �             �/`     �/                                   �              0`      0      x                             �             x0`     x0                                    �             �0`     �0                                          0               �0      4                                                  �0                                                         �1      K                             '                     'M      �                             5                     T      �                             A     0               �Z      �                                                  �r      L                                                   Xb      �      #   :                 	                      @n      �                             #include <inttypes.h>
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
//
// trace_read.h
//

#ifndef __TRACE_READ_H__
#define __TRACE_READ_H__

//
// Trace entry type.
//
typedef enum Trace_Type
{
    Trace_Type_reg = 0,                     // A register-type entry.
    Trace_Type_mem = 1,                     // A memory-type entry.
    Trace_Type_pc  = 2                      // An instruction-pointer-type entry.
}
Trace_Type;

// ---------------------------------------------------------------------------------------------- //

//
// An entry from a trace file.
//
typedef struct Trace_Entry
{
    uint64_t counter;                       // Instruction #.
    uint64_t type;                          // Type of the entry.
    uint64_t destination;                   // Register or address in memory.
    uint64_t value;                         // Value written into register or memory.
}
Trace_Entry;

// ---------------------------------------------------------------------------------------------- //

//
// Reads the next 'count' entries from a trace-file. Returns the number of entries that where
// successfully read.
//
static inline size_t Trace_read(FILE * file, Trace_Entry entries [], size_t count)
{
    for (size_t index = 0; index < count; index++)
    {
        if (fscanf(file, "%" PRIX64 " %" PRIX64 " %" PRIX64 " %" PRIX64 "\n",
               &(entries[index].counter),
               &(entries[index].type),
               &(entries[index].destination),
               &(entries[index].value)
        ) != 4)
        {
            return index;
        }
    }
    
    return count;
}

#endif // __TRACE_READ_H__
#include "wires.h"

hilo unzip(val value) {
    uint64_t hi, lo, val;
    val = value.val;
    hi = lo = 0;
    for (int i=0; i<32; i++) {
	lo |= (val & 1) << i;
	val >>= 1;
	hi |= (val & 1) << i;
	val >>= 1;
    }
    hilo result;
    result.hi = from_int(hi);
    result.lo = from_int(lo);
    return result;
}

val zip(hilo values) {
    uint64_t hi = values.hi.val;
    uint64_t lo = values.lo.val;
    uint64_t result = 0;
    for (int i=0; i<32; i++) {
	result |= (hi & 1) << (2*i+1);
	hi >>= 1;
	result |= (lo & 1) << (2*i);
	lo >>= 1;
    }
    return from_int(result);
}

val pick_bits(int lsb, int sz, val value) {
    uint64_t v = value.val;
    v >>= lsb;
    if (sz < 64) {
        uint64_t mask = 1;
        mask <<= sz;
        mask -= 1;
        v &= mask;
    }
    return from_int(v);
}


bool pick_one(int position, val value) {
  return ((value.val >> position) & 1) == 1;
}

val from_int(uint64_t v) {
    val val;
    val.val = v;
    return val;
}

val reverse_bytes(int num_bytes, val value) {
    uint64_t val = value.val;
    uint64_t res;
    while (num_bytes--) {
        res <<= 8;
        res |= val & 0xFF;
        val >>= 8;
    }
    return from_int(res);
}

val sign_extend(int sign_position, val value) {
    uint64_t sign = value.val & (((uint64_t)1) << sign_position);
    return from_int(value.val - (sign << 1));
}

/*
  Elementary functions for digital logic simulation

  All functions declared here corresponds to wire connections.
  No "real" computation is being carried out by them.

  A single wire, when used as a control signal, is represented by the type 'bool'
  Multiple wires, or a single wire not used as a control signal, is represented
  by the type 'val'
*/

#ifndef WIRES_H
#define WIRES_H

#include <inttypes.h>
#include <stdbool.h>

// A generic bitvector - max 64 bits, though. By accident sufficient for our needs :-)
typedef struct { uint64_t val; } val;

// A pair of bitvectors - used by zip() and unzip()
typedef struct {
    val hi;
    val lo;
} hilo;

// simple conversion
val from_int(uint64_t);

// unzip pairwise into two bitvectors holding even/odd bits
hilo unzip(val);

// zip a pair of bitvectors into one.
val zip(hilo);

// pick a set of bits from a value
val pick_bits(int lsb, int sz, val);

// pick a single bit from a value
bool pick_one(int position, val);

// reverse the order of bytes within value
val reverse_bytes(int num_bytes, val value);

// sign extend by copying a sign bit to all higher positions
val sign_extend(int sign_position, val value);

#endif
