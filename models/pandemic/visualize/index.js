const path = require('path');
const fs = require('fs');

const directoryPath = path.join(__dirname, 'visualize', 'mongodb',  'Pandemic_Data');
const Datastore = require('nedb');

const express = require('express');
const app = express();
app.listen(3000, () => console.log('listening at port 3000'));
app.use(express.static('../visualize'));

const database = new Datastore('database.db');
database.loadDatabase();

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
