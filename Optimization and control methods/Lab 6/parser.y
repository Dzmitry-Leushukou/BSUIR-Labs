%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

extern FILE *yyin;

int m, n;
double *a;
double *b;
double **c;
double **x;
int basis_size;
int *basis_rows;
int *basis_cols;

int parsing_matrix = 0;
int parsing_vector = 0;
int current_row, current_col;
int current_index;
int error_flag = 0;

double optimal_value;

void yyerror(const char *s);
int yylex(void);

void allocate_data() {
    a = (double*)calloc(m, sizeof(double));
    b = (double*)calloc(n, sizeof(double));
    c = (double**)malloc(m * sizeof(double*));
    x = (double**)calloc(m, sizeof(double*));
    for (int i = 0; i < m; i++) {
        c[i] = (double*)calloc(n, sizeof(double));
        x[i] = (double*)calloc(n, sizeof(double));
    }
    basis_size = m + n - 1;
    basis_rows = (int*)malloc(basis_size * sizeof(int));
    basis_cols = (int*)malloc(basis_size * sizeof(int));
}

void free_data() {
    free(a);
    free(b);
    for (int i = 0; i < m; i++) {
        free(c[i]);
        free(x[i]);
    }
    free(c);
    free(x);
    free(basis_rows);
    free(basis_cols);
}

void northwest_corner_method() {
    double *a_rem = (double*)malloc(m * sizeof(double));
    double *b_rem = (double*)malloc(n * sizeof(double));
    for (int i = 0; i < m; i++) a_rem[i] = a[i];
    for (int j = 0; j < n; j++) b_rem[j] = b[j];
    int i = 0, j = 0, idx = 0;
    while (i < m && j < n) {
        double q = (a_rem[i] < b_rem[j]) ? a_rem[i] : b_rem[j];
        x[i][j] = q;
        basis_rows[idx] = i;
        basis_cols[idx] = j;
        idx++;
        a_rem[i] -= q;
        b_rem[j] -= q;
        if (a_rem[i] < 1e-12 && i < m-1) i++;
        else if (b_rem[j] < 1e-12 && j < n-1) j++;
        else { i++; j++; }
    }
    free(a_rem);
    free(b_rem);
}

int is_basic(int row, int col, int cnt, int *br, int *bc) {
    for (int k = 0; k < cnt; k++)
        if (br[k] == row && bc[k] == col) return 1;
    return 0;
}

void potentials(double *u, double *v, int cnt, int *br, int *bc) {
    for (int i = 0; i < m; i++) u[i] = NAN;
    for (int j = 0; j < n; j++) v[j] = NAN;
    u[0] = 0;
    int changed;
    do {
        changed = 0;
        for (int k = 0; k < cnt; k++) {
            int i = br[k], j = bc[k];
            if (!isnan(u[i]) && isnan(v[j])) {
                v[j] = c[i][j] - u[i];
                changed = 1;
            } else if (isnan(u[i]) && !isnan(v[j])) {
                u[i] = c[i][j] - v[j];
                changed = 1;
            }
        }
    } while (changed);
    for (int i = 0; i < m; i++) if (isnan(u[i])) u[i] = 0;
    for (int j = 0; j < n; j++) if (isnan(v[j])) v[j] = 0;
}

int dfs(int sr, int sc, int r, int c, int pr, int pc, int *cyc_r, int *cyc_c, int *len, int cnt, int *br, int *bc, int *vis) {
    for (int k = 0; k < cnt; k++) {
        int nr = br[k], nc = bc[k];
        if (nr == pr && nc == pc) continue;
        if ((nr == r && nc != c) || (nc == c && nr != r)) {
            if (nr == sr && nc == sc && *len >= 2) {
                cyc_r[*len] = nr;
                cyc_c[*len] = nc;
                (*len)++;
                return 1;
            }
            if (!vis[k]) {
                vis[k] = 1;
                cyc_r[*len] = nr;
                cyc_c[*len] = nc;
                (*len)++;
                if (dfs(sr, sc, nr, nc, r, c, cyc_r, cyc_c, len, cnt, br, bc, vis))
                    return 1;
                (*len)--;
                vis[k] = 0;
            }
        }
    }
    return 0;
}

