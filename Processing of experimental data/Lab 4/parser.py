import requests
import pandas as pd
from datetime import datetime

url_countries = "http://api.worldbank.org/v2/country?format=json&per_page=20000"
resp = requests.get(url_countries)
if resp.status_code != 200:
    print("Ошибка получения списка стран")
    exit()
countries_data = resp.json()[1]

country_codes = []
for c in countries_data:
    if c['region']['value'] != "Aggregates":
        country_codes.append(c['id'])  

print(f"Найдено стран: {len(country_codes)}")

indicators = {
    'population': 'SP.POP.TOTL',
    'gdp_per_capita': 'NY.GDP.PCAP.CD'
}

data_by_country = {}

for indicator_name, indicator_code in indicators.items():
    print(f"\nЗагрузка данных для индикатора: {indicator_name} ({indicator_code})")
    
    for year in range(2026, 2013, -1):
        url_data = f"http://api.worldbank.org/v2/country/all/indicator/{indicator_code}?date={str(year)}&format=json&per_page=20000"
        resp_data = requests.get(url_data)

        if resp_data.status_code != 200:
            print(f"  Ошибка получения данных за {year}")
            continue
        
        data = resp_data.json()[1]

        if data is None or len(data) == 0:
            print(f"  Нет данных за {year}")
            continue
        
        count_added = 0
        for entry in data:
            if entry['value'] is not None:
                country_code = entry['countryiso3code']
                if country_code in country_codes:
                    if country_code not in data_by_country:
                        data_by_country[country_code] = {
                            'country_code': country_code,
                            'country_name': entry['country']['value']
                        }
                    data_by_country[country_code][indicator_name] = float(entry['value'])
                    count_added += 1
        
        print(f"  За {year} загружено: {count_added} записей для {indicator_name}")
        
        if len([c for c in data_by_country if indicator_name in data_by_country[c]]) > 100:
            print(f"  Достаточно данных для {indicator_name}, переходим к следующему индикатору")
            break

rows = []
for country_code, country_data in data_by_country.items():
    if 'population' in country_data and 'gdp_per_capita' in country_data:
        rows.append({
            'country_code': country_code,
            'country_name': country_data['country_name'],
            'population': country_data['population'],
            'gdp_per_capita': country_data['gdp_per_capita']
        })

df = pd.DataFrame(rows)
print(f"\nСтран с полными данными: {len(df)}")

if len(df) > 0:
    df.to_csv(f'world_data_full.csv', index=False, encoding='utf-8')
    print(f"Файл сохранён: world_data_full.csv")
    
    print(f"\nСтатистика по данным:")
    print(f"  Население: min={df['population'].min():,.0f}, max={df['population'].max():,.0f}, среднее={df['population'].mean():,.0f}")
    print(f"  ВВП на душу: min={df['gdp_per_capita'].min():,.2f}, max={df['gdp_per_capita'].max():,.2f}, среднее={df['gdp_per_capita'].mean():,.2f}")
    
    print(f"\nГоды, за которые собраны данные:")
    print(f"  Данные не привязаны к конкретному году, собраны доступные данные за 2026-2014")
else:
    print("Не удалось собрать данные. Попробуйте расширить диапазон годов.")