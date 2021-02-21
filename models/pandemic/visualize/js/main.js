loadJavascriptSource("js/globals.js");

/* 
Order shouldn't matter in the following imports...
Any code that runs on load of file should be handled on highest possible scope.
Any global values needed should be defined in globals file.
*/

loadJavascriptSource("js/miscFuncs.js");
loadJavascriptSource("js/mapPlottingLogic.js");
loadJavascriptSource("js/stateManagement.js");