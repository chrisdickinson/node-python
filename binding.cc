// binding.cc
#include <v8.h>
#include <string>
#include <iostream>
#define RETURN_NEW_PYOBJ(scope,pyobject) \
        Local<Object> jsobject = python_function_template_->GetFunction()->NewInstance(); \
        PyObjectWrapper* py_object_wrapper = new PyObjectWrapper(pyobject); \
        py_object_wrapper->Wrap(jsobject);\
        return scope.Close(jsobject);

using namespace v8;
using std::string;

#include <python2.6/Python.h>
#include <node_object_wrap.h>
using namespace node;

class PyObjectWrapper : public ObjectWrap {
    PyObject* mPyObject;
    public:
        static Persistent<FunctionTemplate> python_function_template_;
        PyObjectWrapper(PyObject* obj) : mPyObject(obj), ObjectWrap() { }
        virtual ~PyObjectWrapper() {
            Py_XDECREF(mPyObject);
            mPyObject = NULL;
        }

        static void
        Initialize(Handle<Object> target) {
            HandleScope scope;
            Local<FunctionTemplate> fn_tpl = FunctionTemplate::New();
            Local<ObjectTemplate> obj_tpl = fn_tpl->InstanceTemplate();

            obj_tpl->SetInternalFieldCount(1);

            // this has first priority. see if the properties already exist on the python object
            obj_tpl->SetNamedPropertyHandler(Get, Set);

            // If we're calling `toString`, delegate to our version of ToString
            obj_tpl->SetAccessor(String::NewSymbol("toString"), ToStringAccessor); 

            // likewise for valueOf
            obj_tpl->SetAccessor(String::NewSymbol("valueOf"), ValueOfAccessor); 

            // Python objects can be called as functions.
            obj_tpl->SetCallAsFunctionHandler(Call, Handle<Value>());

            python_function_template_ = Persistent<FunctionTemplate>::New(fn_tpl);
            // let's also export "import"
            Local<FunctionTemplate> import = FunctionTemplate::New(Import);
            target->Set(String::New("import"), import->GetFunction());
        };

        static Handle<Value>
        Get(Local<String> key, const AccessorInfo& info) {
            // returning an empty Handle<Value> object signals V8 that we didn't
            // find the property here, and we should check the "NamedAccessor" functions
            HandleScope scope;
            PyObjectWrapper* wrapper = ObjectWrap::Unwrap<PyObjectWrapper>(info.Holder());
            String::Utf8Value utf8_key(key);
            string value(*utf8_key);
            PyObject* result = wrapper->InstanceGet(value);
            if(result) {
                RETURN_NEW_PYOBJ(scope, result);
            }
            return Handle<Value>();
        }

        static Handle<Value>
        Set(Local<String> key, Local<Value> value, const AccessorInfo& info) {
            // we don't know what to do.
            return Undefined();
        };

        static Handle<Value>
        CallAccessor(Local<String> property, const AccessorInfo& info) {
            HandleScope scope;
            Local<FunctionTemplate> func = FunctionTemplate::New(Call);
            return scope.Close(func->GetFunction());
        };

        static Handle<Value>
        ToStringAccessor(Local<String> property, const AccessorInfo& info) {
            HandleScope scope;
            Local<FunctionTemplate> func = FunctionTemplate::New(ToString);
            return scope.Close(func->GetFunction());
        };

        static Handle<Value>
        ValueOfAccessor(Local<String> property, const AccessorInfo& info) {
            HandleScope scope;
            Local<FunctionTemplate> func = FunctionTemplate::New(ValueOf);
            return scope.Close(func->GetFunction());
        }

        static Handle<Value>
        Call(const Arguments& args) {
            HandleScope scope;
            Local<Object> this_object = args.This(); 
            PyObjectWrapper* pyobjwrap = ObjectWrap::Unwrap<PyObjectWrapper>(args.This());
            Handle<Value> result = pyobjwrap->InstanceCall(args);
            return scope.Close(result);
        }

        static Handle<Value>
        ToString(const Arguments& args) {
            HandleScope scope;
            Local<Object> this_object = args.This(); 
            PyObjectWrapper* pyobjwrap = ObjectWrap::Unwrap<PyObjectWrapper>(args.This());
            Local<String> result = String::New(pyobjwrap->InstanceToString(args).c_str());
            return scope.Close(result);
        }
        static Handle<Value> ValueOf(const Handle<Object>& obj) {
        };

