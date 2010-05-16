#include <v8.h>
#include <python2.6/Python.h>
#include <iostream>
#include <string>
using namespace v8;
using std::string;

class PythonObject {
    protected:
        PyObject* _object;
    public:
        PythonObject(PyObject* pyobj): _object(pyobj) { 

        }
        virtual ~PythonObject() { Py_XDECREF(_object); } 
};
 
class JSPythonObject : public PythonObject {
    public:
        JSPythonObject(PyObject* obj) : PythonObject(obj) { }

        static Handle<Value> MapGet(Local<String> key, const AccessorInfo& info) {
            HandleScope handle_scope;
            JSPythonObject* object = UnwrapPythonObject(info.Holder());
            String::Utf8Value utf8_key(key->ToString());
            std::cout << *utf8_key << '\t';
            if(PyObject_HasAttrString(object->_object, *utf8_key)) {
                PyObject* python_attribute = PyObject_GetAttrString(object->_object, *utf8_key);
                if(python_attribute != NULL) {
                    std::cout << "FOUND\n";
                    JSPythonObject* jspython = new JSPythonObject(python_attribute);
                    return handle_scope.Close(WrapPythonObject(jspython));
                }
            }
/*
            if(std::string("toString") == std::string(*utf8_key)) {
                Local<FunctionTemplate> toString = FunctionTemplate::New(ToString);
                return handle_scope.Close(toString);
            }*/
            std::cout << "NOT FOUND\n";

            return Undefined();
        }
/*
        static Handle<String> ToString(const Arguments& args) {
            HandleScope handle_scope;
            Handle<Object> wrapped = args.This();
            JSPythonObject* obj = JSPythonObject::UnwrapPythonObject(wrapped);
            if(PyString_Check(obj->_object)) {
                char* cstr = PyString_AsString(obj->_object);
                return handle_scope.Close(String::New(PyString_AsString(obj->_object)));
            }
            return handle_scope.Close(String::New("Python object"));
        }
*/
        static Handle<Value> MapSet(Local<String> key, Local<Value> value, const AccessorInfo& info) {
            return Undefined();
        }

        static Handle<Object>   WrapPythonObject(JSPythonObject* obj) {
            HandleScope handle_scope;
            Handle<ObjectTemplate> tpl = python_object_template_;
            Handle<Object> result = tpl->NewInstance();
            Handle<External> py_ptr = External::New(obj);
            result->SetInternalField(0, py_ptr);
            return handle_scope.Close(result);
        }
        static JSPythonObject*  UnwrapPythonObject(Handle<Object> obj) {
            Handle<External> field = Handle<External>::Cast(obj->GetInternalField(0));
            void* ptr = field->Value();
            return static_cast<JSPythonObject*>(ptr);
        }
        static Persistent<ObjectTemplate> python_object_template_;
};

Persistent<ObjectTemplate> JSPythonObject::python_object_template_;

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
        return scope.Close(JSPythonObject::WrapPythonObject(new JSPythonObject(py_module))); 
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
    Handle<ObjectTemplate> obj_template = ObjectTemplate::New();
    obj_template->SetInternalFieldCount(1);
    obj_template->SetNamedPropertyHandler(JSPythonObject::MapGet, JSPythonObject::MapSet);

    JSPythonObject::python_object_template_ = Persistent<ObjectTemplate>::New(obj_template);

    Local<FunctionTemplate> import = FunctionTemplate::New(Import);
    Local<FunctionTemplate> shutdown = FunctionTemplate::New(Shutdown);
    target->Set(String::New("import"), import->GetFunction());
    target->Set(String::New("shutdown"), shutdown->GetFunction());
}
