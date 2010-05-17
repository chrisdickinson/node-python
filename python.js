var sys = require('sys'),
    puts = sys.puts,
    binding = require('./binding'),
    path_additions = require('./path_additions'),
    http = require('http');

var sys = binding.import('sys');
var os = binding.import('os');

os.environ.update({
    'DJANGO_SETTINGS_MODULE':'project.development',
});

/*
var gary_busey = binding.import("gary_busey");
var result = gary_busey.say_hey.call("man i suck");
*/
var django_wsgi = binding.import('django.core.handlers.wsgi');

var wsgi_handler = django_wsgi.WSGIHandler.call()
wsgi_handler.load_middleware();

http.createServer(function (req, res) {
    var wsgi_request = django_wsgi.WSGIRequest({
        'path':req.url,
        'REQUEST_PATH':req.url,
        'REQUEST_METHOD':'GET',
        'HTTP_HOST':'localhost:8000',
    });
    var response = wsgi_handler.get_response(wsgi_request);

    res.writeHead(200, {'Content-Type':'text/html'});
    var content = response.content.toString();
    res.write(content);
    res.close();
}).listen(8000); 
