generateLegend();

// getStatus();

// document.getElementById('map').style.display = "none";
// document.getElementById('legend').style.display = "none";

var margin = { top: 0, left: 0, right: 0, bottom: 0},
    height = 500 - margin.top - margin.bottom,
    width = 900 - margin.left - margin.right;
    
var svg = d3.select("#map")
    .append("svg")
    .attr("height", height + margin.top + margin.bottom)
    .attr("width", width + margin.left + margin.right)
    .append("g")
    .attr("transform", "translate(" + margin.left + "," + margin.top + ")");

var files = ["../us.json"];
var promises = [];
//NEED ARRAY OF STRINGS WITH EACH DATE TO BE READ IN THROUGH JSON FUNCTION
// getData("07-22-2020", "07-22-2020").then((data) => {
//     covidStats = data[0][0];
getSimulationData().then((data) => {
    simulationData = data;
    simulationDataMeta.count = -1;
    simulationDataMeta.length = data.length;

    covidStats = data[0];
    files.forEach(function(url) {
        promises.push(d3.json(url))
    });
    Promise.all(promises).then(function(values) {
        let data = values[0];

        //Loads State Map for Plotting?
        var states = topojson.feature(data, data.objects.states).features
        //Applies State Data and Logic to States (just the drawing of state lines)
        svg.selectAll(".state")
        .data(states)
        .enter().append("path")
        .attr("class", "state")
        .attr("d", path)

        // Loads County Map for Plotting?
        var counties = topojson.feature(data, data.objects.counties).features
        // Applies County Data and Logic to Counties
        svg.selectAll(".county")
            .data(counties)
            .enter().append("path")
            .attr("class", "county")
            .attr("d", path)
            .on("mouseover", function(d) {
                d3.select(this).classed("selected", true);
                d3.select("#tt").style("display", "inline-block");
                // d3.select("#tt").style("top", ((d3.mouse(this)[1]  + document.getElementById('map').getBoundingClientRect().top + window.pageYOffset) - document.getElementById('postSimulationContent').getBoundingClientRect().top) + 'px');
                // d3.select("#tt").style("left", (d3.mouse(this)[0]  + document.getElementById('map').getBoundingClientRect().left + window.pageXOffset) + 'px');

                
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
                //console.log(prnt._groups[0][0].__data__.id);
                //console.log(covidStats);
                for (i = 0; i < covidStats.locations.length; i++){
                    if(parseInt(covidStats.locations[i][0]) == prnt._groups[0][0].__data__.id){
                        loc = covidStats.locations[i];
                        //console.log("Index: " + i);
                        console.log(loc[1] + " " + loc[2]);
                        console.log("Percentage: " + (loc[9]/loc[10]*100));
                        break;
                    }
                }
            })
        
        loadNewData();
    });
    })
    
/*
    Create new projection using Mercator (geoMercator)
    and center it (translate)
    and zoom in a certain amount (scale)
*/

var projection = d3.geoAlbersUsa()  //mercator projection for drawing map
    .translate([ width / 2 + 50, height / 2])
    .scale(850)
    
/*
    create a path (geoPath)
    using the projection
*/

var path = d3.geoPath()
    .projection(projection);

/* EVENT FUNCTIONS */
//Plot Button Press
// d3.select("#plotButton").on("click", function() {
//     loadNewData();

//     setTimeout(function(){ 
//         // document.getElementById('mapStatCheckboxes').style.display = "block";
//         // document.getElementById('map').style.display = "block";
//         // document.getElementById('legend').style.display = "block";
//         d3.select("#plotButton").attr("disabled", true);
//         // document.body.style.backgroundColor = "lightskyblue";
//     }, 50);
// });

//Minimize_Maximize Button Press
d3.select("#minMaxMapControls").on("click", function() {
    var editConfigHtml = `<input type="button" id="editConfig" value="Edit Config">`;
    var labelHtml = `<h3 id="tempLabel" style="margin: 0px;">Map Controls</h3>`;
    if(document.getElementById('minMaxControlsContent').style.display == "none") {
        maximizeMapControlContent("minMaxControlsContent", "0.15");
        document.getElementById('minMaxMapControls').innerHTML = "&minus;";
        // fadeOutDisplay("tempLabel", "0.175");
        // document.getElementById('editConfigBtnContainer').innerHTML = editConfigHtml;
        // fadeInDisplay("editConfig", "0.175");
        

    } else {
        minimizeMapControlsContent("minMaxControlsContent", "0.15");
        document.getElementById('minMaxMapControls').innerHTML = "&plus;";
        // fadeOutDisplay("editConfig", "0.175");
        // document.getElementById('editConfigBtnContainer').innerHTML = labelHtml;
        // fadeInDisplay("tempLabel", "0.175");
    }
});

