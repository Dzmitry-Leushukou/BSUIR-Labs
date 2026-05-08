%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

extern FILE *yyin;

/* === Исходные данные задачи линейного программирования === */
int n, m;                   // n – число переменных, m – число ограничений
char direction[4];          // "min" или "max" – направление оптимизации
double *c;                  // коэффициенты целевой функции (размер n)
double **A;                 // матрица ограничений (размер m x n)
double *b;                  // правые части ограничений (размер m)
char **r;                   // массив строк длины m: тип каждого ограничения ("<=", ">=", "=")
char **sigma;               // массив строк длины n: знак каждой переменной (">=0", "<=0", "free")

/* === Флаги для парсера (чтобы понимать, что сейчас разбираем) === */
int parsing_matrix_A = 0;   // 1 – разбираем матрицу A
int parsing_vector_c = 0;   // 1 – разбираем вектор c
int parsing_vector_b = 0;   // 1 – разбираем вектор b
int parsing_array_r = 0;    // 1 – разбираем массив r (типы ограничений)
int parsing_array_sigma = 0;// 1 – разбираем массив sigma (знаки переменных)
int current_row, current_col; // текущие индексы при заполнении матрицы
int current_index;          // текущий индекс при заполнении вектора или массива строк

int error_flag = 0;         // флаг ошибки разбора

/* === Результаты преобразований === */
int normal_n, normal_m;          // размерность нормальной формы
double *normal_c;                // целевая функция нормальной формы
double **normal_A;               // матрица ограничений нормальной формы
double *normal_b;                // правые части нормальной формы

int canon_n, canon_m;            // размерность канонической формы
double *canon_c;                 // целевая функция канонической формы
double **canon_A;                // матрица ограничений канонической формы
double *canon_b;                 // правые части канонической формы

/* === Прототипы функций === */
void yyerror(const char *s);
int yylex(void);
void allocate_original();
void free_original();
void to_normal_form();
void to_canonical_form();
void output_json(const char *filename);
void write_matrix(FILE *f, double **mat, int rows, int cols);
void write_vector(FILE *f, double *vec, int len);
%}

/* === Терминалы (токены) === */
%token TOKEN_N TOKEN_M TOKEN_DIRECTION TOKEN_C TOKEN_A TOKEN_B TOKEN_R TOKEN_SIGMA
%token <num> NUMBER          // числа
%token <string> STRING        // строки (для direction, r, sigma)
%token ',' ':' '[' ']' '{' '}'

%%

/* === Грамматика входного JSON === */
input: '{' fields '}' ;      // весь файл – объект

fields: field | fields ',' field;   // последовательность полей

/* Возможные поля */
field: n_field | m_field | direction_field | c_field | A_field | b_field | r_field | sigma_field;

/* Поле "n": число переменных */
n_field: TOKEN_N ':' NUMBER { n = (int)$3; };

/* Поле "m": число ограничений – после него сразу выделяем память, так как размеры уже известны */
m_field: TOKEN_M ':' NUMBER { m = (int)$3; allocate_original(); };

/* Поле "direction": направление оптимизации ("min" или "max") */
direction_field: TOKEN_DIRECTION ':' STRING { strcpy(direction, $3); free($3); };

/* Поле "c": вектор коэффициентов целевой функции */
c_field: TOKEN_C ':' { parsing_vector_c = 1; current_index = 0; } vector_c { parsing_vector_c = 0; };

vector_c: '[' number_list_c ']';

number_list_c: NUMBER {
    if (parsing_vector_c) c[current_index++] = $1;
} | number_list_c ',' NUMBER {
    if (parsing_vector_c) c[current_index++] = $3;
};

/* Поле "A": матрица ограничений (m строк, n столбцов) */
A_field: TOKEN_A ':' {
    parsing_matrix_A = 1;
    current_row = 0;
    current_col = 0;
} matrix_A {
    parsing_matrix_A = 0;
};

matrix_A: '[' row_list_A ']';

row_list_A: row_A | row_list_A ',' row_A;

