const path = require('path');
const http = require('http');
const fs = require('fs');
const crypto = require('crypto');
// const request = require('request');
const axios = require('axios');
const querystring = require('querystring');

const directoryPath = path.join(__dirname, 'visualize', 'mongodb',  'Pandemic_Data');
const Datastore = require('nedb');

const express = require('express');
const { config } = require('process');
const app = express();
app.listen(3000, () => console.log('listening at port 3000'));
app.use(express.static('../../../visualize'));

app.use(express.json({}));	//NEEDED FOR req.body TO PRINT

const database = new Datastore('database.db');
database.loadDatabase();

var sessions = new Map();
setInterval(function(){
	const iter = sessions[Symbol.iterator]();

	for (var [key, value] of iter) {
		let currentCount = value.count;
		if (currentCount == 0) {
			sessions.delete(key);
		} else {
			sessions.set(key, {count: currentCount-1});
			// console.log(sessions);
		}
	}
}, 3000);	//Time needs to be the same as time on loginHandler.js setInterval() func

function formatDate(dateValue, dateFormat) {
        let year = dateValue.getFullYear();
        let month = dateValue.getMonth() + 1;
        let day = dateValue.getDate()

        if(month < 10) {
            month = "0" + month;
        }
        if(day < 10) {
            day = "0" + day;
        }

        if(dateFormat == "YYYY-MM-DD") {
            return year + "-" + month + "-" + day;
        } else if(dateFormat == "MM-DD-YYYY") {
            return month + "-" + day + "-" + year;
        } else {
            throw "Invalid date format"
        }
}

function getHtmlTemplate(filePath) {
	return fs.readFileSync(filePath).toString();
}

function getHash(ip) {
	dateTimeString = Date.now();
    let hashInput = ip + " " + dateTimeString;
    let hash = crypto.createHash('md5').update(hashInput).digest('hex');
    return hash;
}

app.get('/loadHtml/:fileName/:session', (request, response) => {
	// var filePath = path.join(__dirname + '../../../test.html');
	// response.sendFile(filePath);
	if (request.params.session != undefined) {
		if (sessions.has(request.params.session)) {
			// console.log(session);
			const fileContents = getHtmlTemplate('../../html/' + request.params.fileName);
			response.status(200).send({response: fileContents});
		} else {
			response.status(401).send({response: "failed"});
		}
	} else {
		response.status(401).send({response: "failed"});
	}
});

app.get('/login/:username/:password/:ip', (request, response) => {
	let password = request.params.password;
	let username = request.params.username;
	// console.log(username);
	// console.log(password);

	if (username == 'admin' && password == 'warped2') {
		// console.log(sessions);
		let userSession = getHash(request.params.ip);
		sessions.set(userSession, {count: 5});
		// console.log(sessions);
		// console.log('Good');
		const fileContents = getHtmlTemplate('../../html/' + "config.html");
		response.status(200).send({response: "success", html: "config.html", session: userSession});

	}
	else {
		// console.log('Bad');
		// session = undefined;
		response.status(401).send({response: "failed", html: ""});
	}
});

app.get('/sessionManager/:session', (request, response) => {
	if (sessions.has(request.params.session)) {
		let currentCount = sessions.get(request.params.session).count;
		// console.log(currentCount);
		sessions.set(request.params.session, {count: currentCount+1});
		// console.log(sessions);
		response.status(200).send({response: "success"});
	} else {
		response.status(401).send({response: "failed"});
	}
});

app.get('/getHash/:ip', (request, response) => {
	dateTimeString = new Date().toLocaleString()
    let hashInput = request.params.ip + " " + dateTimeString;
    let hash = crypto.createHash('md5').update(hashInput).digest('hex');
	response.status(200).send({string: hash});
});

app.get('/getHashFromSession/:session', (request, response) => {
	dateTimeString = new Date().toLocaleString()
    let hashInput = request.params.session + " " + dateTimeString;
    let hash = crypto.createHash('md5').update(hashInput).digest('hex');
	response.status(200).send({jobID: hash});
});

app.get('/isDevEnv', (request, response) => {
	const envPath = path.join(__dirname, '../../../../../../../');

	//console.log(envPath)
	response.status(200).send({path: envPath});
});

