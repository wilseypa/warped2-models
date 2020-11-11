var tooltip = document.getElementById('tt');
color_wheel = ["#ffcccc", "#ff8080", "#ff0000", "#b30000"];
var colorWheelIndex = 0;

var startDate = document.getElementById('startDate');
startDate.value = "2020-07-22"; //"2020-03-25"
var endDate = document.getElementById('endDate');
var isDateRange = document.getElementById('isDateRange');

var incrementButton = document.getElementById("incrementButton");
var decrementButton = document.getElementById("decrementButton");

window.onmousemove = function (e) { //function gets location of mouse for tooltip
    var x = e.clientX,
        y = e.clientY;
    tooltip.style.top = (y + 20) + 'px';
    tooltip.style.left = (x + 20) + 'px';
};

function newDate(startOrEnd){
    // if(startOrEnd) {    //end date change

    // } else {    //start date change

    // }
    
    document.getElementById('plotButton').disabled = false;
}

function enableRange() {
    if(isDateRange.checked){
        document.getElementById('endDate').disabled = false;
        var endDateValue = new Date(startDate.value + "T00:00:00");    //T00:00:00 is required for local timezone
        endDateValue = endDateValue.setDate(endDateValue.getDate() + 1);
        endDateValue = new Date(endDateValue);
        endDate.value = formatDate(endDateValue, "YYYY-MM-DD");

    } else {
        document.getElementById('endDate').disabled = true;
        //endDate.value = undefined;
    }

    document.getElementById('plotButton').disabled = false;
}

function formatDate(dateValue, dateFormat) {
    let year = dateValue.getFullYear();
    let month = dateValue.getMonth() + 1;
    let day = dateValue.getDate()

    if(month < 10) {
        month = "0" + month;
    }
    if(day < 10) {
        day = "0" + day;
    }

    if(dateFormat == "YYYY-MM-DD") {
        return year + "-" + month + "-" + day;
    } else if(dateFormat == "MM-DD-YYYY") {
        return month + "-" + day + "-" + year;
    } else {
        throw "Invalid date format"
    }

}

function dateIncrement(direction) { //1 is increment, 0 is decrement
    var startDateValue = new Date(startDate.value + "T00:00:00");    //T00:00:00 is required for local timezone

    if(document.getElementById('isDateRange').checked){
        var startDateValue = new Date(startDate.value + "T00:00:00");    //T00:00:00 is required for local timezone
        var endDateValue = new Date(endDate.value + "T00:00:00");    //T00:00:00 is required for local timezone
        let timeDiff = endDateValue.getTime() - startDateValue.getTime();

        if(direction) {
            startDateValue = startDateValue.setTime(startDateValue.getTime() + timeDiff);
            endDateValue = endDateValue.setTime(endDateValue.getTime() + timeDiff);
        } else {
            startDateValue = startDateValue.setTime(startDateValue.getTime() - timeDiff);
            endDateValue = endDateValue.setTime(endDateValue.getTime() - timeDiff);
        }
        startDateValue = new Date(startDateValue)
        endDateValue = new Date(endDateValue)
        startDate.value = formatDate(startDateValue, "YYYY-MM-DD");
        endDate.value = formatDate(endDateValue, "YYYY-MM-DD");

    } else {
        if(direction){
            startDateValue.setDate(startDateValue.getDate() + 1);
        } else {
            startDateValue.setDate(startDateValue.getDate() - 1);
        }

        startDate.value = formatDate(startDateValue, "YYYY-MM-DD");
    }
} 



