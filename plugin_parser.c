#include "plugin_parser.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool str_starts_with(const char *str, const char *pre) {
    return strncmp(pre, str, strlen(pre)) == 0;
}

void parse_color_stops(char* file_name, cairo_pattern_t* pattern) {
    char* file_path = malloc(strlen(file_name) + strlen("gradients/.grad") + 1);
    sprintf(file_path, "gradients/%s.grad", file_name);

    FILE *file;
    double offset;
    unsigned char r, g, b;
    int result;

    file = fopen(file_path, "r");
    if (file == NULL) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    while ((result = fscanf(file, "%lf %hhu %hhu %hhu", &offset, &r, &g, &b)) == 4) {
        cairo_pattern_add_color_stop_rgb(pattern, offset, r / 255.0, g / 255.0, b / 255.0);
    }

    if (result != EOF) {
        fprintf(stderr, "Failed to read or parse the file correctly.\n");
        exit(EXIT_FAILURE);
    }

    fclose(file);
}

cairo_pattern_t* parse_solid_color(const char* spec, char** pattern_str) {
    unsigned char r, g, b;
    if (sscanf(spec, "rgb(%hhu,%hhu,%hhu)", &r, &g, &b) != 3) {
        fprintf(stderr, "Invalid solid color spec: %s\n", spec);
        exit(EXIT_FAILURE);
    }

    *pattern_str = strdup(spec);

    cairo_pattern_t* p = cairo_pattern_create_rgb(r / 255.0, g / 255.0, b / 255.0);

    return p;
}

cairo_pattern_t* parse_linear_gradient(const char* spec, char** pattern_str) {
    char file_name[256];
    double x0, y0, x1, y1;
    if (sscanf(spec, "linear(%255[^,],%lf,%lf,%lf,%lf)", file_name, &x0, &y0, &x1, &y1) != 5) {
        fprintf(stderr, "Invalid linear gradient spec: %s\n", spec);
        exit(EXIT_FAILURE);
    }

    cairo_pattern_t* pattern = cairo_pattern_create_linear(x0, y0, x1, y1);

    parse_color_stops(file_name, pattern);

    *pattern_str = malloc(strlen(file_name) + strlen("linear()") + 1);
    sprintf(*pattern_str, "linear(%s)", file_name);

    return pattern;
}

cairo_pattern_t* parse_radial_gradient(const char* spec, char** pattern_str) {
    char file_name[256];
    double cx0, cy0, r0, cx1, cy1, r1;
    if (sscanf(spec, "radial(%255[^,],%lf,%lf,%lf,%lf,%lf,%lf)", file_name, &cx0, &cy0, &r0, &cx1, &cy1, &r1) != 7) {
        fprintf(stderr, "Invalid radial gradient spec: %s\n", spec);
        exit(EXIT_FAILURE);
    }

    cairo_pattern_t* pattern = cairo_pattern_create_radial(cx0, cy0, r0, cx1, cy1, r1);

    parse_color_stops(file_name, pattern);

    *pattern_str = malloc(strlen(file_name) +  + strlen("radial()") + 1);
    sprintf(*pattern_str, "radial(%s)", file_name);

    return pattern;
}

cairo_pattern_t* parse_pattern(ColorPluginType type, const char* spec, char** pattern_str) {
    switch (type) {
        case COLOR_PLUGIN_SOLID:
            return parse_solid_color(spec, pattern_str);
        case COLOR_PLUGIN_LINEAR_GRADIENT:
            return parse_linear_gradient(spec, pattern_str);
        case COLOR_PLUGIN_RADIAL_GRADIENT:
            return parse_radial_gradient(spec, pattern_str);
        default:
            fprintf(stderr, "This is not a pattern plugin: %s\n", spec);
            exit(EXIT_FAILURE);
    }
}

StackDepthLineWidthOptions parse_stack_depth_line_width_options(const char* spec) {
    StackDepthLineWidthOptions options;
    options.step = 1;

    int num_parsed = sscanf(spec, "stack(%lf,%lf,%d)", &options.max_width, &options.min_width, &options.step);

    if (num_parsed < 1) {
        fprintf(stderr, "Invalid format for stack depth line width options: %s\n", spec);
        exit(EXIT_FAILURE);
    }

    if (num_parsed == 1) {
        options.min_width = options.max_width;
    }
    // if num_parsed == 2, step is already set to 1

    return options;
}

void parse_palette_colors(char* file_name, Palette* palette) {
    char* file_path = malloc(strlen(file_name) + strlen("palettes/.pal") + 1);
    sprintf(file_path, "palettes/%s.pal", file_name);

    FILE *file;
    unsigned char r, g, b;
    int result;

    file = fopen(file_path, "r");
    if (file == NULL) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    while ((result = fscanf(file, "%hhu %hhu %hhu", &r, &g, &b)) == 3) {
        palette->colors = realloc(palette->colors, (palette->colors_count + 1) * sizeof(Color));
        palette->colors[palette->colors_count].red = r;
        palette->colors[palette->colors_count].green = g;
        palette->colors[palette->colors_count].blue = b;
        palette->colors_count++;
    }

    if (result != EOF) {
        fprintf(stderr, "Failed to read or parse the file correctly.\n");
        exit(EXIT_FAILURE);
    }

    fclose(file);
}

