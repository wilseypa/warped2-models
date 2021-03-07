document.addEventListener("DOMContentLoaded", function() {
    var getstatusTimeout = undefined;
    var currJobId = undefined;

    console.log("js loaded");

    document.getElementById("simform").addEventListener('submit',
        function (event) {
            console.log("submit clicked!");
            event.preventDefault();

            var jobId = Array(24).join().replace(/(.|$)/g, function(){return ((Math.random()*36)|0).toString(36);});

            console.log("jobid", jobId)
            var graph_typeobj = document.getElementById('graph_typeoption');
            var graph_type = graph_typeobj.options[graph_typeobj.selectedIndex].value;

            var postdata = {
                'jobid' : jobId,
                'start_date' : {
                    'value':document.getElementById('startdate').value
                }, 'runtime_days' : {
                    'value':document.getElementById('runtime_days').value
                }, 'actualplot_end_date' : {
                    'value':document.getElementById('actualplot_end_date').value
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
                    'value':document.getElementById('transmissibilityval').value
                }, 'graph_type':{
                    'ifchecked':document.getElementById('graph_type').checked,
                    'value':graph_type
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
                }, 'fipslist': {
                    'value': document.getElementById('fipslist').value
                }
            };

            axios.post('/simulate', postdata)
                .then(function (response){
                    console.log("simulate response:", response['data']);

                    currJobId = jobId;
                    // document.getElementById("statusmsg").innerText = response["statusmsg"]

                    if (getstatusTimeout) {
                        clearInterval(getstatusTimeout);
                        getstatusTimeout = setTimeout(getStatus, 200, false);
                    } else {
                        getstatusTimeout = setTimeout(getStatus, 200, false);
                    }
                    // console.log("/simulate: set timeout");
                })
                .catch(function (error) {
                    // console.log("!!!!error", error);
                });

            // aja()
            //     .method('POST')
            //     // .header('Content-Type', 'application/json')
            //     .data({'data':JSON.stringify(postdata)})
            //     .type('json')
            //     .url('/simulate')
            //     .timeout(3000)
            //     .on('200', function (response) {
            //         console.log("simulate response:", response);
            //
            //         currJobId = jobId;
            //         // document.getElementById("statusmsg").innerText = response["statusmsg"]
            //
            //         if (getstatusTimeout) {
            //             clearInterval(getstatusTimeout);
            //             getstatusTimeout = setTimeout(getStatus, 200, false);
            //         } else {
            //             getstatusTimeout = setTimeout(getStatus, 200, false);
            //         }
            //
            //         // console.log("/simulate: set timeout");
            //     })
            //     .go();
        });

    var getStatus = function () {
        clearTimeout(getstatusTimeout);
        getstatusTimeout = setTimeout(getStatus, 6000, false);

        if (currJobId == undefined) {
            return;
        }

        axios.post('/getstatus', {'jobid' : currJobId})
            .then(function (response){
                console.log("getstatus response:", response['data']);

                jobstatus = response['data']['status']
                document.getElementById("statusmsg").innerText = jobstatus;

                if (jobstatus == 'SUCCESS' || jobstatus == 'FAILURE') {
                    console.log("!!!!setting currjobid to undefined");
                    currJobId = undefined;
                }
            })
            .catch(function (error) {
                console.log("!!!!error", error);
            });

        // main logic
        // aja()
        //     .method('POST')
        //     // .header('Content-Type', 'application/json')
        //     // .data({'data':JSON.stringify(postdata)})
        //     .data({'jobid':currJobId})
        //     .url('/getstatus')
        //     .timeout(2500)
        //     .on('200', function (response) {
        //         if ("statusmsg" in response) {
        //             document.getElementById("statusmsg").innerText = response["statusmsg"]
        //         }
        //     })
        //     .go();
    }

    getStatus();
});