row_A: '[' number_list_A ']' { current_row++; current_col = 0; };

number_list_A: NUMBER {
    if (parsing_matrix_A) A[current_row][current_col++] = $1;
} | number_list_A ',' NUMBER {
    if (parsing_matrix_A) A[current_row][current_col++] = $3;
};

/* Поле "b": вектор правых частей ограничений */
b_field: TOKEN_B ':' { parsing_vector_b = 1; current_index = 0; } vector_b { parsing_vector_b = 0; };

vector_b: '[' number_list_b ']';

number_list_b: NUMBER {
    if (parsing_vector_b) b[current_index++] = $1;
} | number_list_b ',' NUMBER {
    if (parsing_vector_b) b[current_index++] = $3;
};

/* Поле "r": массив строк – типы ограничений ("<=", ">=", "=") */
r_field: TOKEN_R ':' { parsing_array_r = 1; current_index = 0; } array_r { parsing_array_r = 0; };

array_r: '[' string_list_r ']';

string_list_r: STRING {
    if (parsing_array_r) r[current_index++] = $1;
} | string_list_r ',' STRING {
    if (parsing_array_r) r[current_index++] = $3;
};

/* Поле "sigma": массив строк – знаки переменных (">=0", "<=0", "free") */
sigma_field: TOKEN_SIGMA ':' { parsing_array_sigma = 1; current_index = 0; } array_sigma { parsing_array_sigma = 0; };

array_sigma: '[' string_list_sigma ']';

string_list_sigma: STRING {
    if (parsing_array_sigma) sigma[current_index++] = $1;
} | string_list_sigma ',' STRING {
    if (parsing_array_sigma) sigma[current_index++] = $3;
};

%%

/* === Обработка ошибок парсера === */
void yyerror(const char *s) {
    fprintf(stderr, "Parse error: %s\n", s);
    error_flag = 1;
}

/* === Выделение памяти под исходные данные === */
void allocate_original() {
    c = (double*)malloc(n * sizeof(double));
    b = (double*)malloc(m * sizeof(double));
    A = (double**)malloc(m * sizeof(double*));
    for (int i = 0; i < m; i++)
        A[i] = (double*)malloc(n * sizeof(double));
    r = (char**)malloc(m * sizeof(char*));
    sigma = (char**)malloc(n * sizeof(char*));
    for (int i = 0; i < m; i++) r[i] = NULL;
    for (int i = 0; i < n; i++) sigma[i] = NULL;
}

/* === Освобождение памяти исходных данных === */
void free_original() {
    free(c);
    free(b);
    for (int i = 0; i < m; i++) free(A[i]);
    free(A);
    for (int i = 0; i < m; i++) if (r[i]) free(r[i]);
    free(r);
    for (int i = 0; i < n; i++) if (sigma[i]) free(sigma[i]);
    free(sigma);
}

/* ============================================================
   Функция to_normal_form()
   Приводит исходную задачу ЛП к нормальной форме:
   - Все ограничения имеют вид "≤"
   - Все переменные неотрицательны (≥0)
   - Направление оптимизации – только "max" (если был "min", то умножаем целевую функцию на -1)
   ============================================================ */
