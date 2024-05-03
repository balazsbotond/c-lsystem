#include "viewport.h"

bool viewport_equals(Viewport* vp1, Viewport* vp2) {
    return vp1->width == vp2->width && vp1->height == vp2->height;
}
