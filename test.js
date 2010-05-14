var sys = require('sys'),
    puts = sys.puts,
    binding = require('./binding');


var js_fib = function(num) {
    var one_before = 1,
        two_before = 0,
        current = 1;
    if(num < 1) {
        return 0;
    }
    for(var i = 1; i < num; ++i) {
        current = one_before + two_before;
        two_before = one_before;
        one_before = current;
    }
    return current;
};

var time_fib = function(fibnum, times, callback) {
    var time = new Date();
    var now = time.getTime();
    for(var i = 0; i < 100000; ++i) {
        callback(fibnum);
    }
    time = new Date();
    return(time.getTime() - now);
};

puts(time_fib(20, 1000000, binding.fibonacci));
puts(time_fib(20, 1000000, js_fib));