void to_normal_form() {
    /* Если исходная задача на минимум, то умножаем целевую функцию на -1,
       чтобы она стала на максимум */
    double coeff_mult = (strcmp(direction, "min") == 0) ? -1.0 : 1.0;

    /* ---------- 1. Приведение ограничений к виду "≤" ---------- */
    int temp_m = m;
    /* Для каждого равенства ("=") потребуется два неравенства: одно ≤, другое ≥,
       но ≥ потом домножается на -1, чтобы тоже стало ≤ */
    for (int i = 0; i < m; i++)
        if (strcmp(r[i], "=") == 0) temp_m++;

    double **A_temp = (double**)malloc(temp_m * sizeof(double*));
    double *b_temp = (double*)malloc(temp_m * sizeof(double));
    int row_idx = 0;

    for (int i = 0; i < m; i++) {
        if (strcmp(r[i], "<=") == 0) {
            // Ограничение ≤ – оставляем как есть
            A_temp[row_idx] = (double*)malloc(n * sizeof(double));
            for (int j = 0; j < n; j++) A_temp[row_idx][j] = A[i][j];
            b_temp[row_idx] = b[i];
            row_idx++;
        } else if (strcmp(r[i], ">=") == 0) {
            // Ограничение ≥ – умножаем на -1, чтобы стало ≤
            A_temp[row_idx] = (double*)malloc(n * sizeof(double));
            for (int j = 0; j < n; j++) A_temp[row_idx][j] = -A[i][j];
            b_temp[row_idx] = -b[i];
            row_idx++;
        } else if (strcmp(r[i], "=") == 0) {
            // Равенство = заменяем двумя неравенствами: ≤ и ≥ (≥ потом превращаем в ≤)
            // Первое: A[i][j] * x_j ≤ b[i]
            A_temp[row_idx] = (double*)malloc(n * sizeof(double));
            for (int j = 0; j < n; j++) A_temp[row_idx][j] = A[i][j];
            b_temp[row_idx] = b[i];
            row_idx++;
            // Второе: -A[i][j] * x_j ≤ -b[i]
            A_temp[row_idx] = (double*)malloc(n * sizeof(double));
            for (int j = 0; j < n; j++) A_temp[row_idx][j] = -A[i][j];
            b_temp[row_idx] = -b[i];
            row_idx++;
        }
    }

    /* ---------- 2. Обработка знаков переменных ---------- */
    int new_n = 0;
    for (int j = 0; j < n; j++) {
        if (strcmp(sigma[j], ">=0") == 0) new_n++;           // уже ≥0
        else if (strcmp(sigma[j], "<=0") == 0) new_n++;      // замена x_j = -x'_j, где x'_j ≥0
        else if (strcmp(sigma[j], "free") == 0) new_n += 2;  // свободная переменная: x_j = x'_j - x''_j, x'_j≥0, x''_j≥0
    }

    normal_n = new_n;
    normal_m = temp_m;

    /* Выделяем память под нормальную форму */
    normal_c = (double*)malloc(normal_n * sizeof(double));
    normal_A = (double**)malloc(normal_m * sizeof(double*));
    for (int i = 0; i < normal_m; i++)
        normal_A[i] = (double*)malloc(normal_n * sizeof(double));
    normal_b = (double*)malloc(normal_m * sizeof(double));
    for (int i = 0; i < normal_m; i++) normal_b[i] = b_temp[i];

    /* Заполняем матрицу normal_A и вектор normal_c в соответствии с заменой переменных */
    int new_col = 0;
    for (int j = 0; j < n; j++) {
        if (strcmp(sigma[j], ">=0") == 0) {
            // Переменная неотрицательная – оставляем как есть
            for (int i = 0; i < normal_m; i++)
                normal_A[i][new_col] = A_temp[i][j];
            normal_c[new_col] = coeff_mult * c[j];
            new_col++;
        } else if (strcmp(sigma[j], "<=0") == 0) {
            // x_j ≤ 0, делаем замену x_j = -x'_j, x'_j ≥ 0
            for (int i = 0; i < normal_m; i++)
                normal_A[i][new_col] = -A_temp[i][j];
            normal_c[new_col] = coeff_mult * (-c[j]);  // так как c_j * x_j = c_j*(-x'_j) = (-c_j)*x'_j
            new_col++;
        } else if (strcmp(sigma[j], "free") == 0) {
            // Свободная переменная: x_j = x'_j - x''_j, x'_j≥0, x''_j≥0
            // Первая компонента: x'_j
            for (int i = 0; i < normal_m; i++)
                normal_A[i][new_col] = A_temp[i][j];
            normal_c[new_col] = coeff_mult * c[j];
            new_col++;
            // Вторая компонента: x''_j (с минусом)
            for (int i = 0; i < normal_m; i++)
                normal_A[i][new_col] = -A_temp[i][j];
            normal_c[new_col] = coeff_mult * (-c[j]);
            new_col++;
        }
    }

    /* Освобождаем временные матрицы */
    for (int i = 0; i < temp_m; i++) free(A_temp[i]);
    free(A_temp);
    free(b_temp);
}

