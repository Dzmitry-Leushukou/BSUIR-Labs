%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

extern FILE *yyin;

int n, m;
double *c;
double **A;
double *b;
int *basis;

int parsing_matrix = 0;
int parsing_vector = 0;
int current_row, current_col;
int current_index;

int error_flag = 0;

double *x_opt;
double obj_value;
int infeasible = 0;

void yyerror(const char *s);
int yylex(void);

void allocate_data();
void free_data();
void dual_simplex();
void output_json(const char *filename);
int invert_matrix(double **mat, double **inv, int size);
%}

%union {
    double num;
}

%token TOKEN_N TOKEN_M TOKEN_C TOKEN_A TOKEN_B TOKEN_BASIS
%token <num> NUMBER
%token ',' ':' '[' ']' '{' '}'

%%

input: '{' fields '}' ;

fields: field | fields ',' field;

field: n_field | m_field | c_field | A_field | b_field | basis_field;

n_field: TOKEN_N ':' NUMBER { n = (int)$3; };

m_field: TOKEN_M ':' NUMBER { m = (int)$3; allocate_data(); };

c_field: TOKEN_C ':' { parsing_vector = 1; current_index = 0; } vector_c { parsing_vector = 0; };

vector_c: '[' number_list_c ']';

number_list_c: NUMBER {
    if (parsing_vector) c[current_index++] = $1;
} | number_list_c ',' NUMBER {
    if (parsing_vector) c[current_index++] = $3;
};

A_field: TOKEN_A ':' {
    parsing_matrix = 1;
    current_row = 0;
    current_col = 0;
} matrix_A {
    parsing_matrix = 0;
};

matrix_A: '[' row_list_A ']';

row_list_A: row_A | row_list_A ',' row_A;

row_A: '[' number_list_A ']' { current_row++; current_col = 0; };

number_list_A: NUMBER {
    if (parsing_matrix) A[current_row][current_col++] = $1;
} | number_list_A ',' NUMBER {
    if (parsing_matrix) A[current_row][current_col++] = $3;
};

b_field: TOKEN_B ':' { parsing_vector = 1; current_index = 0; } vector_b { parsing_vector = 0; };

vector_b: '[' number_list_b ']';

number_list_b: NUMBER {
    if (parsing_vector) b[current_index++] = $1;
} | number_list_b ',' NUMBER {
    if (parsing_vector) b[current_index++] = $3;
};

basis_field: TOKEN_BASIS ':' { parsing_vector = 1; current_index = 0; } vector_basis { parsing_vector = 0; };

vector_basis: '[' number_list_basis ']';

number_list_basis: NUMBER {
    if (parsing_vector) basis[current_index++] = (int)$1;
} | number_list_basis ',' NUMBER {
    if (parsing_vector) basis[current_index++] = (int)$3;
};

%%

void yyerror(const char *s) {
    fprintf(stderr, "Parse error: %s\n", s);
    error_flag = 1;
}

void allocate_data() {
    c = (double*)malloc(n * sizeof(double));
    b = (double*)malloc(m * sizeof(double));
    basis = (int*)malloc(m * sizeof(int));
    A = (double**)malloc(m * sizeof(double*));
    for (int i = 0; i < m; i++)
        A[i] = (double*)malloc(n * sizeof(double));
    x_opt = (double*)malloc(n * sizeof(double));
}

void free_data() {
    free(c); free(b); free(basis);
    for (int i = 0; i < m; i++) free(A[i]);
    free(A);
    free(x_opt);
}

int invert_matrix(double **mat, double **inv, int size) {
    double **aug = (double**)malloc(size * sizeof(double*));
    for (int i = 0; i < size; i++) {
        aug[i] = (double*)malloc(2 * size * sizeof(double));
        for (int j = 0; j < size; j++) aug[i][j] = mat[i][j];
        for (int j = size; j < 2 * size; j++) aug[i][j] = (j - size == i) ? 1.0 : 0.0;
    }
    
    for (int col = 0; col < size; col++) {
        int pivot = -1;
        for (int i = col; i < size; i++) {
            if (fabs(aug[i][col]) > 1e-12) { pivot = i; break; }
        }
        if (pivot == -1) { return 0; }
        if (pivot != col) {
            double *tmp = aug[col];
            aug[col] = aug[pivot];
            aug[pivot] = tmp;
        }
        double piv_val = aug[col][col];
        for (int j = 0; j < 2 * size; j++) aug[col][j] /= piv_val;
        for (int i = 0; i < size; i++) {
            if (i != col) {
                double factor = aug[i][col];
                for (int j = 0; j < 2 * size; j++)
                    aug[i][j] -= factor * aug[col][j];
            }
        }
    }
    
    for (int i = 0; i < size; i++)
        for (int j = 0; j < size; j++)
            inv[i][j] = aug[i][size + j];
    
    for (int i = 0; i < size; i++) free(aug[i]);
    free(aug);
    return 1;
}