int find_cycle(int sr, int sc, int *cyc_r, int *cyc_c, int *len, int cnt, int *br, int *bc) {
    int *vis = (int*)calloc(cnt, sizeof(int));
    cyc_r[0] = sr;
    cyc_c[0] = sc;
    *len = 1;
    for (int k = 0; k < cnt; k++) {
        int nr = br[k], nc = bc[k];
        if (nr == sr || nc == sc) {
            vis[k] = 1;
            cyc_r[*len] = nr;
            cyc_c[*len] = nc;
            (*len)++;
            if (dfs(sr, sc, nr, nc, sr, sc, cyc_r, cyc_c, len, cnt, br, bc, vis)) {
                free(vis);
                return 1;
            }
            (*len)--;
            vis[k] = 0;
        }
    }
    free(vis);
    return 0;
}

void transport_simplex() {
    int *br = (int*)malloc(basis_size * sizeof(int));
    int *bc = (int*)malloc(basis_size * sizeof(int));
    int cur = 0;
    for (int i = 0; i < m; i++)
        for (int j = 0; j < n; j++)
            if (x[i][j] > 1e-9 && cur < basis_size) {
                br[cur] = i; bc[cur] = j; cur++;
            }
    if (cur < basis_size) {
        for (int i = 0; i < m && cur < basis_size; i++)
            for (int j = 0; j < n && cur < basis_size; j++)
                if (fabs(x[i][j]) < 1e-9) {
                    int ok = 1;
                    for (int k = 0; k < cur; k++)
                        if (br[k] == i && bc[k] == j) { ok = 0; break; }
                    if (ok) { br[cur] = i; bc[cur] = j; cur++; }
                }
    }
    double *u = (double*)malloc(m * sizeof(double));
    double *v = (double*)malloc(n * sizeof(double));
    int iter = 0;
    while (iter < 100) {
        potentials(u, v, cur, br, bc);
        int ei = -1, ej = -1; double best = 0;
        for (int i = 0; i < m; i++)
            for (int j = 0; j < n; j++)
                if (!is_basic(i, j, cur, br, bc)) {
                    double d = c[i][j] - u[i] - v[j];
                    if (d < best - 1e-9) { best = d; ei = i; ej = j; }
                }
        if (ei == -1) break;
        int *cyc_r = (int*)malloc((m+n+10) * sizeof(int));
        int *cyc_c = (int*)malloc((m+n+10) * sizeof(int));
        int clen = 0;
        if (!find_cycle(ei, ej, cyc_r, cyc_c, &clen, cur, br, bc)) {
            free(cyc_r); free(cyc_c); break;
        }
        double theta = 1e100;
        int li = -1;
        for (int k = 1; k < clen; k += 2) {
            int ii = cyc_r[k], jj = cyc_c[k];
            if (x[ii][jj] < theta - 1e-9) {
                theta = x[ii][jj];
                li = k;
            }
        }
        if (li == -1) { free(cyc_r); free(cyc_c); break; }
        for (int k = 0; k < clen; k++) {
            int ii = cyc_r[k], jj = cyc_c[k];
            if (k % 2 == 0) x[ii][jj] += theta;
            else x[ii][jj] -= theta;
        }
        for (int k = 0; k < cur; k++)
            if (br[k] == cyc_r[li] && bc[k] == cyc_c[li]) {
                br[k] = ei; bc[k] = ej; break;
            }
        free(cyc_r); free(cyc_c);
        iter++;
    }
    optimal_value = 0;
    for (int i = 0; i < m; i++)
        for (int j = 0; j < n; j++)
            optimal_value += c[i][j] * x[i][j];
    for (int i = 0; i < cur; i++) {
        basis_rows[i] = br[i] + 1;
        basis_cols[i] = bc[i] + 1;
    }
    free(u); free(v); free(br); free(bc);
}

