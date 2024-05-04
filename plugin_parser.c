#include "plugin_parser.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool str_starts_with(const char *str, const char *pre) {
    return strncmp(pre, str, strlen(pre)) == 0;
}

PatternType parse_pattern_type(const char* spec) {
    if (str_starts_with(spec, "rgb")) {
        return SOLID;
    } else if (str_starts_with(spec, "linear")) {
        return LINEAR;
    } else if (str_starts_with(spec, "radial")) {
        return RADIAL;
    } else {
        fprintf(stderr, "Invalid pattern type: %s\n", spec);
        exit(EXIT_FAILURE);
    }
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

cairo_pattern_t* parse_pattern(const char* spec, char** pattern_str) {
    PatternType type = parse_pattern_type(spec);
    switch (type) {
        case SOLID:
            return parse_solid_color(spec, pattern_str);
        case LINEAR:
            return parse_linear_gradient(spec, pattern_str);
        case RADIAL:
            return parse_radial_gradient(spec, pattern_str);
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