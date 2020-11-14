const path = require('path');
const fs = require('fs');

const directoryPath = path.join(__dirname, 'visualize', 'mongodb',  'Pandemic_Data');
const Datastore = require('nedb');

const express = require('express');
const app = express();
app.listen(3000, () => console.log('listening at port 3000'));
app.use(express.static('../../../visualize'));

const database = new Datastore('database.db');
database.loadDatabase();


app.get('/pandemic_data/:start_date/:end_date', (request, response) => {
	var startDate = new Date(request.params.start_date);
	var endDate = new Date(request.params.end_date);
	var millisecond_to_day = 86400000
	var number_of_days = ((endDate - startDate) / millisecond_to_day);
	
	console.log(number_of_days);
	console.log(startDate);
	console.log(startDate.setDate(startDate.getDate() + 1));
	console.log(startDate);
	console.log(startDate.setDate(startDate.getDate() + 10));
        console.log(startDate);

	// database.find({date: startDate.toString()})
	database.find({}, (err, data) => {
	if (err) {
		response.end();
		return;
	}
	response.json(data);
	});
});
/*
fs.readdir(directoryPath, function (err, files) {
	if (err) {
		return console.log('Unable to scan directory: ' + err);
	}
	files.forEach(function (file) {
		// console.log(file);
		var read_file = fs.readFileSync(directoryPath + '/' + file);
		var output = JSON.parse(read_file);
		database.insert(output);
	});
});
*/
