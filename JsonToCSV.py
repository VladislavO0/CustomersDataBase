import json
import csv

""" Найденные мною базы данных ФИО были в формате JSON, 
зная что стандартная библиотека не предусматривает работу с этим форматом
решил не усложнять с++ проект и не стал её подключать,
просто написал небольшой скрипт/функцию для выбора нужной мне информации из файлов JSON
и сохранил данные в csv формате"""

def jsonToCsv(jsonPath, csvPath):
    try:
        data = [json.loads(line) for line in open(jsonPath,'r', encoding='utf-8')]
        names = list()
        uniqueName = set()
        field = ['names', 'gender']
        for it in data:
            for attribute, value in it.items():
                if attribute == 'text' and len(value) != 0:
                    name = value
                elif attribute =='gender':
                    gender = value
            if name not in uniqueName:
                names.append({'names' : name, 'gender' : gender})
                uniqueName.add(name)

        with open(csvPath, 'w', encoding='utf8', newline='') as csvf:
            writter = csv.DictWriter(csvf,fieldnames = field)
            writter.writeheader()
            writter.writerows(names)


    except Exception as ex:
        print(f'Error: {str(ex)}')


jsonToCsv('SNM/names.json','names.csv' )
jsonToCsv('SNM/surnames.json','surnamess.csv' )
jsonToCsv('SNM/midnames.json','midnames.csv' )
