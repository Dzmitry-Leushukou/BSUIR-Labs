%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

extern FILE *yyin;

int m = 0, n = 0;
double *a = NULL;
double *b = NULL;
double **c = NULL;
double **x = NULL;
int basis_size;
int *basis_rows = NULL;
int *basis_cols = NULL;

int parsing_matrix = 0;
int parsing_vector = 0;
int vector_target = 0;
int current_row, current_col;
int current_index;
int error_flag = 0;
int data_allocated = 0;
int m_set = 0, n_set = 0;
int has_a = 0, has_b = 0, has_c = 0;

double optimal_value;

void yyerror(const char *s);
int yylex(void);
void semantic_error(const char *message);
void try_allocate_data(void);

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
    data_allocated = 1;
}

void free_data() {
    if (!data_allocated) return;
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
    a = NULL;
    b = NULL;
    c = NULL;
    x = NULL;
    basis_rows = NULL;
    basis_cols = NULL;
    data_allocated = 0;
}

void semantic_error(const char *message) {
    fprintf(stderr, "Error: %s\n", message);
    error_flag = 1;
}

void try_allocate_data(void) {
    if (data_allocated || !m_set || !n_set) return;
    if (m <= 0 || n <= 0) {
        semantic_error("Fields \"m\" and \"n\" must be positive integers.");
        return;
    }
    allocate_data();
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

typedef struct {
    int from;
    int to;
    int rev;
    int next;
    double cap;
    double cost;
} Edge;

int add_residual_edge(Edge *edges, int *head, int *edge_count, int from, int to, double cap, double cost) {
    int fwd = *edge_count;
    int rev = fwd + 1;

    edges[fwd].from = from;
    edges[fwd].to = to;
    edges[fwd].cap = cap;
    edges[fwd].cost = cost;
    edges[fwd].rev = rev;
    edges[fwd].next = head[from];
    head[from] = fwd;

    edges[rev].from = to;
    edges[rev].to = from;
    edges[rev].cap = 0.0;
    edges[rev].cost = -cost;
    edges[rev].rev = fwd;
    edges[rev].next = head[to];
    head[to] = rev;

    *edge_count += 2;
    return fwd;
}

int has_basis_position(int row, int col, int count) {
    for (int k = 0; k < count; k++) {
        if (basis_rows[k] == row && basis_cols[k] == col) return 1;
    }
    return 0;
}

void initialize_potentials(Edge *edges, int edge_count, int node_count, int source, double *potential) {
    const double INF = 1e100;
    const double EPS = 1e-9;
    double *dist = (double*)malloc(node_count * sizeof(double));
    for (int i = 0; i < node_count; i++) {
        dist[i] = INF;
        potential[i] = 0.0;
    }
    dist[source] = 0.0;

    for (int iter = 0; iter < node_count - 1; iter++) {
        int changed = 0;
        for (int e = 0; e < edge_count; e++) {
            if (edges[e].cap <= EPS) continue;
            int u = edges[e].from;
            int v = edges[e].to;
            if (dist[u] >= INF / 2) continue;
            double nd = dist[u] + edges[e].cost;
            if (nd + EPS < dist[v]) {
                dist[v] = nd;
                changed = 1;
            }
        }
        if (!changed) break;
    }

    for (int i = 0; i < node_count; i++) {
        if (dist[i] < INF / 2) potential[i] = dist[i];
    }
    free(dist);
}

int shortest_path_dijkstra(Edge *edges, int *head, int node_count, int source, int sink,
                           double *potential, int *prev_edge) {
    const double INF = 1e100;
    const double EPS = 1e-9;
    double *dist = (double*)malloc(node_count * sizeof(double));
    int *used = (int*)calloc(node_count, sizeof(int));

    for (int i = 0; i < node_count; i++) {
        dist[i] = INF;
        prev_edge[i] = -1;
    }
    dist[source] = 0.0;

    for (int iter = 0; iter < node_count; iter++) {
        int u = -1;
        double best = INF;
        for (int i = 0; i < node_count; i++) {
            if (!used[i] && dist[i] < best) {
                best = dist[i];
                u = i;
            }
        }
        if (u == -1) break;
        used[u] = 1;

        for (int e = head[u]; e != -1; e = edges[e].next) {
            if (edges[e].cap <= EPS) continue;
            int v = edges[e].to;
            double reduced = edges[e].cost + potential[u] - potential[v];
            if (reduced < 0.0 && reduced > -1e-9) reduced = 0.0;
            double nd = dist[u] + reduced;
            if (nd + EPS < dist[v]) {
                dist[v] = nd;
                prev_edge[v] = e;
            }
        }
    }

    int found = (prev_edge[sink] != -1);
    if (found) {
        for (int i = 0; i < node_count; i++) {
            if (dist[i] < INF / 2) potential[i] += dist[i];
        }
    }
    free(used);
    free(dist);
    return found;
}

void transport_simplex() {
    const double EPS = 1e-9;
    int source = 0;
    int supply_start = 1;
    int demand_start = supply_start + m;
    int sink = demand_start + n;
    int node_count = sink + 1;

    int max_edges = 2 * (m + n + m * n) + 10;
    Edge *edges = (Edge*)malloc(max_edges * sizeof(Edge));
    int *head = (int*)malloc(node_count * sizeof(int));
    int edge_count = 0;
    int **transport_edge = (int**)malloc(m * sizeof(int*));
    double total_supply = 0.0;

    for (int i = 0; i < node_count; i++) head[i] = -1;
    for (int i = 0; i < m; i++) {
        transport_edge[i] = (int*)malloc(n * sizeof(int));
        for (int j = 0; j < n; j++) transport_edge[i][j] = -1;
        total_supply += a[i];
        for (int j = 0; j < n; j++) x[i][j] = 0.0;
    }

    for (int i = 0; i < m; i++) {
        add_residual_edge(edges, head, &edge_count, source, supply_start + i, a[i], 0.0);
    }
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            int idx = add_residual_edge(edges, head, &edge_count,
                                        supply_start + i, demand_start + j,
                                        total_supply, c[i][j]);
            transport_edge[i][j] = idx;
        }
    }
    for (int j = 0; j < n; j++) {
        add_residual_edge(edges, head, &edge_count, demand_start + j, sink, b[j], 0.0);
    }

    double flow = 0.0;
    optimal_value = 0.0;
    int *prev_edge = (int*)malloc(node_count * sizeof(int));
    double *potential = (double*)malloc(node_count * sizeof(double));

    initialize_potentials(edges, edge_count, node_count, source, potential);

    while (flow + EPS < total_supply) {
        if (!shortest_path_dijkstra(edges, head, node_count, source, sink, potential, prev_edge)) {
            semantic_error("Failed to find augmenting path while solving transportation problem.");
            break;
        }
        double add_flow = total_supply - flow;
        int v = sink;
        while (v != source) {
            int e = prev_edge[v];
            if (e < 0) {
                add_flow = 0.0;
                break;
            }
            if (edges[e].cap < add_flow) add_flow = edges[e].cap;
            v = edges[e].from;
        }
        if (add_flow <= EPS) {
            semantic_error("Internal error: non-positive augmentation value.");
            break;
        }
        v = sink;
        while (v != source) {
            int e = prev_edge[v];
            edges[e].cap -= add_flow;
            edges[edges[e].rev].cap += add_flow;
            optimal_value += add_flow * edges[e].cost;
            v = edges[e].from;
        }
        flow += add_flow;
    }

    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            int e = transport_edge[i][j];
            x[i][j] = edges[edges[e].rev].cap;
            if (fabs(x[i][j]) < EPS) x[i][j] = 0.0;
        }
    }

    int cur = 0;
    for (int i = 0; i < m && cur < basis_size; i++) {
        for (int j = 0; j < n && cur < basis_size; j++) {
            if (x[i][j] > EPS) {
                basis_rows[cur] = i + 1;
                basis_cols[cur] = j + 1;
                cur++;
            }
        }
    }
    for (int i = 0; i < m && cur < basis_size; i++) {
        for (int j = 0; j < n && cur < basis_size; j++) {
            if (!has_basis_position(i + 1, j + 1, cur)) {
                basis_rows[cur] = i + 1;
                basis_cols[cur] = j + 1;
                cur++;
            }
        }
    }

    for (int i = 0; i < m; i++) free(transport_edge[i]);
    free(transport_edge);
    free(potential);
    free(prev_edge);
    free(head);
    free(edges);
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
m_field: TOKEN_M ':' NUMBER {
        if (m_set) semantic_error("Duplicate field \"m\".");
        if (fabs($3 - round($3)) > 1e-9) semantic_error("Field \"m\" must be an integer.");
        m = (int)llround($3);
        m_set = 1;
        try_allocate_data();
    };