ColorPluginType parse_palette_type(const char* spec) {
    if (str_starts_with(spec, "palette(linear")) {
        return COLOR_PLUGIN_LINEAR_PALETTE;
    } else if (str_starts_with(spec, "palette(stack")) {
        return COLOR_PLUGIN_STACK_DEPTH_PALETTE;
    } else {
        fprintf(stderr, "This is not a palette: %s\n", spec);
        exit(EXIT_FAILURE);
    }
}

PaletteOverflowBehavior parse_palette_overflow_behavior(const char* spec) {
    if (str_starts_with(spec, "wrap")) {
        return PALETTE_OVERFLOW_WRAP;
    } else if (str_starts_with(spec, "clamp")) {
        return PALETTE_OVERFLOW_CLAMP;
    } else {
        fprintf(stderr, "Invalid palette behavior: %s\n", spec);
        exit(EXIT_FAILURE);
    }
}

Palette parse_palette(const char* spec, char** palette_str) {
    char type[7], behavior[6], file_name[256];
    if (sscanf(spec, "palette(%6[^,],%5[^,],%255[^)])", type, behavior, file_name) != 3) {
        fprintf(stderr, "Invalid palette spec: %s\n", spec);
        exit(EXIT_FAILURE);
    }

    ColorPluginType palette_type = parse_palette_type(spec);
    PaletteOverflowBehavior palette_overflow = parse_palette_overflow_behavior(behavior);
    Palette palette;
    palette.colors = NULL;
    palette.colors_count = 0;
    palette.overflow = palette_overflow;
    palette.index = 0;

    parse_palette_colors(file_name, &palette);

    *palette_str = strdup(spec);

    return palette;
}

ColorPluginType parse_color_plugin_type(const char* spec) {
    if (str_starts_with(spec, "rgb")) {
        return COLOR_PLUGIN_SOLID;
    } else if (str_starts_with(spec, "linear")) {
        return COLOR_PLUGIN_LINEAR_GRADIENT;
    } else if (str_starts_with(spec, "radial")) {
        return COLOR_PLUGIN_RADIAL_GRADIENT;
    } else if (str_starts_with(spec, "palette(linear")) {
        return COLOR_PLUGIN_LINEAR_PALETTE;
    } else if (str_starts_with(spec, "palette(stack")) {
        return COLOR_PLUGIN_STACK_DEPTH_PALETTE;
    } else {
        fprintf(stderr, "Invalid color plugin type: %s\n", spec);
        exit(EXIT_FAILURE);
    }
}

Plugin parse_color_plugin(const char* spec, char** plugin_str) {
    ColorPluginType type = parse_color_plugin_type(spec);

    switch (type) {
        case COLOR_PLUGIN_SOLID:
        case COLOR_PLUGIN_LINEAR_GRADIENT:
        case COLOR_PLUGIN_RADIAL_GRADIENT: {
            cairo_pattern_t* pattern = parse_pattern(type, spec, plugin_str);
            return lsys_plugin_pattern_create(pattern);
        }
        case COLOR_PLUGIN_LINEAR_PALETTE: {
            Palette palette = parse_palette(spec, plugin_str);
            return lsys_plugin_linear_palette_create(palette);
        }
        case COLOR_PLUGIN_STACK_DEPTH_PALETTE: {
            Palette palette = parse_palette(spec, plugin_str);
            return lsys_plugin_stack_depth_palette_create(palette);
        }
        default: {
            fprintf(stderr, "This is not a color plugin: %s\n", spec);
            exit(EXIT_FAILURE);
        }
    }
}

Plugin parse_bgcolor_plugin(const char* spec, char** plugin_str) {
    ColorPluginType type = parse_color_plugin_type(spec);

    switch (type) {
        case COLOR_PLUGIN_SOLID:
        case COLOR_PLUGIN_LINEAR_GRADIENT:
        case COLOR_PLUGIN_RADIAL_GRADIENT:
            cairo_pattern_t* pattern = parse_pattern(type, spec, plugin_str);
            return lsys_plugin_bg_pattern_create(pattern);
        case COLOR_PLUGIN_LINEAR_PALETTE:
        case COLOR_PLUGIN_STACK_DEPTH_PALETTE:
            fprintf(stderr, "Palette cannot be used as a background: %s\n", spec);
            exit(EXIT_FAILURE);
        default: {
            fprintf(stderr, "This is not a color plugin: %s\n", spec);
            exit(EXIT_FAILURE);
        }
    }
}
