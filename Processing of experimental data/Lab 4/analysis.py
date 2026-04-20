import glob
import pandas as pd
import numpy as np
from tabulate import tabulate
import matplotlib.pyplot as plt
from scipy import stats
from scipy.stats import pearsonr, linregress

def load_data() -> pd.DataFrame:
    file_pattern = 'world_data_*.csv'
    files = glob.glob(file_pattern)
    if not files:
        print(f"Файлы с шаблоном '{file_pattern}' не найдены.")
        print("Сначала запустите parser.py")
        exit()
    
    csv_files = [f for f in files if 'full' in f or f != 'world_data_2025.csv']
    if csv_files:
        filename = csv_files[0]
    else:
        filename = files[0]
    
    print(f"Выбран файл: {filename}")
    
    df = pd.read_csv(filename, encoding='utf-8')
    
    required_cols = ['population', 'gdp_per_capita']
    for col in required_cols:
        if col not in df.columns:
            print(f"Ошибка: столбец '{col}' не найден в CSV файле")
            exit()
    
    df = df.dropna(subset=required_cols)
    
    print(f"[Загрузка данных] Данные успешно загружены: {len(df)} строк")
    return df

def remove_outliers_mad(data: list) -> tuple:
    median_val = np.median(data)
    abs_dev = [abs(x - median_val) for x in data]
    mad = np.median(abs_dev)
    threshold = 3 * mad
    lower_bound = median_val - threshold
    upper_bound = median_val + threshold
    filtered = [x for x in data if lower_bound <= x <= upper_bound]
    removed = len(data) - len(filtered)
    if removed > 0:
        print(f"[Удаление выбросов] Удалено {removed} выбросов методом MAD (3*MAD).")
        print(f"               Границы: [{lower_bound:,.0f}, {upper_bound:,.0f}]")
    else:
        print("[Удаление выбросов] Выбросы не обнаружены.")
    return filtered, lower_bound, upper_bound

def calculate_correlation_matrix(df: pd.DataFrame) -> tuple:
    df_log = df[['population', 'gdp_per_capita']].copy()
    df_log['log_population'] = np.log(df_log['population'])
    df_log['log_gdp'] = np.log(df_log['gdp_per_capita'])
    
    correlations = {}
    correlations['Население vs ВВП на душу'] = pearsonr(df['population'], df['gdp_per_capita'])[0]
    correlations['log(Население) vs log(ВВП)'] = pearsonr(df_log['log_population'], df_log['log_gdp'])[0]
    correlations['log(Население) vs ВВП на душу'] = pearsonr(df_log['log_population'], df['gdp_per_capita'])[0]
    
    return correlations, df_log

def find_best_predictor(correlations: dict) -> tuple:
    best_var = max(correlations.items(), key=lambda x: abs(x[1]))
    return best_var

def perform_regression(x_data, y_data, x_name: str, y_name: str):
    x = np.array(x_data)
    y = np.array(y_data)
    
    slope, intercept, r_value, p_value, std_err = linregress(x, y)
    
    y_pred = slope * x + intercept
    
    ss_res = np.sum((y - y_pred) ** 2)
    ss_tot = np.sum((y - np.mean(y)) ** 2)
    r_squared = 1 - (ss_res / ss_tot)
    adj_r_squared = 1 - (1 - r_squared) * (len(y) - 1) / (len(y) - 2)
    
    mse = ss_res / (len(y) - 2)
    se_regression = np.sqrt(mse)
    
    t_critical = stats.t.ppf(0.975, len(y) - 2)
    se_slope = std_err
    se_intercept = se_regression * np.sqrt(1/len(y) + np.mean(x)**2 / np.sum((x - np.mean(x))**2))
    
    slope_ci = (slope - t_critical * se_slope, slope + t_critical * se_slope)
    intercept_ci = (intercept - t_critical * se_intercept, intercept + t_critical * se_intercept)
    
    return {
        'slope': slope,
        'intercept': intercept,
        'r_value': r_value,
        'r_squared': r_squared,
        'adj_r_squared': adj_r_squared,
        'p_value': p_value,
        'std_err': std_err,
        'se_regression': se_regression,
        'y_pred': y_pred,
        'slope_ci': slope_ci,
        'intercept_ci': intercept_ci,
        'x_name': x_name,
        'y_name': y_name
    }

