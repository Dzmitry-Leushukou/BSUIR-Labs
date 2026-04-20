%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

extern FILE *yyin;

/* Данные задачи */
int n, m;
double *c;
double **A;
double *b;
int *basis;          /* базисные индексы (1‑индексация) */

/* Флаги парсинга */
int parsing_matrix = 0;
int parsing_vector = 0;
int current_row, current_col;
int current_index;

int error_flag = 0;

/* Результаты симплекс-метода */
double *x_opt;
double obj_value;
int unbounded = 0;   /* 1 если целевая функция не ограничена */
int *final_basis;

/* Прототипы */
void yyerror(const char *s);
int yylex(void);

void allocate_data();
void free_data();
void compute_initial_x();
int invert_matrix(double **mat, double **inv, int size);
void simplex_main_phase();
void output_json(const char *filename);
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
    final_basis = (int*)malloc(m * sizeof(int));
}

void free_data() {
    free(c); free(b); free(basis);
    for (int i = 0; i < m; i++) free(A[i]);
    free(A);
    free(x_opt);
    free(final_basis);
}

/* Вычисление начального допустимого плана: x_B = A_B^{-1} b, x_N = 0 */
void compute_initial_x() {
    double **A_B = (double**)malloc(m * sizeof(double*));
    double **inv_A_B = (double**)malloc(m * sizeof(double*));
    for (int i = 0; i < m; i++) {
        A_B[i] = (double*)malloc(m * sizeof(double));
        inv_A_B[i] = (double*)malloc(m * sizeof(double));
        for (int j = 0; j < m; j++) {
            A_B[i][j] = A[i][basis[j]-1];   /* индексы в 1-индексации -> перевод в 0 */
        }
    }
    if (!invert_matrix(A_B, inv_A_B, m)) {
        fprintf(stderr, "Initial basis matrix is singular!\n");
        exit(1);
    }
    /* x_B = inv_A_B * b */
    double *xB = (double*)malloc(m * sizeof(double));
    for (int i = 0; i < m; i++) {
        xB[i] = 0.0;
        for (int j = 0; j < m; j++)
            xB[i] += inv_A_B[i][j] * b[j];
        if (xB[i] < -1e-9) {
            fprintf(stderr, "Initial basic solution is not feasible (x[%d] = %f < 0)\n", basis[i], xB[i]);
            exit(1);
        }
        if (xB[i] < 0) xB[i] = 0.0;
        x_opt[basis[i]-1] = xB[i];
    }
    /* Небазисные переменные = 0 */
    for (int j = 0; j < n; j++) {
        int is_basic = 0;
        for (int i = 0; i < m; i++)
            if (basis[i]-1 == j) { is_basic = 1; break; }
        if (!is_basic) x_opt[j] = 0.0;
    }
    free(xB);
    for (int i = 0; i < m; i++) { free(A_B[i]); free(inv_A_B[i]); }
    free(A_B); free(inv_A_B);
}

