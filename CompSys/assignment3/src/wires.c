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

