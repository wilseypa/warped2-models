var tooltip = document.getElementById('tt');
color_wheel = ["#ffcccc", "#ff8080", "#ff0000", "#b30000"];
var count = 0;

var slider = document.getElementById("dateRange");
var dateValue = document.getElementById("dateValue");
//dateValue.innerHTML = slider.value; // Display the default slider value
dateArray = ["7-22-2020", "7-23-2020", "7-24-2020", "7-25-2020"]
dateValue.innerHTML = dateArray[slider.value-1]; // Display the default slider value

window.onmousemove = function (e) { //function gets location of mouse for tooltip
    var x = e.clientX,
        y = e.clientY;
    tooltip.style.top = (y + 20) + 'px';
    tooltip.style.left = (x + 20) + 'px';
};

function plot_data() {
    document.getElementById('map').innerHTML = "";
    document.getElementById('sliderContainer').style.visibility = "visible";
(function() {
    d3.json("../07-22-2020.formatted-JHU-data.json", function(data22) {
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

    d3.queue()
        .defer(d3.json, "../us.json")   //json for country drawing
        //.defer(d3.json, "../07-22-2020.formatted-JHU-data.json") //covidStats
        //.defer(d3.json, "../07-23-2020.formatted-JHU-data.json") //covidStats23
        //.defer(d3.json, "../07-24-2020.formatted-JHU-data.json") //covidStats24
        //.defer(d3.json, "../07-25-2020.formatted-JHU-data.json") //covidStats25
        .await(ready)
        
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
    
    function ready (error, data){   //function ready (error, data, data22, data23, data24, data25){
        
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
        
        slider.oninput = function() {
            dateValue.innerHTML = dateArray[slider.value-1];

            jsonDataFilePathString = "../07-" + ((slider.value-1) + 22).toString() + "-2020.formatted-JHU-data.json"   //"../07-22-2020.formatted-JHU-data.json"
            d3.json(jsonDataFilePathString, function(newData) {
                covidStats = newData;
                rePlot(covidStats);
            });
            //ANY CODE HERE WILL BE EXECUTED BEFORE THE d3.json FUNCTION ABOVE
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
                    return color_wheel[count];
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