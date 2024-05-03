#include "coordinate_system.h"
#include <stdlib.h>

CoordinateSystem* cs_fit(Rectangle* bounding_rect, Viewport* viewport, int padding) {
    CoordinateSystem* cs = malloc(sizeof(CoordinateSystem));

    double viewport_width  = viewport->width  - padding * 2;
    double viewport_height = viewport->height - padding * 2;

    double bounding_rect_width  = abs(bounding_rect->right  - bounding_rect->left);
    double bounding_rect_height = abs(bounding_rect->bottom - bounding_rect->top);
    double source_aspect_ratio  = bounding_rect_width / bounding_rect_height;
    double dest_aspect_ratio    = viewport_width      / viewport_height;

    double scale_factor =
        source_aspect_ratio > dest_aspect_ratio
            ? viewport_width  / bounding_rect_width
            : viewport_height / bounding_rect_height;

    double offset_x =
        source_aspect_ratio > dest_aspect_ratio
            ? 0
            : (viewport_width - scale_factor * bounding_rect_width) / 2;

    double offset_y =
        source_aspect_ratio > dest_aspect_ratio
            ? (viewport_height - scale_factor * bounding_rect_height) / 2
            : 0;

    cs->scale_factor = scale_factor;
    cs->origin_x = -(scale_factor * bounding_rect->left) + offset_x + padding;
    cs->origin_y = scale_factor * bounding_rect->top + offset_y + padding;

    return cs;
}

double cs_convert_x(CoordinateSystem* cs, double x) {
    return cs->origin_x + cs->scale_factor * x;
}

double cs_convert_y(CoordinateSystem* cs, double y) {
    return cs->origin_y - cs->scale_factor * y;
}

Vector cs_convert(CoordinateSystem* cs, Vector v) {
    Vector result;
    result.x = cs_convert_x(cs, v.x);
    result.y = cs_convert_y(cs, v.y);
    return result;
}

void cs_destroy(CoordinateSystem* cs) {
    free(cs);
}