/* ============================================================
   Функция to_canonical_form()
   Приводит исходную задачу ЛП к канонической форме:
   - Все ограничения – равенства (добавляем slack/surplus переменные)
   - Все переменные неотрицательны
   - Целевая функция на максимум (или минимум – но обычно каноническая форма для симплекс-метода
     требует максимизации; здесь делаем на максимум)
   ============================================================ */
void to_canonical_form() {
    double coeff_mult = (strcmp(direction, "min") == 0) ? -1.0 : 1.0;

    /* ---------- 1. Добавление дополнительных переменных для неравенств ---------- */
    int num_slack = 0;
    for (int i = 0; i < m; i++)
        if (strcmp(r[i], "<=") == 0 || strcmp(r[i], ">=") == 0) num_slack++;

    int temp_n = n + num_slack;  // исходные переменные + slack/surplus
    double **A_temp = (double**)malloc(m * sizeof(double*));
    double *b_temp = (double*)malloc(m * sizeof(double));

    /* Копируем исходные коэффициенты A */
    for (int i = 0; i < m; i++) {
        A_temp[i] = (double*)malloc(temp_n * sizeof(double));
        for (int j = 0; j < n; j++) A_temp[i][j] = A[i][j];
    }

    /* Добавляем столбцы для slack/surplus переменных */
    int slack_idx = n;
    for (int i = 0; i < m; i++) {
        if (strcmp(r[i], "<=") == 0) {
            // Неравенство ≤: добавляем slack-переменную s_i ≥ 0 с коэффициентом +1 в i-й строке
            for (int k = 0; k < m; k++)
                A_temp[k][slack_idx] = (k == i) ? 1.0 : 0.0;
            slack_idx++;
        } else if (strcmp(r[i], ">=") == 0) {
            // Неравенство ≥: добавляем surplus-переменную s_i ≥ 0 с коэффициентом -1 в i-й строке
            for (int k = 0; k < m; k++)
                A_temp[k][slack_idx] = (k == i) ? -1.0 : 0.0;
            slack_idx++;
        }
        // Для равенств ничего не добавляем
    }

    /* Копируем правые части */
    for (int i = 0; i < m; i++) b_temp[i] = b[i];

    /* ---------- 2. Обработка знаков переменных (как в normal_form, но теперь и для slack) ---------- */
    int new_n = 0;
    for (int j = 0; j < n; j++) {
        if (strcmp(sigma[j], ">=0") == 0) new_n++;
        else if (strcmp(sigma[j], "<=0") == 0) new_n++;
        else if (strcmp(sigma[j], "free") == 0) new_n += 2;
    }
    new_n += num_slack;   // slack-переменные всегда неотрицательны

    canon_n = new_n;
    canon_m = m;          // количество ограничений не меняется (стали все равенства)

    canon_c = (double*)malloc(canon_n * sizeof(double));
    canon_A = (double**)malloc(canon_m * sizeof(double*));
    for (int i = 0; i < canon_m; i++)
        canon_A[i] = (double*)malloc(canon_n * sizeof(double));
    canon_b = (double*)malloc(canon_m * sizeof(double));
    for (int i = 0; i < canon_m; i++) canon_b[i] = b_temp[i];

    /* Заполняем каноническую форму */
    int new_col = 0;

    /* Сначала исходные переменные с учётом их знаков */
    for (int j = 0; j < n; j++) {
        if (strcmp(sigma[j], ">=0") == 0) {
            for (int i = 0; i < canon_m; i++)
                canon_A[i][new_col] = A_temp[i][j];
            canon_c[new_col] = coeff_mult * c[j];
            new_col++;
        } else if (strcmp(sigma[j], "<=0") == 0) {
            for (int i = 0; i < canon_m; i++)
                canon_A[i][new_col] = -A_temp[i][j];
            canon_c[new_col] = coeff_mult * (-c[j]);
            new_col++;
        } else if (strcmp(sigma[j], "free") == 0) {
            for (int i = 0; i < canon_m; i++)
                canon_A[i][new_col] = A_temp[i][j];
            canon_c[new_col] = coeff_mult * c[j];
            new_col++;
            for (int i = 0; i < canon_m; i++)
                canon_A[i][new_col] = -A_temp[i][j];
            canon_c[new_col] = coeff_mult * (-c[j]);
            new_col++;
        }
    }

    /* Добавляем slack/surplus переменные – они входят только в ограничения, в целевую функцию с 0 */
    for (int s = 0; s < num_slack; s++) {
        int col = n + s;   // столбец во временной матрице
        for (int i = 0; i < canon_m; i++)
            canon_A[i][new_col] = A_temp[i][col];
        canon_c[new_col] = 0.0;
        new_col++;
    }

    /* Освобождаем временные данные */
    for (int i = 0; i < m; i++) free(A_temp[i]);
    free(A_temp);
    free(b_temp);
}

