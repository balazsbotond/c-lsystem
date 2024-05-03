#ifndef COORDINATE_SYSTEM_H
#define COORDINATE_SYSTEM_H

#include "rectangle.h"
#include "vector.h"
#include "viewport.h"

typedef struct {
    double scale_factor;
    double origin_x;
    double origin_y;
} CoordinateSystem;

CoordinateSystem* cs_fit(Rectangle* bounding_rect, Viewport* viewport, int padding);
double cs_convert_x(CoordinateSystem* cs, double x);
double cs_convert_y(CoordinateSystem* cs, double y);
Vector cs_convert(CoordinateSystem* cs, Vector v);
void cs_destroy(CoordinateSystem* cs);

#endif // COORDINATE_SYSTEM_H