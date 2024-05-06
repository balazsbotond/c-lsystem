# L-system renderer in C

## TODO

- [x] Palette support
- [ ] Support for writing iterations to disk
- [ ] Research image libraries that might be faster than cairo

## Paper sizes

```c
Viewport viewport = {4960, 7016}; // A4 paper size at 600 DPI
Viewport viewport = {2480, 3508}; // A4 paper size at 300 DPI
Viewport viewport = {3508, 2480}; // A4 paper size at 300 DPI, landscape
```

## Colors

```c
Color colors[] = {
    {234, 173, 237}, // light pink
    {143, 248, 226}, // light cyan
    {235, 190, 237}, // lighter pink
    {199, 220, 208}, // light teal
    {143, 211, 255}, // light blue
    {255, 255, 255},
    {204, 181, 74},
    {110, 240, 74},
    {84, 112, 240}
};
int num_colors = 9;
```

## Gradients

```c
cairo_pattern_t* radpat = cairo_pattern_create_radial(2464, 3223, 100, 2464, 3223, 3000);

// Golden
cairo_pattern_add_color_stop_rgb(radpat, 0.0, 235/255.0,219/255.0,102/255.0);
cairo_pattern_add_color_stop_rgb(radpat, 0.41, 245/255.0, 236/255.0,112/255.0);
cairo_pattern_add_color_stop_rgb(radpat, 0.51, 209/255.0,190/255.0,76/255.0);
cairo_pattern_add_color_stop_rgb(radpat, 0.57, 187/255.0,156/255.0,51/255.0);
cairo_pattern_add_color_stop_rgb(radpat, 0.63, 168/255.0,142/255.0, 42/255.0);
cairo_pattern_add_color_stop_rgb(radpat, 0.69, 202/255.0,174/255.0,68/255.0);
cairo_pattern_add_color_stop_rgb(radpat, 0.75, 218/255.0,202/255.0,86/255.0);
cairo_pattern_add_color_stop_rgb(radpat, 0.81, 208/255.0,187/255.0,73/255.0);
cairo_pattern_add_color_stop_rgb(radpat, 0.87, 187/255.0,156/255.0,51/255.0);
cairo_pattern_add_color_stop_rgb(radpat, 1.00, 137/255.0,108/255.0,26/255.0);

// Simple Pink
cairo_pattern_add_color_stop_rgb(radpat, 0.00, 234/255.0,173/255.0,237/255.0);
cairo_pattern_add_color_stop_rgb(radpat, 0.40, 234/255.0,173/255.0,237/255.0);
cairo_pattern_add_color_stop_rgb(radpat, 1.00, 255/255.0,255/255.0,255/255.0);

// Yellow-Blue
cairo_pattern_add_color_stop_rgb(radpat, 0.00, 251/255.0, 255/255.0, 237/134.0);
cairo_pattern_add_color_stop_rgb(radpat, 1.00, 143/255.0, 211/255.0, 255/255.0);

// Strawberry
cairo_pattern_add_color_stop_rgb(radpat, 0.00, 166/255.0, 203/255.0, 150/255.0);
cairo_pattern_add_color_stop_rgb(radpat, 0.80, 255/255.0, 162/255.0, 172/255.0);
cairo_pattern_add_color_stop_rgb(radpat, 1.00, 255/255.0, 162/255.0, 172/255.0);

cairo_set_source(cr, radpat);
```