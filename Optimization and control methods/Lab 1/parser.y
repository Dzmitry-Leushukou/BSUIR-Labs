%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

extern FILE *yyin;

/* Глобальные переменные программы */
int n;                      // размер матриц (n x n)
double **A, **A_inv;        // исходные матрица и её обратная
double **A_bar, **A_bar_inv;// модифицированная матрица и её обратная
double **Q;                 // вспомогательная матрица Q (для вывода)
double *x, *l, *l_tilde, *l_hat; // векторы
int i;                      // номер заменяемого столбца (1-индексация)

/* Флаги состояния парсера для разбора матриц и векторов */
int parsing_matrix_flag = 0;   // 1 – сейчас разбираем матрицу
int parsing_vector_flag = 0;   // 1 – разбираем вектор
int current_parsing_matrix;    // 0 = A, 1 = A_inv
int current_row, current_col;  // текущие индексы при заполнении матриц
int current_vec_index;         // индекс при заполнении вектора x
int error_flag = 0;            // флаг ошибки (ненулевой, если парсинг провалился)

void yyerror(const char *s);
int yylex(void);

/* Функции управления памятью */
void allocate_matrices(int n);
void allocate_vector(int n);
void free_matrices();
void free_vector();

/* Основные вычислительные функции (шаги алгоритма) */
void compute_A_bar();      // Шаг 1: строим A_bar = A с заменой столбца i на x
void compute_l();          // Шаг 2: вычисляем l = A_inv * x
int is_invertible();       // Шаг 3: проверяем, обратима ли A_bar (l_i != 0)
void compute_l_tilde();    // Шаг 4: формируем вектор l_tilde (l с заменой i-го элемента на -1)
void compute_l_hat();      // Шаг 5: вычисляем l_hat = (-1 / l_i) * l_tilde
void compute_Q();          // Шаг 6: строим матрицу Q (единичная, но i-й столбец заменён на l_hat)
void compute_A_bar_inv();  // Шаг 7: A_bar_inv = Q * A_inv
void output_json(const char *filename); // вывод результатов в JSON
%}

%union {
    double num;
}

/* Терминалы (токены) */
%token TOKEN_SIZE TOKEN_A TOKEN_A_INV TOKEN_X TOKEN_I
%token <num> NUMBER
%token ',' ':' '[' ']' '{' '}'

%%

/* Начальное правило: весь входной файл – JSON-объект с полями size, A, A_inv, x, i */
input: '{' size_field ',' A_field ',' A_inv_field ',' x_field ',' i_field '}' {
    /* После успешного разбора все данные уже считаны в глобальные переменные */
};

/* Поле "size": сохраняем размер матриц и выделяем память */
size_field: TOKEN_SIZE ':' NUMBER {
    n = (int)$3;
    if (n <= 0) {
        yyerror("Invalid size");
        YYERROR;
    }
    allocate_matrices(n);
    allocate_vector(n);
};

/* Поле "A": разбор матрицы A */
A_field: TOKEN_A ':' {
    parsing_matrix_flag = 1;
    current_parsing_matrix = 0;  // 0 означает A
    current_row = 0;
    current_col = 0;
} matrix {
    parsing_matrix_flag = 0;
};

/* Поле "A_inv": разбор матрицы A_inv */
A_inv_field: TOKEN_A_INV ':' {
    parsing_matrix_flag = 1;
    current_parsing_matrix = 1;  // 1 означает A_inv
    current_row = 0;
    current_col = 0;
} matrix {
    parsing_matrix_flag = 0;
};

/* Поле "x": разбор вектора x */
x_field: TOKEN_X ':' {
    parsing_vector_flag = 1;
    current_vec_index = 0;
} vector {
    parsing_vector_flag = 0;
};

/* Поле "i": номер столбца (целое число, 1-индексация) */
i_field: TOKEN_I ':' NUMBER {
    i = (int)$3;
    if (i < 1 || i > n) {
        yyerror("Invalid column index");
        YYERROR;
    }
};

/* Правила для разбора матрицы (список строк, каждая строка – список чисел) */
matrix: '[' row_list ']';

row_list: row | row_list ',' row;

row: '[' number_list ']' {
    current_row++;          /* переходим к следующей строке */
    current_col = 0;
};

