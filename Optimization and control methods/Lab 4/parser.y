
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

int parsing_matrix = 0;
int parsing_vector = 0;
int current_row, current_col;
int current_index;
int error_flag = 0;

double *x_opt;
double obj_value;
int infeasible = 0;
int *final_basis;
int final_basis_size;

void yyerror(const char *s);
int yylex(void);

void allocate_data();
void free_data();
int solve_two_phase();
int invert_matrix(double **mat, double **inv, int size);
void simplex_main_phase(int phase_n, int phase_m, double *phase_c, double **phase_A, double *phase_b, double *phase_x, int *phase_basis, int *phase_basis_size, int *unbounded);
void output_json(const char *filename);
%}

%union {
    double num;
}

%token TOKEN_N TOKEN_M TOKEN_C TOKEN_A TOKEN_B
%token <num> NUMBER
%token ',' ':' '[' ']' '{' '}'

%%

input: '{' fields '}' ;

fields: field | fields ',' field;

field: n_field | m_field | c_field | A_field | b_field;

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

%%

void yyerror(const char *s) {
    fprintf(stderr, "Parse error: %s\n", s);
    error_flag = 1;
}

void allocate_data() {
    c = (double*)malloc(n * sizeof(double));
    b = (double*)malloc(m * sizeof(double));
    A = (double**)malloc(m * sizeof(double*));
    for (int i = 0; i < m; i++)
        A[i] = (double*)malloc(n * sizeof(double));
    x_opt = (double*)malloc(n * sizeof(double));
    final_basis = (int*)malloc(m * sizeof(int));
}

