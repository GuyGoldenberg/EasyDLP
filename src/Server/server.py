from network import CONFIG, network_base
import threading
import Queue
import json
import os
import config_handler
import thread

PROTOCOL_CATEGORY = "protocol"
PROTO_CODES = "protocol_codes"
NETWORK_CATEGORY = "network"

# Load config files
CONFIG = CONFIG
STRINGS = config_handler.ConfigHandler(conf_type=config_handler.ConfigHandler.STRINGS)
SERVER_CONFIG = config_handler.ConfigHandler(conf_type=config_handler.ConfigHandler.SERVER)

# Load protocol configs
SERVER_HELLO = CONFIG.get(PROTOCOL_CATEGORY, "server_hello")
CLIENT_HELLO = CONFIG.get(PROTOCOL_CATEGORY, "client_hello")
SOCKET_CLOSE_DATA = CONFIG.get(PROTOCOL_CATEGORY, "socket_close").decode('string_escape')

# Load proto codes configs
PROTOCOL_STATUS_CODES = {"ok": CONFIG.get(PROTO_CODES, "ok"),
                         "error": CONFIG.get(PROTO_CODES, "error"),
                         "incident_info": CONFIG.get(PROTO_CODES, "incident_info"),
                         "authentication": CONFIG.get(PROTO_CODES, "authentication")
                         }

# Load server configs
SERVER_PORT = SERVER_CONFIG.get(NETWORK_CATEGORY, "port")
INCIDENTS_FILE = SERVER_CONFIG.get("default", "incidents_file")
MAX_RECV = SERVER_CONFIG.getint(NETWORK_CATEGORY, "recv_buffer_size")



class Server(object):
    def __init__(self):
        super(Server, self).__init__()
        self.client_list = []
        self.client_messages = Queue.Queue()
        self.server_socket = network_base.NetworkBase()
        self.init_server_socket(int(SERVER_PORT))  # TODO: import port from config
        accept_th = self.start_accepting()
        message_th = self.handle_messages()
        accept_th.join()
        message_th.join()
        for client in self.client_list:
            client.join()

    def init_server_socket(self, port):
        self.server_socket.bind(str(port))
        self.server_socket.listen(5)

    def start_accepting(self):
        accept_th = threading.Thread(target=self.accept_clients)
        accept_th.daemon = True
        accept_th.start()
        return accept_th

    def accept_clients(self):
        while True:
            client_socket = self.server_socket.accept()
            client = ClientHandler(client_socket, self.client_messages, self)
            self.client_list.append(client)
            client.daemon = True
            client.start()

    def handle_messages(self):
        message_th = MessageHandler(self.client_messages, self)
        message_th.daemon = True
        message_th.start()
        return message_th

    def remove_client(self, client):
        if client in self.client_list:
            client.kill()
            self.client_list.remove(client)


class ClientHandler(threading.Thread):
    auth_statuses = {"auth_no": SERVER_CONFIG.get("default", "auth_no"),
                     "auth_ok": SERVER_CONFIG.get("default", "auth_ok"),
                     "auth_complete": SERVER_CONFIG.get("default", "auth_complete")}

    def __init__(self, client_socket, client_messages, server):
        super(ClientHandler, self).__init__()
        self.client_socket = client_socket
        self.client_messages = client_messages
        self.server = server
        self.authenticated = self.auth_statuses["auth_no"]
        self.status = None
        self.uid = None
        self.client_info = {}
        self.__kill = threading.Event()

    def run(self):
        while True:
            if self.__kill.is_set():
                thread.exit()
                return
            else:
                data = self.client_socket.recv(MAX_RECV)
                if not self.closed_socket(data):
                    self.client_messages.put([self, data])
        return

    def closed_socket(self, data):
        if data == SOCKET_CLOSE_DATA:
            self.server.remove_client(self)
            return True
        return False

    def send(self, data):
        self.client_socket.send(str(data))

    def is_auth(self):
        return self.authenticated == self.auth_statuses["auth_complete"]

    def kill(self):
        self.__kill.set()

    def authenticate(self, message):
        if self.authenticated == self.auth_statuses["auth_complete"]:
            return

        if not self.authenticated and message.strip() == CLIENT_HELLO:
            self.client_socket.send(SERVER_HELLO)
            self.authenticated = self.auth_statuses["auth_ok"]
            return

        if self.authenticated == self.auth_statuses["auth_ok"]:
            response = message.splitlines()
            self.status, self.uid = response[0].strip().split(" ")
            if int(self.status) != PROTOCOL_STATUS_CODES["authentication"]:
                return

            # TODO: check if client already runs.

            response = response[1:]
            if len(response) > 0:
                for line in response:
                    key_end = line.find(":")
                    if key_end > 0:
                        key = response[0:key_end]
                        value = response[key_end + 1:].strip()
                        self.client_info[key] = value
            self.authenticated = self.auth_statuses["auth_complete"]
            self.send(PROTOCOL_STATUS_CODES["ok"])
            print "New client authenticated: {uid}".format(uid=self.uid)
            return
        return

    def new_incident(self, incident):
        if os.path.isfile(INCIDENTS_FILE):
            with open(INCIDENTS_FILE, 'r') as f:
                current_incidents = json.loads(f.read())
        else:
            current_incidents = {}

        with open(INCIDENTS_FILE, 'w') as f:
            if self.uid in current_incidents:
                current_incidents[self.uid] += [incident]
            else:
                current_incidents[self.uid] = [incident]
            f.write(json.dumps(current_incidents, indent=4))


class MessageHandler(threading.Thread):
    def __init__(self, client_messages, server):
        super(MessageHandler, self).__init__()
        self.client_messages = client_messages
        self.server = server

    def run(self):
        while True:
            self.interpret_message(self.client_messages.get())

    def interpret_message(self, message):
        client, data = message
        if not client.is_auth():
            client.authenticate(data)
            return

        if data.find(" ") == -1:
            return

        status = int(data.split(" ")[0])
        if status == PROTOCOL_STATUS_CODES["incident_info"]:
            incident_information = json.loads(data[data.find(" "):])
            client.new_incident(incident_information)
            client.send("0 OK")
if __name__ == "__main__":
    Server()