void dual_simplex() {
    double **A_B = (double**)malloc(m * sizeof(double*));
    double **inv_A_B = (double**)malloc(m * sizeof(double*));
    for (int i = 0; i < m; i++) {
        A_B[i] = (double*)malloc(m * sizeof(double));
        inv_A_B[i] = (double*)malloc(m * sizeof(double));
    }
    
    int *cur_basis = (int*)malloc(m * sizeof(int));
    memcpy(cur_basis, basis, m * sizeof(int));
    
    double *y = (double*)malloc(m * sizeof(double));
    double *kappa = (double*)malloc(n * sizeof(double));
    double *cB = (double*)malloc(m * sizeof(double));
    double *delta_y = (double*)malloc(m * sizeof(double));
    double *mu = (double*)malloc(n * sizeof(double));
    double *sigma = (double*)malloc(n * sizeof(double));
    
    int iteration = 0;
    const int max_iter = 1000;
    
    while (iteration++ < max_iter) {
        for (int i = 0; i < m; i++)
            for (int j = 0; j < m; j++)
                A_B[i][j] = A[i][cur_basis[j] - 1];
        
        if (!invert_matrix(A_B, inv_A_B, m)) {
            fprintf(stderr, "Basis matrix is singular at iteration %d\n", iteration);
            exit(1);
        }
        
        for (int i = 0; i < m; i++) {
            cB[i] = c[cur_basis[i] - 1];
        }
        
        for (int i = 0; i < m; i++) {
            y[i] = 0.0;
            for (int j = 0; j < m; j++) {
                y[i] += cB[j] * inv_A_B[j][i];
            }
        }
        
        for (int i = 0; i < m; i++) {
            kappa[cur_basis[i] - 1] = 0.0;
            for (int j = 0; j < m; j++) {
                kappa[cur_basis[i] - 1] += inv_A_B[i][j] * b[j];
            }
        }
        
        for (int j = 0; j < n; j++) {
            int is_basic = 0;
            for (int i = 0; i < m; i++) {
                if (cur_basis[i] - 1 == j) {
                    is_basic = 1;
                    break;
                }
            }
            if (!is_basic) {
                kappa[j] = 0.0;
            }
        }
        
        int all_nonnegative = 1;
        for (int i = 0; i < m; i++) {
            if (kappa[cur_basis[i] - 1] < -1e-9) {
                all_nonnegative = 0;
                break;
            }
        }
        
        if (all_nonnegative) {
            obj_value = 0.0;
            for (int j = 0; j < n; j++) {
                obj_value += c[j] * kappa[j];
            }
            memcpy(x_opt, kappa, n * sizeof(double));
            break;
        }
        
        int k = -1;
        for (int i = 0; i < m; i++) {
            if (kappa[cur_basis[i] - 1] < -1e-9) {
                k = i;
                break;
            }
        }
        
        for (int i = 0; i < m; i++) {
            delta_y[i] = inv_A_B[k][i];
        }
        
        for (int j = 0; j < n; j++) {
            int is_basic = 0;
            for (int i = 0; i < m; i++) {
                if (cur_basis[i] - 1 == j) {
                    is_basic = 1;
                    break;
                }
            }
            if (!is_basic) {
                mu[j] = 0.0;
                for (int i = 0; i < m; i++) {
                    mu[j] += delta_y[i] * A[i][j];
                }
            }
        }
        
        int all_mu_nonnegative = 1;
        for (int j = 0; j < n; j++) {
            int is_basic = 0;
            for (int i = 0; i < m; i++) {
                if (cur_basis[i] - 1 == j) {
                    is_basic = 1;
                    break;
                }
            }
            if (!is_basic && mu[j] > -1e-12) {
                all_mu_nonnegative = 1;
            } else if (!is_basic && mu[j] < -1e-12) {
                all_mu_nonnegative = 0;
                break;
            }
        }
        
        if (all_mu_nonnegative) {
            infeasible = 1;
            break;
        }
        
        for (int j = 0; j < n; j++) {
            int is_basic = 0;
            for (int i = 0; i < m; i++) {
                if (cur_basis[i] - 1 == j) {
                    is_basic = 1;
                    break;
                }
            }
            if (!is_basic && mu[j] < -1e-12) {
                double A_j_dot_y = 0.0;
                for (int i = 0; i < m; i++) {
                    A_j_dot_y += A[i][j] * y[i];
                }
                sigma[j] = (c[j] - A_j_dot_y) / mu[j];
                if (sigma[j] < 0) sigma[j] = 1e20;
            } else {
                sigma[j] = 1e20;
            }
        }
        
        double sigma0 = 1e20;
        int j0 = -1;
        for (int j = 0; j < n; j++) {
            if (sigma[j] < sigma0 - 1e-12) {
                sigma0 = sigma[j];
                j0 = j;
            }
        }
        
        cur_basis[k] = j0 + 1;
    }
    
    for (int i = 0; i < m; i++) {
        free(A_B[i]);
        free(inv_A_B[i]);
    }
    free(A_B);
    free(inv_A_B);
    free(cur_basis);
    free(y);
    free(kappa);
    free(cB);
    free(delta_y);
    free(mu);
    free(sigma);
}

void output_json(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) { perror("Cannot open output file"); return; }
    
    fprintf(f, "{\n");
    if (infeasible) {
        fprintf(f, "  \"status\": \"infeasible\",\n");
        fprintf(f, "  \"message\": \"Primal problem is infeasible\"\n");
    } else {
        fprintf(f, "  \"status\": \"optimal\",\n");
        fprintf(f, "  \"objective\": %.10f,\n", obj_value);
        fprintf(f, "  \"x\": [");
        for (int j = 0; j < n; j++) {
            if (x_opt[j] < 1e-12 && x_opt[j] > -1e-12) x_opt[j] = 0.0;
            fprintf(f, "%.10f", x_opt[j]);
            if (j < n - 1) fprintf(f, ", ");
        }
        fprintf(f, "]\n");
    }
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
    
    dual_simplex();
    output_json("output.json");
    free_data();
    
    return 0;
}
