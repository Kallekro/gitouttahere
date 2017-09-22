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
ELF          >    €@     @       8t          @ 8 	 @ $ !       @       @ @     @ @     ø      ø                   8      8@     8@                                          @       @     (      (                    .      .`     .`     x                           (.      (.`     (.`     Ğ      Ğ                   T      T@     T@     D       D              Påtd   œ       œ @     œ @     l      l             Qåtd                                                  Råtd   .      .`     .`     ğ      ğ             /lib64/ld-linux-x86-64.so.2          GNU                        GNU FÃZÛ¤ö\û¸FK°)(*Y            €   €        î!c9ò‹                        n                      !                      )                                            N                      9                      U                      \                      T                      s                       @                                                                       @     3       G      0`             libc.so.6 exit fopen error puts putchar __isoc99_fscanf fclose malloc stderr fread fprintf __libc_start_main free __gmon_start__ GLIBC_2.7 GLIBC_2.2.5                               ii   ‚      ui	   Œ       ø/`        
            0`                   0`                    0`                   (0`                   00`                   80`                   @0`                   H0`                   P0`                   X0`        	           `0`                   h0`                   p0`                   HƒìH‹u)  H…Àtèã   HƒÄÃ              ÿ5b)  ÿ%d)  @ ÿ%b)  h    éàÿÿÿÿ%Z)  h   éĞÿÿÿÿ%R)  h   éÀÿÿÿÿ%J)  h   é°ÿÿÿÿ%B)  h   é ÿÿÿÿ%:)  h   éÿÿÿÿ%2)  h   é€ÿÿÿÿ%*)  h   épÿÿÿÿ%")  h   é`ÿÿÿÿ%)  h	   éPÿÿÿÿ%)  h
   é@ÿÿÿÿ%
)  h   é0ÿÿÿÿ%‚(  f        1íI‰Ñ^H‰âHƒäğPTIÇÀà@ HÇÁp@ HÇÇë@ èwÿÿÿôfD  ¸0` UH-ˆ0` HƒøH‰åv¸    H…Àt]¿ˆ0` ÿàf„     ]Ã@ f.„     ¾ˆ0` UHîˆ0` HÁşH‰åH‰ğHÁè?HÆHÑşt¸    H…Àt]¿ˆ0` ÿà ]ÃfD  €=q(   uUH‰åènÿÿÿ]Æ^(  óÃ@ ¿ .` Hƒ? uë“ ¸    H…ÀtñUH‰åÿĞ]ézÿÿÿUH‰å‰ú‰ğˆUüˆEø¶Eü2Eø¶À…À•À]ÃUH‰å‰øˆEü¶Eü…À•Àƒğ¶Àƒà]ÃUH‰åSHƒì(H‰}ØH‰uĞ¶EÙˆEå¶EÚˆEæ¶EØˆEçH‹EĞH‰Æ¿    è­  ˆEèH‹EĞH‰Æ¿   è™  ¶Ø¶Uç¶Eæ‰Ö‰Çèeÿÿÿ
Eå¶À!Ø…À•ÀˆEéH‹EĞH‰Æ¿   èd  ¶Ø¶Uç¶Eæ‰Ö‰Çè0ÿÿÿ¶À!Ø…À•ÀˆEêH‹EĞH‰Æ¿   è2  ¶Ğ¶Eå!Ğ…À•ÀˆEëH‹EĞH‰Æ¿   è  ¶Ø¶Eå‰Çèÿÿÿ¶À!Ø…À•ÀˆEìH‹EĞH‰Æ¿   èä  ¶Ø¶Uç¶Eæ‰Ö‰Çè°şÿÿ¶À‰ÇèÅşÿÿ¶À!Ø…À•ÀˆEíH‹EĞH‰Æ¿   è¨  ¶Ø¶Uç¶Eæ‰Ö‰Çètşÿÿ¶À‰Çè‰şÿÿ¶À!Ã¶Eå‰Çèyşÿÿ¶À!Ø…À•ÀˆEî¶Eè
Eé
Eê
Eë
Eì
Eí
Eî¶À…À•ÀˆEï¶EïHƒÄ([]ÃUH‰åATSHì°   H‰½pÿÿÿH‰µ`ÿÿÿH‰•PÿÿÿH‹…pÿÿÿH‰Æ¿   è
  „ÀuH‹…pÿÿÿH‰Æ¿   èò  „Àt¸   ë¸    ˆE€eH‹…pÿÿÿH‰Æ¿    èÇ  ˆEH‹…`ÿÿÿH‰Æ¿@   è]  H‰Â¶EH‰Ö‰Çè´  H‰Ã¶Eƒğ¶ÀH‹•`ÿÿÿH‰Ö‰Çè–  H‰ŞH‰ÇèÙ  H‰E¶UH‹MH‹…PÿÿÿH‰ÎH‰Çèâ  ‰ÁH‰Ğ‰MĞH‰EØ¶EĞˆEH‹EØH‰E H‹•PÿÿÿH‹…`ÿÿÿH‰ÖH‰Çè¬  H‰ÃH‹…pÿÿÿH‰Æ¿   è  ¶ÀH‰Ş‰Çè  H‰ÃH‹•PÿÿÿH‹…`ÿÿÿH‰ÖH‰Çè"  I‰ÄH‹…pÿÿÿH‰Æ¿   èÍ  ¶ÀL‰æ‰ÇèÕ  H‰ŞH‰Çè  H‰Ã¶EH‹U H‰Ö‰Çèµ  H‰ŞH‰Çèø  H‰Ã¶EH‹U H‰Ö‰Çè•  H‰ŞH‰ÇèØ  H‰E°H‹E°H‰Æ¿?   èg  ˆEÊH‹E°H‰Çèj  ¶À…À•Àƒğ¶ÀƒàˆEÉ€} u€} t€} t¸   ë¸    ƒàˆEÈH‹E°H‰EÀH‹EÀH‰EàH‹EÈH‰EèH‹UàH‹EèH‰ÑH‰ÂH‰ÈHÄ°   [A\]ÃUH‰åSHƒì(H‰}àH‰uĞH‹EàH‰Æ¿?   èÉ  ‰ÃH‹EĞH‰Æ¿?   è¶  1Ø¶À…À•Àƒğ¶ÀƒàHƒÄ([]ÃUH‰åHƒì@H‰øH‰ñH‰ÊH‰EÀH‰UÈH‹EÀH‰Çèô  H‰EĞH‰UØH‹EÈH‰Çèà  H‰EàH‰UèH‹UĞH‹EØH‰ÖH‰Çè  H‰EğH‹UĞH‹EèH‰ÖH‰Çèh  H‰ÂH‹EàH‰ÖH‰Çè{  H‰EøH‹EğH‹UøÉÃUH‰åSHƒìXH‰øH‰ÁH‰ÓH‰óH‰M°H‰]¸H‰U H‹E°H‰Çèa  H‰EÀH‰UÈH‹E¸H‰ÇèM  H‰EĞH‰UØH‹E H‰EèH‹UÈH‹E H‰ÖH‰Çèä  H‰ÂH‹EØH‰ÆH‰×è÷  H‰EàH‹UàH‹EèH‰×H‰Æèœ  HƒÄX[]ÃUH‰åSHì  H‰½ ÿÿÿH‰µğşÿÿ‰Ğˆ…üşÿÿH‹•ğşÿÿH‹… ÿÿÿH‰ÖH‰Çè  H‰E€H‹•ğşÿÿH‹… ÿÿÿH‰ÖH‰Çè\  H‰EˆH‹U€H‹EˆH‰×H‰ÆèrşÿÿH‰EH‰U˜H‹UH‹E˜H‰×H‰ÆèWşÿÿH‰E H‰U¨H‹U H‹E¨H‰×H‰Æè<şÿÿH‰E°H‰U¸H‹U°H‹E¸H‰×H‰Æè!şÿÿH‰EÀH‰UÈH‹UÀH‹EÈH‰×H‰ÆèşÿÿH‰EĞH‰UØ¿   èµ  H‰Â¶…üşÿÿH‰Ö‰ÇèŠ  H‰…ÿÿÿH‹•ÿÿÿH‹MĞH‹EØH‰ÏH‰ÆèJşÿÿH‰… ÿÿÿH‹• ÿÿÿH‹MÀH‹EÈH‰ÏH‰Æè)şÿÿH‰…0ÿÿÿH‹•0ÿÿÿH‹M°H‹E¸H‰ÏH‰ÆèşÿÿH‰…@ÿÿÿH‹•@ÿÿÿH‹M H‹E¨H‰ÏH‰ÆèçıÿÿH‰…PÿÿÿH‹•PÿÿÿH‹MH‹E˜H‰ÏH‰ÆèÆıÿÿH‰…`ÿÿÿH‹•`ÿÿÿH‹M€H‹EˆH‰ÏH‰Æè¥ıÿÿH‰…pÿÿÿH‹•ğşÿÿH‹… ÿÿÿH‰ÖH‰Çè  H‰ÂH‹…pÿÿÿH‰ÆH‰×è  H‰EèH‹…pÿÿÿH‰Æ¿?   èi  ‰ÃH‹…pÿÿÿH‰Æ¿>   èS  1Ø¶À…À•ÀˆEàH‹EàH‹UèH‰ÁH‰Ó‰ÈHÄ  []ÃUH‰åHƒì0H‰}àH‰uĞH‹MĞH‹Eàº    H‰ÎH‰Çè‡ıÿÿ‰ÁH‰Ğ‰MğH‰EøH‹EøÉÃUH‰åHƒì‰øH‰uğˆEü€}ü tH‹Eğë
¿    èğ  ÉÃUH‰åHƒì H‰}ğH‰uàH‹UğH‹EàH!ĞH‰ÇèË  ÉÃUH‰åHƒì H‰}ğH‰uàH‹UğH‹EàH	ĞH‰Çè¦  ÉÃUH‰åHƒì H‰}ğH‰uàH‹UğH‹EàH1ĞH‰Çè  ÉÃUH‰åHƒì ‰}ìH‰uàƒ}ì?-‹Eìº   ‰ÁHÓâH‰ĞHƒèH‰EøH‹EàH÷ĞH#EøH‰Çè?  ëH‹EàH÷ĞH‰Çè.  ÉÃUH‰åH‰}øH‰uğH‹EğH;Eø”À]ÃUH‰åH‰}ğH‹EğH…À•À]ÃUH‰åHƒì‰}üH‰uğH‹Uğ‹EüH‰Ö‰Çè`ÿÿÿH…À”ÀÉÃUH‰åSHìH  ‰½¼şÿÿH‰µ°şÿÿƒ½¼şÿÿ
¿ø@ èë  HÇEØ    ƒ½¼şÿÿuH‹…°şÿÿHƒÀH‹ H‰Çè÷  H‰EØº   ¾   ¿   èN  H‰Eàº   ¾   ¿   è6  H‰EèH‹…°şÿÿHƒÀH‹H‹EèH‰ÖH‰Çè‰  ¿    è)  H‰…àşÿÿÆ…Ñşÿÿ ¶…Ñşÿÿˆ…Òşÿÿ¶…Òşÿÿˆ…ĞşÿÿÆ…Ìşÿÿ Ç…Ôşÿÿ    é¢  ƒ…ÔşÿÿH‹•àşÿÿH‹EèA¸   H‰Ñº   ¾    H‰Çè  H‰…ğşÿÿH‹…ğşÿÿH‰Â¾   ¿   è$  H‰… ÿÿÿH‹…ğşÿÿH‰Â¾   ¿    è  H‰…ÿÿÿH‹… ÿÿÿH‰Æ¿   è7şÿÿˆ…Íşÿÿ¶…Íşÿÿˆ…Îşÿÿ¿   èF  H‰Â¶…ÎşÿÿH‰Ö‰ÇèıÿÿH‰Ã¿   è%  H‰Â¶…Îşÿÿƒğ¶ÀH‰Ö‰ÇèôüÿÿH‰ŞH‰Çè7ıÿÿH‰… ÿÿÿH‹…ğşÿÿH‰Â¾   ¿   èf
  H‰…0ÿÿÿH‹…ğşÿÿH‰Â¾   ¿   èF
  H‰…@ÿÿÿH‹…ğşÿÿH‰Â¾    ¿   è&
  H‰Â¶…ÎşÿÿH‰Ö‰ÇèuüÿÿH‰ÃH‹…ğşÿÿH‰Â¾    ¿   èö	  H‰Â¶…Îşÿÿƒğ¶ÀH‰Ö‰Çè?üÿÿH‰ŞH‰Çè‚üÿÿH‰…PÿÿÿH‹…PÿÿÿH‰…`ÿÿÿH‹…`ÿÿÿH‰Æ¿   è
  H‰…pÿÿÿH‹• ÿÿÿH‹…àşÿÿH‰ÖH‰Çè±ûÿÿH‰E€H‹… ÿÿÿH‰Æ¿    è¾üÿÿˆ…ÌşÿÿH‹•0ÿÿÿH‹Eà¹   ¾    H‰Çè‡  H‰EH‹•@ÿÿÿH‹Eà¹   ¾   H‰Çèf  H‰E H‹EH‰E°H‹…@ÿÿÿH‰EÀ¶…Íşÿÿˆ…ÏşÿÿH‹E€H‰EĞH‹…àşÿÿH‰Æ¿  @ ¸    è¯ñÿÿÇ…Øşÿÿ    ëAH‹•ğşÿÿ‹…ØşÿÿÁà‰ÁHÓêH‰Ğ%ÿ   ‰…Üşÿÿ‹…Üşÿÿ‰Æ¿' @ ¸    èiñÿÿƒ…Øşÿÿ‹…ØşÿÿHcĞH‹… ÿÿÿH9Ârª€½Ïşÿÿ tH‹U°H‹EÀH‰Æ¿+ @ ¸    è*ñÿÿë
