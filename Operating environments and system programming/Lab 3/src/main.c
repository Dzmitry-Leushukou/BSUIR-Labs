#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "morse.h"

void print_usage(const char* progname) {
    fprintf(stderr, "Usage: %s [options] [input_file]\n", progname);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -o <file>   Output file (default: stdout)\n");
    fprintf(stderr, "  -h          Show this help message\n");
    fprintf(stderr, "If input_file is not specified, reads from stdin.\n");
}

int main(int argc, char* argv[]) {
    FILE *in = stdin, *out = stdout;
    const char *out_filename = NULL;
    int opt;
    
    while ((opt = getopt(argc, argv, "ho:")) != -1) {
        switch (opt) {
            case 'o':
                out_filename = optarg;
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }
    
    if (optind < argc) {
        const char* in_filename = argv[optind];
        in = fopen(in_filename, "r");
        if (!in) {
            perror("fopen input");
            return 1;
        }
    }
    
    if (out_filename) {
        out = fopen(out_filename, "w");
        if (!out) {
            perror("fopen output");
            if (in != stdin) fclose(in);
            return 1;
        }
    }
    
    int ch;
    int first_in_line = 1;
    int output_something = 0; 
    
    while ((ch = fgetc(in)) != EOF) {
 
        if (ch == '\r') continue;
        
        if (ch == '\n') {
            fputc('\n', out);
            first_in_line = 1;
            continue;
        }
        
        const char* code = morse_encode((char)ch);
        if (code != NULL) {
            if (!first_in_line) fputc(' ', out);
            fputs(code, out);
            first_in_line = 0;
            output_something = 1;
        }
    }
    
 
    if (!first_in_line && output_something) {
        fputc('\n', out);
    }
    
    if (in != stdin) fclose(in);
    if (out != stdout) fclose(out);
    
    return 0;
}
