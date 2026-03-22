%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

extern FILE *yyin;

// Global variables
int n;                      // matrix size
double **A, **A_inv;        // original matrices
double **A_bar, **A_bar_inv;// modified and its inverse
double **Q;                 // auxiliary matrix
double *x, *l, *l_tilde, *l_hat; // vectors
int i;                      // column to replace (1-indexed)

// Parsing state
int parsing_matrix_flag = 0;   // 1 if currently parsing a matrix
int parsing_vector_flag = 0;   // 1 if parsing a vector
int current_parsing_matrix;    // 0 for A, 1 for A_inv
int current_row, current_col;  // for matrix parsing
int current_vec_index;          // for vector parsing
int error_flag = 0;             // error indicator

void yyerror(const char *s);
int yylex(void);

// Memory management
void allocate_matrices(int n);
void allocate_vector(int n);
void free_matrices();
void free_vector();

// Computation functions
void compute_A_bar();
void compute_l();
int is_invertible();
void compute_l_tilde();
void compute_l_hat();
void compute_Q();
void compute_A_bar_inv();
void output_json(const char *filename);
%}

%union {
    double num;
}

%token TOKEN_SIZE TOKEN_A TOKEN_A_INV TOKEN_X TOKEN_I
%token <num> NUMBER
%token ',' ':' '[' ']' '{' '}'

%%

input: '{' size_field ',' A_field ',' A_inv_field ',' x_field ',' i_field '}' {
    // all data parsed
};

size_field: TOKEN_SIZE ':' NUMBER {
    n = (int)$3;
    if (n <= 0) {
        yyerror("Invalid size");
        YYERROR;
    }
    allocate_matrices(n);
    allocate_vector(n);
};

A_field: TOKEN_A ':' {
    parsing_matrix_flag = 1;
    current_parsing_matrix = 0;
    current_row = 0;
    current_col = 0;
} matrix {
    parsing_matrix_flag = 0;
};

A_inv_field: TOKEN_A_INV ':' {
    parsing_matrix_flag = 1;
    current_parsing_matrix = 1;
    current_row = 0;
    current_col = 0;
} matrix {
    parsing_matrix_flag = 0;
};

x_field: TOKEN_X ':' {
    parsing_vector_flag = 1;
    current_vec_index = 0;
} vector {
    parsing_vector_flag = 0;
};

i_field: TOKEN_I ':' NUMBER {
    i = (int)$3;
    if (i < 1 || i > n) {
        yyerror("Invalid column index");
        YYERROR;
    }
};

matrix: '[' row_list ']';

row_list: row | row_list ',' row;

row: '[' number_list ']' {
    current_row++;
    current_col = 0;
};

number_list: NUMBER {
    if (parsing_matrix_flag) {
        if (current_parsing_matrix == 0)
            A[current_row][current_col] = $1;
        else
            A_inv[current_row][current_col] = $1;
        current_col++;
    } else if (parsing_vector_flag) {
        x[current_vec_index++] = $1;
    }
} | number_list ',' NUMBER {
    if (parsing_matrix_flag) {
        if (current_parsing_matrix == 0)
            A[current_row][current_col] = $3;
        else
            A_inv[current_row][current_col] = $3;
        current_col++;
    } else if (parsing_vector_flag) {
        x[current_vec_index++] = $3;
    }
};

vector: '[' number_list ']';

%%

void yyerror(const char *s) {
    fprintf(stderr, "Parse error: %s\n", s);
    error_flag = 1;
}

void allocate_matrices(int n) {
    A = (double**)malloc(n * sizeof(double*));
    A_inv = (double**)malloc(n * sizeof(double*));
    A_bar = (double**)malloc(n * sizeof(double*));
    A_bar_inv = (double**)malloc(n * sizeof(double*));
    Q = (double**)malloc(n * sizeof(double*));
    for (int i = 0; i < n; i++) {
        A[i] = (double*)malloc(n * sizeof(double));
        A_inv[i] = (double*)malloc(n * sizeof(double));
        A_bar[i] = (double*)malloc(n * sizeof(double));
        A_bar_inv[i] = (double*)malloc(n * sizeof(double));
        Q[i] = (double*)malloc(n * sizeof(double));
    }
}

void allocate_vector(int n) {
    x = (double*)malloc(n * sizeof(double));
    l = (double*)malloc(n * sizeof(double));
    l_tilde = (double*)malloc(n * sizeof(double));
    l_hat = (double*)malloc(n * sizeof(double));
}

void free_matrices() {
    for (int i = 0; i < n; i++) {
        free(A[i]); free(A_inv[i]); free(A_bar[i]); free(A_bar_inv[i]); free(Q[i]);
    }
    free(A); free(A_inv); free(A_bar); free(A_bar_inv); free(Q);
}

