%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

extern FILE *yyin;

/* Исходные данные */
int n, m;
char direction[4];
double *c;
double **A;
double *b;
char **r;      /* массив строк длины m */
char **sigma;  /* массив строк длины n */

/* Флаги парсинга */
int parsing_matrix_A = 0;
int parsing_vector_c = 0;
int parsing_vector_b = 0;
int parsing_array_r = 0;
int parsing_array_sigma = 0;
int current_row, current_col;
int current_index;

int error_flag = 0;

/* Результаты преобразований */
int normal_n, normal_m;
double *normal_c;
double **normal_A;
double *normal_b;

int canon_n, canon_m;
double *canon_c;
double **canon_A;
double *canon_b;

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

%union {
    double num;
    char *string;
}

%token TOKEN_N TOKEN_M TOKEN_DIRECTION TOKEN_C TOKEN_A TOKEN_B TOKEN_R TOKEN_SIGMA
%token <num> NUMBER
%token <string> STRING
%token ',' ':' '[' ']' '{' '}'

%%

input: '{' fields '}' ;

fields: field | fields ',' field;

field: n_field | m_field | direction_field | c_field | A_field | b_field | r_field | sigma_field;

n_field: TOKEN_N ':' NUMBER { n = (int)$3; };

m_field: TOKEN_M ':' NUMBER { m = (int)$3; allocate_original(); };

direction_field: TOKEN_DIRECTION ':' STRING { strcpy(direction, $3); free($3); };

c_field: TOKEN_C ':' { parsing_vector_c = 1; current_index = 0; } vector_c { parsing_vector_c = 0; };

vector_c: '[' number_list_c ']';

number_list_c: NUMBER {
    if (parsing_vector_c) c[current_index++] = $1;
} | number_list_c ',' NUMBER {
    if (parsing_vector_c) c[current_index++] = $3;
};

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

b_field: TOKEN_B ':' { parsing_vector_b = 1; current_index = 0; } vector_b { parsing_vector_b = 0; };

vector_b: '[' number_list_b ']';

number_list_b: NUMBER {
    if (parsing_vector_b) b[current_index++] = $1;
} | number_list_b ',' NUMBER {
    if (parsing_vector_b) b[current_index++] = $3;
};

r_field: TOKEN_R ':' { parsing_array_r = 1; current_index = 0; } array_r { parsing_array_r = 0; };

array_r: '[' string_list_r ']';

string_list_r: STRING {
    if (parsing_array_r) r[current_index++] = $1;
} | string_list_r ',' STRING {
    if (parsing_array_r) r[current_index++] = $3;
};

sigma_field: TOKEN_SIGMA ':' { parsing_array_sigma = 1; current_index = 0; } array_sigma { parsing_array_sigma = 0; };

array_sigma: '[' string_list_sigma ']';

string_list_sigma: STRING {
    if (parsing_array_sigma) sigma[current_index++] = $1;
} | string_list_sigma ',' STRING {
    if (parsing_array_sigma) sigma[current_index++] = $3;
};

%%

void yyerror(const char *s) {
    fprintf(stderr, "Parse error: %s\n", s);
    error_flag = 1;
}

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