app.post('/callSimulate', (req, res) => {
//   console.log(typeof(JSON.stringify(req.body)));
	// console.log(JSON.stringify(req.body));

	// const options = {
	// 	method: 'post',
	// 	url: 'http://localhost:8081/simulate',
	// 	headers: {'Content-Type': 'application/json;charset=UTF-8'},
	// 	// auth: {username: "scooby", password: "doobyd00"},
	// 	data: {
	// 		"jobid":"xshe7i9ieg2iy90yrb9ow9343",
	// 		"start_date":{"value":"07-01-2020"},
	// 		"runtime_days":{"value":"4"},
	// 		"actualplot_end_date":{"value":""},
	// 		"transmissibility":{"ifchecked":false,"value":"2.0"},
	// 		"exposed_confirmed_ratio":{"ifchecked":false,"value":""},
	// 		"mean_incubation_duration_in_days":{"ifchecked":false,"value":""},
	// 		"mean_infection_duration_in_days":{"ifchecked":false,"value":""},
	// 		"mortality_ratio":{"ifchecked":false,"value":""},
	// 		"update_trig_interval_in_hrs":{"ifchecked":false,"value":""},
	// 		"graph_type":{"ifchecked":false,"value":"Watts-Strogatz"},
	// 		"graph_params":{"ifchecked":false,"K_val":"","beta_val":""},
	// 		"diffusion_trig_interval_in_hrs":{"ifchecked":false,"value":""},
	// 		"avg_transport_speed":{"ifchecked":false,"value":""},
	// 		"max_diffusion_cnt":{"ifchecked":false,"value":""},
	// 		"fipslist":{"value":""}
	// 	}//,
	// 	// transformRequest: [(data, headers) => {
	// 	//   // transform the data
	  
	// 	//   return data;
	// 	// }]
	//   };

	var postParams = req.body;
	const options = {
		method: 'post',
		url: 'http://localhost:8081/simulate',
		headers: {'Content-Type': 'application/json;charset=UTF-8'},
		// auth: {username: "scooby", password: "doobyd00"},
		data: req.body
	  };
	  
	  // send the request
	  axios(options).then(function (response) {
		//   console.log(response);
		res.status(200).send({response: response.data.statusmsg});
	});;
})

app.get('/callGetstatus/:jobID', (req, res) => {
	// const options = {
	// 	method: 'post',
	// 	url: 'http://localhost:8081/getstatus',
	// 	headers: {'Content-Type': 'application/json;charset=UTF-8'},
	// 	// auth: {username: "scooby", password: "doobyd00"},
	// 	data: {
	// 		"jobid":"xshe7i9ieg2iy90yrb9ow9343"
	// 	}//,
	// 	// transformRequest: [(data, headers) => {
	// 	//   // transform the data
	  
	// 	//   return data;
	// 	// }]
	//   };

	const options = {
		method: 'post',
		url: 'http://localhost:8081/getstatus',
		headers: {'Content-Type': 'application/json;charset=UTF-8'},
		// auth: {username: "scooby", password: "doobyd00"},
		data: {
			"jobid":req.params.jobID
		}
	  };
	  
	  // send the request
	  axios(options).then(function (response) {
		//   console.log(response.data.statusmsg);
		  res.status(200).send({statusmsg: response.data});
	  });
});

app.get('/send_simulated_data', (request, response) => {
	// Function call to send back files from Vivek's directory to our frontend
	// const vivek_path = '/work/vivek/warped2-models/warped2-models/models/pandemic/scripts/tuningapp/simOutfiles';
	const vivek_path =  '/work/vivek/warped2/warped2-models/models/pandemic/scripts/tuningapp/simOutfiles.bkp';
	const data_folder2 = path.join(__dirname, '../../../../../../../../../', vivek_path);

	let responseArray = [];
	fs.readdir(data_folder2, (err, files) => {
		files.forEach(file => {
			/// console.log(file);
			let extension = path.extname(file);
			if (extension == '.json') {
				let filepath = path.join(data_folder2, file);
				let rawdata = fs.readFileSync(filepath);
				let pandemic_data = JSON.parse(rawdata);
				responseArray.push(pandemic_data);
			}

		});
		response.json(responseArray);
	});
});

app.get('/getSimulationData/:jobID', (request, response) => {

	// const sim_dec = '12-31-2020.simulated-data.json';
	// const simJob_path = 'simJob_' + request.params.jobID + '/' + 'simOutfiles/' + '07-23-2020.simulated-data.json';
	const simJob_path = 'simJob_' + request.params.jobID + '/' + 'simOutfiles';
	
	const vivek_path = '/work/vivek/warped2/warped2-models/models/pandemic/scripts/tuningapp/simJobs';
	const data_folder = path.join(__dirname, '../../../../../../../../../', vivek_path);

	const simJob_folder = path.join(data_folder, simJob_path);
		
	var responseArray = [];
	fs.readdir(simJob_folder, (err, files) => {
		files.forEach(function(file, index) {
			//console.log(simJob_folder + '/' + file);
			let raw_data = fs.readFileSync(simJob_folder + '/' + file);
			//console.log(raw_data);
		
			let parsed_data = JSON.parse(raw_data);
			//console.log(parsed_data);

			responseArray.push(parsed_data);
			//console.log(responseArray.length);
			if (index == files.length-1) {
				response.json(responseArray);
			}
		});
	  });
	//response.json(responseArray);

});

