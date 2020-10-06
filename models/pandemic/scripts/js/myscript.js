document.addEventListener("DOMContentLoaded", function() {

    var funcTimeout = undefined;

    console.log("js loaded");

    document.getElementById("simform").addEventListener('submit',
        function (event) {
            console.log("submit clicked!");
            event.preventDefault();

            var graph_type1obj = document.getElementById('graph_type1');
            var graph_type1 = graph_type1obj.options[graph_type1obj.selectedIndex].value;
            var graph_type2obj = document.getElementById('graph_type2');
            var graph_type2 = graph_type2obj.options[graph_type2obj.selectedIndex].value;

            var postdata = {
                'sim_result_file': {
                    'value':document.getElementById('sim_result_file').value
                }, 'start_date' : {
                    'value':document.getElementById('startdate').value
                }, 'runtime_days' : {
                    'value':document.getElementById('runtime_days').value
                }, 'tweakscount' : {
                    'value':document.getElementById('tweakscount').value
                }, 'transmissibility':{
                    'ifchecked':document.getElementById('transmissibility').checked,
                    'fromval':document.getElementById('transmissibilityfrom').value,
                    'toval':document.getElementById('transmissibilityto').value
                }, 'mean_incubation_duration_in_days' : {
                    'ifchecked':document.getElementById('mean_incubation_duration_in_days').checked,
                    'fromval':document.getElementById('mean_incubation_duration_in_daysfrom').value,
                    'toval':document.getElementById('mean_incubation_duration_in_daysto').value
                }, 'mean_infection_duration_in_days' : {
                    'ifchecked':document.getElementById('mean_infection_duration_in_days').checked,
                    'fromval':document.getElementById('mean_infection_duration_in_daysfrom').value,
                    'toval':document.getElementById('mean_infection_duration_in_daysto').value
                }, 'mortality_ratio' : {
                    'ifchecked':document.getElementById('mortality_ratio').checked,
                    'fromval':document.getElementById('mortality_ratiofrom').value,
                    'toval':document.getElementById('mortality_ratioto').value
                }, 'update_trig_interval_in_hrs' : {
                    'ifchecked':document.getElementById('update_trig_interval_in_hrs').checked,
                    'fromval':document.getElementById('update_trig_interval_in_hrsfrom').value,
                    'toval':document.getElementById('update_trig_interval_in_hrsto').value
                }, 'graph_type' : {
                    'ifchecked':document.getElementById('graph_type').checked,
                    'type1':graph_type1,
                    'type1count':document.getElementById('graph_type1count').value,
                    'type2':graph_type2,
                    'type2count':document.getElementById('graph_type2count').value,
                }, 'graph_params' : {
                    'ifchecked':document.getElementById('graph_params').checked,
                    'fromval':document.getElementById('graph_paramsfrom').value,
                    'toval':document.getElementById('graph_paramsto').value
                }, 'diffusion_trig_interval_in_hrs' : {
                    'ifchecked':document.getElementById('diffusion_trig_interval_in_hrs').checked,
                    'fromval':document.getElementById('diffusion_trig_interval_in_hrsfrom').value,
                    'toval':document.getElementById('diffusion_trig_interval_in_hrsto').value
                }, 'avg_transport_speed' : {
                    'ifchecked':document.getElementById('avg_transport_speed').checked,
                    'fromval':document.getElementById('avg_transport_speedfrom').value,
                    'toval':document.getElementById('avg_transport_speedto').value
                }, 'max_diffusion_cnt' : {
                    'ifchecked':document.getElementById('max_diffusion_cnt').checked,
                    'fromval':document.getElementById('max_diffusion_cntfrom').value,
                    'toval':document.getElementById('max_diffusion_cntto').value
                }, 'distmetrictype' : {
                    'ifchecked':document.getElementById('distmetric').checked,
                    'ifmetricwasschecked':document.getElementById('distmetricwass').checked,
                    'ifmetricshajenchecked':document.getElementById('distmetricshajen').checked
                }
            }

            aja()
                .method('POST')
                // .header('Content-Type', 'application/json')
                .data({'data':JSON.stringify(postdata)})
                .url('/simulate')
                .timeout(3000)
                .on('200', function (response) {
                    console.log("this is the response: ");
                    // console.log(typeof response);
                    // console.log(JSON.parse(response));
                    console.log(response);

                    // var newToastElement = document.createElement('div');

                    // newToastElement.classList.add('toast');
                    // newToastElement.classList.add('toast-primary');

                    // add id
                    // newToastElement.setAttribute("id", "job" + response["jobid"].toString());

                    // newToastElement.innerText = response["bannertext"];

                    // document.getElementById("wrapper").appendChild(newToastElement);

                    if (funcTimeout) {
                        clearInterval(funcTimeout);
                        funcTimeout = setTimeout(getStatus, 200, false);
                    } else {
                        funcTimeout = setTimeout(getStatus, 200, false);
                    }

                    console.log("/simulate: set timeout");
                })
                .go();
        });


    var getStatus = function (ifFetchAll) {
        clearTimeout(funcTimeout);
        funcTimeout = setTimeout(getStatus, 4000, false);
        console.log("/getstatus cleared and set timeout");

        // main logic
        aja()
            .method('GET')
            // .header('Content-Type', 'application/json')
            // .data({'data':JSON.stringify(postdata)})
            .url('/getstatus')
            .timeout(2500)
            .on('200', function (response) {
                console.log("getstatus:", response);

                for (var jobid in response) {

                    if (!response.hasOwnProperty(jobid)) {
                        continue;
                    }

                    idStr = "job" + jobid.toString();

                    if (ifFetchAll == false && response[jobid]["updatebanner"] == "no") {
                        continue;
                    }

                    var bannerObj = document.getElementById(idStr);

                    if (bannerObj) {
                        bannerObj.removeAttribute("class");
                        bannerObj.classList.add("toast");
                    } else {
                        // banner doesn't exist
                        bannerObj = document.createElement('div');
                        bannerObj.setAttribute("id", "job" + jobid.toString());
                        bannerObj.classList.add('toast');
                        document.getElementById("wrapper").appendChild(bannerObj);
                    }

                    if (response[jobid]["status"] == "success") {
                        bannerObj.classList.add("toast-success");
                        bannerObj.innerText = response[jobid]["bannertext"];

                        var fileDownload_a = document.createElement('a');
                        fileDownload_a.setAttribute("href", response[jobid]["filename"]);
                        fileDownload_a.appendChild(document.createTextNode(response[jobid]["filename"]));
                        bannerObj.appendChild(fileDownload_a);
                    } else if (response[jobid]["status"] == "running") {
                        bannerObj.classList.add("toast-primary");
                        bannerObj.innerText = response[jobid]["bannertext"];
                    } else if (response[jobid]["status"] == "failure") {
                        bannerObj.classList.add("toast-error");
                        bannerObj.innerText = response[jobid]["bannertext"];
                    }

                }

            })
            .go();
    };

    getStatus(true);
});
