document.addEventListener("DOMContentLoaded", function() {

    var getstatusTimeout = undefined;
    var jobid_prefix = "job_";

    console.log("js loaded");

    document.getElementById("simform").addEventListener('submit',
        function (event) {
            console.log("submit clicked!");
            event.preventDefault();

            var graph_typeobj = document.getElementById('graph_typeoption');
            var graph_type = graph_typeobj.options[graph_typeobj.selectedIndex].value;
            // var graph_type2obj = document.getElementById('graph_type2');
            // var graph_type2 = graph_type2obj.options[graph_type2obj.selectedIndex].value;

            var postdata = {
                'sim_result_file': {
                    'value':document.getElementById('sim_result_file').value
                }, 'start_date' : {
                    'value':document.getElementById('startdate').value
                }, 'runtime_days' : {
                    'value':document.getElementById('runtime_days').value
                }, /*'tweakscount' : {
                    'value':document.getElementById('tweakscount').value
                },*/'transmissibility':{
                    'ifchecked':document.getElementById('transmissibility').checked,
                    'fromval':document.getElementById('transmissibilityfrom').value,
                    'toval':document.getElementById('transmissibilityto').value,
                    'step':document.getElementById('transmissibilitystep').value
                }, 'mean_incubation_duration_in_days' : {
                    'ifchecked':document.getElementById('mean_incubation_duration_in_days').checked,
                    'fromval':document.getElementById('mean_incubation_duration_in_daysfrom').value,
                    'toval':document.getElementById('mean_incubation_duration_in_daysto').value,
                    'step':document.getElementById('mean_incubation_duration_in_daysstep').value
                }, 'mean_infection_duration_in_days' : {
                    'ifchecked':document.getElementById('mean_infection_duration_in_days').checked,
                    'fromval':document.getElementById('mean_infection_duration_in_daysfrom').value,
                    'toval':document.getElementById('mean_infection_duration_in_daysto').value,
                    'step':document.getElementById('mean_infection_duration_in_daysstep').value
                }, 'mortality_ratio' : {
                    'ifchecked':document.getElementById('mortality_ratio').checked,
                    'fromval':document.getElementById('mortality_ratiofrom').value,
                    'toval':document.getElementById('mortality_ratioto').value,
                    'step':document.getElementById('mortality_ratiostep').value
                }, 'update_trig_interval_in_hrs' : {
                    'ifchecked':document.getElementById('update_trig_interval_in_hrs').checked,
                    'fromval':document.getElementById('update_trig_interval_in_hrsfrom').value,
                    'toval':document.getElementById('update_trig_interval_in_hrsto').value,
                    'step':document.getElementById('update_trig_interval_in_hrsstep').value
                }, 'graph_type' : {
                    'ifchecked':document.getElementById('graph_type').checked,
                    'type':graph_type
                    // 'type1count':document.getElementById('graph_type1count').value,
                    // 'type2':graph_type2,
                    // 'type2count':document.getElementById('graph_type2count').value,
                }, 'graph_params' : {
                    'ifchecked':document.getElementById('graph_params').checked,
                    'K_fromval':document.getElementById('graph_param_Kfrom').value,
                    'K_toval':document.getElementById('graph_param_Kto').value,
                    'K_step':document.getElementById('graph_param_Kstep').value,
                    'beta_fromval':document.getElementById('graph_param_betafrom').value,
                    'beta_toval':document.getElementById('graph_param_betato').value,
                    'beta_step':document.getElementById('graph_param_betastep').value
                }, 'diffusion_trig_interval_in_hrs' : {
                    'ifchecked':document.getElementById('diffusion_trig_interval_in_hrs').checked,
                    'fromval':document.getElementById('diffusion_trig_interval_in_hrsfrom').value,
                    'toval':document.getElementById('diffusion_trig_interval_in_hrsto').value,
                    'step':document.getElementById('diffusion_trig_interval_in_hrsstep').value
                }, 'avg_transport_speed' : {
                    'ifchecked':document.getElementById('avg_transport_speed').checked,
                    'fromval':document.getElementById('avg_transport_speedfrom').value,
                    'toval':document.getElementById('avg_transport_speedto').value,
                    'step':document.getElementById('avg_transport_speedstep').value
                }, 'max_diffusion_cnt' : {
                    'ifchecked':document.getElementById('max_diffusion_cnt').checked,
                    'fromval':document.getElementById('max_diffusion_cntfrom').value,
                    'toval':document.getElementById('max_diffusion_cntto').value,
                    'step':document.getElementById('max_diffusion_cntstep').value
                }, 'distmetrictype' : {
                    'ifchecked':document.getElementById('distmetric').checked,
                    'ifmetricwasschecked':document.getElementById('distmetricwass').checked,
                    'ifmetricjenshanchecked':document.getElementById('distmetricjenshan').checked,
                    'ifmetriceucdchecked':document.getElementById('eucd').checked
                }
            }

            aja()
                .method('POST')
                // .header('Content-Type', 'application/json')
                .data({'data':JSON.stringify(postdata)})
                .url('/simulate')
                .timeout(3000)
                .on('200', function (response) {
                    console.log("simulate response:", response);

                    document.getElementById("statusmsg").innerText = response["statusmsg"]

                    if (getstatusTimeout) {
                        clearInterval(getstatusTimeout);
                        getstatusTimeout = setTimeout(getStatus, 200, false);
                    } else {
                        getstatusTimeout = setTimeout(getStatus, 200, false);
                    }

                    // console.log("/simulate: set timeout");
                })
                .go();
        });


    var getStatus = function (ifFetchAll) {
        clearTimeout(getstatusTimeout);
        getstatusTimeout = setTimeout(getStatus, 4000, false);
        // console.log("/getstatus cleared and set timeout");

        // main logic
        aja()
            .method('GET')
            // .header('Content-Type', 'application/json')
            // .data({'data':JSON.stringify(postdata)})
            .url('/getstatus')
            .timeout(2500)
            .on('200', function (response) {
                // console.log("getstatus resp:", response);
                // return;

                if ("statusmsg" in response) {
                    document.getElementById("statusmsg").innerText = response["statusmsg"]
                }

                var respJobstatus = response["jobstatus"];

                for (var jobid in respJobstatus) {

                    idStr = jobid_prefix + jobid.toString();

                    if (ifFetchAll == false && respJobstatus[jobid]["updatebanner"] == "no") {
                        continue;
                    }

                    console.log("adding updating banner");

                    var bannerObj = document.getElementById(idStr);

                    if (bannerObj) {
                        bannerObj.removeAttribute("class");
                        bannerObj.classList.add("toast");
                    } else {
                        // banner doesn't exist; create one
                        bannerObj = document.createElement('div');
                        bannerObj.setAttribute("id", jobid_prefix + jobid.toString());
                        bannerObj.classList.add('toast');
                        document.getElementById("jobbanners").prepend(bannerObj);
                    }

                    if (respJobstatus[jobid]["status"] == "success") {
                        bannerObj.classList.add("toast-success");
                        // bannerObj.innerText = response[jobid]["bannertext"];
                        bannerObj.innerHTML = respJobstatus[jobid]["bannertext"]; // HACK

                        // file download link
                        var fileDownload_a = document.createElement('a');
                        fileDownload_a.setAttribute("href", respJobstatus[jobid]["filename"]);
                        fileDownload_a.appendChild(document.createTextNode(respJobstatus[jobid]["filename"]));
                        bannerObj.appendChild(fileDownload_a);
                    } else if (respJobstatus[jobid]["status"] == "failure") {
                        bannerObj.classList.add("toast-error");
                        // bannerObj.innerText = response[jobid]["bannertext"];
                        bannerObj.innerHTML = respJobstatus[jobid]["bannertext"];
                    } else if (respJobstatus[jobid]["status"] == "running") {
                        bannerObj.classList.add("toast-primary");
                        // bannerObj.innerText = response[jobid]["bannertext"];
                        bannerObj.innerHTML = respJobstatus[jobid]["bannertext"];
                    }

                }

            })
            .go();
    };

    getStatus(true);
});