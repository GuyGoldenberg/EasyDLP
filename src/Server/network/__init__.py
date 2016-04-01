import os
import config_handler

__all__ = ["network_base"]
CONFIG = config_handler.ConfigHandler()
STRINGS = config_handler.ConfigHandler(conf_type=config_handler.ConfigHandler.STRINGS, section="network")

DLL_FILENAME = CONFIG.get("default", "dll_file_name")
DLL_PATH = "{0}\\{1}".format(os.path.dirname(os.path.realpath(__file__)), DLL_FILENAME)
DLL_PATH = r"C:\Users\Guy\Documents\EasyDLP\src\Network\Debug\network_lib.dll"
