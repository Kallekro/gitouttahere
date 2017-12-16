//
// Nis Wegmann, 2017
//
// isa_register.h
//

#ifndef __ISA_REGISTER_H__
#define __ISA_REGISTER_H__

// ---------------------------------------------------------------------------------------------- //

//
// Register type. A register is encoded by 4 bits.
//
typedef enum isa_Register
{
    isa_Register_rax = 0x0,                     //
    isa_Register_rcx = 0x1,                     //
    isa_Register_rdx = 0x2,                     //
    isa_Register_rbx = 0x3,                     //
    isa_Register_rsp = 0x4,                     //
    isa_Register_rbp = 0x5,                     //
    isa_Register_rsi = 0x6,                     //
    isa_Register_rdi = 0x7,                     //
    isa_Register_r8  = 0x8,                     //
    isa_Register_r9  = 0x9,                     //
    isa_Register_r10 = 0xA,                     //
    isa_Register_r11 = 0xB,                     //
    isa_Register_r12 = 0xC,                     //
    isa_Register_r13 = 0xD,                     //
    isa_Register_r14 = 0xE,                     //
    isa_Register_r15 = 0xF                      //
}
isa_Register;

// ---------------------------------------------------------------------------------------------- //

#endif // __ISA_REGISTER_H__
