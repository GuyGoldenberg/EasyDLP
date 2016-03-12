import os
import config_handler

__all__ = ["network_base"]

NETWORK_CONFIG_CATEGORY = "Network"

CONFIG = config_handler.ConfigHandler(NETWORK_CONFIG_CATEGORY)
STRINGS = config_handler.ConfigHandler(NETWORK_CONFIG_CATEGORY, strings=True)

DLL_FILENAME = CONFIG.get(option="dll_file_name")
#DLL_PATH = "{0}\\{1}".format(os.path.dirname(os.path.realpath(__file__)), DLL_FILENAME)
DLL_PATH = r"C:\Users\Guy\Documents\EasyDLP\NetworkLib\Release\network_lib.dll"
