import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns
import glob
import os
from tabulate import tabulate

pd.set_option('display.max_columns', None)
pd.set_option('display.width', None)
pd.set_option('display.float_format', '{:.3f}'.format)

files = glob.glob("worldbank_data_*.csv")
if not files:
    print("❌ Файл с данными не найден. Сначала запустите parser.py")
    exit()
latest_file = max(files, key=os.path.getctime)
print(f"📁 Загружаем данные из файла: {latest_file}")

df = pd.read_csv(latest_file)
print(f"✅ Загружено стран: {len(df)}")

def format_df_for_display(df):
    df_disp = df.copy()
    df_disp['x1_gdp'] = df_disp['x1_gdp'].apply(lambda x: f'{x:,.0f}')
    for col in ['x2_urban', 'x3_energy', 'x4_health', 'x5_income', 'y_pop']:
        df_disp[col] = df_disp[col].apply(lambda x: f'{x:,.2f}')
    return df_disp

print(f"\n🔍 Все {len(df)} стран (полная выборка):")
df_display_all = format_df_for_display(df)
print(tabulate(df_display_all, headers='keys', tablefmt='grid', showindex=False))

with open('full_data_sample.txt', 'w', encoding='utf-8') as f:
    f.write(tabulate(df_display_all, headers='keys', tablefmt='grid', showindex=False))
print("\n📄 Полная выборка сохранена в файл full_data_sample.txt")

print("\n📊 Описательная статистика исходных данных:")
desc_stats = df.describe().round(3)
print(tabulate(desc_stats, headers='keys', tablefmt='grid', floatfmt='.3f'))
desc_stats.to_csv('descriptive_stats.csv')

feature_cols = ['y_pop', 'x1_gdp', 'x2_urban', 'x3_energy', 'x4_health', 'x5_income']
data = df[feature_cols].copy()

print("\n" + "="*70)
print("🔷 ЭТАП 1: ПЕРВИЧНЫЙ КОРРЕЛЯЦИОННЫЙ АНАЛИЗ (все 5 факторов)")
print("="*70)

corr_matrix = data.corr()
print("\n📊 Матрица коэффициентов парной корреляции (округлено до 3 знаков):")
print(tabulate(corr_matrix.round(3), headers='keys', tablefmt='grid', floatfmt='.3f'))

plt.figure(figsize=(10, 8))
sns.heatmap(corr_matrix, annot=True, fmt='.3f', cmap='coolwarm', center=0,
            square=True, linewidths=1, cbar_kws={"shrink": 0.8})
plt.title('Тепловая карта корреляций (все факторы)', fontsize=16)
plt.tight_layout()
plt.savefig('corr_initial.png', dpi=150)
plt.show()

print("\n🔎 Анализ зависимостей:")
print("1️⃣  Зависимость численности населения (y) от факторов:")
for col in feature_cols[1:]:
    r = corr_matrix.loc['y_pop', col]
    if abs(r) >= 0.7:
        strength = "сильная"
    elif abs(r) >= 0.4:
        strength = "умеренная"
    elif abs(r) >= 0.2:
        strength = "слабая"
    else:
        strength = "очень слабая или отсутствует"
    direction = "положительная" if r > 0 else "отрицательная"
    print(f"   • {col:10} : r = {r:6.3f}  — {direction}, {strength} связь")

print("\n2️⃣  Связи между факторами (проверка мультиколлинеарности):")
high_corr_pairs = []
for i in range(1, len(feature_cols)):
    for j in range(i+1, len(feature_cols)):
        f1 = feature_cols[i]
        f2 = feature_cols[j]
        r = corr_matrix.loc[f1, f2]
        if abs(r) > 0.8:
            high_corr_pairs.append((f1, f2, r))
            print(f"   ⚠️  {f1} и {f2}: r = {r:.3f} — очень высокая корреляция (мультиколлинеарность)")
        elif abs(r) > 0.6:
            print(f"   🔸 {f1} и {f2}: r = {r:.3f} — заметная корреляция")
if not high_corr_pairs:
    print("   ✅ Высоких корреляций (>0.8) между факторами не обнаружено.")

print("\n" + "="*70)
print("🔷 ЭТАП 2: ИСКЛЮЧЕНИЕ ФАКТОРОВ С КОРРЕЛЯЦИЕЙ |r| < 0.4 С y")
print("="*70)

