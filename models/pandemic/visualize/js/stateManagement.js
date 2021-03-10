// Submitting ConfigAPI Form
d3.select("#submitConfigApi").on("click", function() {
    if (document.getElementById('postSimulationContent').style.display != "none") {
        hideAnimation("postSimulationContent", "0.15");
    }
    hideAnimation("configApi", "0.15");

    document.getElementById('startDate').value = document.getElementById('startdate').value;

    // Form Submission Logic
    handleDefaultConfigValue('runtime_days');
    handleDefaultConfigValue('transmissibilityval');
    handleDefaultConfigValue('exposed_confirmed_ratioval');
    handleDefaultConfigValue('mean_incubation_duration_in_daysval');
    handleDefaultConfigValue('mean_infection_duration_in_daysval');
    handleDefaultConfigValue('mortality_ratioval');
    handleDefaultConfigValue('update_trig_interval_in_hrsval');
    handleDefaultConfigValue('graph_param_Kval');
    handleDefaultConfigValue('graph_param_betaval');
    handleDefaultConfigValue('diffusion_trig_interval_in_hrsval');
    handleDefaultConfigValue('avg_transport_speedval');
    handleDefaultConfigValue('max_diffusion_cntval');

    // Creation of API object to be sent
    var postdata = {
        'start_date' : {
            'value':formatDate(document.getElementById('startdate').value, "MM-DD-YYYY", "html") //document.getElementById('startdate').value
        }, 'runtime_days' : {
            'value':document.getElementById('runtime_days').value
        }, 'transmissibility':{
            'ifchecked':document.getElementById('transmissibility').checked,
            'value':document.getElementById('transmissibilityval').value
        }, 'exposed_confirmed_ratio':{
            'ifchecked':document.getElementById('exposed_confirmed_ratio').checked,
            'value':document.getElementById('exposed_confirmed_ratioval').value
        }, 'mean_incubation_duration_in_days':{
            'ifchecked':document.getElementById('mean_incubation_duration_in_days').checked,
            'value':document.getElementById('mean_incubation_duration_in_daysval').value
        }, 'mean_infection_duration_in_days':{
            'ifchecked':document.getElementById('mean_infection_duration_in_days').checked,
            'value':document.getElementById('mean_infection_duration_in_daysval').value
        }, 'mortality_ratio':{
            'ifchecked':document.getElementById('mortality_ratio').checked,
            'value':document.getElementById('mortality_ratioval').value
        }, 'update_trig_interval_in_hrs':{
            'ifchecked':document.getElementById('update_trig_interval_in_hrs').checked,
            'value':document.getElementById('update_trig_interval_in_hrsval').value//'value':document.getElementById('transmissibilityval').value
        }, 'graph_type':{
            'ifchecked':document.getElementById('graph_type').checked,
            'value':document.getElementById('graph_typeoption').options[document.getElementById('graph_typeoption').selectedIndex].value//'value':graph_type
        }, 'graph_params':{
            'ifchecked':document.getElementById('graph_params').checked,
            'K_val':document.getElementById('graph_param_Kval').value,
            'beta_val':document.getElementById('graph_param_betaval').value
        }, 'diffusion_trig_interval_in_hrs':{
            'ifchecked':document.getElementById('diffusion_trig_interval_in_hrs').checked,
            'value':document.getElementById('diffusion_trig_interval_in_hrsval').value
        }, 'avg_transport_speed':{
            'ifchecked':document.getElementById('avg_transport_speed').checked,
            'value':document.getElementById('avg_transport_speedval').value
        }, 'max_diffusion_cnt':{
            'ifchecked':document.getElementById('max_diffusion_cnt').checked,
            'value':document.getElementById('max_diffusion_cntval').value
        }/*, 'fipslist': {
            'value': document.getElementById('fipslist').value
        }*/
    };

    // console.log(postdata);
    // console.log(JSON.stringify(postdata));
    
    // callSimulate(JSON.stringify(postdata)).then(function(data) { //UNCOMMENT WHEN WORKING
    //     console.log(data);
    // });

    pageState = "viewingMap";
    document.getElementById('map').style.display = "none";
    document.getElementById('plotButton').disabled = false;
    waitForSimulationData();

});

d3.select("#editConfig").on("click", function() {
    showAnimation('configApi', "0.15", "0.00");
    pageState = "editingConfig";
});