/* Приведение к нормальной форме */
void to_normal_form() {
    double coeff_mult = (strcmp(direction, "min") == 0) ? -1.0 : 1.0;

    /* ---- Приведение ограничений к виду "≤" ---- */
    int temp_m = m;
    for (int i = 0; i < m; i++)
        if (strcmp(r[i], "=") == 0) temp_m++;

    double **A_temp = (double**)malloc(temp_m * sizeof(double*));
    double *b_temp = (double*)malloc(temp_m * sizeof(double));
    int row_idx = 0;
    for (int i = 0; i < m; i++) {
        if (strcmp(r[i], "<=") == 0) {
            A_temp[row_idx] = (double*)malloc(n * sizeof(double));
            for (int j = 0; j < n; j++) A_temp[row_idx][j] = A[i][j];
            b_temp[row_idx] = b[i];
            row_idx++;
        } else if (strcmp(r[i], ">=") == 0) {
            A_temp[row_idx] = (double*)malloc(n * sizeof(double));
            for (int j = 0; j < n; j++) A_temp[row_idx][j] = -A[i][j];
            b_temp[row_idx] = -b[i];
            row_idx++;
        } else if (strcmp(r[i], "=") == 0) {
            A_temp[row_idx] = (double*)malloc(n * sizeof(double));
            for (int j = 0; j < n; j++) A_temp[row_idx][j] = A[i][j];
            b_temp[row_idx] = b[i];
            row_idx++;
            A_temp[row_idx] = (double*)malloc(n * sizeof(double));
            for (int j = 0; j < n; j++) A_temp[row_idx][j] = -A[i][j];
            b_temp[row_idx] = -b[i];
            row_idx++;
        }
    }

    /* ---- Обработка переменных ---- */
    int new_n = 0;
    for (int j = 0; j < n; j++) {
        if (strcmp(sigma[j], ">=0") == 0) new_n++;
        else if (strcmp(sigma[j], "<=0") == 0) new_n++;
        else if (strcmp(sigma[j], "free") == 0) new_n += 2;
    }

    normal_n = new_n;
    normal_m = temp_m;
    normal_c = (double*)malloc(normal_n * sizeof(double));
    normal_A = (double**)malloc(normal_m * sizeof(double*));
    for (int i = 0; i < normal_m; i++)
        normal_A[i] = (double*)malloc(normal_n * sizeof(double));
    normal_b = (double*)malloc(normal_m * sizeof(double));
    for (int i = 0; i < normal_m; i++) normal_b[i] = b_temp[i];

    int new_col = 0;
    for (int j = 0; j < n; j++) {
        if (strcmp(sigma[j], ">=0") == 0) {
            for (int i = 0; i < normal_m; i++)
                normal_A[i][new_col] = A_temp[i][j];
            normal_c[new_col] = coeff_mult * c[j];
            new_col++;
        } else if (strcmp(sigma[j], "<=0") == 0) {
            for (int i = 0; i < normal_m; i++)
                normal_A[i][new_col] = -A_temp[i][j];
            normal_c[new_col] = coeff_mult * (-c[j]);
            new_col++;
        } else if (strcmp(sigma[j], "free") == 0) {
            for (int i = 0; i < normal_m; i++)
                normal_A[i][new_col] = A_temp[i][j];
            normal_c[new_col] = coeff_mult * c[j];
            new_col++;
            for (int i = 0; i < normal_m; i++)
                normal_A[i][new_col] = -A_temp[i][j];
            normal_c[new_col] = coeff_mult * (-c[j]);
            new_col++;
        }
    }

    /* Освобождение временных данных */
    for (int i = 0; i < temp_m; i++) free(A_temp[i]);
    free(A_temp);
    free(b_temp);
}

/* Приведение к канонической форме */
void to_canonical_form() {
    double coeff_mult = (strcmp(direction, "min") == 0) ? -1.0 : 1.0;

    /* ---- Добавление slack/surplus переменных для неравенств ---- */
    int num_slack = 0;
    for (int i = 0; i < m; i++)
        if (strcmp(r[i], "<=") == 0 || strcmp(r[i], ">=") == 0) num_slack++;

    int temp_n = n + num_slack;
    double **A_temp = (double**)malloc(m * sizeof(double*));
    double *b_temp = (double*)malloc(m * sizeof(double));
    for (int i = 0; i < m; i++) {
        A_temp[i] = (double*)malloc(temp_n * sizeof(double));
        for (int j = 0; j < n; j++) A_temp[i][j] = A[i][j];
    }
    int slack_idx = n;
    for (int i = 0; i < m; i++) {
        if (strcmp(r[i], "<=") == 0) {
            for (int k = 0; k < m; k++)
                A_temp[k][slack_idx] = (k == i) ? 1.0 : 0.0;
            slack_idx++;
        } else if (strcmp(r[i], ">=") == 0) {
            for (int k = 0; k < m; k++)
                A_temp[k][slack_idx] = (k == i) ? -1.0 : 0.0;
            slack_idx++;
        }
    }
    for (int i = 0; i < m; i++) b_temp[i] = b[i];

    /* ---- Обработка переменных (исходные + slack) ---- */
    int new_n = 0;
    for (int j = 0; j < n; j++) {
        if (strcmp(sigma[j], ">=0") == 0) new_n++;
        else if (strcmp(sigma[j], "<=0") == 0) new_n++;
        else if (strcmp(sigma[j], "free") == 0) new_n += 2;
    }
    new_n += num_slack;

    canon_n = new_n;
    canon_m = m;
    canon_c = (double*)malloc(canon_n * sizeof(double));
    canon_A = (double**)malloc(canon_m * sizeof(double*));
    for (int i = 0; i < canon_m; i++)
        canon_A[i] = (double*)malloc(canon_n * sizeof(double));
    canon_b = (double*)malloc(canon_m * sizeof(double));
    for (int i = 0; i < canon_m; i++) canon_b[i] = b_temp[i];

    int new_col = 0;
    /* Исходные переменные */
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
    /* Slack переменные */
    for (int s = 0; s < num_slack; s++) {
        int col = n + s;
        for (int i = 0; i < canon_m; i++)
            canon_A[i][new_col] = A_temp[i][col];
        canon_c[new_col] = 0.0;
        new_col++;
    }

    /* Освобождение временных данных */
    for (int i = 0; i < m; i++) free(A_temp[i]);
    free(A_temp);
    free(b_temp);
}

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
    to_normal_form();
    to_canonical_form();
    output_json("output.json");

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