n_field: TOKEN_N ':' NUMBER {
        if (n_set) semantic_error("Duplicate field \"n\".");
        if (fabs($3 - round($3)) > 1e-9) semantic_error("Field \"n\" must be an integer.");
        n = (int)llround($3);
        n_set = 1;
        try_allocate_data();
    };
a_field: TOKEN_A ':' {
        parsing_vector = 1;
        vector_target = 1;
        current_index = 0;
        if (!data_allocated) semantic_error("Fields \"m\" and \"n\" must be defined before \"a\".");
    } vector_a {
        parsing_vector = 0;
        vector_target = 0;
        if (data_allocated && current_index != m)
            semantic_error("Vector \"a\" must contain exactly m elements.");
        has_a = 1;
    };
vector_a: '[' number_list_a ']';
number_list_a: NUMBER {
        if (parsing_vector && data_allocated && vector_target == 1) {
            if (current_index >= m) semantic_error("Vector \"a\" has too many elements.");
            else a[current_index++] = $1;
        }
    }
    | number_list_a ',' NUMBER {
        if (parsing_vector && data_allocated && vector_target == 1) {
            if (current_index >= m) semantic_error("Vector \"a\" has too many elements.");
            else a[current_index++] = $3;
        }
    };
b_field: TOKEN_B ':' {
        parsing_vector = 1;
        vector_target = 2;
        current_index = 0;
        if (!data_allocated) semantic_error("Fields \"m\" and \"n\" must be defined before \"b\".");
    } vector_b {
        parsing_vector = 0;
        vector_target = 0;
        if (data_allocated && current_index != n)
            semantic_error("Vector \"b\" must contain exactly n elements.");
        has_b = 1;
    };
