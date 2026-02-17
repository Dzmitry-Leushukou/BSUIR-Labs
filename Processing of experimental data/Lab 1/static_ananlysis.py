import glob
import csv
import math
from collections import Counter
from tabulate import tabulate
import matplotlib.pyplot as plt
import numpy as np

def load_data() -> list:
    file_pattern = 'world_population_countries_*.csv'
    files = glob.glob(file_pattern)
    if not files:
        print(f"Files with pattern '{file_pattern}' doesn't exist.")
        exit()
    filename = files[0]
    print(f"Choosed file: {filename}")
    data = []
    with open(filename, 'r', encoding='utf-8') as f:
        reader = csv.DictReader(f)
        if 'population' not in reader.fieldnames:
            print("Error: column 'population' not found in CSV.")
            exit()
        for row in reader:
            try:
                data.append(int(row['population']))
            except ValueError:
                print(f"Warning: invalid population value '{row['population']}', skipping.")
                continue
    if not data:
        print("No valid data loaded.")
        exit()
    print(f"[Loading data] Data successfully loaded: {len(data)}")
    return data

def remove_outliers_mad(data: list):
    median_val = find_median(data)
    abs_dev = [abs(x - median_val) for x in data]
    sorted_abs = sorted(abs_dev)
    n = len(sorted_abs)
    mad = sorted_abs[n // 2]
    threshold = 3 * mad
    lower_bound = median_val - threshold
    upper_bound = median_val + threshold
    filtered = [x for x in data if lower_bound <= x <= upper_bound]
    removed = len(data) - len(filtered)
    if removed > 0:
        print(f"[Outlier removal] Removed {removed} outliers using MAD method (3*MAD).")
        print(f"               Bounds: [{lower_bound:,.0f}, {upper_bound:,.0f}]")
    else:
        print("[Outlier removal] No outliers detected.")
    return filtered, lower_bound, upper_bound

def find_mode(data: list):
    counter = Counter(data)
    max_freq = max(counter.values())
    if max_freq == 1:
        print("[Finding mode] No mode found, all values have the same frequency.")
        return None
    modes = [val for val, freq in counter.items() if freq == max_freq]
    if len(modes) == 1:
        print(f"[Finding mode] Mode found: {modes[0]}")
        return modes[0]
    else:
        print(f"[Finding mode] Multiple modes found: {modes}")
        return modes

def find_median(data: list) -> float:
    sorted_data = sorted(data)
    n = len(sorted_data)
    if n % 2 == 0:
        median = (sorted_data[n//2 - 1] + sorted_data[n//2]) / 2
    else:
        median = sorted_data[n//2]
    print(f"[Finding median] Median found: {median}")
    return median

def find_average(data: list) -> float:
    avg = sum(data) / len(data)
    print(f"[Finding average] Average found: {avg}")
    return avg

def find_variance(data: list) -> float:
    avg = find_average(data)
    variance = sum((x - avg) ** 2 for x in data) / len(data)
    print(f"[Finding variance] Variance found: {variance}")
    return variance

def find_std_deviation(data: list) -> float:
    variance = find_variance(data)
    std_dev = variance ** 0.5
    print(f"[Finding std deviation] Standard deviation found: {std_dev}")
    return std_dev

def find_coefficient_of_variation(data: list) -> float:
    avg = find_average(data)
    if avg == 0:
        print("Warning: average is zero, coefficient of variation is undefined.")
        return float('nan')
    std_dev = find_std_deviation(data)
    cv = std_dev / avg
    print(f"[Finding CV] Coefficient of variation found: {cv}")
    return cv

def assess_homogeneity(cv: float) -> str:
    if math.isnan(cv):
        return "Undefined (average is zero)"
    if cv < 0.33:
        return "Homogeneous (CV < 33%)"
    elif cv <= 1.0:
        return "Moderately heterogeneous (33% ≤ CV ≤ 100%)"
    else:
        return "Highly heterogeneous (CV > 100%)"

def create_intervals(data):
    n = len(data)
    s = int(1 + math.log2(n))
    if s < 2:
        s = 2
    data_min = min(data)
    data_max = max(data)
    width = (data_max - data_min) / s
    intervals = []
    for i in range(s):
        left = data_min + i * width
        right = data_min + (i + 1) * width
        if i == s - 1:
            freq = sum(1 for x in data if left <= x <= right)
            intervals.append([f"[{left:,.0f}, {right:,.0f}]", freq])
        else:
            freq = sum(1 for x in data if left <= x < right)
            intervals.append([f"[{left:,.0f}, {right:,.0f})", freq])
    return intervals, s, width

def compute_group_statistics(data, n_bins):
    data_min = min(data)
    data_max = max(data)
    width = (data_max - data_min) / n_bins
    bins = [data_min + i * width for i in range(n_bins + 1)]
    groups = [[] for _ in range(n_bins)]
    for x in data:
        if x == data_max:
            idx = n_bins - 1
        else:
            idx = int((x - data_min) / width)
            if idx >= n_bins:
                idx = n_bins - 1
        groups[idx].append(x)
    group_sizes = []
    group_means = []
    group_vars = []
    for g in groups:
        ni = len(g)
        group_sizes.append(ni)
        if ni > 0:
            mean_i = sum(g) / ni
            var_i = sum((x - mean_i) ** 2 for x in g) / ni
            group_means.append(mean_i)
            group_vars.append(var_i)
        else:
            group_means.append(0.0)
            group_vars.append(0.0)
    return group_sizes, group_means, group_vars, width

if __name__ == '__main__':
    raw_data = load_data()
    clean_data, low, high = remove_outliers_mad(raw_data)
    if not clean_data:
        print("After outlier removal no data left. Exiting.")
        exit()

    mode = find_mode(raw_data)
    median = find_median(raw_data)
    average = find_average(raw_data)
    variance = find_variance(raw_data)
    std_dev = find_std_deviation(raw_data)
    cv = find_coefficient_of_variation(raw_data)
    homogeneity = assess_homogeneity(cv)

    table = []
    if mode is None:
        table.append(["Mode", "not defined (all values unique)"])
    elif isinstance(mode, list):
        mode_str = ', '.join(str(m) for m in mode)
        table.append(["Mode (multiple)", mode_str])
    else:
        table.append(["Mode", f"{mode:,.0f}"])
    table.append(["Median", f"{median:,.20f}"])
    table.append(["Average", f"{average:,.20f}"])
    table.append(["Variance", f"{variance:,.20f}"])
    table.append(["Standard deviation", f"{std_dev:,.20f}"])
    table.append(["Coefficient of variation", f"{cv:.20f}"])
    table.append(["Homogeneity assessment", homogeneity])
    table.append(["Sample size (after removal)", f"{len(clean_data)}"])
    table.append(["MAD interval", f"[{low:,.0f}; {high:,.0f}]"])

    print("\n" + tabulate(table, headers=["Statistic", "Value"], tablefmt="grid"))

    intervals, num_int, w = create_intervals(clean_data)
    print(f"\nInterval distribution for cleaned data (Sturges' rule, {num_int} intervals, width = {w:.2f}):")
    print(tabulate(intervals, headers=["Interval", "Frequency"], tablefmt="grid"))

    plt.figure(figsize=(10, 5))
    plt.hist(clean_data, bins=num_int, edgecolor='black', alpha=0.7, color='skyblue')
    plt.title('Histogram of Population (After Outlier Removal)')
    plt.xlabel('Population')
    plt.ylabel('Frequency')
    plt.grid(True, linestyle='--', alpha=0.5)
    plt.tight_layout()
    plt.savefig('histogram.png')

    counts, bin_edges = np.histogram(clean_data, bins=num_int)
    bin_centers = (bin_edges[:-1] + bin_edges[1:]) / 2
    plt.figure(figsize=(10, 5))
    plt.plot(bin_centers, counts, marker='o', linestyle='-', color='red', linewidth=2)
    plt.title('Frequency Polygon of Population')
    plt.xlabel('Population')
    plt.ylabel('Frequency')
    plt.grid(True, linestyle='--', alpha=0.5)
    plt.tight_layout()
    plt.savefig('frequency_polygon.png')

    print("\n" + "="*60)
    print("GROUP VARIANCE ANALYSIS FOR DIFFERENT NUMBER OF INTERVALS")
    print("="*60)

    n_total = len(clean_data)
    overall_mean = sum(clean_data) / n_total
    total_var = sum((x - overall_mean) ** 2 for x in clean_data) / n_total
    print(f"\nOverall statistics on cleaned data:")
    print(f"  Sample size: {n_total}")
    print(f"  Overall mean: {overall_mean:,.20f}")
    print(f"  Total variance (population): {total_var:,.20f}")

    s0 = num_int
    splits = [max(2, s0 - 1), s0, s0 + 1]
    results = []

    for nb in splits:
        print(f"\n{'-'*40}")
        print(f"Split into {nb} intervals:")
        sizes, means, vars_, width = compute_group_statistics(clean_data, nb)
        within_var = 0.0
        between_var = 0.0
        for ni, mi, vari in zip(sizes, means, vars_):
            if ni > 0:
                within_var += ni * vari
                between_var += ni * (mi - overall_mean) ** 2
        within_var /= n_total
        between_var /= n_total
        total_check = within_var + between_var
        eta2 = between_var / total_var if total_var != 0 else float('nan')

        results.append({
            'n_bins': nb,
            'width': width,
            'within_var': within_var,
            'between_var': between_var,
            'total_check': total_check,
            'eta2': eta2
        })

        print(f"  Interval width: {width:,.2f}")
        print(f"  Within‑group variance:  {within_var:,.20f}")
        print(f"  Between‑group variance: {between_var:,.20f}")
        print(f"  Sum (within + between):  {total_check:,.20f}")
        print(f"  Total variance (original): {total_var:,.20f}")
        print(f"  Difference: {abs(total_check - total_var):.2e}")
        print(f"  Proportion of between‑group variance (η²): {eta2:.6f}")

    print("\n" + "="*60)
    print("COMPARISON OF SPLITS")
    print("="*60)
    comp_table = []
    for r in results:
        comp_table.append([
            r['n_bins'],
            f"{r['width']:,.2f}",
            f"{r['within_var']:,.4e}",
            f"{r['between_var']:,.4e}",
            f"{r['eta2']:.6f}"
        ])
    print(tabulate(comp_table,
                   headers=["Intervals", "Width", "Within var", "Between var", "η²"],
                   tablefmt="grid"))

    best_split = max(results, key=lambda x: x['eta2'])
    print(f"\nThe split with the highest proportion of between‑group variance is {best_split['n_bins']} intervals (η² = {best_split['eta2']:.6f}).")
    print("This grouping explains the largest fraction of total variability, hence it is the best among the three.")