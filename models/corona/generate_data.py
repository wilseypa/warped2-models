#!/usr/bin/env python3

try:
    import time
    import re
    import argparse
    import json
except Exception as e:
    print(str(type(e).__name__) + ": " + str(e), file=sys.stderr)
    sys.exit(1)

    
def getpopulationforcountystate(filename, countyname, statename):

    to_match = countyname + " County, " + statename;
    
    fileobj = open(filename, 'r')
    line = fileobj.readline()

    while line != '':

        matches = re.match(r'(.*),([^,\s]*).*', line);
        
        if to_match.lower() == matches.group(1).lower():
            fileobj.close()
            return matches.group(2)

        line = fileobj.readline()

    fileobj.close()
    return (-1)



def parse_cmdargs():
    """
    Using argparse module, get commandline arguments
    :return:
    """
    parser = argparse.ArgumentParser(description='Parse Covid19 dataset from JHU-CSSE, add '
                                     'population data and dump combined data to '
                                     'file in csv or json format')
    parser.add_argument('--covid_data', help='JHU_CSSE Covid19 data csv filepath',
                        required=True)
    parser.add_argument('--pop_data', help='Population data csv filepath',
                        required=True)
    parser.add_argument('--date', help='Date of dataset in YYYY-MM-DD format',
                        required=False)
    parser.add_argument('--out_file', help='Output data filepath',
                        required=True)
    parser.add_argument('--format_json', help='Dump output as json', action='store_true',
                        default=False,
                        required=False)
    
    args = parser.parse_args()

    return (args.covid_data, args.pop_data, args.date, args.out_file, args.format_json)



def prepare_data():

    transmissibility, mean_incubation_duration_in_days, mean_infection_duration_in_days,\
        mortality_ratio, update_trig_interval_in_hrs, diffusion_trig_interval_in_hrs = 2.2, 2.2,\
            2.3, 0.05, 24, 48

    disease_model = [transmissibility, mean_incubation_duration_in_days,
                     mean_infection_duration_in_days, mortality_ratio,
                     update_trig_interval_in_hrs, diffusion_trig_interval_in_hrs]


    (covid_csse_data_filepath, pop_data_filepath, data_date, out_fname, format_json) = \
        parse_cmdargs()

    outfile = open(out_fname, 'w')

    if not format_json:
        strout = ','.join(str(v) for v in disease_model + [data_date])
        outfile.write(strout + "\n")

    final_dict = {
        "disease_model": {
            "transmissibility": transmissibility,
            "mean_incubation_duration_in_days": mean_incubation_duration_in_days,
            "mean_infection_duration_in_days": mean_infection_duration_in_days,
            "mortality_ratio": mortality_ratio,
            "update_trig_interval_in_hrs": update_trig_interval_in_hrs,
            "diffusion_trig_interval_in_hrs": diffusion_trig_interval_in_hrs,
            "data_date": data_date
        },
        "locations":[]
    }
        
    with open(covid_csse_data_filepath, 'r') as filedata:
    
        # skip header
        next(filedata)

        line = filedata.readline()

        while line != '':

            matches = re.match(r'^([0-9]+),([^,]*)\s*,\s*([^,]*)\s*,\s*([^,]*)\s*,\s*([^,]*)\s*,\s*([^,]*)\s*,\s*([^,]*)\s*,\s*([^,]*)\s*,\s*([^,]*)\s*,\s*([^,]*)\s*,\s*([^,]*)\s*,.*', line)

            line = filedata.readline()
        
            if matches:

                fips_code = matches.group(1)
                countyname = matches.group(2)
                statename = matches.group(3)
                loc_lat = matches.group(6)
                loc_long = matches.group(7)
                confirmed = matches.group(8)
                deaths = matches.group(9)
                recovered = matches.group(10)

            else:
                continue

        
            population = getpopulationforcountystate(pop_data_filepath, countyname=countyname,
                                                     statename=statename)

            location_array = [fips_code, countyname, statename, loc_lat, loc_long, population,
                              confirmed, recovered, deaths]
            
            final_dict["locations"].append(location_array)

            if not format_json:
                strout = ','.join(str(v) for v in location_array)
                outfile.write(strout + "\n")

        if format_json:
            outfile.write(json.dumps(final_dict))


if __name__ == '__main__':
    prepare_data()

