#ifndef CHAR_READER_H
#define CHAR_READER_H

#include <stddef.h>

typedef struct CharReader {
    int (*read)(struct CharReader *reader);
    int (*peek)(struct CharReader *reader);
    void (*reset)(struct CharReader *reader);
    void (*destroy)(struct CharReader *reader);
    void *context;
} CharReader;

CharReader *string_reader_create(const char *str);
void string_reader_destroy(CharReader *reader);
void string_reader_reset(CharReader *reader);

CharReader *file_reader_create(const char *filename);
void file_reader_destroy(CharReader *reader);
void file_reader_reset(CharReader *reader);
char* file_reader_get_filename(CharReader *writer);

typedef struct CharWriter {
    void (*write)(struct CharWriter *writer, char c);
    void (*destroy)(struct CharWriter *writer);
    void *context;
} CharWriter;

CharWriter *string_writer_create(size_t initial_capacity);
void string_writer_destroy(CharWriter *writer);
char* string_writer_get(CharWriter *writer);

CharWriter *file_writer_create(const char *filename);
void file_writer_destroy(CharWriter *writer);
char* file_writer_get_filename(CharWriter *writer);

#endif // CHAR_READER_H