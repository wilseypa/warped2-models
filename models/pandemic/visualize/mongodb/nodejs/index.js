const path = require('path');
const fs = require('fs');
//const request = require('request');
const axiosTest = require('axios');
const querystring = require('querystring');

const directoryPath = path.join(__dirname, 'visualize', 'mongodb',  'Pandemic_Data');
const Datastore = require('nedb');

const express = require('express');
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

app.get('/login/:username/:password', (request, response) => {
	let password = request.params.password;
	let username = request.params.username;
	// console.log(username);
	// console.log(password);
	if (username == 'admin' && password == 'warped2') {
		// console.log('Good');
		response.status(200).send({response: "success"});

	}
	else {
		// console.log('Bad');
		response.status(401).send({response: "failed"});
	}
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

	const first_date = '08-01-2020.simulated-data.json';
	const second_date = '09-15-2020.simulated-data.json';
	const first_date2 = '08-01-2020.formatted-JHU-data.json';
	const second_date2 = '09-15-2020.formatted-JHU-data.json';
	const third_date = '12-01-2020.simulated-data.json';
	const third_date2 = '12-01-2020.formatted-JHU-data.json';
	const fourth_date = '01-01-2021.simulated-data.json';
	const fourth_date2 = '01-01-2021.formatted-JHU-data.json';
	const fifth_date = '02-01-2021.simulated-data.json';
	const fifth_date2 = '02-01-2021.formatted-JHU-data.json';
	const sixth_date = '03-01-2021.simulated-data.json';
	const sixth_date2 = '03-01-2021.formatted-JHU-data.json';
	const seventh_date = '03-15-2021.simulated-data.json';
	const seventh_date2 = '03-15-2021.formatted-JHU-data.json';
	const eigth_date = '02-15-2021.simulated-data.json';
	const eight_date2 = '02-15-2021.formatted-JHU-data.json';

	const vivek_actual_path = '/work/vivek/warped2/warped2-models/models/pandemic/data';
	const vivek_simulated_path = '/work/vivek/warped2/warped2-models/models/pandemic/scripts/tuningapp/simOutfiles.bkp';
	const actual_data_folder = path.join(__dirname, '../../../../../../../../../', vivek_actual_path);
	const simulated_data_folder = path.join(__dirname, '../../../../../../../../../', vivek_simulated_path);
	const auguest_simulated = path.join(simulated_data_folder, first_date);
	const auguest_actual = path.join(actual_data_folder, first_date2);
	const september_simulted = path.join(simulated_data_folder, second_date);
	const september_actual = path.join(actual_data_folder, second_date2);
	const december_actual = path.join(actual_data_folder, third_date2);
	const december_simulated = path.join(simulated_data_folder, third_date);
	const january_actual = path.join(actual_data_folder, fourth_date2);
	const january_simulated = path.join(simulated_data_folder, fourth_date);
	const february_actual = path.join(actual_data_folder, fifth_date2);
	const february_simulated = path.join(simulated_data_folder, fifth_date);
	const march_actual = path.join(actual_data_folder, sixth_date2);
	const march_simulated = path.join(simulated_data_folder, sixth_date);
	const march_actual2 = path.join(actual_data_folder, seventh_date2);
	const march_simulated2 = path.join(simulated_data_folder, seventh_date);
	const february_actual2 = path.join(actual_data_folder, eight_date2);
	const february_simulated2 = path.join(simulated_data_folder, eigth_date);


	let rawdata1 = fs.readFileSync(auguest_simulated);
	let data1 = JSON.parse(rawdata1);

	let rawdata2 = fs.readFileSync(auguest_actual);
	let data2 = JSON.parse(rawdata2);

	let rawdata3 = fs.readFileSync(september_simulted);
	let data3 = JSON.parse(rawdata3);

	let rawdata4 = fs.readFileSync(september_actual);
	let data4 = JSON.parse(rawdata4);

	let rawdata5 = fs.readFileSync(december_actual);
	let data5 = JSON.parse(rawdata5);

	let rawdata6 = fs.readFileSync(december_simulated);
	let data6 = JSON.parse(rawdata6);

	let rawdata7 = fs.readFileSync(january_actual);
	let data7 = JSON.parse(rawdata7);

	let rawdata8 = fs.readFileSync(january_simulated);
	let data8 = JSON.parse(rawdata8);

	let rawdata9 = fs.readFileSync(february_actual);
	let data9 = JSON.parse(rawdata9);

	let rawdata10 = fs.readFileSync(february_simulated);
	let data10 = JSON.parse(rawdata10);

	let rawdata11 = fs.readFileSync(march_actual);
	let data11 = JSON.parse(rawdata11);

	let rawdata12 = fs.readFileSync(march_simulated);
	let data12 = JSON.parse(rawdate12);


	let rawdata13 = fs.readFileSync(march_actual2);
	let data13 = JSON.parse(rawdata13);


	let rawdata14 = fs.readFileSync(march_simulated2);
	let data14 = JSON.parse(rawdata14);

	let rawdata15 = fs.readFileSync(february_actual2);
	let data15 = JSON.parse(rawdata15);

	let rawdata16 = fs.readFileSync(february_simulated2);
	let data16 = JSON.parse(rawdata16);


	// let responseArray = [data1, data2, data3, data4, data5, data6, data7, data8, data9, data10, data11, data12, data13, data14];
	let responseArray = [data9, data10, data15, data16, data11, data12, data13, data14];
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
