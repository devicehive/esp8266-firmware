function request_xhr(verb, url, params, key, cb) {
	var xmlhttp = new XMLHttpRequest();
	xmlhttp.open(verb, url);
	xmlhttp.setRequestHeader('Authorization', 'Bearer ' + key);
	xmlhttp.onreadystatechange = function () {
		if (xmlhttp.readyState == 4)
			cb(xmlhttp);
	};
	if(params == null || verb.match(/^get$/i))
		xmlhttp.send();
	else if(params.length == 0)
		xmlhttp.send();
	else {
		if (typeof params == 'object')
			params = JSON.stringify(params);
		xmlhttp.send(params);
	}
}

function rest(command, params, key, cb) {
	rest_xmlhttp('POST', '/api/' + command, params, key, function(xmlhttp) {
		var res = xmlhttp.responseText;
		try {
			res = JSON.parse(res);
		} catch (e) { }
		if (xmlhttp.status < 200 || xmlhttp.status > 299)
			cb(true, res);
		else
			cb(false, res);
	});
}

function rest_common(command, params, cb) {
	var key = document.getElementById('accesskey').value;
	localStorage['accesskey'] = key;
	rest(command, params, key, cb);
}
