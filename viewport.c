#include "viewport.h"
#include <stdio.h>
#include <stdlib.h>

bool viewport_equals(Viewport vp1, Viewport vp2) {
    return vp1.width == vp2.width && vp1.height == vp2.height;
}

Viewport viewport_parse(const char* str) {
    Viewport vp;
    if (sscanf(str, "%lfx%lf", &vp.width, &vp.height) == 2) {
        return vp;
    } else {
        printf("Invalid viewport string: %s\n", str);
        exit(EXIT_FAILURE);
    }
}
