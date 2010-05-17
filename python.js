var sys = require('sys'),
    puts = sys.puts,
    binding = require('./binding'),
    http = require('http');

var sys = binding.import('sys');
var posix = binding.import('os');

sys.path.append(posix.getcwd().toString());
var gary_busey = binding.import("gary_busey");
var result = gary_busey.say_hey.call("man i suck");

/*
var django_wsgi = binding.import('django.core.handlers.wsgi');

var wsgi_handler = django_wsgi.WSGIHandler.call()

http.createServer(function (req, res) {
    var wsgi_request = django_wsgi.WSGIRequest.call({
        'path':req.url,
    });
    var result = wsgi_handler.get_response.call(wsgi_request);

    res.writeHead(200, {'Content-Type':'text/plain'});
    res.write('Heyyyyy\n');
    res.close();
}).listen(8000); 
*/
puts(result.toString());
