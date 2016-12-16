function rest_xmlhttp(host, key, command, params, cb) {
	var xmlhttp = new XMLHttpRequest();
	xmlhttp.open('POST', "http://" + host + '/api/' + command, true);
	xmlhttp.setRequestHeader("Authorization", "Bearer " + key);
	xmlhttp.onreadystatechange = function () {
		if (xmlhttp.readyState == 4)
			cb(xmlhttp);
	};
	if(params == null || params == undefined)
		xmlhttp.send();
	else if(params.length == 0)
		xmlhttp.send();
	else
		xmlhttp.send(JSON.stringify(params));
}

function rest(host, key, command, params, cb) {
	rest_xmlhttp(host, key, command, params, function(xmlhttp) {
		var res = '';
		try {
			res = JSON.parse(xmlhttp.responseText);
		} catch (e) { }
		if (xmlhttp.status < 200 || xmlhttp.status > 299)
			cb(true, res);
		else
			cb(false, res);
	});
}

function rest_common(command, params, cb) {
	key = document.getElementById('accesskey').value;
	localStorage['accesskey'] = key;
	rest(window.location.hostname, key, command, params, cb);
}
