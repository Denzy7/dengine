import importlib.abc
import importlib.machinery
import sys


class DengineFinder(importlib.abc.MetaPathFinder):
    def find_spec(self, fullname, path, target=None):
        if fullname in sys.builtin_module_names:
            return importlib.machinery.ModuleSpec(
                    fullname,
                    importlib.machinery.BuiltinImporter)


sys.meta_path.append(DengineFinder())

import dengine.logging as logging
logging.log("INFO::Python import hook loaded")