void free_data() {
    free(c);
    free(b);
    for (int i = 0; i < m; i++) free(A[i]);
    free(A);
    free(x_opt);
    free(final_basis);
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
        if (pivot == -1) {
            for (int i = 0; i < size; i++) free(aug[i]);
            free(aug);
            return 0;
        }
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

void simplex_main_phase(int phase_n, int phase_m, double *phase_c, double **phase_A, double *phase_b, double *phase_x, int *phase_basis, int *phase_basis_size, int *unbounded) {
    double **A_B = (double**)malloc(phase_m * sizeof(double*));
    double **inv_A_B = (double**)malloc(phase_m * sizeof(double*));
    for (int i = 0; i < phase_m; i++) {
        A_B[i] = (double*)malloc(phase_m * sizeof(double));
        inv_A_B[i] = (double*)malloc(phase_m * sizeof(double));
    }
    
    double *u = (double*)malloc(phase_m * sizeof(double));
    double *delta = (double*)malloc(phase_n * sizeof(double));
    double *z = (double*)malloc(phase_m * sizeof(double));
    double *theta = (double*)malloc(phase_m * sizeof(double));
    double *xB = (double*)malloc(phase_m * sizeof(double));
    
    for (int i = 0; i < phase_m; i++) {
        xB[i] = phase_x[phase_basis[i] - 1];
    }
    
    int iteration = 0;
    const int max_iter = 1000;
    *unbounded = 0;
    
    while (iteration++ < max_iter) {
        for (int i = 0; i < phase_m; i++)
            for (int j = 0; j < phase_m; j++)
                A_B[i][j] = phase_A[i][phase_basis[j] - 1];
        
        if (!invert_matrix(A_B, inv_A_B, phase_m)) {
            fprintf(stderr, "Basis matrix singular at iteration %d\n", iteration);
            exit(1);
        }
        
        double *cB = (double*)malloc(phase_m * sizeof(double));
        for (int i = 0; i < phase_m; i++) cB[i] = phase_c[phase_basis[i] - 1];
        
        for (int i = 0; i < phase_m; i++) {
            u[i] = 0.0;
            for (int j = 0; j < phase_m; j++)
                u[i] += cB[j] * inv_A_B[j][i];
        }
        
        for (int j = 0; j < phase_n; j++) {
            double uA = 0.0;
            for (int i = 0; i < phase_m; i++) uA += u[i] * phase_A[i][j];
            delta[j] = phase_c[j] - uA;
        }
        
        int optimal = 1;
        for (int j = 0; j < phase_n; j++) {
            if (delta[j] > 1e-9) { optimal = 0; break; }
        }
        
        if (optimal) {
            free(cB);
            break;
        }
        
        int j0 = -1;
        for (int j = 0; j < phase_n; j++) {
            if (delta[j] > 1e-9) { j0 = j; break; }
        }
        
        for (int i = 0; i < phase_m; i++) {
            z[i] = 0.0;
            for (int k = 0; k < phase_m; k++)
                z[i] += inv_A_B[i][k] * phase_A[k][j0];
        }
        
        double theta0 = INFINITY;
        int k = -1;
        for (int i = 0; i < phase_m; i++) {
            if (z[i] > 1e-12) {
                theta[i] = xB[i] / z[i];
                if (theta[i] < theta0) {
                    theta0 = theta[i];
                    k = i;
                }
            } else {
                theta[i] = INFINITY;
            }
        }
        
        if (theta0 == INFINITY) {
            *unbounded = 1;
            free(cB);
            break;
        }
        
        phase_basis[k] = j0 + 1;
        
        double new_xB_j0 = theta0;
        for (int i = 0; i < phase_m; i++) {
            if (i != k) {
                xB[i] = xB[i] - theta0 * z[i];
                if (xB[i] < 0 && xB[i] > -1e-9) xB[i] = 0.0;
            }
        }
        xB[k] = new_xB_j0;
        
        for (int j = 0; j < phase_n; j++) phase_x[j] = 0.0;
        for (int i = 0; i < phase_m; i++) phase_x[phase_basis[i] - 1] = xB[i];
        
        free(cB);
    }
    
    for (int i = 0; i < phase_m; i++) {
        free(A_B[i]);
        free(inv_A_B[i]);
    }
    free(A_B);
    free(inv_A_B);
    free(u);
    free(delta);
    free(z);
    free(theta);
    free(xB);
}

int solve_two_phase() {
    for (int i = 0; i < m; i++) {
        if (b[i] < 0) {
            b[i] = -b[i];
            for (int j = 0; j < n; j++)
                A[i][j] = -A[i][j];
        }
    }
    
    int phase_n = n + m;
    double *phase_c = (double*)malloc(phase_n * sizeof(double));
    double **phase_A = (double**)malloc(m * sizeof(double*));
    for (int i = 0; i < m; i++) {
        phase_A[i] = (double*)malloc(phase_n * sizeof(double));
        for (int j = 0; j < n; j++)
            phase_A[i][j] = A[i][j];
        for (int j = 0; j < m; j++)
            phase_A[i][n + j] = (i == j) ? 1.0 : 0.0;
    }
    
    for (int j = 0; j < n; j++)
        phase_c[j] = 0.0;
    for (int j = 0; j < m; j++)
        phase_c[n + j] = -1.0;
    
    double *phase_x = (double*)malloc(phase_n * sizeof(double));
    for (int j = 0; j < n; j++) phase_x[j] = 0.0;
    for (int i = 0; i < m; i++) phase_x[n + i] = b[i];
    
    int *phase_basis = (int*)malloc(m * sizeof(int));
    for (int i = 0; i < m; i++)
        phase_basis[i] = n + i + 1;
    
    int phase_basis_size = m;
    int unbounded_flag;
    
    simplex_main_phase(phase_n, m, phase_c, phase_A, b, phase_x, phase_basis, &phase_basis_size, &unbounded_flag);
    
    double artificial_sum = 0.0;
    for (int i = 0; i < m; i++)
        artificial_sum += phase_x[n + i];
    
    if (artificial_sum > 1e-9) {
        infeasible = 1;
        free(phase_c);
        for (int i = 0; i < m; i++) free(phase_A[i]);
        free(phase_A);
        free(phase_x);
        free(phase_basis);
        return 0;
    }
    
    for (int j = 0; j < n; j++)
        x_opt[j] = phase_x[j];
    
    int remaining = 0;
    for (int i = 0; i < m; i++) {
        if (phase_basis[i] <= n) {
            final_basis[remaining++] = phase_basis[i];
        }
    }
    
    if (remaining == m) {
        final_basis_size = m;
        free(phase_c);
        for (int i = 0; i < m; i++) free(phase_A[i]);
        free(phase_A);
        free(phase_x);
        free(phase_basis);
        return 1;
    }
    
    int rows_to_keep = m;
    int *keep_row = (int*)malloc(m * sizeof(int));
    for (int i = 0; i < m; i++) keep_row[i] = 1;
    
    while (1) {
        int artificial_found = -1;
        for (int i = 0; i < phase_basis_size; i++) {
            if (phase_basis[i] > n) {
                artificial_found = i;
                break;
            }
        }
        
        if (artificial_found == -1) break;
        
        int i_idx = phase_basis[artificial_found] - n - 1;
        
        double **A_B_curr = (double**)malloc(phase_basis_size * sizeof(double*));
        double **inv_A_B_curr = (double**)malloc(phase_basis_size * sizeof(double*));
        for (int ii = 0; ii < phase_basis_size; ii++) {
            A_B_curr[ii] = (double*)malloc(phase_basis_size * sizeof(double));
            inv_A_B_curr[ii] = (double*)malloc(phase_basis_size * sizeof(double));
            for (int jj = 0; jj < phase_basis_size; jj++) {
                A_B_curr[ii][jj] = phase_A[ii][phase_basis[jj] - 1];
            }
        }
        
        invert_matrix(A_B_curr, inv_A_B_curr, phase_basis_size);
        
        int replacement_found = 0;
        for (int j = 0; j < n; j++) {
            int is_basic = 0;
            for (int ii = 0; ii < phase_basis_size; ii++) {
                if (phase_basis[ii] == j + 1) {
                    is_basic = 1;
                    break;
                }
            }
            if (is_basic) continue;
            
            double *lvec = (double*)malloc(phase_basis_size * sizeof(double));
            for (int ii = 0; ii < phase_basis_size; ii++) {
                lvec[ii] = 0.0;
                for (int kk = 0; kk < phase_basis_size; kk++) {
                    lvec[ii] += inv_A_B_curr[ii][kk] * phase_A[kk][j];
                }
            }
            
            if (fabs(lvec[artificial_found]) > 1e-12) {
                phase_basis[artificial_found] = j + 1;
                replacement_found = 1;
                free(lvec);
                break;
            }
            free(lvec);
        }
        
        for (int ii = 0; ii < phase_basis_size; ii++) {
            free(A_B_curr[ii]);
            free(inv_A_B_curr[ii]);
        }
        free(A_B_curr);
        free(inv_A_B_curr);
        
        if (!replacement_found) {
            keep_row[i_idx] = 0;
            rows_to_keep--;
            for (int k = artificial_found; k < phase_basis_size - 1; k++) {
                phase_basis[k] = phase_basis[k + 1];
            }
            phase_basis_size--;
        }
    }
    
    final_basis_size = 0;
    for (int i = 0; i < phase_basis_size; i++) {
        if (phase_basis[i] <= n) {
            final_basis[final_basis_size++] = phase_basis[i];
        }
    }
    
    if (rows_to_keep < m) {
        double **new_A = (double**)malloc(rows_to_keep * sizeof(double*));
        double *new_b = (double*)malloc(rows_to_keep * sizeof(double));
        int new_row = 0;
        for (int i = 0; i < m; i++) {
            if (keep_row[i]) {
                new_A[new_row] = (double*)malloc(n * sizeof(double));
                for (int j = 0; j < n; j++)
                    new_A[new_row][j] = A[i][j];
                new_b[new_row] = b[i];
                new_row++;
            }
        }
        
        for (int i = 0; i < m; i++) free(A[i]);
        free(A);
        free(b);
        
        m = rows_to_keep;
        A = new_A;
        b = new_b;
    }
    
    free(keep_row);
    free(phase_c);
    for (int i = 0; i < m; i++) free(phase_A[i]);
    free(phase_A);
    free(phase_x);
    free(phase_basis);
    
    return 1;
}

void output_json(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) { perror("Cannot open output file"); return; }
    
    fprintf(f, "{\n");
    if (infeasible) {
        fprintf(f, "  \"status\": \"infeasible\",\n");
        fprintf(f, "  \"message\": \"The problem has no feasible solutions\"\n");
    } else {
        obj_value = 0.0;
        for (int j = 0; j < n; j++)
            obj_value += c[j] * x_opt[j];
        
        fprintf(f, "  \"status\": \"optimal\",\n");
        fprintf(f, "  \"objective\": %.10f,\n", obj_value);
        fprintf(f, "  \"x\": [");
        for (int j = 0; j < n; j++) {
            fprintf(f, "%.10f", x_opt[j]);
            if (j < n - 1) fprintf(f, ", ");
        }
        fprintf(f, "],\n");
        fprintf(f, "  \"basis\": [");
        for (int i = 0; i < final_basis_size; i++) {
            fprintf(f, "%d", final_basis[i]);
            if (i < final_basis_size - 1) fprintf(f, ", ");
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
    
    solve_two_phase();
    output_json("output.json");
    free_data();
    return 0;
}