¿
   èÎğÿÿHƒ}Ø •À¶Ğ¶…Ìşÿÿƒğ¶À!Ğ…Àt;H‹UĞ‹ÔşÿÿH‹EØ‰ÎH‰Çè¼  €½Ïşÿÿ tH‹M°H‹UÀ‹µÔşÿÿH‹EØH‰Çè'  H‹EĞH‰…àşÿÿ¶µÏşÿÿH‹M°H‹UÀH‹EàA‰ğ¾   H‰Çè~  ¶…Ìşÿÿƒğ„À…Lüÿÿ¿9 @ èKğÿÿ¸    HÄH  []ÃUH‰åHƒì ‰}ì‰uè‰Uä¿   èğÿÿH‰EøH‹Eø‹Uì‰P‹EìH˜HÁàH‰ÇèbğÿÿH‰ÂH‹EøH‰H‹EøÉÃUH‰åHƒìH‰}øH‹EøH‹ H‰Çè§ïÿÿH‹EøH‰Çè›ïÿÿÉÃUH‰åHƒì0H‰}ØH‰uĞH‹EĞ¾> @ H‰ÇèğÿÿH‰EèHƒ}è u
¿@ @ è´  H‹EØ‹@H˜H‰EğH‹EØH‹ H‰EøH‹MèH‹UğH‹Eø¾   H‰ÇèqïÿÿÉÃUH‰åHƒì H‰}ø‰uôH‰Uà‰ÈˆEğ€}ğ tH‹EøH‹ H‹UàHÁâHĞH‹ H‰Çè  ë
¿    èù  ÉÃUH‰åHƒì ‰}üH‰uğH‰Uàƒ}ü~#¿    èÔ  H‰Â‹EüHøH‹EàH‰Æ‰ÏèÆÿÿÿëH‹Uğ‹EüÁà‰ÁHÓêH‰Ğ¶ÀÉÃUH‰åHƒìpH‰}¨‰u¤‰U H‰MD‰ÀˆEœH‹EH‰Â¾=   ¿   èø  H‰EÀH‹EÀHƒÀH‰Çè^  H‰EĞH‹Eƒà‰E¼¶MœH‹UÀ‹u¤H‹E¨H‰ÇèùşÿÿH‰Eà¶MœH‹UĞ‹u H‹E¨H‰ÇèŞşÿÿH‰EğHÇEø    ÇE¸   ë*HÁeø‹U¼‹E¸H‹UğH‹EàH‰Æ‰ÏèòşÿÿH˜H	Eøƒm¸ƒ}¸ yĞH‹EøH‰ÇèÍ  ÉÃUH‰åH‰}ø‰uôH‰UàH‰MĞD‰ÀˆEÜ€}Ü t+H‹UàH‹Eø‹@H˜H9ÂsH‹EøH‹ H‹UàHÁâHÂH‹EĞH‰]ÃUH‰åHƒì0H‰}èH‰uàH‰UØHÇEø    é‰   H‹EøHÁàH‰ÂH‹EàHĞHpH‹EøHÁàH‰ÂH‹EàHĞHHH‹EøHÁàH‰ÂH‹EàHĞHPH‹EøHÁàH‰ÇH‹EàHøH‰ÇH‹EèI‰ñI‰ÈH‰ÑH‰ú¾T @ H‰Ç¸    èòìÿÿƒøtH‹EøëHƒEøH‹EøH;EØ‚iÿÿÿH‹EØÉÃUH‰åHƒìH‰}øH‹‹  H‹Uø¾e @ H‰Ç¸    èíÿÿ¿ÿÿÿÿè+íÿÿUH‰åHƒì H‰}è¿˜   èõìÿÿH‰EøH‹Eè¾i @ H‰ÇèğìÿÿH‰ÂH‹EøH‰H‹EøH‹ H…Àu
¿k @ è‚ÿÿÿH‹EøÇ@    H‹EøÇ@    H‹EøÇ@    H‹EøÉÃUH‰åHƒìH‰}øH‹EøH‹ H‰Çè>ìÿÿH‹EøH‰ÇèâëÿÿÉÃUH‰åHƒì@H‰}Ø‰uÔ‰UĞH‰MÈL‰EÀë\H‹EØ‹@H˜HÁàHPH‹EØHĞHHH‹EØH‹ º   H‰ÎH‰Çè$şÿÿH‰EğHƒ}ğuH‹EØ‹@PH‹EØ‰PëH‹EØÇ@   H‹EØ‹@…À”ÂH‹EØ‹@ƒøÀ!Ğ„Àu…ÇEì    éâ   ‹EìH˜HÁàHPH‹EØHĞHƒÀH‰EøH‹EøH‹P‹EÔH˜H9Â…¬   H‹EøH‹@H;EÈ…š   H‹EøH‹@H;EÀ…ˆ   H‹EøH‹‹EĞH˜H9ÂuwëQ‹EìHH‹EØ‹UìHcÒHÁâHĞHƒÀH‹UØHcÉHÁáHÊHƒÂH‹JH‰HH‹JH‰HH‹JH‰HH‹R H‰P ƒEì‹EìPH‹EØ‹@9Â|H‹EØ‹@PÿH‹EØ‰PëƒEìH‹EØ‹@;Eìÿÿÿ¿… @ è’ıÿÿÉÃUH‰åHƒì0H‰}ø‰uôH‰UàH‰MĞH‹uĞH‹Mà‹UôH‹EøI‰ğ¾    H‰Çè)şÿÿÉÃUH‰åHƒì0H‰}ø‰uôH‰UàH‰MĞH‹uĞH‹Mà‹UôH‹EøI‰ğ¾   H‰ÇèğıÿÿÉÃUH‰åHƒì H‰}ø‰uôH‰UàH‹Mà‹UôH‹EøI‰È¹    ¾   H‰ÇèºıÿÿÉÃUH‰åHƒì@H‰}ÀH‹EÀH‰EèHÇEà    H‹EàH‰EØÇEÔ    ë>H‹EèƒàH‰Â‹EÔ‰ÁHÓâH‰ĞH	EàHÑmèH‹EèƒàH‰Â‹EÔ‰ÁHÓâH‰ĞH	EØHÑmèƒEÔƒ}Ô~¼H‹EØH‰Çè+  H‰EğH‹EàH‰Çè  H‰EøH‹EğH‹UøÉÃUH‰åHƒì0H‰øH‰ñH‰ÊH‰EĞH‰UØH‹EĞH‰EèH‹EØH‰EğHÇEø    ÇEä    ëEH‹EèƒàH‰Â‹EäÀƒÀ‰ÁHÓâH‰ĞH	EøHÑmèH‹EğƒàH‰Â‹EäÀ‰ÁHÓâH‰ĞH	EøHÑmğƒEäƒ}ä~µH‹EøH‰Çè|   ÉÃUH‰åHƒì ‰}ì‰uèH‰UàH‹EàH‰Eğ‹Eì‰ÁHÓmğƒ}è?HÇEø   ‹Eè‰ÁHÓeøHƒmøH‹EøH!EğH‹EğH‰Çè'   ÉÃUH‰å‰}üH‰uğH‹Uğ‹Eü‰ÁHÓêH‰ĞƒàH…À•À]ÃUH‰åH‰}èH‹EèH‰EğH‹Eğ]ÃUH‰åHƒì ‰}ìH‰uàH‹EàH‰EğëHÁeøH‹Eğ¶ÀH	EøHÁmğ‹EìPÿ‰Uì…ÀuŞH‹EøH‰Çè£ÿÿÿÉÃUH‰åHƒì ‰}ìH‰uàH‹Uà‹Eì¾   ‰ÁHÓæH‰ğH!ĞH‰EøH‹EàH‹UøHÒH)ĞH‰ÇèaÿÿÿÉÃf.„      AWAVA‰ÿAUATL%  UH-  SI‰öI‰ÕL)åHƒìHÁıè×æÿÿH…ít 1Û„     L‰êL‰öD‰ÿAÿÜHƒÃH9ëuêHƒÄ[]A\A]A^A_Ãf.„     óÃ  HƒìHƒÄÃ         missing name of programfile to simulate %lx :  %x  		r%ld = %lx
 Done r Failed to open file %lX %lX %lX %lX
 %s
 r Unable to open trace file Trace validation error ;h  ,   æÿÿ´  äæÿÿ„  ÚçÿÿÜ  ùçÿÿü  èÿÿ  ¹éÿÿD  ìÿÿl  eìÿÿ”  ğìÿÿ´  „íÿÿÜ  Õïÿÿ  ğÿÿ$  8ğÿÿD  ]ğÿÿd  ‚ğÿÿ„  §ğÿÿ¤  úğÿÿÄ  ñÿÿä  'ñÿÿ  Oñÿÿ$  öÿÿL  Röÿÿl  |öÿÿŒ  æöÿÿ¬  /÷ÿÿÌ  ‚÷ÿÿì  [øÿÿ  ¨øÿÿ,  fùÿÿL  ™ùÿÿl  úÿÿŒ  5úÿÿ¬  ÖûÿÿÌ  üÿÿì  Hüÿÿ  ~üÿÿ,  ıÿÿL  ¬ıÿÿl  şÿÿŒ  &şÿÿ¬  <şÿÿÌ  …şÿÿì  Ôşÿÿ  DÿÿÿT         zR x      Xåÿÿ*                  zR x  $      HäÿÿĞ    FJw€ ?;*3$"       D   öåÿÿ    A†CZ      d   õåÿÿ    A†CX   $   „   òåÿÿ£   A†CEƒ™      $   ¬   mçÿÿ]   A†CJŒƒN    $   Ô   ¢éÿÿO    A†CEƒE          ü   Ééÿÿ‹    A†C†  $     4êÿÿ”    A†CEƒŠ       $   D   êÿÿQ   A†CHƒD         l  Éìÿÿ:    A†Cu      Œ  ãìÿÿ)    A†Cd      ¬  ììÿÿ%    A†C`      Ì  ñìÿÿ%    A†C`      ì  öìÿÿ%    A†C`        ûìÿÿS    A†CN     ,  .íÿÿ    A†CT      L  'íÿÿ    A†CO      l  íÿÿ(    A†Cc   $   Œ  #íÿÿ¹   A†CHƒ¬         ´  ´ñÿÿJ    A†CE     Ô  Şñÿÿ*    A†Ce      ô  èñÿÿj    A†Ce       2òÿÿI    A†CD     4  [òÿÿS    A†CN     T  òÿÿÙ    A†CÔ     t  GóÿÿM    A†CH     ”  tóÿÿ¾    A†C¹     ´  ôÿÿ3    A†C          Ô  %ôÿÿr    A†Cm     ô  wôÿÿ*    A†Ce        ôÿÿ¡   A†Cœ    4  öÿÿ9    A†Ct      T  öÿÿ9    A†Ct      t  4öÿÿ6    A†Cq      ”  Jöÿÿ›    A†C–     ´  Åöÿÿ“    A†C     Ô  8÷ÿÿU    A†CP     ô  m÷ÿÿ%    A†C`        r÷ÿÿ    A†CQ      4  h÷ÿÿI    A†CD     T  ‘÷ÿÿB    A†C}   D   t  À÷ÿÿe    BBE B(ŒH0†H8ƒM@r8A0A(B BBB    ¼  è÷ÿÿ                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   P@     0@                                  x@            ä@            .`                          .`                   õşÿo    ˜@            @@            À@     
       ˜                                            0`                                        X@            (@            0       	              şÿÿo    ø@     ÿÿÿo           ğÿÿo    Ø@                                                                                                             (.`                     ¶@     Æ@     Ö@     æ@     ö@     @     @     &@     6@     F@     V@     f@                     GCC: (Ubuntu 5.4.0-6ubuntu1~16.04.4) 5.4.0 20160609 ,             v@     <                      ,           ²@     9                      ,           ë@     ¹                      ,    ¥       ¤@                            ,    ‘       D@     Ö                      ,    û       @     I                             æ      ‡   v@     <          H   f  int 2  F   m   Y   %   7i   T   …   val ^     val p   º   of º    zf º   sf º    ¥  “     ì      …    cc Á         Ì     of º       …    p  ÷   	È   º   v@            œ\  
a º   ‘l
b º   ‘h 	«  º   •@            œŠ  
a º   ‘l ×   º   ²@     £      œ\  
c Á   ‘H
op …   ‘@zf º   ‘Usf 	º   ‘Vof 
º   ‘W`  º   ‘X…  º   ‘Y€   º   ‘Z?   º   ‘[  º   ‘\.   º   ‘]f   º   ‘^res º   ‘_    ì   U
@     ]      œ
op …   ‘à~V  …   ‘Ğ~[  …   ‘À~   ì   ‘°Œ  º   ‘ı~F  º   ‘ş~Ñ   …   ‘€x    ‘@M  º   ‘ÿ~5   …   ‘res …   ‘   _   ı   æ   Î  ‡   ²@     9      ,  H   f  int 2  F   m   Y   %   7i   T   …   val ^     val p   ¯   hi …    lo …    5     Ú   of Ú       …    ¥  p  º   	  p …    g …    pg ì   	  Ú   ²@     O       œM  
a …   ‘P
b …   ‘@ 	O  	  @     ‹       œ¨  É  	  ‘°x  ¯   ‘@Ç  ¯   ‘Pc  	  ‘` 	  …   Œ@     ”       œ  É  	  ‘ ?  …   ‘x  ¯   ‘°Ç  ¯   ‘@Û  ¯   ‘P 	´  $á    @     Q      œ2  Ñ   $…   ‘ğ}ñ  $…   ‘à}#  $Ú   ‘ì}÷  &	  ‘ğ~ü  *	  ‘€  +	  ‘  ,	  ‘ ë  -	  ‘°I  .	  ‘@l  0…   ‘€~Â  1…   ‘~a  2…   ‘ ~c_8 3…   ‘°~c_4 4…   ‘À~c_2 5…   ‘Ğ~c_1 6…   ‘à~   8á   ‘P add @…   q@     :       œz  
a @…   ‘P
b @…   ‘@tmp Aá   ‘` 	q  F…   «@     )       œ¸    FÚ   ‘lf  F…   ‘` and K…   Ô@     %       œò  
a K…   ‘`
b K…   ‘P or O…   ù@     %       œ+  
a O…   ‘`
b O…   ‘P xor S…   @     %       œe  
a S…   ‘`
b S…   ‘P neg W…   C@     S       œÁ  ,  W;   ‘\
a W…   ‘PX@     -       D  Y^   ‘h  is `Ú   –@            œü  :  `^   ‘h
a `…   ‘` á  dÚ   ¯@            œ*  
a d…   ‘` V  hÚ   Ã@     (       œ,  h;   ‘l
a h…   ‘`      ;  æ   ;  ‡   ë@     ¹        2  T   int -  x  F   m   Y   H   f    €   O   %   74   §   val ‡     val ’   i  ½   Ã   	_  ¥  '  Ú   à   	„    of È    zf È   sf È    “  å   
î  ;   ë@     ¹      œ    ;   ‘¬}d    ‘ }  ²   ‘H¾  (Ï   ‘Pmem -Ï   ‘Xpc 0§   ‘Ğ}cc 1  ‘À}µ  4È   ‘¼}†  7;   ‘Ä}×@     ¢      ¦  >§   ‘à}  A§   ‘ğ}ó  B§   ‘€~ü  CÈ   ‘½}º  DÈ   ‘¾}°  E§   ‘~  G§   ‘ ~
  H§   ‘°~Ö  I§   ‘À~imm K§   ‘Ğ~R  L§   ‘à~™  M§   ‘ğ~V  Q§   ‘€[  R§   ‘B  U§   ‘ Ë  X§   ‘°à  YÈ   ‘¿}Ã  \§   ‘@a@     b       j a;   ‘È}m@     :       ‹  b^   ‘Ì}    z    è   ÿ  æ     ‡   ¤@              §  Ø8   T   F   m   Y   H   f  int 2  ‹  ƒi   h  „i     •   O     0§     Øñ$    òb    “  ÷     ø   r  ù   ø  ú    x  û   (ù  ü   0Å  ı   8%  ş   @	›      H	Ï     P	€     X	+  \  `	   b  h		  b   p	¯  b   t	E  p   x	1  F   €	B  T   ‚	®  h  ƒ	©  x  ˆ	I  !{   	—  )   ˜	  *    	¥  +   ¨	¬  ,   °	³  .-   ¸	·  /b   À	º  1~  Ä 
  –m  œ\  b  \     b  &  ¢b    +  §   •   x  †     $  •     †    ”  •   -  %   78   À  val      val «  '  Ö  Ü  „    –     ô  	b    ï       _  Ë  ¤@     J       œl  ô  b   ‘\ç  b   ‘XY  b   ‘Tres Ë  ‘h é  î@     *       œ˜  mem Ë  ‘h Ò  @     j       œú  mem Ë  ‘Hğ    ‘@f ú  ‘XQ  -   ‘`ptr    ‘h œ   Ã  &À  ‚@     I       œZ  mem &Ë  ‘hQ  &b   ‘d=  &À  ‘Pç  &Z  ‘` ¥  †  .b   Ë@     S       œ©  pos .b   ‘la .À  ‘`b .À  ‘P Ñ  6À  @     Ù       œ‹  mem 6Ë  ‘˜½  6b   ‘”Ç  6b   ‘=  6À  ‘€ç  6Z  ‘Œ  7À  ‘°ß  8À  ‘@è  9b   ‘¬4  :À  ‘P;  ;À  ‘`   <   ‘h°@     9       k >b   ‘¨  ¸  I÷@     M       œmem IË  ‘h  Ib   ‘d=  IÀ  ‘Pf  IÀ  ‘@ä  IZ  ‘L  f   S  æ   Ø  ‡   D@     Ö      G  H   f  int 2  F   m   Y   %   7i   T   §  Øi   ‹  ƒB   h  „B         O     0²     Øñ/    ò;    “  ÷š     øš   r  ùš   ø  úš    x  ûš   (ù  üš   0Å  ıš   8%  şš   @	›   š   H	Ï  š   P	€  š   X	+  g  `	   m  h		  ;   p	¯  ;   t	E  {   x	1  P   €	B  -   ‚	®  s  ƒ	©  ƒ  ˆ	I  !†   	—  )˜   ˜	  *˜    	¥  +˜   ¨	¬  ,˜   °	³  .p   ¸	·  /;   À	º  1‰  Ä 
  –m  œg  b  g     m  &  ¢;    6  ²       ƒ  ‘     /      ™  ‘    Ÿ      -  x  	Ç  val 	^     val 	²  i  İ  ã  _  ˜*  f •   Ä  ;   â  ;   '  ;   …  ›   «  W   M  (     ï   L   Š  ¼  ^    "  ^   @  ^   f  ^    L  M  §   Š  «  ‘    5  'p   D@     ¾       œ  ã  '•  ‘X…  '  ‘P  'p   ‘HX@     ¤       ı  )p   ‘h  Š  ¶  @     3       œI  £  ™  ‘h X  Ò  5@     r       œ‡  ğ  ™  ‘Xres Ò  ‘h    §@     *       œ³     Ò  ‘h 7  %Ñ@     ¡      œk    %Ò  ‘H@  %;   ‘D†  %;   ‘@’  &^   ‘¸f  &^   ‘°  4;   ‘\í@     \       J  |  *p   ‘` t@     â         6  ‘h  l  Gr@     9       œÁ    GÒ  ‘h†  G;   ‘d’  GÇ  ‘Pf  GÇ  ‘@ “  K«@     9       œ    KÒ  ‘h†  K;   ‘d’  KÇ  ‘Pf  KÇ  ‘@   Oä@     6       œ^    OÒ  ‘h†  O;   ‘dpc OÇ  ‘P   ªm   L   ß  æ   F  ‡   @     I      Ä  H   f  int 2  F   m   Y   %   7i   T   …   val ^     val p   ¯   hi …    lo …    5     N  ¯   @     ›       œ?  	f  …   ‘°
hi ^   ‘H
lo ^   ‘P
val ^   ‘X   ¯   ‘`>@     M       
i ;   ‘D  zip …   µ@     “       œµ  	”  ¯   ‘@
hi ^   ‘X
lo ^   ‘`   ^   ‘hæ@     T       
i ;   ‘T  T   …   H@     U       œ,  lsb  ;   ‘\sz  ;   ‘X	f   …   ‘P
v !^   ‘`q@            D  $^   ‘h  }  -j  @     %       œj  	   -;   ‘l	f  -…   ‘` ¥  j  1…   Â@            œ­  v 1^   ‘X
val 2…   ‘` †  7…   Ø@     I       œ  	s  7;   ‘\	f  7…   ‘P
val 8^   ‘`
res 9^   ‘h ^  B…   !@     B       œ	›  B;   ‘\	f  B…   ‘P  C^   ‘h  %  $ >  $ >   :;I  :;   :;I8   :;I   :;I8  	.?:;'I@—B  
 :;I  .?:;'I@–B  4 :;I  4 :;I  .?:;'I@–B   :;I   %  $ >  $ >   :;I  :;   :;I8   :;I   :;I8  	.?:;'I@–B  
 :;I   :;I  4 :;I  4 :;I  .?:;'I@–B    .?:;'I@—B  .?:;'I@—B  .?:;'I@–B   %  $ >  $ >   I   :;I  :;   :;I8   :;I  	 <  
.?:;'I@–B   :;I  4 :;I  4 :;I     %   :;I  $ >  $ >      I  :;   :;I8  	 :;I8  
 :;  I  ! I/  & I  :;   :;I8   :;I  .?:;'I@–B   :;I  4 :;I  .?:;'@–B   :;I  4 :;I    .?:;'@—B   %  $ >  $ >   :;I      I  :;   :;I8  	 :;I8  
 :;  I  ! I/  & I  :;   :;I8   :;I  I:;  (   .:;'I@–B   :;I    4 :;I  .?:;'@–B  .?:;'I@–B  4 :;I     :;I  4 :;I?<   %  $ >  $ >   :;I  :;   :;I8   :;I  .?:;'I@–B  	 :;I  
4 :;I  4 :;I    .?:;'I@–B   :;I  .?:;'I@—B  .?:;'I@–B   (   Y   û      /usr/include  alu.c    wires.h    alu.h    stdint.h   arithmetic.h      	v@     Ö>uuu=52",<LóKv# t t t X u g R & u ƒ ¯ 9 , : \ = å f f f t X g ƒ É ç    W   û      /usr/include  arithmetic.c    wires.h    stdint.h   arithmetic.h      	²@     7tƒ=>g)ƒ0ó=>ƒ)/v"»¼ŸŸŸŸ %óóóóóõ27×¼ô$K0 fgŸ0ó/0ó/0ó/0åguLæ0»­0ƒŸ0åg ë    e   û      /usr/include  main.c    stdint.h   support.h    memories.h    wires.h    alu.h      	ë@     „‘ ƒ‘—yu¼õw Zw-åå‘ÉWååu×Ÿ»“óõ…­Ë†ƒ » # d tN‘® Îu‘”­B#À  å
 =   Ë   û      /usr/lib/gcc/x86_64-linux-gnu/5/include /usr/include/x86_64-linux-gnu/bits /usr/include  memories.c    stddef.h   types.h   stdio.h   libio.h   stdint.h   memories.h    wires.h      	¤@     ×ŸŸK0»å»@óKu É®ƒ?ugØŸ1/ f#K2­‘=ŸŸŸ„ ‘ Y ğ Jj»4ƒg!ƒ y   Ù   û      /usr/include /usr/lib/gcc/x86_64-linux-gnu/5/include /usr/include/x86_64-linux-gnu/bits  trace_read.h    support.c    stdint.h   stddef.h   types.h   stdio.h   libio.h   support.h    wires.h      	D@     '=ÎMLK* wf XÖKQ.»»»×Ÿ»Ÿ­­­K0»å»?“1s«@u0x¬ÖuY­ X!  /MH0tJòŸ0g×>g×>/å Î    B   û      /usr/include  wires.c    wires.h    stdint.h     	@     ¼ƒó ‘ ƒ K ƒ F Jmóóƒ0ƒƒƒƒ ‘ É K Ÿ F Jl»0!ƒ‘gƒ‘Y„»1­u0„ƒK0å„/Y­UÍ»0åŸY alu_execute_result alu.c alu_execute uint64_t res_ge adder_res res_eq unsigned char long unsigned int res_gt short unsigned int res_lt /home/bjorn/Documents/school/compsys/assignments/assignment3/src bool_xor val_a eval_condition GNU C11 5.4.0 20160609 -mtune=generic -march=x86-64 -ggdb -O0 -std=c11 -fstack-protector-strong is_add adder_of op_a op_b res_a short int generic_adder_result res_le is_sub conditions res_ne _Bool bool_not generic_adder c_32 g_prev arithmetic.c c_out reduce_or pg_16 val_b pg_1 pg_2 pg_4 pg_8 gen_c same_sign control carry_in num_bits hilo cnst c_in mask pg_32 gen_pg reduce_and c_16 value c_64 use_if p_prev tracer instruction_number next_inst_pc inst_word size stop has_regs next_pc target_reg imm_bytes reg_wr_enable main minor_op is_move reg_a reg_b major_op argc sizetype mem_p long long int main.c datapath_result sign_extended_imm argv trace_reader_p long double memory __off_t _IO_read_ptr _chain size_t _shortbuf memory_write _IO_buf_base memory_read_from_file pos_in_word num_elements wr_port _fileno _IO_read_end _flags _IO_buf_end _cur_column address _old_offset elements_left memory_create _IO_marker _IO_write_ptr pick_byte _sbuf data _IO_save_base _lock _flags2 _mode rd_port_a rd_port_b memory_read_unaligned rd_ports filename _IO_write_end _IO_lock_t _IO_FILE memories.c _pos _markers word_a word_b _vtable_offset rd_port wr_ports _next __off64_t _IO_read_base _IO_save_end frst_addr __pad1 __pad2 __pad3 __pad4 __pad5 _unused2 memory_read _IO_backup_base next_addr memory_destroy _IO_write_base count entry delete_trace_reader entries_valid Trace_read destination Trace_Entry create_trace_reader validate_reg_wr num_read entries match validate_mem_wr message Trace_Type error counter current_insn_number support.c input_failed Trace_Type_pc index Trace_Type_mem stderr validate_pc_wr Trace_Type_reg validate_magic wires.c unzip pick_bits sign_extend from_int num_bytes pick_one reverse_bytes values sign_position                               8@                   T@                   t@                   ˜@                   À@                   @@                   Ø@                   ø@                  	 (@                  
 X@                   x@                    @                   p@                   €@                   ä@                   ğ@                   œ @                   "@                   .`                   .`                    .`                   (.`                   ø/`                    0`                   x0`                    0`                                                                                                                                                      ñÿ                      .`                  °@                  ğ@             .     0@             D     ¨0`            S     .`             z     P@             †     .`             ¥    ñÿ                «    ñÿ                ¸    ñÿ                ¿    ñÿ                Ê    ñÿ                Ô     D@     ¾       ß    ñÿ                    ñÿ                ç     (@             õ      .`                  ñÿ                     .`                 (.`                  .`             .     œ @             A     0`             W    à@            Ë    µ@     “       g                     y                         !@     B       š    C@     S                             º    ¤@     J            x0`             È    q@     :       Ì                     ç    §@     *       û    «@     )                                                 '    «@     9       7    ²@     £      F    ˆ0`             M                     a    Â@            j    @     3       p    ¯@            a    ä@             z    5@     r           @     j       <                     W    Ô@     %       ¤    ä@     6       ³    @     Ù       É    @     ›       Ï    î@     *       Ş    @     %       ç                         Ñ@     ¡          x0`             "    ‚@     I       .    H@     U       8    –@            ;                     P    Ã@     (       [                      j   €0`             w     @     Q      …    ğ@            ”    Œ@     ”       š    p@     e       ª                     ¾    Ø@     I       Ì    @     ‹           °0`                 €@     *       Ó    v@            Ø    @     %       Ü    •@            å    ˆ0`             ñ    ë@     ¹      w    ù@     %       ö    r@     9           ÷@     M                            &                      :    Ë@     S       D                     V   ˆ0`             b                      |    ²@     O       †    U
@     ]      ¤    x@             ’     0`             crtstuff.c __JCR_LIST__ deregister_tm_clones __do_global_dtors_aux completed.7585 __do_global_dtors_aux_fini_array_entry frame_dummy __frame_dummy_init_array_entry alu.c arithmetic.c main.c memories.c support.c Trace_read wires.c __FRAME_END__ __JCR_END__ __init_array_end _DYNAMIC __init_array_start __GNU_EH_FRAME_HDR _GLOBAL_OFFSET_TABLE_ __libc_csu_fini free@@GLIBC_2.2.5 putchar@@GLIBC_2.2.5 sign_extend neg _ITM_deregisterTMCloneTable memory_create add __isoc99_fscanf@@GLIBC_2.7 delete_trace_reader use_if puts@@GLIBC_2.2.5 fread@@GLIBC_2.2.5 validate_mem_wr eval_condition _edata fclose@@GLIBC_2.2.5 from_int error reduce_or create_trace_reader memory_read_from_file validate_pc_wr memory_read_unaligned unzip memory_destroy pick_one __libc_start_main@@GLIBC_2.2.5 validate_magic __data_start memory_read pick_bits is fprintf@@GLIBC_2.2.5 reduce_and __gmon_start__ __dso_handle generic_adder _IO_stdin_used gen_c __libc_csu_init malloc@@GLIBC_2.2.5 reverse_bytes gen_pg bool_xor bool_not __bss_start main validate_reg_wr memory_write fopen@@GLIBC_2.2.5 _Jv_RegisterClasses pick_byte exit@@GLIBC_2.2.5 __TMC_END__ _ITM_registerTMCloneTable same_sign alu_execute stderr@@GLIBC_2.2.5  .symtab .strtab .shstrtab .interp .note.ABI-tag .note.gnu.build-id .gnu.hash .dynsym .dynstr .gnu.version .gnu.version_r .rela.dyn .rela.plt .init .plt.got .text .fini .rodata .eh_frame_hdr .eh_frame .init_array .fini_array .jcr .dynamic .got.plt .data .bss .comment .debug_aranges .debug_info .debug_abbrev .debug_line .debug_str                                                                                    8@     8                                    #             T@     T                                     1             t@     t      $                              D   öÿÿo       ˜@     ˜      (                             N             À@     À      €                          V             @@     @      ˜                              ^   ÿÿÿo       Ø@     Ø                                   k   şÿÿo       ø@     ø      0                            z             (@     (      0                            „      B       X@     X                                              x@     x                                    ‰              @            Ğ                             ”             p@     p                                                 €@     €      b                             £             ä@     ä      	                              ©             ğ@     ğ      ¬                              ±             œ @     œ       l                             ¿             "@     "                                   É             .`     .                                    Õ             .`     .                                    á              .`      .                                    æ             (.`     (.      Ğ                           ˜             ø/`     ø/                                   ï              0`      0      x                             ø             x0`     x0                                    ş              0`     ˆ0                                          0               ˆ0      4                                                  ¼0                                                         Ü1      K                             '                     'M      ò                             5                     T      –                             A     0               ¯Z      ©                                                  ær      L                                                   Xb      è      #   :                 	                      @n      ¦                             #include <inttypes.h>
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