// Increment/Decrement Button Press
d3.selectAll(".arrow").on("click", function(){
    //get which button pressed
    var direction = 0
    if(d3.select(this)._groups[0][0].id == "decrementButton"){
        direction = 0
    } else if(d3.select(this)._groups[0][0].id == "incrementButton"){
        direction = 1
    } else {
        throw "unknown button pressed"
    }

    // Increment dates appropriately
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
        startDate.value = formatDate(startDateValue, "YYYY-MM-DD", "javascript");
        endDate.value = formatDate(endDateValue, "YYYY-MM-DD", "javascript");

    } else {
        if(direction){
            startDateValue.setDate(startDateValue.getDate() + 1);
        } else {
            startDateValue.setDate(startDateValue.getDate() - 1);
        }

        startDate.value = formatDate(startDateValue, "YYYY-MM-DD", "javascript");
    }

    //load new data for new dates
    loadNewData();
});

// Date Range New Input
var timelapseToggle = 1;
var intervalId;
d3.select("#timelapseButton").on("click", function() {
    // if(document.getElementById('map').style.display == "none") {
    //     document.getElementById('mapStatCheckboxes').style.display = "block";
    //     document.getElementById('map').style.display = "block";
    //     document.getElementById('legend').style.display = "block";
    // }

    if(timelapseToggle){
        intervalId = setInterval(function() {
            document.getElementById("incrementButton").click();
        }, 1500);
        timelapseToggle = 0;

        var timelapseEl = document.createElement("div");
        timelapseEl.setAttribute("id", "timelapseAnimation");
        document.getElementById("timelapseButton").appendChild(timelapseEl);
        let timelapseHtml = 
        `<div class="timelapseIndicator"></div>`;
        document.getElementById("timelapseAnimation").innerHTML = timelapseHtml;
        document.getElementById("timelapseAnimation").style.position = "absolute";
        document.getElementById("timelapseAnimation").style.display = "inline-block";
        let top = document.getElementById("timelapseButton").offsetTop + document.getElementById("timelapseButton").offsetHeight/2 - 2.5;
        let left = document.getElementById("timelapseButton").offsetLeft + document.getElementById("timelapseButton").offsetWidth/2 - document.getElementById("timelapseAnimation").offsetWidth/2 + 0.45;
        document.getElementById("timelapseAnimation").style.top = top + "px";
        document.getElementById("timelapseAnimation").style.left = left + "px";
    } else {
        clearInterval(intervalId);
        document.getElementById('timelapseAnimation').remove();
        timelapseToggle = 1;
    }
    //needs a stopping function when at end of data
});

// Date Range New Input
d3.selectAll(".dateInput").on("input", function() {
    //get which button pressed
    var startOrEnd = 0
    if(d3.select(this)._groups[0][0].id == "startDate"){
        startOrEnd = 0
    } else if(d3.select(this)._groups[0][0].id == "endDate"){
        startOrEnd = 1
    } else {
        throw "unknown button pressed"
    }

    // if(startOrEnd) {    //end date change

    // } else {    //start date change

    // }
    
    document.getElementById('plotButton').disabled = false;
});

// Range? Checkbox Toggle
d3.select("#isDateRange").on("click", function() {
    if(isDateRange.checked){
        document.getElementById('endDate').disabled = false;
        var endDateValue = new Date(startDate.value + "T00:00:00");    //T00:00:00 is required for local timezone
        endDateValue = endDateValue.setDate(endDateValue.getDate() + 1);
        endDateValue = new Date(endDateValue);
        endDate.value = formatDate(endDateValue, "YYYY-MM-DD", "javascript");

    } else {
        document.getElementById('endDate').disabled = true;
        //endDate.value = undefined;
    }

    document.getElementById('plotButton').disabled = false;
});


/* NON-EVENT FUNCTIONS */
// Updates #mapDate value
function updateMapDateValue(startDateParam, endDateParam) {
    if (endDateParam === undefined) {
        document.getElementById('mapDate').innerHTML = startDateParam;
    } else {
        document.getElementById('mapDate').innerHTML = startDateParam + " - " + endDateParam;
    }
}

