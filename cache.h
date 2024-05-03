#ifndef CACHE_H
#define CACHE_H

#include "coordinate_system.h"
#include "lsys.h"
#include "viewport.h"
#include <stdbool.h>

void cache_save(
    LSystem* lsys,
    Viewport* viewport,
    CoordinateSystem* cs,
    char* instructions,
    const char* file_path
);

bool cache_load(
    LSystem** lsys,
    Viewport** viewport,
    CoordinateSystem** cs,
    char** instructions,
    const char* file_path
);

bool cache_stale(
    LSystem* lsys,
    LSystem* cached_lsys,
    Viewport* viewport,
    Viewport* cached_viewport
);

#endif // CACHE_H