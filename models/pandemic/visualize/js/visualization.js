//Highest Level Globals
var isDevEnv = false;
var ip = "0.0.0.0";

//Backend API Functions
async function isDevEnvFunc() {
    const response = await fetch('/isDevEnv');
    const data = await response.json();
    //console.log(data);
    if(data.path === "/work/" || data.path === "/home/") {
        isDevEnv = false;
    } else {
        isDevEnv = true;
    }
    //console.log(isDevEnv);
    return data;
}
async function getClientIP() {
    const ipRes = await fetch('https://ipapi.co/json/', {mode: 'cors'});
    const ipObj = await ipRes.json();
    // console.log(ipObj.ip);
    ip = ipObj.ip;
}
async function loadHtml(fileName, parentElement) {
    var session = sessionStorage.getItem("session");
    const response = await fetch('/loadHtml/' + fileName + "/" + session);
    const data = await response.json();
    // console.log(data.response);
    document.getElementById(parentElement).innerHTML = data.response;
    return data;
}
async function sessionManager() {
    var session = sessionStorage.getItem("session");
    const response = await fetch('/sessionManager/' + session);
    const data = await response.json();
    // console.log(data.response);
    return data;
}
async function getHashFromSession() {
    var session = sessionStorage.getItem("session");
    const response = await fetch('/getHashFromSession/' + session);
    const data = await response.json();
    // console.log(data.response);
    return data;
}
async function sendData() {
    const response = await fetch('/send_data/');
    const data = await response.json();
    console.log(data);
    return data;
}
//sendData();
async function getData(startDate, endDate) {
    const response = await fetch('/pandemic_data/' + startDate + '/' + endDate);
    const data = await response.json();
    //console.log(data);
    return data;
}
async function requestLogin(username, password, ip) {
    const response = await fetch('/login/' + username + '/' + password + "/" + ip);
    const data = await response.json();
    //console.log(data);
    return data;
}
async function callSimulate(simulateJson) {
    //console.log(simulateJson)
    const params = {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: simulateJson
    };
    const response = await fetch('/callSimulate', params);
    const data = await response.json();
    //console.log(data);
    return data;
}
async function callGetstatus() {
    var jobID = sessionStorage.getItem("jobID");
    const response = await fetch('/callGetstatus/' + jobID);
    const data = await response.json();
    //console.log(data);
    return data;
}
async function send_simulated_data() {
    const response = await fetch('/send_simulated_data');
    const data = await response.json();
    //console.log(data);
    return data;
}
async function send_actual_data() {
    const response = await fetch('/send_actual_data');
    const data = await response.json();
    //console.log(data);
    return data;
}
async function send_plot_data() {
    const response = await fetch('/send_plot_data');
    const data = await response.json();
    console.log(data);
    return data;
}

function loadJavascriptSource(filePathArray) {
    try {
        var d3BodyScript = document.createElement('script');
        d3BodyScript.onload = function(eventData) {
            //console.log("onload");
            filePathArray.shift();
            if (filePathArray.length > 0) {
                loadJavascriptSource(filePathArray);
            }
        }
        //console.log("offload")
        d3BodyScript.type = "text/javascript";
        d3BodyScript.src = filePathArray[0];  // Directory root path with respect to "index.html"
        document.body.appendChild(d3BodyScript);
        // return true;
    }
    catch(err) {
        console.log(err);
        return false;
    }
}

sessionStorage.clear();

isDevEnvFunc();//.then(function() {});
getClientIP();

var pageState = "login"; // login, config, viewingMap, editingConfig

document.getElementById('username').focus();

// loadJavascriptSource("js/topLevelFuncs.js");
// loadJavascriptSource("js/loginHandler.js");

var scriptsArrayVisualization = ["js/topLevelFuncs.js", "js/loginHandler.js"];
loadJavascriptSource(scriptsArrayVisualization);

// loadHtml('config.html', 'configHandle')
//     .then(function (){
//         return loadHtml('map.html', 'mapHandle');
//     })
//     .then(function (){
//         loadJavascriptSource("js/main.js");
// });