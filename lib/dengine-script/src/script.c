#include "dengine-script/script.h"

#include "dengine-utils/logging.h"
#include "dengine-utils/filesys.h"

PyMODINIT_FUNC PyInit_inpt(); //input_mod
PyMODINIT_FUNC PyInit_timer(); //timer_mod
PyMODINIT_FUNC PyInit_logging(); //logging_mod
PyMODINIT_FUNC PyInit_filesys(); //filesys_mod

PyMODINIT_FUNC PyInit_common(); //common_mod
PyMODINIT_FUNC PyInit_scene(); //scene_mod

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
    Py_DontWriteBytecodeFlag = 1;
    Py_NoSiteFlag = 1;
    Py_IgnoreEnvironmentFlag = 1;
    Py_InteractiveFlag = 0;
    Py_IsolatedFlag = 1;
    Py_VerboseFlag = 1;

    char boostrap[1024];
    snprintf(boostrap, sizeof(boostrap), "%s/%s", dengineutils_filesys_get_assetsdir(), "scripts/bootstrap");
    dengineutils_logging_log("%s", boostrap);
    wchar_t* path = Py_DecodeLocale(boostrap, NULL);
    Py_SetPath(path);
    PyMem_RawFree(path);

    int append_dengine = PyImport_AppendInittab("dengine", &PyInit_dengine);
    PyImport_AppendInittab("dengine.inpt", &PyInit_inpt);
    PyImport_AppendInittab("dengine.timer", &PyInit_timer);
    PyImport_AppendInittab("dengine.logging", &PyInit_logging);
    PyImport_AppendInittab("dengine.filesys", &PyInit_filesys);
    PyImport_AppendInittab("dengine.common", &PyInit_common);
    PyImport_AppendInittab("dengine.scene", &PyInit_scene);

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

int denginescript_python_compile(const char* src, const char* name, Script* script)
{
    if(!py_compiler)
        return 0;

    PyObject* bytecode = PyObject_CallFunction(py_compiler,"sss",
                                                src, name, "exec");
    if(!bytecode)
    {
        dengineutils_logging_log("ERROR::failed to compile script %s", name);
        PyErr_Print();
        return 0;
    }

    PyObject* module = NULL,* fn_start = NULL,* fn_update;
    module =  PyImport_ExecCodeModule(name, bytecode);
    if(!module)
    {
        dengineutils_logging_log("ERROR::failed to import script %s", name);
        PyErr_Print();
        return 0;
    }else
    {
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

    return 1;
}

void denginescript_python_call(const Script* script, ScriptFunc func, PyObject* args)
{
    PyObject* callable = script->fn_start;
    if(func == DENGINE_SCRIPT_FUNC_UPDATE)
        callable = script->fn_update;

    if(callable)
    {
        PyObject_CallFunctionObjArgs(callable, args, NULL);
        PyErr_Print();
    }
}

void denginescript_terminate()
{
    Py_Finalize();
}