/* Обращение матрицы методом Гаусса-Жордана (размер size) */
int invert_matrix(double **mat, double **inv, int size) {
    double **aug = (double**)malloc(size * sizeof(double*));
    for (int i = 0; i < size; i++) {
        aug[i] = (double*)malloc(2*size * sizeof(double));
        for (int j = 0; j < size; j++) aug[i][j] = mat[i][j];
        for (int j = size; j < 2*size; j++) aug[i][j] = (j - size == i) ? 1.0 : 0.0;
    }
    for (int col = 0; col < size; col++) {
        int pivot = -1;
        for (int i = col; i < size; i++) {
            if (fabs(aug[i][col]) > 1e-12) { pivot = i; break; }
        }
        if (pivot == -1) { /* вырождена */ return 0; }
        if (pivot != col) {
            double *tmp = aug[col];
            aug[col] = aug[pivot];
            aug[pivot] = tmp;
        }
        double piv_val = aug[col][col];
        for (int j = 0; j < 2*size; j++) aug[col][j] /= piv_val;
        for (int i = 0; i < size; i++) {
            if (i != col) {
                double factor = aug[i][col];
                for (int j = 0; j < 2*size; j++)
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

/* Основная фаза симплекс-метода (алгоритм из методички) */
void simplex_main_phase() {
    double **A_B = (double**)malloc(m * sizeof(double*));
    double **inv_A_B = (double**)malloc(m * sizeof(double*));
    for (int i = 0; i < m; i++) {
        A_B[i] = (double*)malloc(m * sizeof(double));
        inv_A_B[i] = (double*)malloc(m * sizeof(double));
    }
    double *u = (double*)malloc(m * sizeof(double));
    double *delta = (double*)malloc(n * sizeof(double));
    double *z = (double*)malloc(m * sizeof(double));
    double *theta = (double*)malloc(m * sizeof(double));
    double *xB = (double*)malloc(m * sizeof(double));

    /* Текущий базис и значения базисных переменных */
    int *cur_basis = (int*)malloc(m * sizeof(int));
    memcpy(cur_basis, basis, m * sizeof(int));
    for (int i = 0; i < m; i++) xB[i] = x_opt[cur_basis[i]-1];

    int iteration = 0;
    const int max_iter = 1000;
    while (iteration++ < max_iter) {
        /* Построение базисной матрицы */
        for (int i = 0; i < m; i++)
            for (int j = 0; j < m; j++)
                A_B[i][j] = A[i][cur_basis[j]-1];
        /* Обращение базисной матрицы */
        if (!invert_matrix(A_B, inv_A_B, m)) {
            fprintf(stderr, "Basis matrix singular at iteration %d\n", iteration);
            exit(1);
        }
        /* Вектор потенциалов u^T = c_B^T * inv_A_B */
        double *cB = (double*)malloc(m * sizeof(double));
        for (int i = 0; i < m; i++) cB[i] = c[cur_basis[i]-1];
        for (int i = 0; i < m; i++) {
            u[i] = 0.0;
            for (int j = 0; j < m; j++)
                u[i] += cB[j] * inv_A_B[j][i];
        }
        /* Вектор оценок delta = c - u^T * A */
        for (int j = 0; j < n; j++) {
            double uA = 0.0;
            for (int i = 0; i < m; i++) uA += u[i] * A[i][j];
            delta[j] = c[j] - uA;
        }
        /* Проверка оптимальности */
        int optimal = 1;
        for (int j = 0; j < n; j++) {
            if (delta[j] > 1e-9) { optimal = 0; break; }
        }
        if (optimal) {
            obj_value = 0.0;
            for (int j = 0; j < n; j++) obj_value += c[j] * x_opt[j];
            memcpy(final_basis, cur_basis, m * sizeof(int));
            break;
        }
        /* Выбор входящего индекса (первый с положительной оценкой) */
        int j0 = -1;
        for (int j = 0; j < n; j++) {
            if (delta[j] > 1e-9) { j0 = j; break; }
        }
        /* Вычисление z = inv_A_B * A_{j0} */
        for (int i = 0; i < m; i++) {
            z[i] = 0.0;
            for (int k = 0; k < m; k++)
                z[i] += inv_A_B[i][k] * A[k][j0];
        }
        /* Вычисление theta_i */
        double theta0 = INFINITY;
        int k = -1;
        for (int i = 0; i < m; i++) {
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
            unbounded = 1;
            free(cB); break;
        }
        /* Обновление базиса: замена cur_basis[k] на j0+1 */
        cur_basis[k] = j0 + 1;
        /* Обновление базисных переменных */
        double new_xB_j0 = theta0;
        for (int i = 0; i < m; i++) {
            if (i != k) {
                xB[i] = xB[i] - theta0 * z[i];
                if (xB[i] < 0 && xB[i] > -1e-9) xB[i] = 0.0;
            }
        }
        xB[k] = new_xB_j0;
        /* Обновление вектора x_opt */
        for (int j = 0; j < n; j++) x_opt[j] = 0.0;
        for (int i = 0; i < m; i++) x_opt[cur_basis[i]-1] = xB[i];
        free(cB);
    }
    free(A_B[0]); free(A_B); free(inv_A_B[0]); free(inv_A_B);
    free(u); free(delta); free(z); free(theta); free(xB); free(cur_basis);
}

void output_json(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) { perror("Cannot open output file"); return; }
    fprintf(f, "{\n");
    if (unbounded) {
        fprintf(f, "  \"status\": \"unbounded\",\n");
        fprintf(f, "  \"message\": \"Objective function is not bounded above on the feasible set\"\n");
    } else {
        fprintf(f, "  \"status\": \"optimal\",\n");
        fprintf(f, "  \"objective\": %.10f,\n", obj_value);
        fprintf(f, "  \"x\": [");
        for (int j = 0; j < n; j++) {
            fprintf(f, "%.10f", x_opt[j]);
            if (j < n-1) fprintf(f, ", ");
        }
        fprintf(f, "],\n");
        fprintf(f, "  \"basis\": [");
        for (int i = 0; i < m; i++) {
            fprintf(f, "%d", final_basis[i]);
            if (i < m-1) fprintf(f, ", ");
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
    /* Проверка корректности входных данных */
    if (m != n && m > n) {
        fprintf(stderr, "Warning: m > n, system may be overdetermined.\n");
    }
    compute_initial_x();
    simplex_main_phase();
    output_json("output.json");
    free_data();
    return 0;
}