        static Handle<Value>
        ValueOf(const Arguments& args) {
            HandleScope scope;
            Local<Object> this_object = args.This(); 
            PyObjectWrapper* pyobjwrap = ObjectWrap::Unwrap<PyObjectWrapper>(args.This());
            PyObject* py_obj = pyobjwrap->InstanceGetPyObject();
            if(PyCallable_Check(py_obj)) {
                Local<FunctionTemplate> call = FunctionTemplate::New(Call);
                return scope.Close(call->GetFunction());
            } else if (PyNumber_Check(py_obj)) {
                long long_result = PyLong_AsLong(py_obj);
                return scope.Close(Integer::New(long_result));
            } else if (PySequence_Check(py_obj)) {
                int len = PySequence_Length(py_obj);
                Local<Array> array = Array::New(len);
                for(int i = 0; i < len; ++i) {
                    Handle<Object> jsobj = python_function_template_->GetFunction()->NewInstance();
                    PyObject* py_obj_out = PySequence_GetItem(py_obj, i);
                    PyObjectWrapper* obj_out = new PyObjectWrapper(py_obj_out);
                    obj_out->Wrap(jsobj);
                    array->Set(i, jsobj);
                }
                return scope.Close(array);
            } else if (PyMapping_Check(py_obj)) {
                int len = PyMapping_Length(py_obj);
                Local<Object> object = Object::New();
                PyObject* keys = PyMapping_Keys(py_obj);
                PyObject* values = PyMapping_Values(py_obj);
                for(int i = 0; i < len; ++i) {
                    PyObject *key = PySequence_GetItem(keys, i),
                        *value = PySequence_GetItem(values, i),
                        *key_as_string = PyObject_Str(key);
                    char* cstr = PyString_AsString(key_as_string);

                    Local<Object> jsobj = python_function_template_->GetFunction()->NewInstance();
                    PyObjectWrapper* obj_out = new PyObjectWrapper(value);
                    obj_out->Wrap(jsobj);
                    Py_XDECREF(key);
                    Py_XDECREF(key_as_string);
                }
                Py_XDECREF(keys);
                Py_XDECREF(values);
                return scope.Close(object);
            }
            return Undefined();
        }

        static Handle<Value>
        Import(const Arguments& args) {
            HandleScope scope; 
            if(args.Length() < 1 || !args[0]->IsString()) {
                return ThrowException(
                    Exception::Error(String::New("I don't know how to import that."))
                );
            }
            PyObject* module_name = PyString_FromString(*String::Utf8Value(args[0]->ToString()));
            PyObject* module = PyImport_Import(module_name);
            Py_XDECREF(module_name);

            PyObjectWrapper* pyobject_wrapper = new PyObjectWrapper(module);
            RETURN_NEW_PYOBJ(scope, module);
        }

        static PyObject*
        ConvertToPython(const Handle<Value>& value) {
            int len;
            HandleScope scope;
            if(value->IsString()) {
                return PyString_FromString(*String::Utf8Value(value->ToString()));
            } else if(value->IsNumber()) {
                return PyFloat_FromDouble(value->NumberValue());
            } else if(value->IsObject()) {
                Local<Object> obj = value->ToObject();
                if(!obj->FindInstanceInPrototypeChain(python_function_template_).IsEmpty()) {
                    PyObjectWrapper* python_object = ObjectWrap::Unwrap<PyObjectWrapper>(value->ToObject());
                    PyObject* pyobj = python_object->InstanceGetPyObject();
                    return pyobj;
                } else {
                    Local<Array> property_names = obj->GetPropertyNames();
                    len = property_names->Length();
                    PyObject* py_dict = PyDict_New();
                    for(int i = 0; i < len; ++i) {
                        Local<String> str = property_names->Get(i)->ToString();
                        Local<Value> js_val = obj->Get(str);
                        PyDict_SetItemString(py_dict, *String::Utf8Value(str), ConvertToPython(js_val));
                    }
                    return py_dict;
                }
                return NULL;
            } else if(value->IsArray()) {
                Local<Array> array = Array::Cast(*value);
                len = array->Length();
                PyObject* py_list = PyList_New(len);
                for(int i = 0; i < len; ++i) {
                    Local<Value> js_val = array->Get(i);
                    PyList_SET_ITEM(py_list, i, ConvertToPython(js_val));
                }
                return py_list;
            } else if(value->IsUndefined()) {
                Py_INCREF(Py_None);
                return Py_None;
            }
            return NULL;
        }

        PyObject*
        InstanceGetPyObject() {
            return mPyObject;
        }

        Handle<Value> InstanceCall(const Arguments& args) {
            // for now, we don't do anything.
            HandleScope scope;
            int len = args.Length();
            PyObject* args_tuple = PyTuple_New(len);
            for(int i = 0; i < len; ++i) {
                PyObject* py_arg = ConvertToPython(args[i]);
                PyTuple_SET_ITEM(args_tuple, i, py_arg);
            }
            PyObject* result = PyObject_CallObject(mPyObject, args_tuple);
            Py_XDECREF(args_tuple);
            if(!result) {
                PyErr_Clear();
                return ThrowException(
                    Exception::Error(
                        String::New("Python exception")
                    )
                );
            } else {
                RETURN_NEW_PYOBJ(scope, result);
            }
            return Undefined();
        }

        string InstanceToString(const Arguments& args) {
            PyObject* as_string = PyObject_Str(mPyObject);
            string native_string(PyString_AsString(as_string));
            Py_XDECREF(as_string);
            return native_string;
        }

        PyObject* InstanceGet(const string& key) {
            if(PyObject_HasAttrString(mPyObject, key.c_str())) {
                PyObject* attribute = PyObject_GetAttrString(mPyObject, key.c_str());
                return attribute;
            }
            return (PyObject*)NULL;
        }
};

Persistent<FunctionTemplate> PyObjectWrapper::python_function_template_;
// all v8 plugins must emit
// a "init" function
extern "C" void
init (Handle<Object> target) {
    HandleScope scope;
    Py_Initialize();
    PyObjectWrapper::Initialize(target);
}

