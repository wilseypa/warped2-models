const path = require('path');
const fs = require('fs');
const request = require('request');

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
		response.status(400).send({response: "failed"});
	}
});

app.get('/isDevEnv', (request, response) => {
	const envPath = path.join(__dirname, '../../../../../../../');

	//console.log(envPath)
	response.status(200).send({path: envPath});
});

app.post('/callSimulate', (req, res) => {
  console.log(typeof(req.body));
  request.post({
	headers: {'content-type' : 'application/x-www-form-urlencoded'},
	url:     'http://localhost:8082/simulate',
	body:    JSON.stringify(req.body)
  }, function(error, response, bodyRes){
	if (error) { return console.log(error); }
	//console.log(response)
	//console.log(bodyRes);
	res.status(200).send({response: bodyRes});
  });
})

app.get('/callGetstatus', (req, res) => {
	request('http://localhost:8082/getstatus', { json: true }, (error, response, body) => {
		if (error) { return console.log(error); }
		//console.log(body);
		res.status(200).send({statusmsg: body});
	});
});

app.get('/send_data', (request, response) => {
	// Function call to send back files from Vivek's directory to our frontend
	const vivek_path = '/work/vivek/warped2-models/warped2-models/models/pandemic/scripts/tuningapp/simOutfiles';
	const data_folder2 = path.join(__dirname, '../../../../../../../../../', vivek_path);

	let responseArray = [];
	fs.readdir(data_folder2, (err, files) => {
		files.forEach(file => {
			// console.log(file);
			let filepath = path.join(data_folder2, file);
			let rawdata = fs.readFileSync(filepath);
			let pandemic_data = JSON.parse(rawdata);
			responseArray.push(pandemic_data);
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
