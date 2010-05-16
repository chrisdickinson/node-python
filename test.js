var sys = require('sys'),
    puts = sys.puts,
    binding = require('./binding');

var py_sys = binding.import('sys');
var something = py_sys.version

puts(something);
binding.shutdown();
