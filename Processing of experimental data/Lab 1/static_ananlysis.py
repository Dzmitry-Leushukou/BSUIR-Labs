import glob
import csv
from collections import Counter

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

if __name__ == '__main__':
    data = load_data()
    mode = find_mode(data)
    median = find_median(data)
    average = find_average(data)
    variance = find_variance(data)
    std_dev = find_std_deviation(data)
    cv = find_coefficient_of_variation(data)

    # Final output
    print("\n=== Results ===")
    print(f"Mode: {mode}")
    print(f"Median: {median}")
    print(f"Average: {average}")
    print(f"Variance: {variance}")
    print(f"Standard deviation: {std_dev}")
    print(f"Coefficient of variation: {cv}")