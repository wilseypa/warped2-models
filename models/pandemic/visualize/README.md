# **Visualization**  

## **Notes**
This is running on d3 v5 and does not currently work with d3 v3 or v4. Updated from v4 to move away from asynchronous data loading  
Currently the files from JHU need to be modified because Oglala Lakota South Dakota and Kusilvak Alaska have incorrect county IDs  
LOTS OF DATA IS MISSING IN CERTAIN DAYS (5/26/2020)(8/23/2020 only with range) WHICH THROWS ERRORS SPECIFICALLY WITH RANGES  

## **TODO**
### ***Front End***
Replace placeholder placeholders (waiting for actual placeholders) with correct placeholder values  
Remove checkboxes if placeholder values (placeholder values as background unless typed then forget placeholder values)  
Re-add slider to bottom of map  
Update tooltip data on new day when hovering with timelapse function enabled  
Improve timelapse placement  
Make timelapse stop at end of data  
Cleanup front-end (ongoing)  
### ***Back End***
Create basic API to load dummy data  
Create API call to return json file for POSTed date in format of "MM-DD-YYYY"  
Create API call to return list/array of json files between two POSTed dates in format of "MM-DD-YYYY"  
Investigate setting up AWS account for warped2 cloud database  

### ***--Completed--***
Investigate d3 and other mapping libraries such as google  
Plot counties on a map  
Plot data on counties for hover  
Create a heatmap effect for US county projection  
Create server setup to host website  
Investigate mongodb for a database  
The tooltip for county data makes scrollbars visible when the mouse is near edges of the screen  
Change the slider to an up/down arrow to increment/decrement days?  
Create local mongodb database  
Create script to set up mongodb database on a local machine  
Create script to fix and import JHU data into mongodb database  
Change the data to be looking at periods of time ie looking at the month of march  
Create timelapse functionality  
Refactor plotting function  
Calculate the true minimum and maximums for each data file, not hardcoded as it is currently  
Add legend to heatmap  
Add current date to top right corner of map  
Add new form for viveks api  
Create single username password for webapp (user=admin, pass=warped2)  
Make calendar picker for start date (MM-DD-YYYY dash delimeter)  
Fixed tooltip position consistency (position was modified relative to parent element to page)  
Fix bug where ranges can cause not all files to load and cumulate (can test with a ~7 day range at end of july) (possibly bad data) (currently irrelevant since switching how data is loaded)  
Position legend properly for windows resizing  
Use placeholder values on configApi form submit  
Make timelapse clearly enabled/disabled  
Create information icon hover for each parameter  

## **Resources**
https://www.youtube.com/watch?v=aNbgrqRuoiE  --Help for US state map  
https://www.youtube.com/watch?v=G-VggTK-Wlg  --Help for US county map

https://stackoverflow.com/questions/49724821/get-favicon-ico-error-but-can-not-find-it-in-the-code  --This explains favicon error  

https://stackoverflow.com/questions/16823757/d3-selectelement-not-working-when-code-above-the-html-element  --Reminder to load scripts at end of body  

https://stackoverflow.com/questions/19649886/d3-geo-albers-usa-map-showing-puerto-rico-and-virginia-incorrectly  --Explanation for missing Puerto Rico and Virgin Islands
