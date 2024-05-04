#ifndef VIEWPORT_H
#define VIEWPORT_H

#include <stdbool.h>

typedef struct {
    double width;
    double height;
} Viewport;

bool viewport_equals(Viewport vp1, Viewport vp2);
Viewport viewport_parse(const char* str);

#endif // VIEWPORT_H