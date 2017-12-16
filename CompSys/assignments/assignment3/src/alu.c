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

