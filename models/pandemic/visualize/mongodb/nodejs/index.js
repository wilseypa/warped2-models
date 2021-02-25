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

function getIpInfo() {
  axios.get('https://ipinfo.io')
  .then((response) => {
    // console.log(response.data.ip);
	return response.data.ip;
  });
}

function getHash(string) {
    let hash = crypto.createHash('md5').update(string).digest('hex');
    return hash;
}

app.get('/loadHtml/:fileName', (request, response) => {
	// var filePath = path.join(__dirname + '../../../test.html');
	// response.sendFile(filePath);

	const fileContents = getHtmlTemplate('../../html/' + request.params.fileName);
	response.status(200).send({response: fileContents});
});

app.get('/login/:username/:password', (request, response) => {
	let password = request.params.password;
	let username = request.params.username;
	// console.log(username);
	// console.log(password);

	if (username == 'admin' && password == 'warped2') {
		// console.log('Good');
		const fileContents = getHtmlTemplate('../../html/' + "config.html");
		response.status(200).send({response: "success", html: "config.html"});

	}
	else {
		// console.log('Bad');
		response.status(401).send({response: "failed", html: ""});
	}
});

app.get('/getHash/:string', (request, response) => {
	// let name = getIpInfo();
    let name = request.params.string;
    let hash = crypto.createHash('md5').update(name).digest('hex');

    var ip = request.headers['x-forwarded-for'] ||
        request.connection.remoteAddress ||
        request.socket.remoteAddress ||
        request.connection.socket.remoteAddress;
    ip = ip.split(',')[0];
    ip = ip.split(':').slice(-1);

    response.status(200).send({string: ip});
});

app.get('/isDevEnv', (request, response) => {
	const envPath = path.join(__dirname, '../../../../../../../');

	//console.log(envPath)
	response.status(200).send({path: envPath});
});

app.post('/callSimulate', (req, res) => {
  console.log(typeof(JSON.stringify(req.body)));
  console.log(JSON.stringify(req.body));

	const options = {
		url: 'http://localhost:8082/simulate',
		form: { start_date:{value:"07-22-2020"},
				runtime_days:{value:"1"},
				transmissibility:{ifchecked:true,value:"2.2"},
				exposed_confirmed_ratio:{ifchecked:false,value:"0.00"},
				mean_incubation_duration_in_days:{ifchecked:false,value:"2.2"},
				mean_infection_duration_in_days:{ifchecked:false,value:"2.3"},
				mortality_ratio:{ifchecked:false,value:"0.05"},
				update_trig_interval_in_hrs:{ifchecked:false,value:"24"},
				graph_type:{ifchecked:false,value:"Watts-Strogatz"},
				graph_params:{ifchecked:false,K_val:"8",beta_val:"0.1"},
				diffusion_trig_interval_in_hrs:{ifchecked:false,value:"48"},
				avg_transport_speed:{ifchecked:false,value:"100"},
				max_diffusion_cnt:{ifchecked:false,value:"10"} 
			}
	};

	request.post(options, (err, reqRes, body) => {
		if (err) {
			res.status(400).send({Error: err});
			return console.log(err);
		}
		console.log(body);
		res.status(200).send({response: body});
	});
})

app.get('/callGetstatus', (req, res) => {
	request('http://localhost:8082/getstatus', { json: true }, (error, response, body) => {
		if (error) { return console.log(error); }
		//console.log(body);
		res.status(200).send({statusmsg: body});
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
