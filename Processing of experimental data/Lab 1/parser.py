import requests
import pandas as pd
from datetime import datetime


# Get data without aggregates
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

print(f"Found countries: {len(country_codes)}")

# Get population data for all countries for latest year
year = datetime.now().year
while True:
    indicator = 'SP.POP.TOTL'
    url_data = f"http://api.worldbank.org/v2/country/all/indicator/{indicator}?date={str(year)}&format=json&per_page=20000"
    resp_data = requests.get(url_data)

    if resp_data.status_code != 200:
        print("Ошибка получения данных")
        exit()
    data = resp_data.json()[1]

    if data == None:
        print(f"No data for {year}")
        year -= 1
        continue

    rows = []
    for entry in data:
        if entry['value'] is not None:
            country_code = entry['countryiso3code'] 
            if country_code in country_codes:
                rows.append({
                    'country_code': country_code,
                    'country_name': entry['country']['value'],
                    'population': entry['value']
                })

    df = pd.DataFrame(rows)
    print(f"Countries with population data: {len(df)}")
    df.to_csv(f'world_population_countries_{year}.csv', index=False, encoding='utf-8')
    print(f"File saved: world_population_countries_{year}.csv")
    exit()