/* === Вспомогательные функции для вывода JSON === */
void write_matrix(FILE *f, double **mat, int rows, int cols) {
    fprintf(f, "[");
    for (int i = 0; i < rows; i++) {
        fprintf(f, "[");
        for (int j = 0; j < cols; j++) {
            fprintf(f, "%.6f", mat[i][j]);
            if (j < cols-1) fprintf(f, ", ");
        }
        fprintf(f, "]");
        if (i < rows-1) fprintf(f, ", ");
    }
    fprintf(f, "]");
}

void write_vector(FILE *f, double *vec, int len) {
    fprintf(f, "[");
    for (int i = 0; i < len; i++) {
        fprintf(f, "%.6f", vec[i]);
        if (i < len-1) fprintf(f, ", ");
    }
    fprintf(f, "]");
}

/* === Вывод результатов в JSON-файл "output.json" === */
void output_json(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        fprintf(stderr, "Cannot open output file %s\n", filename);
        return;
    }
    fprintf(f, "{\n");
    fprintf(f, "  \"normal_form\": {\n");
    fprintf(f, "    \"direction\": \"max\",\n");
    fprintf(f, "    \"c\": ");
    write_vector(f, normal_c, normal_n);
    fprintf(f, ",\n");
    fprintf(f, "    \"A\": ");
    write_matrix(f, normal_A, normal_m, normal_n);
    fprintf(f, ",\n");
    fprintf(f, "    \"b\": ");
    write_vector(f, normal_b, normal_m);
    fprintf(f, "\n");
    fprintf(f, "  },\n");
    fprintf(f, "  \"canonical_form\": {\n");
    fprintf(f, "    \"direction\": \"max\",\n");
    fprintf(f, "    \"c\": ");
    write_vector(f, canon_c, canon_n);
    fprintf(f, ",\n");
    fprintf(f, "    \"A\": ");
    write_matrix(f, canon_A, canon_m, canon_n);
    fprintf(f, ",\n");
    fprintf(f, "    \"b\": ");
    write_vector(f, canon_b, canon_m);
    fprintf(f, "\n");
    fprintf(f, "  }\n");
    fprintf(f, "}\n");
    fclose(f);
}

/* === Главная функция === */
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
    yyparse();               // запускаем синтаксический анализатор
    fclose(file);

    if (error_flag) {
        fprintf(stderr, "Parsing failed.\n");
        return 1;
    }

    /* Преобразование в нормальную и каноническую формы */
    to_normal_form();
    to_canonical_form();

    /* Вывод результата */
    output_json("output.json");

    /* Освобождение всей выделенной памяти */
    free_original();
    free(normal_c);
    for (int i = 0; i < normal_m; i++) free(normal_A[i]);
    free(normal_A);
    free(normal_b);
    free(canon_c);
    for (int i = 0; i < canon_m; i++) free(canon_A[i]);
    free(canon_A);
    free(canon_b);

    return 0;
}