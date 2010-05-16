var sys = require('sys'),
    puts = sys.puts,
    binding = require('./binding');

var sys = binding.import('sys'),
path = sys.getrecursionlimit;
puts(path.valueOf().apply(path).toString());
/*var recursion_limit = sys.getrecursionlimit.valueOf();
var result = recursion_limit();
puts(result);*/
