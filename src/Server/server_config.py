import logger
import network
import network.network_base
import config_handler
LOG_FILENAME = "server.log"
LOGGER = logger.logger_generator(LOG_FILENAME)
network.network_base.LOGGER = LOGGER

PROTOCOL_CATEGORY = "protocol"
PROTO_CODES = "protocol_codes"
NETWORK_CATEGORY = "network"
DATABASE_CATEGORY = "database"
ADMIN_CATEGORY = "admin"

# Load config files
CONFIG = network.CONFIG
STRINGS = config_handler.ConfigHandler(conf_type=config_handler.ConfigHandler.STRINGS)
SERVER_CONFIG = config_handler.ConfigHandler(conf_type=config_handler.ConfigHandler.SERVER)

# Load protocol configs
SERVER_HELLO = network.CONFIG.get(PROTOCOL_CATEGORY, "server_hello")
CLIENT_HELLO = network.CONFIG.get(PROTOCOL_CATEGORY, "client_hello")
ADMIN_HELLO = network.CONFIG.get(PROTOCOL_CATEGORY, "admin_hello")
INJECTOR_HELLO = network.CONFIG.get(PROTOCOL_CATEGORY, "injector_hello")

SOCKET_CLOSE_DATA = network.CONFIG.get(PROTOCOL_CATEGORY, "socket_close").decode('string_escape')

# Load proto codes configs
PROTOCOL_STATUS_CODES = {"ok": network.CONFIG.getint(PROTO_CODES, "ok"),
                         "error": network.CONFIG.getint(PROTO_CODES, "error"),
                         "incident_info": network.CONFIG.getint(PROTO_CODES, "incident_info"),
                         "authentication": network.CONFIG.getint(PROTO_CODES, "authentication"),
                         "get_rules": network.CONFIG.getint(PROTO_CODES, "get_rules"),
                         "add_rule": network.CONFIG.getint(PROTO_CODES, "add_rule"),
                         "update_rule": network.CONFIG.getint(PROTO_CODES, "update_rule"),
                         "delete_rule": network.CONFIG.getint(PROTO_CODES, "delete_rule"),
                         "get_log":network.CONFIG.getint(PROTO_CODES, "get_log"),
                         "get_connection_history":network.CONFIG.getint(PROTO_CODES, "get_connection_history"),
                         "get_incident_history":network.CONFIG.getint(PROTO_CODES, "get_incident_history")
                         }

# Load server configs
SERVER_PORT = SERVER_CONFIG.get(NETWORK_CATEGORY, "port")
INCIDENTS_FILE = SERVER_CONFIG.get("default", "incidents_file")
MAX_RECV = SERVER_CONFIG.getint(NETWORK_CATEGORY, "recv_buffer_size")

# Load admin config
ADMIN_LIST_FILENAME = DATABASE_NAME = SERVER_CONFIG.get(ADMIN_CATEGORY, "filename")

# Load database configs
DATABASE_NAME = SERVER_CONFIG.get(DATABASE_CATEGORY, "filename")
DATABASE_STRUCTURE_FILENAME = SERVER_CONFIG.get(DATABASE_CATEGORY, "structure")
