// Frontend Globals and Setup
var startDate = document.getElementById('startDate');
var endDate = document.getElementById('endDate');
var isDateRange = document.getElementById('isDateRange');

var tooltip = document.getElementById('tt');
var color_wheel = ["#ffffcc", "#ffeda0", "#fed976", "#feb24c", "#fd8d3c", "#fc4e2a", "#e31a1c", "#bd0026", "#800026"];
//var color_wheel = ["#00FF00", "#33FF33", "#66FF66", "#99FF99", "#800026", "#bd0026", "#e31a1c", "#fc4e2a", "#fd8d3c"];
var percentageArray = new Array(9);
var colorWheelIndex = 0;

var getstatusTimeout = undefined;

var simulationData = undefined;
// var simulationDataMeta = {
//     "count":undefined,
//     "length":undefined
// };

// Frontend Basic Functions
function generateLegend(){
    for(var i = 0; i < color_wheel.length + 1; i++) {
        if (i < color_wheel.length) {
            var legendInnerContainers = document.createElement("div");
            legendInnerContainers.setAttribute("id", "legendInnerContainer" + i);
            document.getElementById("legendValues").appendChild(legendInnerContainers);
        
            var legendColorEl = document.createElement("div");
            legendColorEl.setAttribute("id", "legendColor" + i);
            legendColorEl.setAttribute("class", "legend-fill");
            document.getElementById("legendInnerContainer" + i).appendChild(legendColorEl);
        
            var legendValueEl = document.createElement("div");
            legendValueEl.setAttribute("id", "legendValue" + i);
            legendValueEl.setAttribute("class", "legend-value");
            document.getElementById("legendInnerContainer" + i).appendChild(legendValueEl);
            
            document.getElementById('legendColor' + i).style.backgroundColor = color_wheel[i];
            document.getElementById('legendValue' + i).innerHTML = 0 + "%";
        }
    
        if (i == color_wheel.length) {
            var legendInnerContainers = document.createElement("div");
            legendInnerContainers.setAttribute("id", "legendInnerContainer" + i);
            document.getElementById("legendValues").appendChild(legendInnerContainers);
        
            var legendColorEl = document.createElement("div");
            legendColorEl.setAttribute("id", "legendColor" + i);
            legendColorEl.setAttribute("class", "legend-fill");
            document.getElementById("legendInnerContainer" + i).appendChild(legendColorEl);
        
            var legendValueEl = document.createElement("div");
            legendValueEl.setAttribute("id", "legendValue" + i);
            legendValueEl.setAttribute("class", "legend-value");
            document.getElementById("legendInnerContainer" + i).appendChild(legendValueEl);
            
            document.getElementById('legendColor' + i).style.backgroundColor = "black";
            document.getElementById('legendValue' + i).innerHTML = 0 + "%";
    
            let selectString = "#legendValue" + i;
            let htmlString = "No Data"
            d3.select(selectString).html(htmlString);
        }
    }
}

window.onmousemove = function (e) { //function gets location of mouse for tooltip
    var x = e.clientX,
        y = e.clientY - document.getElementById('postSimulationContent').getBoundingClientRect().top;
    tooltip.style.top = (y + 5) + 'px';
    tooltip.style.left = (x + 5) + 'px';
};

// Function for handling javascript and html dates
function formatDate(dateValue, dateFormat, dateType) {  //this is in both statemanager and map currently
    var year, month, day;
    if (dateType == "javascript") {
        year = dateValue.getFullYear();
        month = dateValue.getMonth() + 1;
        day = dateValue.getDate()

        if(month < 10) {
            month = "0" + month;
        }
        if(day < 10) {
            day = "0" + day;
        }
    } else if (dateType == "html") {
        let dateArray = dateValue.split("-");
        year = dateArray[0];
        month = dateArray[1];
        day = dateArray[2];
    }

    if(dateFormat == "YYYY-MM-DD") {
        return year + "-" + month + "-" + day;
    } else if(dateFormat == "MM-DD-YYYY") {
        return month + "-" + day + "-" + year;
    } else {
        throw "Invalid date format"
    }

}

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

getSimulationData().then((data) => {
    simulationData = data;

    document.getElementById('dateSlider').max = simulationData.length - 1;

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
//Slider Value Change
d3.select("#dateSlider").on("change", function() {
    loadNewData();
});

function decrementSlider() {
    let prevVal = document.getElementById('dateSlider').value;
    document.getElementById('dateSlider').value = parseInt(prevVal) - 1;
}
function incrementSlider() {
    let prevVal = document.getElementById('dateSlider').value;
    document.getElementById('dateSlider').value = parseInt(prevVal) + 1;
}

// Increment/Decrement Button Press
d3.selectAll(".arrow").on("click", function(){
    //get which button pressed
    var direction = 0;
    if(d3.select(this)._groups[0][0].id == "decrementButton"){
        direction = 0;
        decrementSlider();

    } else if(d3.select(this)._groups[0][0].id == "incrementButton"){
        direction = 1;
        incrementSlider();

    } else {
        throw "unknown button pressed"

    }

    loadNewData();
});

// Date Range New Input
var timelapseToggle = 1;
var intervalId;
d3.select("#timelapseButton").on("click", function() {
    let timeLapse = document.getElementById('timelapseButton');

    if(timelapseToggle){
        let speed = document.getElementById('autoIncrementSpeed').value;
        if (speed === undefined) {
            speed = 1;
        }

        intervalId = setInterval(function() {
            document.getElementById("incrementButton").click();
        }, speed*1000);
        timelapseToggle = 0;

        timeLapse.innerHTML = "<b>&#8545;</b>";
        timeLapse.style.fontFamily = "sans-serif";
        timeLapse.style.fontSize = "larger";

    } else {
        clearInterval(intervalId);

        timeLapse.innerHTML = "&#x23F1;";

        timelapseToggle = 1;
    }
    //needs a stopping function when at end of data
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
    updateMapDateValue(simulationData[document.getElementById('dateSlider').value].date);

    if (document.getElementById('dateSlider').value == simulationData.length-1) {
        clearInterval(intervalId);
        document.getElementById('timelapseButton').innerHTML = "&#x23F1;";
    }
    
    covidStats = simulationData[document.getElementById('dateSlider').value];
    rePlot(covidStats);
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
