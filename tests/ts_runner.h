#ifndef TS_RUNNER_INCLUDE
#define TS_RUNNER_INCLUDE

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>

/* Dynamic array implementation taken from:
    https://gist.github.com/rexim/48d7087bfc8ba2c28a8b0b3aa3183558
*/

typedef struct {
    size_t count;
    size_t capacity;
} Header;
#define ARR_INIT_CAPACITY 1

#define arr_push(arr, x)                                                              \
    do {                                                                              \
        if (arr == NULL) {                                                            \
            Header *header = malloc(sizeof(*arr)*ARR_INIT_CAPACITY + sizeof(Header)); \
            header->count = 0;                                                        \
            header->capacity = ARR_INIT_CAPACITY;                                     \
            arr = (void*)(header + 1);                                                \
        }                                                                             \
        Header *header = (Header*)(arr) - 1;                                          \
        if (header->count >= header->capacity) {                                      \
            header->capacity *= 2;                                                    \
            header = realloc(header, sizeof(*arr)*header->capacity + sizeof(Header)); \
            arr = (void*)(header + 1);                                                \
        }                                                                             \
        (arr)[header->count++] = (x);                                                 \
    } while(0)

#define arr_len(arr) ((Header*)(arr) - 1)->count
#define arr_free(arr) free((Header*)(arr) - 1)

/* String view implementation taken from:
    https://gist.github.com/rexim/2bc46ae1aa18b9d05a4b3316f4f1aa1f
*/

typedef struct {
    const char *data;
    size_t count;
} String_View;

#define SV_Fmt "%.*s" 
#define SV_Arg(s) (int)(s).count, (s).data

String_View sv(const char *cstr);

bool sv_contains(String_View haystack, String_View needle);

typedef struct {
    char *data;
    size_t count;
    size_t capacity;
} String_Builder;

void sb_ensure_size(String_Builder *sb, size_t additional_bytes);

void sb_appendf(String_Builder *sb, const char *fmt, ...);

void sb_free(String_Builder *sb);

String_View sb_to_sv(String_Builder sb); 

#define CLR_GREEN "\x1b[32m"
#define CLR_RED "\x1b[31m"
#define CLR_RESET "\x1b[0m"

typedef struct {
    String_View name;
    String_View command;
    String_View expected;
} TestCase;

bool run_test(TestCase t);

void add_test_opt(TestCase **suite, TestCase test);

#define add_test(suite, ...) add_test_opt(suite, (TestCase){__VA_ARGS__})

typedef struct {
    TestCase **save_failed;
} _run_entire_suite_options;

void run_entire_suite_opt(TestCase *suite, TestCase **save_failed);

#define run_entire_suite(suite, ...) \
    run_entire_suite_opt((suite), ((_run_entire_suite_options){__VA_ARGS__}).save_failed)

#endif /* TS_RUNNER_INCLUDE */

#ifdef TS_RUNNER_IMPLEMENTATION

String_View sv(const char *cstr) {

    return (String_View) {
        .data = cstr,
        .count = strlen(cstr),
    };
}

bool sv_contains(String_View haystack, String_View needle) {
    if(needle.count == 0) return true;
    if(needle.count > haystack.count) return 0;

    for(size_t i = 0; i <= haystack.count - needle.count; ++i) {
        if(memcmp(haystack.data + i, needle.data, needle.count) == 0) {
            return true;
        }
    }
    return false;
}

void sb_ensure_size(String_Builder *sb, size_t additional_bytes) {
    size_t needed = sb->count + additional_bytes;
    if (needed > sb->capacity) {
        if(sb->capacity == 0) {
            sb->capacity = 256;
        }
        while(sb->capacity < needed) {
            sb->capacity *= 2;
        }
        sb->data = realloc(sb->data, sb->capacity);
    }
}

void sb_appendf(String_Builder *sb, const char *fmt, ...) {
    
    va_list args;

    va_start(args, fmt);
    int needed = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    if (needed <= 0) return;

    sb_ensure_size(sb, (size_t)needed);

    va_start(args, fmt);

    vsnprintf(sb->data + sb->count, (size_t)needed + 1, fmt, args);
    va_end(args);

    sb->count += (size_t)needed;
}

void sb_free(String_Builder *sb) {
    free(sb->data);
    sb->data = NULL;
    sb->count = 0;
    sb->capacity = 0;
}

String_View sb_to_sv(String_Builder sb) {
    return(String_View){.data = sb.data, .count = sb.count};
}

bool run_test(TestCase t) {
    char cmd_with_redir[1024];
    char *full_output = NULL;
    int c;

    snprintf(cmd_with_redir, sizeof(cmd_with_redir), SV_Fmt " 2>&1", SV_Arg(t.command));

    int target_width = 60;
    int name_len = (int)t.name.count;
    int padding = target_width - name_len;
    if(padding < 0) padding = 0;

    printf("Checking " SV_Fmt "...%*s", SV_Arg(t.name), padding, "");
    fflush(stdout);

    FILE *fp = popen(cmd_with_redir, "r");

    if(!fp) {
        printf(CLR_RED "FAIL (popen error)" CLR_RESET "\n");
        return false;
    }

    while ((c = getc(fp)) != EOF) {
        arr_push(full_output, (char)c);
    }
    pclose(fp);

    String_View output_sv = {.data = full_output,
                             .count = arr_len(full_output) };

    bool found = sv_contains(output_sv, t.expected);

    if(found) {
        printf(CLR_GREEN "SUCCESS" CLR_RESET "\n");
    } else {
        printf(CLR_RED "FAIL" CLR_RESET "\n");
    }

    arr_free(full_output);
    return found;
}

void add_test_opt(TestCase **suite, TestCase test) {

    arr_push(*suite, test);

}

void run_entire_suite_opt(TestCase *suite, TestCase **save_failed) {

    bool should_save_failed = false;
    if(save_failed != NULL) should_save_failed = true;

    for(size_t i = 0; i < arr_len(suite); ++i) {
        if(!run_test(suite[i]) && should_save_failed) {
            arr_push(*save_failed, suite[i]);
        }

    }

}

#endif /* TS_RUNNER_IMPLEMENTATION */