keep_cols = ['y_pop']
drop_cols = []
for col in feature_cols[1:]:
    r = corr_matrix.loc['y_pop', col]
    if abs(r) >= 0.4:
        keep_cols.append(col)
        print(f"   ✅ {col:10} : r = {r:6.3f}  → ОСТАВЛЯЕМ")
    else:
        drop_cols.append(col)
        print(f"   ❌ {col:10} : r = {r:6.3f}  → ИСКЛЮЧАЕМ (|r| < 0.4)")

if not drop_cols:
    print("\n👉 Все факторы удовлетворяют условию |r| ≥ 0.4. Модель не изменилась.")
    data_filtered = data[keep_cols].copy()
else:
    data_filtered = data[keep_cols].copy()

print(f"\n🔹 Оставшиеся факторы: {keep_cols[1:] if len(keep_cols)>1 else 'нет'}")

print("\n" + "="*70)
print("🔷 ЭТАП 3: ПОВТОРНЫЙ КОРРЕЛЯЦИОННЫЙ АНАЛИЗ (после исключения)")
print("="*70)

if len(keep_cols) > 1:
    corr_matrix_filt = data_filtered.corr()
    print("\n📊 Новая матрица коэффициентов парной корреляции:")
    print(tabulate(corr_matrix_filt.round(3), headers='keys', tablefmt='grid', floatfmt='.3f'))

    plt.figure(figsize=(8, 6))
    sns.heatmap(corr_matrix_filt, annot=True, fmt='.3f', cmap='coolwarm', center=0,
                square=True, linewidths=1, cbar_kws={"shrink": 0.8})
    plt.title('Тепловая карта корреляций (после исключения)', fontsize=16)
    plt.tight_layout()
    plt.savefig('corr_filtered.png', dpi=150)
    plt.show()

    print("\n🔎 Итоговые выводы:")
    print("1️⃣  Зависимость y от оставшихся факторов:")
    for col in keep_cols[1:]:
        r = corr_matrix_filt.loc['y_pop', col]
        print(f"   • {col:10} : r = {r:6.3f}")

    print("\n2️⃣  Связи между оставшимися факторами (мультиколлинеарность):")
    if len(keep_cols) > 2:
        high_corr_filt = []
        for i in range(1, len(keep_cols)):
            for j in range(i+1, len(keep_cols)):
                f1 = keep_cols[i]
                f2 = keep_cols[j]
                r = corr_matrix_filt.loc[f1, f2]
                if abs(r) > 0.8:
                    high_corr_filt.append((f1, f2, r))
                    print(f"   ⚠️  {f1} и {f2}: r = {r:.3f} — очень высокая корреляция (мультиколлинеарность)")
                elif abs(r) > 0.6:
                    print(f"   🔸 {f1} и {f2}: r = {r:.3f} — заметная корреляция")
        if not high_corr_filt:
            print("   ✅ Высоких корреляций между оставшимися факторами не обнаружено.")
    else:
        print("   ➡️ Остался только один фактор – мультиколлинеарность отсутствует.")
else:
    print("\n❌ Не осталось ни одного фактора с |r| ≥ 0.4.")
    print("   Это означает, что ни один из выбранных факторов не имеет умеренной или сильной линейной связи с численностью населения.")
    print("   Возможно, стоит рассмотреть нелинейные зависимости или другие факторы.")

print("\n" + "="*70)
print("✅ АНАЛИЗ ПОЛНОСТЬЮ ЗАВЕРШЕН. Результаты сохранены в файлы:")
print("   - full_data_sample.txt (полная выборка)")
print("   - descriptive_stats.csv (описательная статистика)")
print("   - corr_initial.png (тепловая карта всех факторов)")
print("   - corr_initial.csv (матрица корреляций всех факторов)")
if len(keep_cols) > 1:
    print("   - corr_filtered.png (тепловая карта после фильтрации)")
    print("   - corr_filtered.csv (матрица корреляций после фильтрации)")
print("="*70)

corr_matrix.round(3).to_csv('corr_initial.csv')
if len(keep_cols) > 1:
    corr_matrix_filt.round(3).to_csv('corr_filtered.csv')