var sys = require('sys'),
    puts = sys.puts,
    binding = require('./binding'),
    path_additions = require('./path_additions'),
    http = require('http'),
    url = require('url');

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
    var path_and_query = url.parse(req.url);
    if(!path_and_query.pathname.match(/^\/media/)) {
        var wsgi_request = django_wsgi.WSGIRequest({
            'PATH_INFO':path_and_query.pathname,
            'QUERY_STRING':path_and_query.query,
            'HTTP_VERSION':req.httpVersion,
            'HTTP_ACCEPT':req.headers['http-accept'],
            'HTTP_ACCEPT_CHARSET':req.headers['http-accept-charset'],
            'HTTP_ACCEPT_ENCODING':req.headers['http-accept-encoding'],
            'HTTP_ACCEPT_LANGUAGE':req.headers['http-accept-language'],
            'HTTP_CACHE_CONTROL':req.headers['http-cache-control'],
            'REQUEST_METHOD':req.method,
            'HTTP_HOST':req.headers['http-host']
        });
        var response = wsgi_handler.get_response(wsgi_request),
            headers = response._headers.valueOf(),
            content = response.content.toString(),
            headers_out = {},
            status_code = response.status_code.valueOf();

        for(var i in headers) {
            var as_array = headers[i].valueOf();
            headers_out[as_array[0]] = as_array[1].toString();
        };
        puts(req.method + ' - ' + response.status_code.valueOf() + ' - ' + path_and_query.pathname + " - " + JSON.stringify(headers_out));
        res.writeHead(status_code, headers_out);
        res.write(content);
        res.close();
    } else {
        puts(req.method + ' - 404 - ' + path_and_query.pathname + " - {}");
        res.writeHead(404, {"Content-Type":"text/html"});
        res.write("<h1>sorry, no images.</h1>");
        res.close();
    } 
}).listen(8000); 
