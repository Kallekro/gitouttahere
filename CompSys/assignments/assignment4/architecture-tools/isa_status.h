//
// Nis Wegmann, 2017
//
// isa_status.h
//

#ifndef __ISA_STATUS_H__
#define __ISA_STATUS_H__

//
// Status type.
//
typedef enum isa_Status
{
    isa_Status_aok = 1,                         // Ok.
    isa_Status_hlt = 2,                         // Halt.
    isa_Status_adr = 3,                         // Invalid address.
    isa_Status_ins = 4,                         // Invalid instruction.
    isa_Status_uaa = 5                          // Unaligned address.
}
isa_Status;

#endif // __ISA_STATUS_H__