vector_b: '[' number_list_b ']';
number_list_b: NUMBER {
        if (parsing_vector && data_allocated && vector_target == 2) {
            if (current_index >= n) semantic_error("Vector \"b\" has too many elements.");
            else b[current_index++] = $1;
        }
    }
    | number_list_b ',' NUMBER {
        if (parsing_vector && data_allocated && vector_target == 2) {
            if (current_index >= n) semantic_error("Vector \"b\" has too many elements.");
            else b[current_index++] = $3;
        }
    };
c_field: TOKEN_C ':' {
        parsing_matrix = 1;
        current_row = 0;
        current_col = 0;
        if (!data_allocated) semantic_error("Fields \"m\" and \"n\" must be defined before \"c\".");
    } matrix_c {
        parsing_matrix = 0;
        if (data_allocated && current_row != m)
            semantic_error("Matrix \"c\" must contain exactly m rows.");
        has_c = 1;
    };
matrix_c: '[' row_list_c ']';
row_list_c: row_c | row_list_c ',' row_c;
row_c: '[' number_list_c ']' {
        if (parsing_matrix && data_allocated) {
            if (current_col != n) semantic_error("Each row in matrix \"c\" must contain exactly n elements.");
            if (current_row >= m) semantic_error("Matrix \"c\" has too many rows.");
            else current_row++;
        }
        current_col = 0;
    };
number_list_c: NUMBER {
        if (parsing_matrix && data_allocated) {
            if (current_row >= m) semantic_error("Matrix \"c\" has too many rows.");
            else if (current_col >= n) semantic_error("Matrix \"c\" has too many elements in a row.");
            else c[current_row][current_col++] = $1;
        }
    }
    | number_list_c ',' NUMBER {
        if (parsing_matrix && data_allocated) {
            if (current_row >= m) semantic_error("Matrix \"c\" has too many rows.");
            else if (current_col >= n) semantic_error("Matrix \"c\" has too many elements in a row.");
            else c[current_row][current_col++] = $3;
        }
    };

%%

void yyerror(const char *s) { fprintf(stderr, "Parse error: %s\n", s); error_flag = 1; }

int main(int argc, char **argv) {
    if (argc != 2) { fprintf(stderr, "Usage: %s input.json\n", argv[0]); return 1; }
    FILE *file = fopen(argv[1], "r");
    if (!file) { fprintf(stderr, "Cannot open input file\n"); return 1; }
    yyin = file;
    int parse_status = yyparse();
    fclose(file);
    if (parse_status != 0 || error_flag) {
        fprintf(stderr, "Parsing failed.\n");
        free_data();
        return 1;
    }
    if (!m_set || !n_set || !has_a || !has_b || !has_c || !data_allocated) {
        fprintf(stderr, "Error: Input must contain fields \"m\", \"n\", \"a\", \"b\", and \"c\".\n");
        free_data();
        return 1;
    }
    double ta = 0, tb = 0;
    for (int i = 0; i < m; i++) {
        if (a[i] < -1e-9) {
            fprintf(stderr, "Error: Supply values in \"a\" must be non-negative.\n");
            free_data();
            return 1;
        }
        ta += a[i];
    }
    for (int j = 0; j < n; j++) {
        if (b[j] < -1e-9) {
            fprintf(stderr, "Error: Demand values in \"b\" must be non-negative.\n");
            free_data();
            return 1;
        }
        tb += b[j];
    }
    if (fabs(ta - tb) > 1e-9) {
        fprintf(stderr, "Error: Balance condition violated (sum a=%.2f, sum b=%.2f)\n", ta, tb);
        free_data();
        return 1;
    }
    transport_simplex();
    if (error_flag) {
        fprintf(stderr, "Solving failed.\n");
        free_data();
        return 1;
    }
    print_solution();
    output_json("output.json");
    free_data();
    return 0;
}
