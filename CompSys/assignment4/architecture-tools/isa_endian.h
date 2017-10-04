//
// Nis Wegmann, 2017
//
// isa_endian.h
//

#ifndef __ISA_ENDIAN_H__
#define __ISA_ENDIAN_H__

#include "isa_word.h"

// ---------------------------------------------------------------------------------------------- //

#define ISA_BIGENDIAN 0 // Non-portable.

// ---------------------------------------------------------------------------------------------- //

//
// Reads a byte from a block of memory.
//
static inline isa_Byte isa_readByte(unsigned char const * memory)
{
    return *(memory);
}

//
// Writes a byte into a block of memory.
//
static inline void isa_writeByte(unsigned char * memory, isa_Byte byte)
{
    *(memory) = byte;
}

//
// Reads a word from a block of memory.
//
static inline isa_Word isa_readWord(unsigned char const * memory)
{
  #if ISA_BIGENDIAN
    isa_Word word = 0;
    
    word |= ((isa_Word) memory[0]) << 0;
    word |= ((isa_Word) memory[1]) << 8;
    
    return word;
  #else
    return *((isa_Word *) memory);
  #endif
}

//
// Writes a word into a block of memory.
//
static inline void isa_writeWord(unsigned char * memory, isa_Word word)
{
  #if ISA_BIGENDIAN
    memory[0] = (unsigned char) ((word >> 8) & 0xFF);
    memory[1] = (unsigned char) ((word >> 0) & 0xFF);
  #else
    *((isa_Word *) memory) = word;
  #endif
}

//
// Reads a long from a block of memory.
//
static inline isa_Long isa_readLong(unsigned char const * memory)
{
  #if ISA_BIGENDIAN
    isa_Long _long = 0;
    
    _long |= ((isa_Long) memory[0]) << 24;
    _long |= ((isa_Long) memory[1]) << 16;
    _long |= ((isa_Long) memory[2]) <<  8;
    _long |= ((isa_Long) memory[3]) <<  0;
    
    return _long;
  #else
    return *((isa_Long *) memory);
  #endif
}

//
// Writes a long into a block of memory.
//
static inline void isa_writeLong(unsigned char * memory, isa_Long _long)
{
  #if ISA_BIGENDIAN
    memory[0] = (unsigned char) ((_long >> 24) & 0xFF);
    memory[1] = (unsigned char) ((_long >> 16) & 0xFF);
    memory[2] = (unsigned char) ((_long >>  8) & 0xFF);
    memory[3] = (unsigned char) ((_long >>  0) & 0xFF);
  #else
    *((isa_Long *) memory) = _long;
  #endif
}

//
// Reads a quad from a block of memory.
//
static inline isa_Quad isa_readQuad(unsigned char const * memory)
{
  #if ISA_BIGENDIAN
    isa_Quad quad = 0;
    
    quad |= ((isa_Quad) memory[0]) << 56;
    quad |= ((isa_Quad) memory[1]) << 48;
    quad |= ((isa_Quad) memory[2]) << 40;
    quad |= ((isa_Quad) memory[3]) << 32;
    quad |= ((isa_Quad) memory[4]) << 24;
    quad |= ((isa_Quad) memory[5]) << 16;
    quad |= ((isa_Quad) memory[6]) <<  8;
    quad |= ((isa_Quad) memory[7]) <<  0;
    
    return quad;
  #else
    return *((isa_Quad *) memory);
  #endif
}

//
// Writes a quad into a block of memory.
//
static inline void isa_writeQuad(unsigned char * memory, isa_Quad quad)
{
  #if ISA_BIGENDIAN
    memory[0] = (unsigned char) ((quad >> 56) & 0xFF);
    memory[1] = (unsigned char) ((quad >> 48) & 0xFF);
    memory[2] = (unsigned char) ((quad >> 40) & 0xFF);
    memory[3] = (unsigned char) ((quad >> 32) & 0xFF);
    memory[4] = (unsigned char) ((quad >> 24) & 0xFF);
    memory[5] = (unsigned char) ((quad >> 16) & 0xFF);
    memory[6] = (unsigned char) ((quad >>  8) & 0xFF);
    memory[7] = (unsigned char) ((quad >>  0) & 0xFF);
  #else
    *((isa_Quad *) memory) = quad;
  #endif
}

// ---------------------------------------------------------------------------------------------- //

#endif // __ISA_ENDIAN_H__
