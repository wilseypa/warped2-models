document.addEventListener("DOMContentLoaded", function() {

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
                'start_date' : {
                    'value':document.getElementById('startdate').value
                }, 'end_date' : {
                    'value':document.getElementById('enddate').value
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
                    'toval':document.getElementById('mean_incubation_duration_in_daysto').value
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
                .timeout(2500)
                .on('200', function (response) {
                    console.log("this is the response: ");
                    console.log(JSON.parse(response));
                    // console.log(response);
                })
                .go();



        });

});
