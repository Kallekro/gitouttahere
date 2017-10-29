//
// Nis Wegmann, 2017
//
// sim_dump.h
//

#ifndef __SIMULATOR_DUMP_H__
#define __SIMULATOR_DUMP_H__

#include "isa.h"

// ---------------------------------------------------------------------------------------------- //

//
// .
//
void sim_dumpState
    (
        isa_State const *,                      // State.
        unsigned char const *                   // Backup of original memory block.
    );

// ---------------------------------------------------------------------------------------------- //

#endif // __SIMULATOR_DUMP_H__
