//
// Nis Wegmann, 2017
//
// sim_step.h
//

#ifndef __SIM_STEP_H__
#define __SIM_STEP_H__

//#define USE_ANNOTATION

#ifdef USE_ANNOTATION
#include "ano.h"
#else
typedef void Annotation;
#endif

#include "isa.h"

// ---------------------------------------------------------------------------------------------- //

// ---------------------------------------------------------------------------------------------- //

//
// .
//
isa_Status sim_step
    (
        isa_State * restrict,                   // State. (nonnull)
        Annotation *,                           // Annotation. (nullable)
        FILE *                                  // Trace-file. (nullable)
    );

// ---------------------------------------------------------------------------------------------- //

#endif // __SIM_STEP_H__
