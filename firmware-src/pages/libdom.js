function byId(id) {
	return document.getElementById(id);
}

function print(text, err, id) {
	var out = byId(id || 'output');
	out.style.color = (!!err) ? 'red' : 'green';
	out.innerHTML = '';
	out.appendChild(document.createTextNode(text));
}

function replaceDom(target, obj) {
	target.innerHTML = '';
	target.appendChild(renderDom(obj));
}

function renderDom(obj) {
	if (typeof obj == 'string') {
		return document.createTextNode(obj);
	} else if (Array.isArray(obj)) {
		var dom = document.createElement(obj[0]);
		var attrs = obj[1];
		// doesn't work for style attribute in IE
		Object.keys(attrs).forEach(
			function (key) { dom.setAttribute(key, attrs[key]); });
		obj.slice(2).forEach(
			function (child) { dom.appendChild(renderDom(child)); });
		return dom;
	} else
		throw 'Cannot make dom of: ' + obj;
}

function getKey() {
	return byId('accesskey').value;
}
