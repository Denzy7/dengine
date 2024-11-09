#include "dengine-script/python/python.h"
#include "dengine-script/script.h"
#include "dengine-utils/logging.h"
#include "dengine-utils/os.h"
#include "dengine-utils/filesys.h"
#include "dengine-utils/stream.h"
#include "dengine-utils/zipread.h"

extern PyObject* PyInit_inpt(); //input_mod
extern PyObject* PyInit_timer(); //timer_mod
extern PyObject* PyInit_logging(); //logging_mod
extern PyObject* PyInit_filesys(); //filesys_mod

extern PyObject* PyInit_common(); //common_mod
extern PyObject* PyInit_scene(); //scene_mod
                               //
extern unsigned char importhook_py[];
PyObject* dengineinit(PyObject* self, PyObject* args)
{
    const char* famouswords = "Hello World!";
    printf("%s\n", famouswords);
    return PyUnicode_FromString(famouswords);
}

PyMethodDef DengineInitMethod[]=
{
    { "__init__", dengineinit, METH_VARARGS, "Returns and prints some famous words" },
    { NULL, NULL, 0, NULL}
};

PyModuleDef dengine =
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

struct _inittab denginescript_python_inittab[] = 
{
    {"dengine", PyInit_dengine},
    {"dengine.inpt", PyInit_inpt},
    {"dengine.timer", PyInit_timer},
    {"dengine.logging", PyInit_logging},
    {"dengine.filesys", PyInit_filesys},
    {"dengine.common", PyInit_common},
    {"dengine.scene", PyInit_scene},
    {0, 0}
};

DENGINE_EXPORT int denginescript_python_compile(const char* src, const char* name, Script* script)
{
    memset(script, 0, sizeof(Script));
    PyObject* py_builtins = NULL,* py_compiler = NULL;
    py_builtins = PyEval_GetBuiltins();
    py_compiler = PyDict_GetItemString(py_builtins, "compile");
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

    script->type = DENGINE_SCRIPT_TYPE_PYTHON;

    return 1;
}

int denginescript_python_isinit()
{
    return Py_IsInitialized();
}

void denginescript_python_terminate()
{
    Py_Finalize();
}

int denginescript_python_init()
{
    int usingbootstrap = 0;
#ifndef DENGINE_USE_SYSTEM_PYTHON
    //Py_DontWriteBytecodeFlag = 1;
    Py_NoSiteFlag = 1;
    //Py_IgnoreEnvironmentFlag = 1;
    //Py_InteractiveFlag = 0;
    //Py_IsolatedFlag = 1;

    char boostrap[8192];
    snprintf(boostrap, sizeof(boostrap),
            "%s/python_bootstrap/%s",
            dengineutils_filesys_get_filesdir_dengine(),DENGINE_VERSION);

    if(!dengineutils_os_direxist(boostrap))
    {
        char* bootstrapfix = strdup(boostrap);

        static const char* bootstrap_zip_file = "scripts/bootstrap.zip";
        Stream bootstrap_zip_stream;
        int streamok;
#if defined(DENGINE_ANDROID)
        streamok = dengineutils_stream_new(bootstrap_zip_file,
                DENGINEUTILS_STREAM_TYPE_ANDROIDASSET, 
                DENGINEUTILS_STREAM_MODE_READ,
                &bootstrap_zip_stream);
#else
        snprintf(boostrap, sizeof(boostrap), "%s/%s", dengineutils_filesys_get_assetsdir(), bootstrap_zip_file);
        streamok = dengineutils_stream_new(boostrap,
                DENGINEUTILS_STREAM_TYPE_FILE, DENGINEUTILS_STREAM_MODE_READ,
                &bootstrap_zip_stream);
#endif
        if(!streamok)
        {
            dengineutils_logging_log("ERROR::Cannot open stream to %s", bootstrap_zip_file);
            free(bootstrapfix);
            return 0;
        }

        ZipRead bootstrap_zip_zipread;
        int stat = dengineutils_zipread_load(&bootstrap_zip_stream, &bootstrap_zip_zipread);
        if(!stat)
        {
            dengineutils_logging_log("ERROR::Cannot ZipRead %s", bootstrap_zip_file);
            free(bootstrapfix);
            return 0;
        }

        /* i know python can bootstrap from zip but using a filesystem path
         * does not need linking zlib to open zip file
         */
        stat = dengineutils_zipread_decompress_zip(&bootstrap_zip_stream, &bootstrap_zip_zipread, bootstrapfix);
        if(!stat)
        {
            dengineutils_logging_log("ERROR::Cannot extract %s to %s", bootstrap_zip_file, bootstrapfix);
            free(bootstrapfix);
            return 0;
        }

        dengineutils_logging_log("TODO::Extracted bootstrap.zip to %s. %hu records processed", bootstrapfix, bootstrap_zip_zipread.eocdr.cd_records);

        dengineutils_stream_destroy(&bootstrap_zip_stream);
        dengineutils_zipread_free(&bootstrap_zip_zipread);

        snprintf(boostrap, sizeof(boostrap), "%s", bootstrapfix);

        free(bootstrapfix);
    }

    wchar_t* path = Py_DecodeLocale(boostrap, NULL);
    Py_SetPythonHome(path);
    Py_SetPath(path);
    PyMem_RawFree(path);

    if(usingbootstrap)
        dengineutils_logging_log("INFO::initialzing python from bootstrap.zip. This might take some seconds...");
#endif

    if(PyImport_ExtendInittab(denginescript_python_inittab) == -1)
    {
        dengineutils_logging_log("ERROR::Failed to import dengine frozen modules. Out of MEMORY!");
        return 0;
    }

    Py_InitializeEx(0);
    if(Py_IsInitialized()){
        dengineutils_logging_log("INFO::Using Python: %s", Py_GetVersion());
    }

    if(usingbootstrap)
        dengineutils_logging_log("INFO::done!");
    //Import hook
    PyRun_SimpleString((const char*)importhook_py);

    /* OK PYTHON. I'LL DO IT MYSELF */
    size_t i = 0;
    while(1)
    {
        struct _inittab* tab = &denginescript_python_inittab[i];
        if(!tab->name)
            break;
        PyObject* ready = tab->initfunc();
        Py_DECREF(ready);
        i++;
    }

    return 1;
}

DENGINE_EXPORT int denginescript_python_call(const Script* script, ScriptFunc func, PyObject* args)
{
    PyObject* callable = script->fn_start;
    PyObject* ret = NULL;
    if(func == DENGINE_SCRIPT_FUNC_UPDATE)
        callable = script->fn_update;

    if(callable)
    {
        ret = PyObject_CallFunctionObjArgs(callable, args, NULL);
        PyErr_Print();
    }
    if(ret)
        return 1;
    else
        return 0;
}
