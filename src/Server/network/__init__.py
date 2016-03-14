import os
import config_handler

__all__ = ["network_base"]

CONFIG = config_handler.ConfigHandler()

DLL_FILENAME = CONFIG.get("default", "dll_file_name")
DLL_PATH = "{0}\\{1}".format(os.path.dirname(os.path.realpath(__file__)), DLL_FILENAME)
print DLL_PATH
