# **Visualization**  

## **Notes**
This is running on d3 v4 and does not currently work with d3 v3  
Currently the files from JHU need to be modified because Oglala Lakota South Dakota and Kusilvak Alaska have incorrect county IDs  

## **TODO**
### ***Front End***
Calculate the true minimum and maximums for each data file, not hardcoded as it is currently  
Change the data to be looking at periods of time ie looking at the month of march  
Add legend to heatmap  
### ***Back End***
Create local mongodb database  
Create script to set up mongodb database on a local machine  
Create script to fix and import JHU data into mongodb database  
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

## **Resources**
https://www.youtube.com/watch?v=aNbgrqRuoiE  --Help for US state map  
https://www.youtube.com/watch?v=G-VggTK-Wlg  --Help for US county map

https://stackoverflow.com/questions/49724821/get-favicon-ico-error-but-can-not-find-it-in-the-code  --This explains favicon error  

https://stackoverflow.com/questions/16823757/d3-selectelement-not-working-when-code-above-the-html-element  --Reminder to load scripts at end of body  

https://stackoverflow.com/questions/19649886/d3-geo-albers-usa-map-showing-puerto-rico-and-virginia-incorrectly  --Explanation for missing Puerto Rico and Virgin Islands
