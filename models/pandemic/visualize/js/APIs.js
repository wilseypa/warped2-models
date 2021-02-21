// CURRENTLY NOT USED -- NOT WORKING WHEN TESTED AS SEPARATE FILE

//Backend Functions
var isDevEnv = false;
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

async function loadHtml(fileName, parentElement) {
    const response = await fetch('/loadHtml/' + fileName);
    const data = await response.json();
    // console.log(data.response);
    document.getElementById(parentElement).innerHTML = data.response;
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

async function requestLogin(username, password) {
    const response = await fetch('/login/' + username + '/' + password);
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
    const response = await fetch('/callGetstatus');
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