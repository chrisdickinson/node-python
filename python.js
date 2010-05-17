var sys = require('sys'),
    puts = sys.puts,
    binding = require('./binding');

var sys = binding.import('sys');
var posix = binding.import('os');

sys.path.append.call(posix.getcwd.call().toString());
var gary_busey = binding.import("gary_busey");
var result = gary_busey.say_hey.call("man i suck");

puts(result.toString());
