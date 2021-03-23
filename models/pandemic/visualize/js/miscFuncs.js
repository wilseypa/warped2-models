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
            // getSimulationData();

            let scriptsArrayMain = ["js/mapPlottingLogic.js"];
            loadJavascriptSource(scriptsArrayMain);
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