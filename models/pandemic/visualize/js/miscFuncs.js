// function minimizeMapControlsContent(elId, durationOfAnimation) { //INPUTS MUST BE STRINGS
//     document.getElementById(elId).style.animation = "minimizeMapControlsContent " + durationOfAnimation + "s";
//     document.getElementById(elId).style.animationFillMode = "forwards";
//     setTimeout(function(){ document.getElementById(elId).style.display = "none"; }, durationOfAnimation*1000);
// }
// function maximizeMapControlContent(elId, durationOfAnimation, delayOfAnimation) { //INPUTS MUST BE STRINGS
//     document.getElementById(elId).style.visibility = "hidden";
//     document.getElementById(elId).style.display = "block";
//     document.getElementById(elId).style.animation = "maximizeMapControlsContent " + durationOfAnimation + "s";
//     document.getElementById(elId).style.animationFillMode = "forwards";
//     document.getElementById(elId).style.animationDelay = delayOfAnimation + "s";
// }

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