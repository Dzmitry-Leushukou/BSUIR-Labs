import requests
import pandas as pd
from datetime import datetime
import time

INDICATORS = {
    'y_pop': 'SP.POP.TOTL',
    'x1_gdp': 'NY.GDP.MKTP.CD',
    'x2_urban': 'SP.URB.TOTL.IN.ZS',
    'x3_energy': 'EG.USE.ELEC.KH.PC',
    'x4_health': 'SH.XPD.CHEX.GD.ZS',
    'x5_income': 'NY.GNP.PCAP.CD'
}
MIN_COUNTRIES = 40
MAX_YEARS_BACK = 10

def fetch_country_codes_and_names():
    url = "http://api.worldbank.org/v2/country?format=json&per_page=20000"
    resp = requests.get(url)
    if resp.status_code != 200:
        print("Ошибка получения списка стран")
        return {}, {}
    data = resp.json()[1]
    codes = {}
    names = {}
    for country in data:
        if country['region']['value'] != 'Aggregates':
            code = country['id']
            name = country['name']
            codes[code] = name
            names[code] = name
    print(f"Найдено стран (без агрегатов): {len(codes)}")
    return codes, names

def fetch_indicator_data(indicator, year, country_codes):
    url = f"http://api.worldbank.org/v2/country/all/indicator/{indicator}?date={year}&format=json&per_page=20000"
    resp = requests.get(url)
    if resp.status_code != 200:
        return {}
    try:
        data = resp.json()
        if not isinstance(data, list) or len(data) < 2 or data[1] is None:
            return {}
        entries = data[1]
    except:
        return {}
    result = {}
    for entry in entries:
        if entry.get('value') is not None:
            country_code = entry.get('countryiso3code')
            if country_code and country_code in country_codes:
                result[country_code] = entry['value']
    return result

def main():
    country_codes_dict, country_names = fetch_country_codes_and_names()
    if not country_codes_dict:
        return
    country_codes = list(country_codes_dict.keys())

    current_year = datetime.now().year
    for year in range(current_year, current_year - MAX_YEARS_BACK - 1, -1):
        print(f"\nПробуем год: {year}")
        all_data = {}

        success = True
        for name, code in INDICATORS.items():
            print(f"  Загрузка {name} ({code})...")
            data = fetch_indicator_data(code, year, country_codes)
            if not data:
                print(f"    Нет данных для {name} в {year}")
                success = False
                break
            for cc, val in data.items():
                if cc not in all_data:
                    all_data[cc] = {}
                all_data[cc][name] = val
            time.sleep(0.5)

        if not success:
            continue

        rows = []
        for cc, values in all_data.items():
            if len(values) == len(INDICATORS):
                rows.append({
                    'country_name': country_names.get(cc, cc),
                    'country_code': cc,
                    **values
                })

        df = pd.DataFrame(rows)
        print(f"Собрано стран с полными данными: {len(df)}")

        if len(df) >= MIN_COUNTRIES:
            print(f"\nДостигнуто минимальное количество стран ({MIN_COUNTRIES}) за {year}.")
            filename = f"worldbank_data_{year}.csv"
            df.to_csv(filename, index=False, encoding='utf-8')
            print(f"Файл сохранён: {filename}")
            print("\nПервые 5 записей:")
            print(df.head())
            return
        else:
            print(f"Недостаточно стран: {len(df)} < {MIN_COUNTRIES}, пробуем следующий год")

    print(f"\nНе удалось собрать {MIN_COUNTRIES} стран за последние {MAX_YEARS_BACK} лет.")

if __name__ == "__main__":
    main()