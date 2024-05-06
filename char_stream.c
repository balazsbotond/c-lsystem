#include "char_stream.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

/*
 * String Reader
 */

typedef struct {
    const char *str;
    size_t pos;
} StringReaderContext;

int string_reader_read(CharReader* reader) {
    StringReaderContext* context = (StringReaderContext*)reader->context;
    if (context->str[context->pos] == '\0') {
        return EOF;
    } else {
        return context->str[context->pos++];
    }
}

int string_reader_peek(CharReader* reader) {
    StringReaderContext* context = (StringReaderContext*)reader->context;
    if (context->str[context->pos] == '\0') {
        return EOF;
    } else {
        return context->str[context->pos];
    }
}

void string_reader_reset(CharReader *reader) {
    ((StringReaderContext*)reader->context)->pos = 0;
}

void string_reader_destroy(CharReader *reader) {
    free(reader->context);
    free(reader);
}

CharReader* string_reader_create(const char *str) {
    CharReader* reader = malloc(sizeof(CharReader));
    StringReaderContext *context = malloc(sizeof(StringReaderContext));
    context->str = str;
    context->pos = 0;

    reader->read = string_reader_read;
    reader->peek = string_reader_peek;
    reader->reset = string_reader_reset;
    reader->destroy = string_reader_destroy;
    reader->context = context;

    return reader;
}

/*
 * String Writer
 */

typedef struct {
    char *buffer;
    size_t capacity;
    size_t length;
} StringWriterContext;

void string_writer_write(CharWriter *writer, char c) {
    StringWriterContext *context = (StringWriterContext *)writer->context;
    if (context->length + 1 >= context->capacity) {
        size_t new_capacity = context->capacity * 2;
        context->buffer = realloc(context->buffer, new_capacity);
        context->capacity = new_capacity;
    }
    context->buffer[context->length++] = c;
    context->buffer[context->length] = '\0';
}

char* string_writer_get(CharWriter *writer) {
    return ((StringWriterContext *)writer->context)->buffer;
}

void string_writer_destroy(CharWriter *writer) {
    free(writer->context);
    free(writer);
}

CharWriter *string_writer_create(size_t initial_capacity) {
    CharWriter *writer = malloc(sizeof(CharWriter));
    StringWriterContext *context = malloc(sizeof(StringWriterContext));
    context->capacity = initial_capacity;
    context->length = 0;
    context->buffer = malloc(context->capacity);
    context->buffer[0] = '\0';

    writer->write = string_writer_write;
    writer->destroy = string_writer_destroy;
    writer->context = context;
    return writer;
}

/*
 * File Reader
 */

typedef struct {
    char* filename;
    FILE *file;
    bool buffered;
    int buffer;
} FileReaderContext;

int file_reader_peek(CharReader* reader) {
    FileReaderContext* context = (FileReaderContext*)reader->context;
    if (!context->buffered) {
        context->buffer = fgetc(context->file);
        if (context->buffer == EOF) {
            return EOF;
        }
        context->buffered = true;
    }
    return context->buffer;
}

int file_reader_read(CharReader* reader) {
    FileReaderContext* context = (FileReaderContext*)reader->context;
    if (context->buffered) {
        context->buffered = 0;
        return context->buffer;
    } else {
        return fgetc(context->file);
    }
}

void file_reader_reset(CharReader *reader) {
    FileReaderContext* context = (FileReaderContext*)reader->context;
    fseek(context->file, 0, SEEK_SET);
    context->buffered = false;
    context->buffer = EOF;
}

char* file_reader_get_filename(CharReader *reader) {
    return strdup(((FileReaderContext*)reader->context)->filename);
}

void file_reader_destroy(CharReader *reader) {
    FileReaderContext* context = (FileReaderContext*)reader->context;
    fclose(context->file);
    free(context->filename);
    free(reader->context);
    free(reader);
}

CharReader* file_reader_create(const char* filename) {
    CharReader* reader = malloc(sizeof(CharReader));
    FileReaderContext* context = malloc(sizeof(FileReaderContext));
    context->filename = strdup(filename);
    context->file = fopen(filename, "r");
    context->buffered = false;
    context->buffer = EOF;

    if (context->file == NULL) {
        free(context);
        free(reader);
        return NULL;
    }

    reader->read = file_reader_read;
    reader->peek = file_reader_peek;
    reader->reset = file_reader_reset;
    reader->destroy = file_reader_destroy;
    reader->context = context;
    return reader;
}

/*
 * File Writer
 */

typedef struct {
    char* filename;
    FILE *file;
} FileWriterContext;

void file_writer_write(CharWriter *writer, char c) {
    FileWriterContext *context = (FileWriterContext *)writer->context;
    fputc(c, context->file);
}

char* file_writer_get_filename(CharWriter *writer) {
    return strdup(((FileWriterContext *)writer->context)->filename);
}

void file_writer_destroy(CharWriter *writer) {
    FileWriterContext *context = (FileWriterContext *)writer->context;
    fclose(context->file);
    free(context->filename);
    free(writer->context);
    free(writer);
}

CharWriter *file_writer_create(const char *filename) {
    CharWriter *writer = malloc(sizeof(CharWriter));
    FileWriterContext *context = malloc(sizeof(FileWriterContext));
    context->filename = strdup(filename);
    context->file = fopen(filename, "w");
    if (!context->file) {
        free(context);
        free(writer);
        return NULL;
    }

    writer->write = file_writer_write;
    writer->destroy = file_writer_destroy;
    writer->context = context;
    return writer;
}