def plot_regression(x, y, regression_results, filename='regression_plot.png'):
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 6))
    
    ax1.scatter(x, y, alpha=0.6, color='blue', label='Фактические данные')
    
    sort_idx = np.argsort(x)
    x_sorted = x[sort_idx]
    y_pred_sorted = regression_results['y_pred'][sort_idx]
    
    ax1.plot(x_sorted, y_pred_sorted, color='red', linewidth=2, label='Линия регрессии')
    ax1.set_xlabel(regression_results['x_name'])
    ax1.set_ylabel(regression_results['y_name'])
    ax1.set_title(f'Линейная регрессия\nR² = {regression_results["r_squared"]:.4f}, p = {regression_results["p_value"]:.2e}')
    ax1.legend()
    ax1.grid(True, alpha=0.3)
    
    residuals = y - regression_results['y_pred']
    ax2.scatter(regression_results['y_pred'], residuals, alpha=0.6, color='green')
    ax2.axhline(y=0, color='red', linestyle='--', linewidth=1)
    ax2.set_xlabel('Предсказанные значения')
    ax2.set_ylabel('Остатки')
    ax2.set_title('График остатков')
    ax2.grid(True, alpha=0.3)
    
    plt.tight_layout()
    plt.savefig(filename, dpi=150)
    plt.show()
    print(f"График сохранён: {filename}")