void print_solution() {
    printf("\n========================================\n");
    printf("OPTIMAL TRANSPORTATION PLAN:\n");
    printf("========================================\n");
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) printf("%8.2f ", x[i][j]);
        printf("| %8.2f\n", a[i]);
    }
    printf("----------------------------------------\n");
    for (int j = 0; j < n; j++) printf("%8.2f ", b[j]);
    printf("\n\nTOTAL COST: %.2f\n", optimal_value);
    printf("========================================\n");
}

void output_json(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) return;
    fprintf(f, "{\n  \"status\": \"optimal\",\n  \"objective\": %.10f,\n  \"x\": [\n", optimal_value);
    for (int i = 0; i < m; i++) {
        fprintf(f, "    [");
        for (int j = 0; j < n; j++) {
            fprintf(f, "%.10f", x[i][j]);
            if (j < n-1) fprintf(f, ", ");
        }
        fprintf(f, "]");
        if (i < m-1) fprintf(f, ",");
        fprintf(f, "\n");
    }
    fprintf(f, "  ],\n  \"basis\": [\n");
    for (int i = 0; i < basis_size; i++) {
        fprintf(f, "    [%d, %d]", basis_rows[i], basis_cols[i]);
        if (i < basis_size-1) fprintf(f, ",");
        fprintf(f, "\n");
    }
    fprintf(f, "  ]\n}\n");
    fclose(f);
}

%}

%union { double num; }
%token TOKEN_M TOKEN_N TOKEN_A TOKEN_B TOKEN_C
%token <num> NUMBER
%token ',' ':' '[' ']' '{' '}'

%%

input: '{' fields '}' ;
fields: field | fields ',' field ;
field: m_field | n_field | a_field | b_field | c_field ;
m_field: TOKEN_M ':' NUMBER { m = (int)$3; };
n_field: TOKEN_N ':' NUMBER { n = (int)$3; allocate_data(); };
a_field: TOKEN_A ':' { parsing_vector = 1; current_index = 0; } vector_a { parsing_vector = 0; };
vector_a: '[' number_list_a ']';
number_list_a: NUMBER { if (parsing_vector) a[current_index++] = $1; }
    | number_list_a ',' NUMBER { if (parsing_vector) a[current_index++] = $3; };
b_field: TOKEN_B ':' { parsing_vector = 1; current_index = 0; } vector_b { parsing_vector = 0; };
vector_b: '[' number_list_b ']';
number_list_b: NUMBER { if (parsing_vector) b[current_index++] = $1; }
    | number_list_b ',' NUMBER { if (parsing_vector) b[current_index++] = $3; };
c_field: TOKEN_C ':' { parsing_matrix = 1; current_row = 0; current_col = 0; } matrix_c { parsing_matrix = 0; };
matrix_c: '[' row_list_c ']';
row_list_c: row_c | row_list_c ',' row_c;
row_c: '[' number_list_c ']' { current_row++; current_col = 0; };
number_list_c: NUMBER { if (parsing_matrix) c[current_row][current_col++] = $1; }
    | number_list_c ',' NUMBER { if (parsing_matrix) c[current_row][current_col++] = $3; };

%%

void yyerror(const char *s) { fprintf(stderr, "Parse error: %s\n", s); error_flag = 1; }

int main(int argc, char **argv) {
    if (argc != 2) { fprintf(stderr, "Usage: %s input.json\n", argv[0]); return 1; }
    FILE *file = fopen(argv[1], "r");
    if (!file) { fprintf(stderr, "Cannot open input file\n"); return 1; }
    yyin = file; yyparse(); fclose(file);
    if (error_flag) { fprintf(stderr, "Parsing failed.\n"); return 1; }
    double ta = 0, tb = 0;
    for (int i = 0; i < m; i++) ta += a[i];
    for (int j = 0; j < n; j++) tb += b[j];
    if (fabs(ta - tb) > 1e-9) {
        fprintf(stderr, "Error: Balance condition violated (sum a=%.2f, sum b=%.2f)\n", ta, tb);
        free_data(); return 1;
    }
    northwest_corner_method();
    transport_simplex();
    print_solution();
    output_json("output.json");
    free_data();
    return 0;
}
