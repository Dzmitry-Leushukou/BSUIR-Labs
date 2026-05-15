import json
import sys
from collections import deque, defaultdict

def read_input(filename):
    with open(filename, 'r') as f:
        data = json.load(f)
    return data['m'], data['n'], data['a'], data['b'], data['c']

def northwest_corner(m, n, a, b):
    a_copy = a[:]
    b_copy = b[:]
    x = [[0]*n for _ in range(m)]
    B = set()
    i, j = 0, 0
    while i < m and j < n:
        val = min(a_copy[i], b_copy[j])
        x[i][j] = val
        a_copy[i] -= val
        b_copy[j] -= val
        B.add((i, j))
        if a_copy[i] == 0:
            i += 1
        if b_copy[j] == 0:
            j += 1
    if len(B) < m + n - 1:
        B = add_artificial_basis(m, n, B, x)
    return x, B

def add_artificial_basis(m, n, B, x):
    parent = list(range(m + n))
    rank = [0]*(m + n)
    def find(v):
        while parent[v] != v:
            parent[v] = parent[parent[v]]
            v = parent[v]
        return v
    def union(v1, v2):
        r1, r2 = find(v1), find(v2)
        if r1 == r2:
            return False
        if rank[r1] < rank[r2]:
            parent[r1] = r2
        elif rank[r1] > rank[r2]:
            parent[r2] = r1
        else:
            parent[r2] = r1
            rank[r1] += 1
        return True
    for i, j in B:
        union(i, m + j)
    for i in range(m):
        for j in range(n):
            if (i, j) not in B and find(i) != find(m + j):
                B.add((i, j))
                x[i][j] = 0
                union(i, m + j)
                if len(B) == m + n - 1:
                    break
        if len(B) == m + n - 1:
            break
    return B

def compute_potentials(m, n, B, c):
    u = [None]*m
    v = [None]*n
    adj = [[] for _ in range(m + n)]
    for i, j in B:
        adj[i].append((m + j, c[i][j]))
        adj[m + j].append((i, c[i][j]))
    u[0] = 0
    q = deque([0])
    while q:
        node = q.popleft()
        for nb, cost in adj[node]:
            if node < m:   # строка -> столбец
                if v[nb - m] is None:
                    v[nb - m] = cost - u[node]
                    q.append(nb)
            else:          # столбец -> строка
                if u[nb] is None:
                    u[nb] = cost - v[node - m]
                    q.append(nb)
    for i in range(m):
        if u[i] is None:
            u[i] = 0
    for j in range(n):
        if v[j] is None:
            v[j] = 0
    return u, v

def find_cycle(B, enter_cell, m, n):
    cells = list(B) + [enter_cell]
    row_cnt = [0]*m
    col_cnt = [0]*n
    for i, j in cells:
        row_cnt[i] += 1
        col_cnt[j] += 1
    removed_rows = [False]*m
    removed_cols = [False]*n
    changed = True
    while changed:
        changed = False
        for i in range(m):
            if not removed_rows[i] and row_cnt[i] <= 1:
                to_remove = [(r, c) for (r, c) in cells if r == i]
                for rc in to_remove:
                    cells.remove(rc)
                    row_cnt[rc[0]] -= 1
                    col_cnt[rc[1]] -= 1
                removed_rows[i] = True
                changed = True
        for j in range(n):
            if not removed_cols[j] and col_cnt[j] <= 1:
                to_remove = [(r, c) for (r, c) in cells if c == j]
                for rc in to_remove:
                    cells.remove(rc)
                    row_cnt[rc[0]] -= 1
                    col_cnt[rc[1]] -= 1
                removed_cols[j] = True
                changed = True
    if not cells:
        return []
    rows = defaultdict(list)
    cols = defaultdict(list)
    for i, j in cells:
        rows[i].append(j)
        cols[j].append(i)
    start = enter_cell
    cycle = [start]
    curr = start
    step = 0
    while True:
        i_curr, j_curr = curr
        if step % 2 == 0:
            for j in rows[i_curr]:
                if j != j_curr:
                    nxt = (i_curr, j)
                    break
        else:
            for i in cols[j_curr]:
                if i != i_curr:
                    nxt = (i, j_curr)
                    break
        if nxt == start:
            break
        cycle.append(nxt)
        curr = nxt
        step += 1
    return cycle

def transport_solver(m, n, a, b, c, debug=False):
    if sum(a) != sum(b):
        raise ValueError("Задача не сбалансирована")
    x, B = northwest_corner(m, n, a, b)
    if debug:
        print("\n=== НАЧАЛЬНЫЙ ПЛАН (северо-западный угол) ===")
        print_plan(x, a, b)
        print(f"Базис: {sorted(B)}")
    iteration = 0
    while True:
        iteration += 1
        u, v = compute_potentials(m, n, B, c)
        if debug:
            print(f"\n--- Итерация {iteration} ---")
            print("Потенциалы u:", [round(val,2) for val in u])
            print("Потенциалы v:", [round(val,2) for val in v])
        min_delta = float('inf')
        enter_cell = None
        for i in range(m):
            for j in range(n):
                if (i, j) not in B:
                    delta = c[i][j] - u[i] - v[j]
                    if debug:
                        print(f"  delta({i},{j}) = {c[i][j]} - {u[i]:.2f} - {v[j]:.2f} = {delta:.2f}")
                    if delta < 0:
                        if delta < min_delta or (delta == min_delta and (i, j) < enter_cell):
                            min_delta = delta
                            enter_cell = (i, j)
        if enter_cell is None:
            if debug:
                print("Все оценки неотрицательны -> оптимальный план достигнут.")
            break
        if debug:
            print(f"Выбрана вводимая клетка {enter_cell} с оценкой {min_delta:.2f}")
        cycle = find_cycle(B, enter_cell, m, n)
        if debug:
            print("Цикл (в порядке обхода):", cycle)
        theta = float('inf')
        exit_cell = None
        for idx, (i, j) in enumerate(cycle):
            if idx % 2 == 1:
                if x[i][j] < theta or (x[i][j] == theta and (i, j) < exit_cell):
                    theta = x[i][j]
                    exit_cell = (i, j)
        if debug:
            print(f"theta = {theta}, выводимая клетка: {exit_cell}")
        for idx, (i, j) in enumerate(cycle):
            if idx % 2 == 0:
                x[i][j] += theta
            else:
                x[i][j] -= theta
        B.remove(exit_cell)
        B.add(enter_cell)
        if debug:
            print("Обновлённый план:")
            print_plan(x, a, b)
            print(f"Новый базис: {sorted(B)}")
    total_cost = sum(c[i][j] * x[i][j] for i in range(m) for j in range(n))
    return x, total_cost

def print_plan(x, a, b):
    m, n = len(x), len(x[0])
    for i in range(m):
        row_str = '  '.join(f"{x[i][j]:8.2f}" for j in range(n))
        print(f"{row_str}   | {a[i]:8.2f}")
    print('-' * (12 * n + 10))
    for j in range(n):
        print(f"{b[j]:8.2f}  ", end='')
    print()

def main():
    if len(sys.argv) != 2:
        print("Usage: python main.py <input.json>")
        sys.exit(1)
    filename = sys.argv[1]
    m, n, a, b, c = read_input(filename)
    x, total_cost = transport_solver(m, n, a, b, c, debug=True)
    print("\n=== ОКОНЧАТЕЛЬНЫЙ ОПТИМАЛЬНЫЙ ПЛАН ===")
    print_plan(x, a, b)
    print(f"Общая стоимость: {total_cost:.2f}")

if __name__ == "__main__":
    main()