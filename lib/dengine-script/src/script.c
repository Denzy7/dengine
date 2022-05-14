#include "dengine-script/script.h"

#include "dengine-utils/logging.h"
#include "dengine-utils/filesys.h"
#include "dengine-utils/os.h"
#include "dengine-utils/zipread.h"

#ifdef DENGINE_ANDROID
#include "dengine-utils/platform/android.h"
#endif

#ifdef DENGINE_SCRIPTING_PYTHON //init table for main python module

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
#endif

int denginescript_init()
{
#ifdef DENGINE_SCRIPTING_PYTHON
    //Py_DontWriteBytecodeFlag = 1;
    Py_NoSiteFlag = 1;
    //Py_IgnoreEnvironmentFlag = 1;
    //Py_InteractiveFlag = 0;
    //Py_IsolatedFlag = 1;

    int usingbootstrap = 0;
    char boostrap[8192];
    snprintf(boostrap, sizeof(boostrap),
             "%s/python_bootstrap/%s",
             dengineutils_filesys_get_filesdir_dengine(),DENGINE_VERSION);

    if(!dengineutils_os_direxist(boostrap))
    {
        char* bootstrapfix = strdup(boostrap);

        static const char* bootstrap_zip_file = "scripts/bootstrap.zip";
        Stream* bootstrap_zip_stream = NULL;
    #if defined(DENGINE_ANDROID)
        bootstrap_zip_stream = dengineutils_stream_new(bootstrap_zip_file,
                                                       DENGINEUTILS_STREAM_TYPE_ANDROIDASSET, DENGINEUTILS_STREAM_MODE_READ);
    #else
        snprintf(boostrap, sizeof(boostrap), "%s/%s", dengineutils_filesys_get_assetsdir(), bootstrap_zip_file);
        bootstrap_zip_stream = dengineutils_stream_new(boostrap,
                                                       DENGINEUTILS_STREAM_TYPE_FILE, DENGINEUTILS_STREAM_MODE_READ);
    #endif
        if(!bootstrap_zip_stream)
        {
            dengineutils_logging_log("ERROR::Cannot open stream to %s", bootstrap_zip_file);
            free(bootstrapfix);
            return 0;
        }

        ZipRead bootstrap_zip_zipread;
        int stat = dengineutils_zipread_load(bootstrap_zip_stream, &bootstrap_zip_zipread);
        if(!stat)
        {
            dengineutils_logging_log("ERROR::Cannot ZipRead %s", bootstrap_zip_file);
            free(bootstrapfix);
            return 0;
        }

        stat = dengineutils_zipread_decompress_zip(bootstrap_zip_stream, &bootstrap_zip_zipread, bootstrapfix);
        if(!stat)
        {
            dengineutils_logging_log("ERROR::Cannot extract %s to %s", bootstrap_zip_file, bootstrapfix);
            free(bootstrapfix);
            return 0;
        }

        dengineutils_logging_log("TODO::Extracted bootstrap.zip to %s. %hu records processed", bootstrapfix, bootstrap_zip_zipread.eocdr->cd_records);

        dengineutils_stream_destroy(bootstrap_zip_stream);
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

    if(append_dengine < 0) {
        dengineutils_logging_log("ERROR::cannot append dengine module to python!");
    }else {
        if(usingbootstrap)
            dengineutils_logging_log("INFO::done!");
    }

    return append_dengine;
#endif

    //no scripting lang found;
    dengineutils_logging_log("WARNING::Could not find a valid scripting library. Scripting not initialized");
    return 0;
}

#ifdef DENGINE_SCRIPTING_PYTHON
int denginescript_python_compile(const char* src, const char* name, Script* script)
{
    memset(script, 0, sizeof(Script));
    PyObject* py_builtins = NULL,* py_compiler = NULL;
    py_builtins = PyEval_GetBuiltins();
    py_compiler = PyDict_GetItemString(py_builtins, "compile");
    if(!py_compiler)
    {
        dengineutils_logging_log("ERROR::cannot get python compiler!");
        return 0;
    }

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

int denginescript_python_call(const Script* script, ScriptFunc func, const PyObject* args)
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
#endif

void denginescript_terminate()
{
#ifdef DENGINE_SCRIPTING_PYTHON
    Py_Finalize();
#endif
}

int denginescript_compile(const char* src, const char* name, ScriptType type, Script* script)
{
    if(type == DENGINE_SCRIPT_TYPE_PYTHON)
    {
        #ifdef DENGINE_SCRIPTING_PYTHON
        return denginescript_python_compile(src, name, script);
        #else
        dengineutils_logging_log("ERROR::Script %s not compiled. Python was not linked at build", name);
        return 0;
        #endif
    }else if(type == DENGINE_SCRIPT_TYPE_NSL)
    {
        dengineutils_logging_log("WARNING::Why are you compiling an NSL library?");
        return 0;
    }
    {
        dengineutils_logging_log("ERROR::Unknown script type");
        return 0;
    }
}

int denginescript_call(const Script* script, ScriptFunc func, void* args)
{
    if(script->type == DENGINE_SCRIPT_TYPE_PYTHON)
    {
        #ifdef DENGINE_SCRIPTING_PYTHON
        return denginescript_python_call(script, func, args);
        #else
        dengineutils_logging_log("ERROR::Script not callable. Python was not linked at build");
        return 0;
        #endif
    }else if(script->type == DENGINE_SCRIPT_TYPE_NSL)
    {
        return denginescript_nsl_call(script, func, args);
    }else{
        dengineutils_logging_log("ERROR::Unknown script type");
        return 0;
    }
}

int denginescript_isavailable(ScriptType type)
{
    if(type == DENGINE_SCRIPT_TYPE_PYTHON)
    {
        #ifdef DENGINE_SCRIPTING_PYTHON
        return 1;
        #else
        return 0;
        #endif
    }else if(type == DENGINE_SCRIPT_TYPE_NSL)
    {
        return 1;
    }else{
        dengineutils_logging_log("ERROR::Unknown script type");
        return 0;
    }
}

NSL denginescript_nsl_load(const char* file)
{
    NSL load = NULL;
    char* errmsg = NULL;
#ifdef DENGINE_LINUX
    load = dlopen(file, RTLD_LAZY);
    if(!load)
        errmsg = strdup(dlerror());
#elif defined(DENGINE_WIN32)
    char* dlpath = strdup(file);
    for(size_t i = 0; i < strlen(dlpath); i++)
    {
        if(dlpath[i] == '/')
            dlpath[i] = '\\';
    }
    load = LoadLibrary(dlpath);
    if(!load)
    {
          DWORD dw = GetLastError();
//        LPVOID lpMsgBuf;


//        FormatMessage(
//            FORMAT_MESSAGE_ALLOCATE_BUFFER |
//            FORMAT_MESSAGE_FROM_SYSTEM |
//            FORMAT_MESSAGE_IGNORE_INSERTS,
//            NULL,
//            dw,
//            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
//            (LPTSTR) &lpMsgBuf,
//            0, NULL );
//        int len = lstrlen((LPCTSTR)lpMsgBuf);
        errmsg = malloc(256);
        snprintf(errmsg, 256, "Failed to load DLL : %ld", dw);
//        LocalFree(lpMsgBuf);
    }
    free(dlpath);
#else
    dengineutils_logging_log("ERROR::NSL not supported on this platform!");
    return NULL;;
#endif
    if(errmsg)
    {
        dengineutils_logging_log("ERROR::Cannot load NSL. Reason : %s\n", file, errmsg);
        free(errmsg);
    }else
    {
        dengineutils_logging_log("ERROR::Cannot load NSL");
    }
    return load;
}

void denginescript_nsl_free(NSL nsl)
{
#ifdef DENGINE_LINUX
    dlclose(nsl);
#elif defined(DENGINE_WIN32)
    FreeLibrary(nsl);
#endif
}

void* _denginescript_nsl_getsym(const NSL nsl, const char* name)
{
#ifdef DENGINE_LINUX
    return dlsym(nsl, name);
#elif defined(DENGINE_WIN32)
    return GetProcAddress(nsl, name);
#endif
}

int denginescript_nsl_get_script(const char* name, Script* script, const NSL nsl)
{
    memset(script, 0, sizeof(Script));
    char buf[256];
    snprintf(buf, sizeof(buf), "%s_start",name);
    script->nsl_start = _denginescript_nsl_getsym(nsl, buf);
    snprintf(buf, sizeof(buf), "%s_update",name);
    script->nsl_update = _denginescript_nsl_getsym(nsl, buf);
    if(script->nsl_start || script->nsl_update)
    {
        script->type = DENGINE_SCRIPT_TYPE_NSL;
        return 1;
    }else
    {
        dengineutils_logging_log("WARNING::NSL has no script [%s] with start or update", name);
        return 0;
    }
}

int denginescript_nsl_call(const Script* script, ScriptFunc func, void* args)
{
    int nsl_call = 0;
    if(func == DENGINE_SCRIPT_FUNC_START)
    {
        if(script->nsl_start)
            nsl_call = script->nsl_start(args);
    }else if(func == DENGINE_SCRIPT_FUNC_UPDATE)
    {
        if(script->nsl_update)
            nsl_call = script->nsl_update(args);
    }
    return nsl_call;
}
