// Mapping enter to submit buttons without using forms
document.addEventListener("keyup", event => {
    if (event.key !== "Enter") return; // Use `.key` instead.
    
    let submitLogin = document.getElementById("submitLogin");
    let submitConfigApi = document.getElementById("submitConfigApi");

    if (submitLogin && pageState === "login") {
        submitLogin.click();
    } else if (submitConfigApi && (pageState === "config" || pageState == "editingConfig")) {
        submitConfigApi.click();
    }
});

function createRequiredWarning(elToAttachTo, warningId, warningMsg, display) {
    var warningEl = document.createElement("div");
        warningEl.setAttribute("id", warningId);
        document.getElementById(elToAttachTo).appendChild(warningEl);
        document.getElementById(warningId).innerHTML = warningMsg;
        document.getElementById(warningId).className = "center";
        document.getElementById(warningId).style.color = "#ff0033";
        document.getElementById(warningId).style.width = "fit-content";
        document.getElementById(warningId).style.display = display;
        document.getElementById(warningId).style.marginTop = "4px";
        document.getElementById(warningId).style.paddingLeft = "5px";
        document.getElementById(warningId).style.paddingRight = "5px";
        document.getElementById(warningId).style.paddingTop = "2px";
        document.getElementById(warningId).style.paddingBottom = "2px";
        document.getElementById(warningId).style.borderRadius = "7px";
        document.getElementById(warningId).style.backgroundColor = "#f2dede";
        document.getElementById(warningId).style.border = "solid 1px #ebccd1";
}
function removeRequiredWarning(warningId) {
    if (document.getElementById(warningId) != null) {
        document.getElementById(warningId).remove();
    } else {
        // Doesn't exist, can't remove -> do nothing
    }
}

function hideAnimation(elId, durationOfAnimation) { //INPUTS MUST BE STRINGS
    document.getElementById(elId).style.animation = "hide " + durationOfAnimation + "s";
    document.getElementById(elId).style.animationFillMode = "forwards";
    setTimeout(function(){ document.getElementById(elId).style.display = "none"; }, durationOfAnimation*1000);
}
function showAnimation(elId, durationOfAnimation, delayOfAnimation) { //INPUTS MUST BE STRINGS
    document.getElementById(elId).style.visibility = "hidden";
    document.getElementById(elId).style.display = "block";
    document.getElementById(elId).style.animation = "show " + durationOfAnimation + "s";
    document.getElementById(elId).style.animationFillMode = "forwards";
    document.getElementById(elId).style.animationDelay = delayOfAnimation + "s";
}