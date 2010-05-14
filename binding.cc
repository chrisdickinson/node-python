#include <v8.h>
#include <python2.6/Python.h>
using namespace v8;

static Handle<Value>
Import (const Arguments& args) {
    HandleScope scope;
    if (args.Length() == 0 || !args[0]->IsString()) {
        return ThrowException(
            Exception::TypeError(String::New("First argument must be a string"))
        );
    }
    String::Utf8Value utf_module_name(args[0]->ToString());
    PyObject* py_module_name = PyString_FromString(*utf_module_name),
    *py_module = PyImport_Import(py_module_name);
    Py_DECREF(py_module_name);
    if(py_module != NULL) {


        PyObject* py_path = PyObject_Dir(py_module);
        int sys_path_length = PyList_Size(py_path), 
        i;
        char* cstr;

        Local<Array> output = Array::New(sys_path_length);
        for(i = 0; i < sys_path_length; ++i) {
            PyObject* item = PyList_GetItem(py_path, i);
            if(!PyArg_Parse(item, "s", &cstr)) {
                
            } else {
                output->Set(Number::New(i), String::New(cstr));
            }
            Py_DECREF(item);
        }

        Py_XDECREF(py_path);
        Py_DECREF(py_module);
        return scope.Close(output);
    }
    return ThrowException(
        Exception::TypeError(String::New("Could not import that module."))
    );
}

static Handle<Value>
Shutdown (const Arguments& args) {
    Py_Finalize();
    return Undefined();
}

extern "C" void
init (Handle<Object> target) {
    HandleScope scope;
    Py_Initialize();
    // create a new function template and assign it to the exported variable "fibonacci"
    Local<FunctionTemplate> import = FunctionTemplate::New(Import);
    Local<FunctionTemplate> shutdown = FunctionTemplate::New(Shutdown);
    target->Set(String::New("import"), import->GetFunction());
    target->Set(String::New("shutdown"), shutdown->GetFunction());
}
