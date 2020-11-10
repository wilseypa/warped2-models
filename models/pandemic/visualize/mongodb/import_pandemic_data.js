const path = require('path');
const fs = require('fs');

const directoryPath = path.join(__dirname, 'Pandemic_Data');

fs.readdir(directoryPath, function (err, files) {
	if (err) {
		return console.log('Unable to scan directory: ' + err);
	}
	files.forEach(function (file) {
		// console.log(file);
		var read_file = fs.readFileSync(directoryPath + '/' + file);
		var output = JSON.parse(read_file);
		// console.log(output);
	});
});
