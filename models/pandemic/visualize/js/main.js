/* 
Order shouldn't matter in the following imports...
Any code that runs on load of file should be handled on highest possible scope.
Any global values needed should be defined in globals file.
*/

// loadJavascriptSource("js/globals.js")
// loadJavascriptSource("js/miscFuncs.js");
// loadJavascriptSource("js/mapPlottingLogic.js");
// loadJavascriptSource("js/stateManagement.js");

// let scriptsArrayMain = ["js/globals.js", "js/miscFuncs.js", "js/mapPlottingLogic.js", "js/stateManagement.js"];
let scriptsArrayMain = ["js/miscFuncs.js", "js/stateManagement.js"];
loadJavascriptSource(scriptsArrayMain);