app.get('/send_plot_data', (request, response) => {

	const sim_dec = '12-31-2020.simulated-data.json';
	const act_dec = '12-31-2020.formatted-JHU-data.json';

	const sim_jan = '01-31-2021.simulated-data.json';
	const act_jan = '01-31-2021.formatted-JHU-data.json';

	const sim_feb = '02-28-2021.simulated-data.json';

	const sim_mar = '03-31-2021.simulated-data.json';

	const sim_feb1 = '02-01-2021.simulated-data.json';
	const act_feb1 = '02-01-2021.formatted-JHU-data.json';
	const sim_mar1 = '03-01-2021.simulated-data.json';
	
	
	const vivek_actual_path = '/work/vivek/warped2/warped2-models/models/pandemic/data';
	const vivek_simulated_path = '/work/vivek/warped2/warped2-models/models/pandemic/scripts/tuningapp/simOutfiles.dec1_mar31_exponentfactor48';
	const actual_data_folder = path.join(__dirname, '../../../../../../../../../', vivek_actual_path);
	const simulated_data_folder = path.join(__dirname, '../../../../../../../../../', vivek_simulated_path);

	const december_simulated = path.join(simulated_data_folder, sim_dec);
	const december_actual = path.join(actual_data_folder, act_dec);

	const january_simulated = path.join(simulated_data_folder, sim_jan);
	const january_actual = path.join(actual_data_folder, act_jan);

	const february_simulated = path.join(simulated_data_folder, sim_feb);

	const march_simulated = path.join(simulated_data_folder, sim_mar);

	const february1_simulated = path.join(simulated_data_folder, sim_feb1);
	const february1_actual = path.join(actual_data_folder, act_feb1);
	const march1_simulated = path.join(simulated_data_folder, sim_mar1);


	let raw_dec_sim = fs.readFileSync(december_simulated);
	let raw_dec_act = fs.readFileSync(december_actual);

	let raw_jan_sim = fs.readFileSync(january_simulated);
	let raw_jan_act = fs.readFileSync(january_actual);

	let raw_feb_sim = fs.readFileSync(february_simulated);
	
	let raw_mar_sim = fs.readFileSync(march_simulated);

	let raw_feb1_sim = fs.readFileSync(february1_simulated);
	let raw_feb1_act = fs.readFileSync(february1_actual);
	let raw_mar1_sim = fs.readFileSync(march1_simulated);



	let data_dec_sim = JSON.parse(raw_dec_sim);
	let data_dec_act = JSON.parse(raw_dec_act);

	let data_jan_sim = JSON.parse(raw_jan_sim);
	let data_jan_act = JSON.parse(raw_jan_act);

	let data_feb_sim = JSON.parse(raw_feb_sim);

	let data_mar_sim = JSON.parse(raw_mar_sim);

	let data_feb1_sim = JSON.parse(raw_feb1_sim);
	let data_feb1_act = JSON.parse(raw_feb1_act);
	let data_mar1_sim = JSON.parse(raw_mar1_sim);

	let responseArray = [data_dec_sim, data_dec_act, data_jan_sim, data_jan_act, data_feb_sim, data_mar_sim, data_feb1_sim, data_feb1_act, data_mar1_sim];
	response.json(responseArray);


});

app.get('/send_actual_data', (request, response) => {
	// Function call to send back files from Vivek's directory to our frontend
	const vivek_path = '/work/vivek/warped2/warped2-models/models/pandemic/data';
	const data_folder2 = path.join(__dirname, '../../../../../../../../../', vivek_path);

	let responseArray = [];
	fs.readdir(data_folder2, (err, files) => {
		files.forEach(file => {
			// console.log(file);
			let extension = path.extname(file);
			if (extension == '.json') {
				let filepath = path.join(data_folder2, file);
				let rawdata = fs.readFileSync(filepath);
				let pandemic_data = JSON.parse(rawdata);
				responseArray.push(pandemic_data);
			}
		});
		response.json(responseArray);
	});
});


app.get('/pandemic_data/:start_date/:end_date', (request, response) => {
	var startDate = new Date(request.params.start_date);
	var endDate = new Date(request.params.end_date);
	var millisecond_to_day = 86400000
	var number_of_days = ((endDate - startDate) / millisecond_to_day);

	let responseArray = [];
	var responses = 0;
	
	for(i = 0; i <= number_of_days; i++) {
		formatted_date = formatDate(startDate, "MM-DD-YYYY");
		database.find({"date": formatted_date}, (err, data) => {
			if (err) {
				response.end();
				return;
			}
			responseArray.push(data);
			responses += 1;
			if (responses > number_of_days) {
				response.json(responseArray);
				return;
			}	
		});
		startDate.setDate(startDate.getDate() + 1);
	}
});
