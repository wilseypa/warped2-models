import os 
import json

bad_path = 'bad_Pandemic_data'
files = os.listdir(bad_path)
data = {}
for file in files:
    file_path = bad_path + '/' + file
    filename = file.split('.')[0]
    with open(file_path, 'r') as outfile:
        data = json.load(outfile)
        data['date'] = filename
    with open('Pandemic_data/' + file, 'w') as outputfile:
        outputfile.write(json.dumps(data))