void free_vector() {
    free(x); free(l); free(l_tilde); free(l_hat);
}

void compute_A_bar() {
    for (int r = 0; r < n; r++) {
        for (int c = 0; c < n; c++) {
            A_bar[r][c] = A[r][c];
        }
        A_bar[r][i-1] = x[r];
    }
}

void compute_l() {
    for (int r = 0; r < n; r++) {
        l[r] = 0.0;
        for (int k = 0; k < n; k++) {
            l[r] += A_inv[r][k] * x[k];
        }
    }
}

int is_invertible() {
    return fabs(l[i-1]) > 1e-12;
}

void compute_l_tilde() {
    for (int j = 0; j < n; j++) {
        l_tilde[j] = l[j];
    }
    l_tilde[i-1] = -1.0;
}

void compute_l_hat() {
    double factor = -1.0 / l[i-1];
    for (int j = 0; j < n; j++) {
        l_hat[j] = factor * l_tilde[j];
    }
}

void compute_Q() {
    for (int r = 0; r < n; r++) {
        for (int c = 0; c < n; c++) {
            Q[r][c] = (r == c) ? 1.0 : 0.0;
        }
        Q[r][i-1] = l_hat[r];
    }
}

void compute_A_bar_inv() {
    // A_bar_inv = Q * A_inv
    for (int j = 0; j < n; j++) {
        for (int k = 0; k < n; k++) {
            if (j == i-1) {
                A_bar_inv[j][k] = l_hat[j] * A_inv[i-1][k];
            } else {
                A_bar_inv[j][k] = A_inv[j][k] + l_hat[j] * A_inv[i-1][k];
            }
        }
    }
}

void output_json(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        fprintf(stderr, "Cannot open output file %s\n", filename);
        return;
    }

    fprintf(f, "{\n");

    // A_bar
    fprintf(f, "  \"A_bar\": [\n");
    for (int r = 0; r < n; r++) {
        fprintf(f, "    [");
        for (int c = 0; c < n; c++) {
            fprintf(f, "%.6f", A_bar[r][c]);
            if (c < n-1) fprintf(f, ", ");
        }
        fprintf(f, "]");
        if (r < n-1) fprintf(f, ",");
        fprintf(f, "\n");
    }
    fprintf(f, "  ],\n");

    // A_bar_inv
    fprintf(f, "  \"A_bar_inv\": [\n");
    for (int r = 0; r < n; r++) {
        fprintf(f, "    [");
        for (int c = 0; c < n; c++) {
            fprintf(f, "%.6f", A_bar_inv[r][c]);
            if (c < n-1) fprintf(f, ", ");
        }
        fprintf(f, "]");
        if (r < n-1) fprintf(f, ",");
        fprintf(f, "\n");
    }
    fprintf(f, "  ],\n");

    // l
    fprintf(f, "  \"l\": [");
    for (int j = 0; j < n; j++) {
        fprintf(f, "%.6f", l[j]);
        if (j < n-1) fprintf(f, ", ");
    }
    fprintf(f, "],\n");

    // l_tilde
    fprintf(f, "  \"l_tilde\": [");
    for (int j = 0; j < n; j++) {
        fprintf(f, "%.6f", l_tilde[j]);
        if (j < n-1) fprintf(f, ", ");
    }
    fprintf(f, "],\n");

    // l_hat
    fprintf(f, "  \"l_hat\": [");
    for (int j = 0; j < n; j++) {
        fprintf(f, "%.6f", l_hat[j]);
        if (j < n-1) fprintf(f, ", ");
    }
    fprintf(f, "],\n");

    // Q
    fprintf(f, "  \"Q\": [\n");
    for (int r = 0; r < n; r++) {
        fprintf(f, "    [");
        for (int c = 0; c < n; c++) {
            fprintf(f, "%.6f", Q[r][c]);
            if (c < n-1) fprintf(f, ", ");
        }
        fprintf(f, "]");
        if (r < n-1) fprintf(f, ",");
        fprintf(f, "\n");
    }
    fprintf(f, "  ]\n");

    fprintf(f, "}\n");
    fclose(f);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s input.json\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    if (!file) {
        fprintf(stderr, "Cannot open input file %s\n", argv[1]);
        return 1;
    }

    yyin = file;
    yyparse();
    fclose(file);

    if (error_flag) {
        fprintf(stderr, "Parsing failed.\n");
        return 1;
    }

    compute_A_bar();
    compute_l();

    if (!is_invertible()) {
        fprintf(stderr, "Matrix A_bar is singular (l_i = 0).\n");
        return 1;
    }

    compute_l_tilde();
    compute_l_hat();
    compute_Q();
    compute_A_bar_inv();

    output_json("output.json");

    free_matrices();
    free_vector();

    return 0;
}