#include <v8.h>
#include <python2.6/Python.h>
using namespace v8;

// so yeah my algorithms are a little rusty it turns out. 
// this is largely from memory.
// )`: don't hate
int64_t fibonacci(int64_t input) {
    int64_t two_before = 0;
    int64_t one_before = 1;
    int64_t current = 1;
    if(input < 1) {
        return 0;
    }
    for(int64_t i = 1; i < input; ++i) {
        current = one_before + two_before;
        two_before = one_before;
        one_before = current;
    }
    return current;
}

static Handle<Value>
Fibonacci (const Arguments& args) {
    HandleScope scope;

    // let's do some error checking.
    if (args.Length() == 0 || !args[0]->IsNumber()) {
        return ThrowException(
            Exception::TypeError(String::New("First argument must be a number"))
        );
    }

    // hand it off to our native code.
    int64_t input(args[0]->IntegerValue());
    int64_t fib = fibonacci(input);

    // and close the scope over the result. 
    return scope.Close(Number::New(fib));
}

extern "C" void
init (Handle<Object> target) {
    HandleScope scope;

    // create a new function template and assign it to the exported variable "fibonacci"
    Local<FunctionTemplate> t = FunctionTemplate::New(Fibonacci);
    target->Set(String::New("fibonacci"), t->GetFunction());
}

/*******
#   write this out to "wscript" in the same directory.
#   to build, type
#       node-waf configure build
#
#   then import it in node
#      var binding = require('<full path to this directory>/binding');
#      binding.fibonacci(4);
#
#   SO EASY
#
import Options
from os import unlink, symlink, popen
from os.path import exists 

srcdir = '.'
blddir = 'build'
VERSION = '0.0.1'

def set_options(opt):
  opt.tool_options('compiler_cxx')

def configure(conf):
  conf.check_tool('compiler_cxx')
  conf.check_tool('node_addon')

def build(bld):
  obj = bld.new_task_gen('cxx', 'shlib', 'node_addon')
  obj.target = 'binding'
  obj.source = "binding.cc"

def shutdown():
  # HACK to get binding.node out of build directory.
  # better way to do this?
  if Options.commands['clean']:
    if exists('binding.node'): unlink('binding.node')
  else:
    if exists('build/default/binding.node') and not exists('binding.node'):
      symlink('build/default/binding.node', 'binding.node')

******/
