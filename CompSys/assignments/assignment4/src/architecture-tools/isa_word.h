//
// Nis Wegmann, 2017
//
// isa_word.h
//

#ifndef __ISA_WORD_H__
#define __ISA_WORD_H__

#include <stdbool.h>
#include <stdint.h>

// ---------------------------------------------------------------------------------------------- //

// Basic types.

typedef uint8_t  isa_Half;                      // Half =  4 bits.
typedef uint8_t  isa_Byte;                      // Byte =  8 bits.
typedef uint16_t isa_Word;                      // Word = 16 bits.
typedef uint32_t isa_Long;                      // Long = 32 bits.
typedef uint64_t isa_Quad;                      // Quad = 64 bits.

// ---------------------------------------------------------------------------------------------- //

// Sign-of operators.

static inline bool _isa_signofByte(isa_Byte b) { return (b & 0x80U) ? 1 : 0; }
static inline bool _isa_signofWord(isa_Word w) { return (w & 0x8000U) ? 1 : 0; }
static inline bool _isa_signofLong(isa_Long l) { return (l & 0x80000000UL) ? 1 : 0; }
static inline bool _isa_signofQuad(isa_Quad q) { return (q & 0x8000000000000000ULL) ? 1 : 0; }

//
// Returns the sign of a byte, word, long, or quad (interpreted as a signed integer).
//
#define isa_isSigned(__x__)\
    _Generic((__x__),\
        isa_Byte: _isa_signofByte,\
        isa_Word: _isa_signofWord,\
        isa_Long: _isa_signofLong,\
        isa_Quad: _isa_signofQuad)\
    (__x__)

// ---------------------------------------------------------------------------------------------- //

// Signed constructors.

static inline isa_Quad _isa_signedByte(int8_t  b) { return (uint8_t)  b; }
static inline isa_Quad _isa_signedWord(int16_t w) { return (uint16_t) w; }
static inline isa_Quad _isa_signedLong(int32_t l) { return (uint32_t) l; }
static inline isa_Quad _isa_signedQuad(int64_t q) { return (uint64_t) q; }

//
// Constructs a byte, word, long, or quad from a signed integer.
//
#define isa_signed(__x__)\
    _Generic((__x__),\
        int8_t : _isa_signedByte,\
        int16_t: _isa_signedWord,\
        int32_t: _isa_signedLong,\
        int64_t: _isa_signedQuad)\
    (__x__)

// ---------------------------------------------------------------------------------------------- //

// Extension (zero/sign).

static inline isa_Word isa_zeroExtendByteToWord(isa_Byte b) { return b; }
static inline isa_Word isa_signExtendByteToWord(isa_Byte b) { return (uint16_t) ((int8_t)  b); }
static inline isa_Long isa_zeroExtendByteToLong(isa_Byte b) { return b; }
static inline isa_Long isa_signExtendByteToLong(isa_Byte b) { return (uint32_t) ((int8_t)  b); }
static inline isa_Long isa_zeroExtendWordToLong(isa_Word w) { return w; }
static inline isa_Long isa_signExtendWordToLong(isa_Word w) { return (uint32_t) ((int16_t) w); }
static inline isa_Quad isa_zeroExtendByteToQuad(isa_Byte b) { return b; }
static inline isa_Quad isa_signExtendByteToQuad(isa_Byte b) { return (uint64_t) ((int8_t)  b); }
static inline isa_Quad isa_zeroExtendWordToQuad(isa_Word w) { return w; }
static inline isa_Quad isa_signExtendWordToQuad(isa_Word w) { return (uint64_t) ((int16_t) w); }
static inline isa_Quad isa_zeroExtendLongToQuad(isa_Long l) { return l; }
static inline isa_Quad isa_signExtendLongToQuad(isa_Long l) { return (uint64_t) ((int32_t) l); }

// ---------------------------------------------------------------------------------------------- //

// Packing.

static inline isa_Byte isa_packHalfs(isa_Half lo, isa_Half hi)
    { return (isa_Byte) ((hi & 0xF) << 4) | (lo & 0xF); }
static inline isa_Word isa_packBytes(isa_Byte lo, isa_Byte hi)
    { return (isa_Word) (((isa_Word) hi) <<  8) | lo; }
static inline isa_Long isa_packWords(isa_Word lo, isa_Word hi)
    { return (isa_Long) (((isa_Long) hi) << 16) | lo; }
static inline isa_Quad isa_packLongs(isa_Long lo, isa_Long hi)
    { return (isa_Quad) (((isa_Quad) hi) << 32) | lo; }

//
// Packs two bytes into a word, two words into a long, or two longs into a quad.
//
#define isa_pack(__x__, __y__)\
    _Generic((__x__),\
        isa_Byte: isa_packBytes,\
        isa_Word: isa_packWords,\
        isa_Long: isa_packLongs)\
    (__x__, __y__)

// Unpacking.

static inline isa_Half _isa_byteLo(isa_Byte b) { return b & 0xF; }
static inline isa_Half _isa_byteHi(isa_Byte b) { return b >> 4; }
static inline isa_Byte _isa_wordLo(isa_Word w) { return w & 0xFF; }
static inline isa_Byte _isa_wordHi(isa_Word w) { return w >> 8; }
static inline isa_Word _isa_longLo(isa_Long l) { return l & 0xFFFF; }
static inline isa_Word _isa_longHi(isa_Long l) { return l >> 16; }
static inline isa_Long _isa_quadLo(isa_Quad q) { return q & 0xFFFFFFFF; }
static inline isa_Long _isa_quadHi(isa_Quad q) { return q >> 32; }

//
// Returns the low byte of a word, the low word of a long, or the low long of a quad.
//
#define isa_lo(__x__)\
    _Generic((__x__),\
        isa_Byte: _isa_byteLo,\
        isa_Word: _isa_wordLo,\
        isa_Long: _isa_longLo,\
        isa_Quad: _isa_quadLo)\
    (__x__)

//
// Returns the high byte of a word, the high word of a long, or the high long of a quad.
//
#define isa_hi(__x__)\
    _Generic((__x__),\
        isa_Byte: _isa_byteHi,\
        isa_Word: _isa_wordHi,\
        isa_Long: _isa_longHi,\
        isa_Quad: _isa_quadHi)\
    (__x__)

// ---------------------------------------------------------------------------------------------- //

#endif // __ISA_WORD_H__
