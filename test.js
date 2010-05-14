var sys = require('sys'),
    puts = sys.puts,
    binding = require('./binding');

var items;
var modules = ['sys', 'os', 'StringIO'];
for (var index in modules) {
    items = binding.import(modules[index]);
    puts(modules[index] + ":\t" + items.join(', '));
    puts('----');
}

binding.shutdown();
