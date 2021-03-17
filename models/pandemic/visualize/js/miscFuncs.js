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
function formatDate(dateValue, dateFormat, dateType) {
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

function minimizeMapControlsContent(elId, durationOfAnimation) { //INPUTS MUST BE STRINGS
    document.getElementById(elId).style.animation = "minimizeMapControlsContent " + durationOfAnimation + "s";
    document.getElementById(elId).style.animationFillMode = "forwards";
    setTimeout(function(){ document.getElementById(elId).style.display = "none"; }, durationOfAnimation*1000);
}
function maximizeMapControlContent(elId, durationOfAnimation, delayOfAnimation) { //INPUTS MUST BE STRINGS
    document.getElementById(elId).style.visibility = "hidden";
    document.getElementById(elId).style.display = "block";
    document.getElementById(elId).style.animation = "maximizeMapControlsContent " + durationOfAnimation + "s";
    document.getElementById(elId).style.animationFillMode = "forwards";
    document.getElementById(elId).style.animationDelay = delayOfAnimation + "s";
}

// function fadeOutDisplay(elId, durationOfAnimation) { //INPUTS MUST BE STRINGS
//     document.getElementById(elId).style.animation = "fadeOutDisplay " + durationOfAnimation + "s";
//     document.getElementById(elId).style.animationFillMode = "forwards";
//     setTimeout(function(){ document.getElementById(elId).style.display = "none"; }, durationOfAnimation*1000);
// }
// function fadeInDisplay(elId, durationOfAnimation, delayOfAnimation) { //INPUTS MUST BE STRINGS
//     document.getElementById(elId).style.visibility = "hidden";
//     document.getElementById(elId).style.display = "block";
//     document.getElementById(elId).style.animation = "fadeInDisplay " + durationOfAnimation + "s";
//     document.getElementById(elId).style.animationFillMode = "forwards";
//     document.getElementById(elId).style.animationDelay = delayOfAnimation + "s";
// }

function createLoadingBar() {
    var loadingElHandle = document.createElement("div");
    loadingElHandle.setAttribute("id", "loadingBarHandle");
    document.getElementById("loadingDiv").appendChild(loadingElHandle);

    var loadingEl = document.createElement("div");
    loadingEl.setAttribute("id", "loadingCircle");
    document.getElementById("loadingBarHandle").appendChild(loadingEl);
    let loadingHtml =   `<div class="sk-circle">
                            <div class="sk-circle1 sk-child"></div>
                            <div class="sk-circle2 sk-child"></div>
                            <div class="sk-circle3 sk-child"></div>
                            <div class="sk-circle4 sk-child"></div>
                            <div class="sk-circle5 sk-child"></div>
                            <div class="sk-circle6 sk-child"></div>
                            <div class="sk-circle7 sk-child"></div>
                            <div class="sk-circle8 sk-child"></div>
                            <div class="sk-circle9 sk-child"></div>
                            <div class="sk-circle10 sk-child"></div>
                            <div class="sk-circle11 sk-child"></div>
                            <div class="sk-circle12 sk-child"></div>
                        </div>`
    document.getElementById("loadingBarHandle").innerHTML = loadingHtml;
}
function removeLoadingBar() {
    document.getElementById("loadingBarHandle").remove();
    showAnimation('postSimulationContent', "0.15", "0.15");
}

function handleDefaultConfigValue(inputId) {
    if (document.getElementById(inputId).value === "") {
        document.getElementById(inputId).value = document.getElementById(inputId).getAttribute("placeholder");
    }
}

var getStatus = function () {
    callGetstatus().then(function(data) {    //UNCOMMENT WHEN WORKING
        // console.log(data.statusmsg.status);
        if (data.statusmsg.status == "SUCCESS") {
            removeLoadingBar();
            getSimulationData();
        } else {
            setTimeout(function(){ getStatus(); }, 6000);
        }
    });
}

function waitForSimulationData() {
// Create loading spinner and wait for simulation data
    setTimeout(() => { createLoadingBar(); }, 500);

    getStatus();

    // setTimeout(function(){ removeLoadingBar(); }, 1000);
}