if __name__ == '__main__':
    print("="*80)
    print("АНАЛИЗ ЛИНЕЙНОЙ РЕГРЕССИИ")
    print("="*80)
    
    df = load_data()
    
    clean_population, low_pop, high_pop = remove_outliers_mad(df['population'].tolist())
    clean_gdp, low_gdp, high_gdp = remove_outliers_mad(df['gdp_per_capita'].tolist())
    
    clean_df = df[(df['population'] >= low_pop) & (df['population'] <= high_pop) &
                   (df['gdp_per_capita'] >= low_gdp) & (df['gdp_per_capita'] <= high_gdp)]
    
    print(f"\nДанные после удаления выбросов: {len(clean_df)} строк")
    
    print("\n" + "="*80)
    print("КОРРЕЛЯЦИОННЫЙ АНАЛИЗ")
    print("="*80)
    
    correlations, df_log = calculate_correlation_matrix(clean_df)
    
    corr_table = []
    for var_name, corr_value in correlations.items():
        corr_table.append([var_name, f"{corr_value:.6f}", f"{abs(corr_value):.6f}"])
    
    print(tabulate(corr_table, headers=["Переменные", "Корреляция", "|Корреляция|"], tablefmt="grid"))
    
    best_var, best_corr = find_best_predictor(correlations)
    print(f"\nЛучший предиктор: {best_var} (|r| = {abs(best_corr):.6f})")
    
    if best_var == 'log(Население) vs log(ВВП)':
        x_data = df_log['log_population'].tolist()
        x_name = "log(Население)"
        y_data = df_log['log_gdp'].tolist()
        y_name = "log(ВВП на душу)"
    elif best_var == 'log(Население) vs ВВП на душу':
        x_data = df_log['log_population'].tolist()
        x_name = "log(Население)"
        y_data = clean_df['gdp_per_capita'].tolist()
        y_name = "ВВП на душу (USD)"
    else:
        x_data = clean_df['population'].tolist()
        x_name = "Население"
        y_data = clean_df['gdp_per_capita'].tolist()
        y_name = "ВВП на душу (USD)"
    
    print("\n" + "="*80)
    print("РЕЗУЛЬТАТЫ ЛИНЕЙНОЙ РЕГРЕССИИ")
    print("="*80)
    
    reg_results = perform_regression(x_data, y_data, x_name, y_name)
    
    results_table = [
        ["Зависимая переменная (Y)", reg_results['y_name']],
        ["Независимая переменная (X)", reg_results['x_name']],
        ["", ""],
        ["Уравнение регрессии", f"Y = {reg_results['intercept']:.6f} + {reg_results['slope']:.6f} * X"],
        ["", ""],
        ["Коэффициент наклона (β₁)", f"{reg_results['slope']:.6f}"],
        ["95% ДИ для наклона", f"[{reg_results['slope_ci'][0]:.6f}, {reg_results['slope_ci'][1]:.6f}]"],
        ["Свободный член (β₀)", f"{reg_results['intercept']:.6f}"],
        ["95% ДИ для свободного члена", f"[{reg_results['intercept_ci'][0]:.6f}, {reg_results['intercept_ci'][1]:.6f}]"],
        ["", ""],
        ["Коэффициент корреляции (r)", f"{reg_results['r_value']:.6f}"],
        ["Коэффициент детерминации (R²)", f"{reg_results['r_squared']:.6f}"],
        ["Скорректированный R²", f"{reg_results['adj_r_squared']:.6f}"],
        ["", ""],
        ["p-значение (значимость)", f"{reg_results['p_value']:.2e}"],
        ["Стандартная ошибка наклона", f"{reg_results['std_err']:.6f}"],
        ["Стандартная ошибка регрессии", f"{reg_results['se_regression']:.6f}"]
    ]
    
    print(tabulate(results_table, headers=["Параметр", "Значение"], tablefmt="grid"))
    
    print("\n" + "="*80)
    print("ИНТЕРПРЕТАЦИЯ")
    print("="*80)
    
    if reg_results['p_value'] < 0.05:
        print(f"✓ Модель статистически значима (p < 0.05)")
    else:
        print(f"✗ Модель НЕ статистически значима (p ≥ 0.05)")
    
    if abs(reg_results['r_value']) < 0.3:
        strength = "очень слабая"
    elif abs(reg_results['r_value']) < 0.5:
        strength = "слабая"
    elif abs(reg_results['r_value']) < 0.7:
        strength = "умеренная"
    elif abs(reg_results['r_value']) < 0.9:
        strength = "сильная"
    else:
        strength = "очень сильная"
    
    print(f"✓ Сила корреляции: {strength} (r = {reg_results['r_value']:.4f})")
    print(f"✓ R² = {reg_results['r_squared']:.4f} означает, что {reg_results['r_squared']*100:.1f}% дисперсии {reg_results['y_name']} объясняется переменной {reg_results['x_name']}")
    
    if reg_results['slope'] > 0:
        direction = "положительная"
    else:
        direction = "отрицательная"
    
    print(f"✓ Связь является {direction}")
    
    plot_regression(np.array(x_data), np.array(y_data), reg_results)
    
    print("\n" + "="*80)
    print("ПРИМЕРЫ ПРОГНОЗИРОВАНИЯ")
    print("="*80)
    
    if x_name == "log(Население)":
        example_x_values = [np.log(1e6), np.log(1e7), np.log(1e8)]
        example_labels = ["1 000 000", "10 000 000", "100 000 000"]
        pred_table = []
        for x_val, label in zip(example_x_values, example_labels):
            y_pred = reg_results['intercept'] + reg_results['slope'] * x_val
            if y_name == "log(ВВП на душу)":
                y_pred = np.exp(y_pred)
                pred_table.append([f"Население = {label}", f"${y_pred:,.2f}"])
            else:
                pred_table.append([f"Население = {label}", f"${y_pred:,.2f}"])
    else:
        example_x_values = [1e6, 1e7, 1e8]
        example_labels = ["1 000 000", "10 000 000", "100 000 000"]
        pred_table = []
        for x_val, label in zip(example_x_values, example_labels):
            y_pred = reg_results['intercept'] + reg_results['slope'] * x_val
            pred_table.append([f"Население = {label}", f"${y_pred:,.2f}"])
    
    print(tabulate(pred_table, headers=["Значение X", "Предсказанный ВВП на душу"], tablefmt="grid"))