// Grabs Dates from Input and Plots to D3 Map
function loadNewData() {
    if (isDateRange.checked) {    //load all data from files across range
        let startDateValue = new Date(startDate.value + "T00:00:00");    //T00:00:00 is required for local timezone
        let endDateValue = new Date(endDate.value + "T00:00:00");    //T00:00:00 is required for local timezone
        let timeDiffInDays = (endDateValue.getTime() - startDateValue.getTime())/(1000*60*60*24);

        var startDateParam = formatDate(startDateValue, "MM-DD-YYYY", "javascript");
        var endDateParam = formatDate(endDateValue, "MM-DD-YYYY", "javascript");
        
        updateMapDateValue(startDateParam, endDateParam);

        //console.log(startDateParam + "   " + endDateParam);
        var covidStatsAPIArray;
        getData(startDateParam, endDateParam).then((data) => {
            covidStatsAPIArray = data;
            //console.log(covidStatsAPIArray);

            var locationsLength = covidStats.locations.length;
            var locationsDataLength = covidStats.locations[0].length;

            for (var i = 0; i < locationsLength; i++) {
                for(var n = 0; n < locationsDataLength; n++) {
                    if(n == 6 || n == 7 || n == 8 || n == 9) {
                        covidStats.locations[i][n] = 0;
                    }
                }
            }

            covidStatsAPIArray.forEach(function (value, index) { // (item, index)
                locationsLength = covidStatsAPIArray[index][0].locations.length;
                locationsDataLength = covidStatsAPIArray[index][0].locations[0].length;

                for (var i = 0; i < locationsLength; i++) {
                    //console.log(covidStats.locations[i])
                    for (var n = 0; n < locationsDataLength; n++) {
                        if (n == 6 || n == 7 || n == 8 || n == 9) {
                            covidStats.locations[i][n] += covidStatsAPIArray[index][0].locations[i][n];
                        } /*else {    //ELSE NOT NEEDED BC OF NEXT IF STATEMENT
                                covidStats.locations[i][n] = covidStatsAPIArray[index][0].locations[i][n];
                            }*/
                    }
                }

                if ((index) == timeDiffInDays) {   //final file iteration
                    for (var i = 0; i < locationsLength; i++) {
                        for (var n = 0; n < locationsDataLength; n++) {
                            if (n != 6 && n != 7 && n != 8 && n != 9) {
                                covidStats.locations[i][n] = covidStatsAPIArray[index][0].locations[i][n];  //get stats (population) of latest date in range
                            }
                        }
                    }
                    rePlot(covidStats);
                }
            })
        })

    } else {
        let startDateValue = new Date(startDate.value + "T00:00:00");    //T00:00:00 is required for local timezone
        startDateValue.setDate(startDateValue.getDate() + 1);   //THIS IS WRONG - HTML VALUE IS ALWAYS 1 BEHIND AND INCORRECT
	    var startDateParam = formatDate(startDateValue, "MM-DD-YYYY", "javascript");

        updateMapDateValue(startDateParam, endDateParam);

        // console.log(startDateParam);
        // console.log(simulationData);
        simulationDataMeta.count++;
        if (simulationDataMeta.count == simulationDataMeta.length) {
            console.error("Data ended");
        }
        covidStats = simulationData[simulationDataMeta.count];
        rePlot(covidStats);
	    // getData(startDateParam, startDateParam).then((data) => {
        //     covidStats = data[0][0];
        //     rePlot(covidStats);
        // })
    }
};

// The Plotting of Colors to Each County    -----------DYNAMIC MAX PERCENTAGE BUT DOESNT WORK WELL BECAUSE OF OUTLIERS....---------
function rePlot(covidStats){
    var highestPercentage = 0;
    for (i = 0; i < covidStats.locations.length; i++) {
        tempHighestPercentage =  ((covidStats.locations[i][9] / covidStats.locations[i][10]) * 100);
        if(tempHighestPercentage > highestPercentage) {
            highestPercentage = tempHighestPercentage;
        }
    }
    for (i = 0; i < percentageArray.length; i++) {   // DYNAMIC LEGEND RANGE
        percentageArray[i] = ((highestPercentage / percentageArray.length) * i).toFixed(2);
    }
    
    for (i = 0; i < percentageArray.length; i++) {
        let selectString = "#legendValue" + i;
        let htmlString = percentageArray[i] + "%" + " - " + percentageArray[i + 1] + "%"
        if(i == percentageArray.length-1) {
            htmlString = percentageArray[i] + "%" + "+";
        }
        d3.select(selectString).html(htmlString);
    }

    svg.selectAll(".county")
        .style("fill", function() {
            var currentCountyId = d3.select(this)._groups[0][0].__data__.id;
            var currentLocationData;
            for (i = 0; i < covidStats.locations.length; i++){
                if(parseInt(covidStats.locations[i][0]) == currentCountyId){
                    currentLocationData = covidStats.locations[i];
                    break;
                }
            }
            if(typeof currentLocationData == 'undefined'){
                return "black";
            }
            var percentage = ((currentLocationData[9] / currentLocationData[10]) * 100);

            for (i = 0; i < percentageArray.length; i++) {
                if(i == percentageArray.length-1) {
                    if (percentage >= percentageArray[i]){
                        return color_wheel[i];
                    }
                }
                if (percentage >= percentageArray[i] && percentage < percentageArray[i + 1]){
                    return color_wheel[i];
                }
            }
        });
}
