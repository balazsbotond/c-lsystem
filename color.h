#ifndef COLOR_H
#define COLOR_H

typedef struct {
    unsigned char red;
    unsigned char green;
    unsigned char blue;
} Color;

Color color_parse(const char* str);
char* color_str(Color color);

#endif // COLOR_H
