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