/* Универсальное правило для списка чисел (используется и для строк матриц, и для векторов) */
number_list: NUMBER {
    if (parsing_matrix_flag) {
        /* Заполняем текущий элемент матрицы A или A_inv */
        if (current_parsing_matrix == 0)
            A[current_row][current_col] = $1;
        else
            A_inv[current_row][current_col] = $1;
        current_col++;
    } else if (parsing_vector_flag) {
        /* Заполняем вектор x */
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

/* Вектор – просто список чисел в квадратных скобках */
vector: '[' number_list ']';

%%

void yyerror(const char *s) {
    fprintf(stderr, "Parse error: %s\n", s);
    error_flag = 1;
}

/* ----- Управление памятью ----- */
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

/* ----- ШАГ 1: построение A_bar ---- */
void compute_A_bar() {
    /* A_bar – копия A, но i-й столбец (индекс i-1) заменён на вектор x */
    for (int r = 0; r < n; r++) {
        for (int c = 0; c < n; c++) {
            A_bar[r][c] = A[r][c];
        }
        A_bar[r][i-1] = x[r];
    }
}

/* ----- ШАГ 2: вычисление l = A_inv * x ----- */
void compute_l() {
    for (int r = 0; r < n; r++) {
        l[r] = 0.0;
        for (int k = 0; k < n; k++) {
            l[r] += A_inv[r][k] * x[k];
        }
    }
}

/* ----- ШАГ 3: проверка обратимости A_bar (необходимо l_i != 0) ----- */
int is_invertible() {
    /* используем эпсилон = 1e-12 для сравнения с нулём */
    return fabs(l[i-1]) > 1e-12;
}

/* ----- ШАГ 4: формирование l_tilde ----- */
void compute_l_tilde() {
    /* l_tilde = l, но i-й элемент заменяется на -1 */
    for (int j = 0; j < n; j++) {
        l_tilde[j] = l[j];
    }
    l_tilde[i-1] = -1.0;
}

/* ----- ШАГ 5: вычисление l_hat = (-1 / l_i) * l_tilde ----- */
void compute_l_hat() {
    double factor = -1.0 / l[i-1];   // -1 / l_i
    for (int j = 0; j < n; j++) {
        l_hat[j] = factor * l_tilde[j];
    }
}
/* Вектор l_hat обладает свойством:
   - Для j != i-1: l_hat[j] = -l_j / l_i
   - Для j == i-1: l_hat[i-1] = 1 / l_i
*/

/* ----- ШАГ 6: построение матрицы Q (единичная, но i-й столбец заменён на l_hat) ----- */
void compute_Q() {
    for (int r = 0; r < n; r++) {
        for (int c = 0; c < n; c++) {
            Q[r][c] = (r == c) ? 1.0 : 0.0;
        }
        Q[r][i-1] = l_hat[r];
    }
}
/* Q = I + (l_hat - e_{i}) * e_i^T, где e_i – единичный вектор с 1 в позиции i */

/* ----- ШАГ 7: вычисление A_bar_inv = Q * A_inv ----- */
void compute_A_bar_inv() {
    /* Эффективное вычисление без полного перемножения:
       Для строки r = i-1: новая строка = l_hat[i-1] * (старая строка i-1 из A_inv)
       Для строки r != i-1: новая строка = старая строка + l_hat[r] * (строка i-1 из A_inv)
       Это эквивалентно умножению Q на A_inv.
    */
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

/* ----- Вывод результатов в JSON-файл "output.json" ----- */
void output_json(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        fprintf(stderr, "Cannot open output file %s\n", filename);
        return;
    }

    fprintf(f, "{\n");
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

    fprintf(f, "  \"l\": [");
    for (int j = 0; j < n; j++) {
        fprintf(f, "%.6f", l[j]);
        if (j < n-1) fprintf(f, ", ");
    }
    fprintf(f, "],\n");

    fprintf(f, "  \"l_tilde\": [");
    for (int j = 0; j < n; j++) {
        fprintf(f, "%.6f", l_tilde[j]);
        if (j < n-1) fprintf(f, ", ");
    }
    fprintf(f, "],\n");

    fprintf(f, "  \"l_hat\": [");
    for (int j = 0; j < n; j++) {
        fprintf(f, "%.6f", l_hat[j]);
        if (j < n-1) fprintf(f, ", ");
    }
    fprintf(f, "],\n");

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

/* ----- Главная функция ----- */
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

    /* Последовательное выполнение шагов алгоритма */
    compute_A_bar();        // 1. строим A_bar
    compute_l();            // 2. вычисляем l = A_inv * x

    if (!is_invertible()) { // 3. проверяем, не нулевой ли l_i
        fprintf(stderr, "Matrix A_bar is singular (l_i = 0).\n");
        return 1;
    }

    compute_l_tilde();      // 4. строим l_tilde
    compute_l_hat();        // 5. вычисляем l_hat
    compute_Q();            // 6. строим Q (для вывода)
    compute_A_bar_inv();    // 7. получаем A_bar_inv = Q * A_inv

    output_json("output.json");

    /* Освобождение памяти */
    free_matrices();
    free_vector();

    return 0;
}