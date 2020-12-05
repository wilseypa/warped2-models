#!/usr/bin/env python3

# This script will preprocess *.formatted-JHU-data.json files in 
# a given date range and create an ouput csv file with columns:
# Date,Confirmed,Deaths,Recovered,Active,Population
#
# Simply edit start, end date values in line 43 & 44
# Execute this script from the same directory.

import json
import datetime

def getStatsFromFile(filename):
    """
    """
    with open(filename) as f:
        jsonData = json.load(f)

    totalConfirmed = 0
    totalDeaths = 0
    totalRecovered = 0
    totalActive = 0
    totalPopulation = 0

    for item in jsonData["locations"]:
        confirmed = item[6]
        deaths = item[7]
        recovered = item[8]
        active = item[9]
        population = item[10]

        totalConfirmed += confirmed
        totalDeaths += deaths
        totalRecovered += recovered
        totalActive += active
        totalPopulation += population

    return totalConfirmed, totalDeaths, totalRecovered, totalActive, totalPopulation


def processFilesForDateRange():
    """
    """
    FROM_DATESTR = "07-01-2020"
    END_DATESTR = "08-10-2020"
    to_write_to_file = "Date,Confirmed,Deaths,Recovered,Active,Population\n"

    from_date = curr_date = datetime.datetime.strptime(FROM_DATESTR, "%m-%d-%Y")
    end_date = datetime.datetime.strptime(END_DATESTR, "%m-%d-%Y")

    # print(from_date, end_date)

    while curr_date <= end_date:

        curr_datestr = curr_date.strftime("%m-%d-%Y")
        filename = curr_datestr + ".formatted-JHU-data.json"
        totalConfirmed, totalDeaths, totalRecovered, totalActive, totalPopulation = \
            getStatsFromFile(filename)

        curr_date += datetime.timedelta(days=1)

        to_write_to_file += ",".join([str(curr_datestr), str(totalConfirmed),
                                      str(totalDeaths),
                                      str(totalRecovered),
                                      str(totalActive),
                                      str(totalPopulation)])
        to_write_to_file += '\n'

        with open("diseaseStatsUS.actual.csv", "w") as f:
            f.write(to_write_to_file)


if __name__ == '__main__':
    processFilesForDateRange()


