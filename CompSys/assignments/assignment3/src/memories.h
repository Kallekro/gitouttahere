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
