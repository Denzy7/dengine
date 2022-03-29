#include "dengine-script/script.h"

#include "dengine-utils/logging.h"

#include "py_modules/inptmodule.h" //input_mod
#include "py_modules/timermodule.h" //timer_mod
#include "py_modules/loggingmodule.h" //logging_mod

static PyObject* dengineinit(PyObject* self, PyObject* args)
{
    const char* famouswords = "Hello World!";
    printf("%s\n", famouswords);
    return PyUnicode_FromString(famouswords);
}

static PyMethodDef DengineInitMethod[]=
{
    { "__init__", dengineinit, METH_VARARGS, "Returns and prints some famous words" },
    { NULL, NULL, 0, NULL}
};

static PyModuleDef dengine =
{
    PyModuleDef_HEAD_INIT,
    "dengine",
    "Dengine scripting",
    -1,
    DengineInitMethod
};
PyMODINIT_FUNC PyInit_dengine()
{
    PyObject* mod = PyModule_Create(&dengine);
    PyModule_AddObject(mod, "__path__", Py_BuildValue("()"));
    return mod;
}

static PyObject* py_builtins = NULL,* py_compiler = NULL;

int denginescript_init()
{
    int append_dengine = PyImport_AppendInittab("dengine", &PyInit_dengine);
    PyImport_AppendInittab("dengine.inpt", &PyInit_inpt);
    PyImport_AppendInittab("dengine.timer", &PyInit_timer);
    PyImport_AppendInittab("dengine.logging", &PyInit_logging);

    Py_Initialize();

    //Import hook
    PyRun_SimpleString(
        "import importlib.abc\n" \
        "import importlib.machinery\n" \
        "import sys\n" \
        "\n" \
        "\n" \
        "class Finder(importlib.abc.MetaPathFinder):\n" \
        "    def find_spec(self, fullname, path, target=None):\n" \
        "        if fullname in sys.builtin_module_names:\n" \
        "            return importlib.machinery.ModuleSpec(\n" \
        "                fullname,\n" \
        "                importlib.machinery.BuiltinImporter,\n" \
        "            )\n" \
        "\n" \
        "\n" \
        "sys.meta_path.append(Finder())\n" \
    );

    py_builtins = PyEval_GetBuiltins();
    py_compiler = PyDict_GetItemString(py_builtins, "compile");
    if(!py_compiler)
    {
        dengineutils_logging_log("ERROR::cannot get python compiler!");
    }

    return append_dengine && py_builtins && py_compiler;
}

PyScript* denginescript_python_new(const char* src, const char* name)
{
    if(!py_compiler)
        return NULL;

    PyObject* bytecode = PyObject_CallFunction(py_compiler,"sss",
                                                src, name, "exec");
    if(!bytecode)
    {
        dengineutils_logging_log("ERROR::failed to compile script %s", name);
        PyErr_Print();
        return NULL;
    }

    PyObject* module = NULL,* fn_start = NULL,* fn_update;
    PyScript* script = NULL;
    module =  PyImport_ExecCodeModule(name, bytecode);
    if(!module)
    {
        dengineutils_logging_log("ERROR::failed to import script %s", name);
        PyErr_Print();
        return NULL;
    }else
    {
        script = calloc(1, sizeof(PyScript));
        script->bytecode = bytecode;
        script->module = module;

        fn_start = PyObject_GetAttrString(module, "start");
        PyErr_Clear();
        if(fn_start)
            script->fn_start = fn_start;

        fn_update = PyObject_GetAttrString(module, "update");
        PyErr_Clear();
        if(fn_update)
            script->fn_update = fn_update;
    }

    return script;
}

void denginescript_python_call(const PyScript* script, PyScriptFunc func, PyObject* args)
{
    PyObject* callable = script->fn_start;
    if(func == DENGINE_SCRIPT_FUNC_UPDATE)
        callable = script->fn_update;

    if(callable)
    {
        PyObject_CallObject(callable, args);
        PyErr_Print();
    }
}

void denginescript_terminate()
{
    Py_Finalize();
}
