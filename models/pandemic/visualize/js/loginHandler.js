// Submitting Login Form
d3.select("#submitLogin").on("click", function() {
    // if(isDevEnv) {
    //     removeRequiredWarning("requiredWarning");

    //     hideAnimation('loginDiv', "0.15");
    //     showAnimation('passwordProtected', "0.15", "0.15");

    //     pageState = "config";
    //     setTimeout(function(){ document.getElementById('startdate').focus(); }, 500);   // need to wait until animation is complete for keyfocus
    //     return
    // }

    let username = document.getElementById('username').value;
    let password = document.getElementById('password').value;

    if (username === "" || password === "") {
        createRequiredWarning("login-warning-handle", "requiredWarning", "Incorrect username and/or password", "inline-block");

    } else {

        requestLogin(username, password, ip).then(function(response) {
            sessionStorage.setItem("session", response.session);
            setInterval(function(){ sessionManager(); }, 3000); //Time needs to be the same as time on index.js setInterval() func

            if(response.response == "success") {
                removeRequiredWarning("requiredWarning");

                hideAnimation('loginDiv', "0.15");

                pageState = "config";
                loadHtml(response.html, 'configHandle')
                    .then(function (){
                        // return loadHtml('map.html', 'mapHandle');
                    })
                    .then(function (){
                        var scriptsArrayLogin = ["js/miscFuncs.js", "js/stateManagement.js"];
                        loadJavascriptSource(scriptsArrayLogin);
                        showAnimation('passwordProtected', "0.15", "0.15");
                        setTimeout(function(){ document.getElementById('startdate').focus(); }, 500);   // need to wait until animation is complete for keyfocus
                });

            } else {
                createRequiredWarning("login-warning-handle", "requiredWarning", "Incorrect username and/or password", "inline-block");
            }
        });
    }
});