function plot_data() {
    document.getElementById('plotButton').disabled = true;
    document.getElementById('map').innerHTML = "";
    document.getElementById('map').style.display = "block";
(function() {
    d3.json("../mongodb/Pandemic_Data/07-22-2020.formatted-JHU-data.json").then(function(data22) {
        covidStats = data22;
    });

    var margin = { top: 0, left: 0, right: 0, bottom: 0},
        height = 400 - margin.top - margin.bottom,
        width = 800 - margin.left - margin.right;
        
    var svg = d3.select("#map")
        .append("svg")
        .attr("height", height + margin.top + margin.bottom)
        .attr("width", width + margin.left + margin.right)
        .append("g")
        .attr("transform", "translate(" + margin.left + "," + margin.top + ")");
        
    /*
        Read in wold.topojson
        Read in capiols.csv
    */

    // d3.queue()
    //     .defer(d3.json, "../us.json")   //json for country drawing
    //     .await(ready)

    var files = ["../us.json"];
    var promises = [];
    files.forEach(function(url) {
        promises.push(d3.json(url))
    });
    Promise.all(promises).then(function(values) {
        ready(values[0])
    });
        
    /*
        Create new projection using Mercator (geoMercator)
        and center it (translate)
        and zoom in a certain amount (scale)
    */
    
    var projection = d3.geoAlbersUsa()  //mercator projection for drawing map
        .translate([ width / 2, height / 2])
        .scale(850)
        
    /*
        create a path (geoPath)
        using the projection
    */
    
    var path = d3.geoPath()
        .projection(projection)
    
    function ready (data){   //function ready (error, data, data22, data23, data24, data25){
        
        var counties = topojson.feature(data, data.objects.counties).features
            
        svg.selectAll(".county")
            .data(counties)
            .enter().append("path")
            .attr("class", "county")
            .attr("d", path)
            .on("mouseover", function(d) {
                d3.select(this).classed("selected", true);
                d3.select("#tt").style("display", "inline-block");
                
                var prnt = d3.select(this);
                var loc;
                for (i = 0; i < covidStats.locations.length; i++){
                    if(parseInt(covidStats.locations[i][0]) == prnt._groups[0][0].__data__.id){
                        loc = covidStats.locations[i];
                        break;
                    }
                }
                var countyNameEl = document.createElement("span");
                countyNameEl.setAttribute("id", "countyName");
                document.getElementById("tt").appendChild(countyNameEl);
                d3.select("#countyName").html(loc[1] + " " + loc[2] + "<br>");
                if(document.getElementById('Confirmed').checked){
                    var confirmedEl = document.createElement("span");
                    confirmedEl.setAttribute("id", "confirmed");
                    document.getElementById("tt").appendChild(confirmedEl);
                    d3.select("#confirmed").html("confirmed: " + loc[6] + "<br>");
                }
                if(document.getElementById('Dead').checked){
                    var deadEl = document.createElement("span");
                    deadEl.setAttribute("id", "dead");
                    document.getElementById("tt").appendChild(deadEl);
                    d3.select("#dead").html("dead: " + loc[7] + "<br>");
                }
                if(document.getElementById('Recovered').checked){
                    var recoveredEl = document.createElement("span");
                    recoveredEl.setAttribute("id", "recovered");
                    document.getElementById("tt").appendChild(recoveredEl);
                    d3.select("#recovered").html("recovered: " + loc[8] + "<br>");
                }
                if(document.getElementById('Active').checked){
                    var activeEl = document.createElement("span");
                    activeEl.setAttribute("id", "active");
                    document.getElementById("tt").appendChild(activeEl);
                    d3.select("#active").html("active: " + loc[9] + "<br>");
                }
                var totalPopEl = document.createElement("span");
                totalPopEl.setAttribute("id", "totalPop");
                document.getElementById("tt").appendChild(totalPopEl);
                d3.select("#totalPop").html("total pop: " + loc[10]);
                /*if(document.getElementById('Confirmed').checked){
                    d3.select("#tt").html(loc[1] + " " + loc[2] + "<br>" + "confirmed: " + loc[6] + "<br>" + "dead: " + loc[7] + "<br>" + "recovered: " + loc[8] + "<br>" + "active: " + loc[9] + "<br>" + "total pop: " + loc[10])
                }*/
                
            })
            .on("mouseout", function(d) {
                d3.select(this).classed("selected", false);
                d3.select("#tt").style("display", "none");
                if(document.getElementById("countyName")){
                    document.getElementById("countyName").remove();
                }
                if(document.getElementById("confirmed")){
                    document.getElementById("confirmed").remove();
                }
                if(document.getElementById("dead")){
                    document.getElementById("dead").remove();
                }
                if(document.getElementById("recovered")){
                    document.getElementById("recovered").remove();
                }
                if(document.getElementById("active")){
                    document.getElementById("active").remove();
                }
                if(document.getElementById("totalPop")){
                    document.getElementById("totalPop").remove();
                }
            })
            .on("click", function(d) {
                d3.select(this).classed("selected", true)
                var prnt = d3.select(this);
                console.log(prnt._groups[0][0].__data__.id);
                console.log(covidStats);
                for (i = 0; i < covidStats.locations.length; i++){
                    if(parseInt(covidStats.locations[i][0]) == prnt._groups[0][0].__data__.id){
                        loc = covidStats.locations[i];
                        console.log("Index: " + i);
                        console.log(loc[1] + " " + loc[2]);
                        break;
                    }
                }
            })
        
        /*
            topojson.feature converts
            our RAW geo data into USABLE geo data
            always pass it data, then data.objects.__something__
            then get .features out of it
        */
        
        var states = topojson.feature(data, data.objects.states).features
        
        /*
            Add a path for each state
        */


        d3.selectAll(".arrow").on("click", function() {
            loadNewData();
        });

        function loadNewData() {
            if (isDateRange.checked) {    //load all data from files across range
                let startDateValue = new Date(startDate.value + "T00:00:00");    //T00:00:00 is required for local timezone
                let endDateValue = new Date(endDate.value + "T00:00:00");    //T00:00:00 is required for local timezone
                let timeDiffInDays = (endDateValue.getTime() - startDateValue.getTime())/(1000*60*60*24);

                var covidStatsArray = new Array(timeDiffInDays + 1); //will be length of date range
                var locationsDataLength = covidStats.locations[0].length;
                var locationsLength = covidStats.locations.length;

                var dummyForEachArray = new Array(timeDiffInDays + 1).fill(0);
                
                

                for (var i = 0; i < locationsLength; i++) {
                    for(var n = 0; n < locationsDataLength; n++) {
                        if(n == 6 || n == 7 || n == 8 || n == 9) {
                            covidStats.locations[i][n] = 0;
                        }
                    }
                }

                var startingDate = startDateValue;
                startingDate = new Date(startingDate.setDate(startingDate.getDate() - 1));

                dummyForEachArray.forEach(function(value, currentJsonFile){ //need the scope of forEach vs normal for loop
                //for (var currentJsonFile = 0; currentJsonFile < timeDiffInDays + 1; currentJsonFile++) {
                    startingDate = new Date(startingDate.setDate(startingDate.getDate() + 1));
                    var innerJsonDate = formatDate(startingDate, "MM-DD-YYYY");
                    jsonDataFilePathString = "../mongodb/Pandemic_Data/" + innerJsonDate + ".formatted-JHU-data.json"   //"../07-22-2020.formatted-JHU-data.json"

                    d3.json(jsonDataFilePathString).then(function(newData) {
                        covidStatsArray[currentJsonFile] = newData;

                        for (var i = 0; i < locationsLength; i++) {
                            for(var n = 0; n < locationsDataLength; n++) {
                                if(n == 6 || n == 7 || n == 8 || n == 9) {
                                    covidStats.locations[i][n] += covidStatsArray[currentJsonFile].locations[i][n];
                                } else {
                                    covidStats.locations[i][n] = covidStatsArray[currentJsonFile].locations[i][n];
                                }
                            }
                        }

                        if((currentJsonFile) == timeDiffInDays) {   //final file iteration
                            for (var i = 0; i < locationsLength; i++) {
                                for(var n = 0; n < locationsDataLength; n++) {
                                    if(n != 6 && n != 7 && n != 8 && n != 9) {
                                        covidStats.locations[i][n] = covidStatsArray[currentJsonFile].locations[i][n];  //get stats (population) of latest date in range
                                    }
                                }
                            }
                            rePlot(covidStats);
                        }
                        //ANYTHING AFTER THIS (and/or outside of this d3.json) WILL BE EXECUTED BEFORE THIS CODE
                    });
                })

            } else {
                let startDateValue = new Date(startDate.value + "T00:00:00");    //T00:00:00 is required for local timezone
                let innerJsonDate = formatDate(startDateValue, "MM-DD-YYYY");
                jsonDataFilePathString = "../mongodb/Pandemic_Data/" + innerJsonDate + ".formatted-JHU-data.json"   //"../07-22-2020.formatted-JHU-data.json"
                
                d3.json(jsonDataFilePathString).then(function(newData) {
                    covidStats = newData;
                    rePlot(covidStats);
                });
                //ANY CODE HERE WILL BE EXECUTED BEFORE THE d3.json FUNCTION ABOVE
            }
        };

        function rePlot(covidStats){
            svg.selectAll(".state")
            .data(states)
            .enter().append("path")
            .attr("class", "state")
            .attr("d", path)
            
            svg.selectAll(".county")
                .style("fill", function() {
                    var currentCountyId = d3.select(this)._groups[0][0].__data__.id;
                    var locData
                    for (i = 0; i < covidStats.locations.length; i++){
                        if(parseInt(covidStats.locations[i][0]) == currentCountyId){
                            locData = covidStats.locations[i];
                            break;
                        }
                    }
                    if(typeof locData == 'undefined'){
                        return "black";
                    }
                    var percentage = (((locData[9] / locData[10])*3)*100);
                    if (percentage >= 0 && percentage < 1.231){
                        return color_wheel[0];
                    } else if (percentage >= 1.231 && percentage < 5.231){
                        return color_wheel[1];
                    } else if (percentage >= 5.231 && percentage < 9.231){
                        return color_wheel[2];
                    } else if (percentage >= 9.231){
                        return color_wheel[3];
                    }
                    return color_wheel[colorWheelIndex];
                    });
        }
        
        rePlot(covidStats); //for the initial plot
            
        /*
            Add the cities
            Get the x/y from the lat/long + projection
        */
    }
    


})()
};
