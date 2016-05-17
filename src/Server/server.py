import datetime

import network
import network.network_base
import threading
import Queue
import json
import os
import config_handler
import thread
import sqlite3
import logger

LOGGER = logger.Logger("serverLogger")

PROTOCOL_CATEGORY = "protocol"
PROTO_CODES = "protocol_codes"
NETWORK_CATEGORY = "network"
DATABASE_CATEGORY = "database"

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
                         "get_rules": network.CONFIG.getint(PROTO_CODES, "get_rules")
                         }

# Load server configs
SERVER_PORT = SERVER_CONFIG.get(NETWORK_CATEGORY, "port")
INCIDENTS_FILE = SERVER_CONFIG.get("default", "incidents_file")
MAX_RECV = SERVER_CONFIG.getint(NETWORK_CATEGORY, "recv_buffer_size")

# Load database configs
DATABASE_NAME = SERVER_CONFIG.get(DATABASE_CATEGORY, "filename")
DATABASE_STRUCTURE_FILENAME = SERVER_CONFIG.get(DATABASE_CATEGORY, "structure")


class DatabaseHandler:
    def __init__(self, lock):
        self.db = sqlite3.connect(DATABASE_NAME, check_same_thread=False)
        self.create_tables()
        self.lock = lock

    def create_tables(self):
        try:
            with open(DATABASE_STRUCTURE_FILENAME) as f:
                structure = json.load(f)

        except:
            LOGGER.error("Couldn't open database structure file: {file}".format(file=DATABASE_STRUCTURE_FILENAME))
            exit()

        for table in structure:
            query = "create table if not exists {table} ({structure})"
            structure_query = []
            for key, type in structure[table].iteritems():
                structure_query.append("{key} {type}".format(key=key, type=type))
            query = query.format(table=table, structure=",".join(structure_query))
            self.db.execute(query)

    def insert_data(self, table, fields, values):
        self.lock.acquire()
        query = "INSERT INTO {table}({fields}) VALUES ({questions})".format(table=table, fields=",".join(fields), questions=",".join("?" for x in xrange(len(values))))
        cursor = self.db.cursor()
        cursor.execute(query, values)
        self.db.commit()
        self.lock.release()

    def update_data(self, table, fields, values, id_name, id_value):
        self.lock.acquire()
        # set_data_string = ",".join(["{key}={value}".format(key=item[0], value=item[1])for item in zip(fields, values)])

        set_data_string = ",".join("{key}=?".format(key=item) for item in fields)

        query = "UPDATE {table} set {data} WHERE {idname} = ?".format(table=table, data=set_data_string,
                                                                      idname=id_name)
        cursor = self.db.cursor()
        cursor.execute(query, values + [id_value])
        self.db.commit()

        self.lock.release()

    def add_incident(self, incident_info):
        pass

    def new_connection(self, type, addr, uid, time, auth_status):
        self.insert_data("connections", ["uId", "userType", "address", "connectionTime", "authenticated"],
                         [uid, type, addr, time, auth_status])

    def set_connection_auth_state(self, uid, state):
        self.update_data("connections", ["authenticated"], [state], "uId", uid)

    def add_rule(self, rule_info):
        pass

    def delete_rule(self, rule_id):
        pass

    def modify_rule(self, rule_id, rule_info):
        pass

    def get_incidents(self):
        pass

    def get_rules(self):
        pass

    def get_connections(self):
        pass


class Server(object):
    def __init__(self):
        super(Server, self).__init__()
        self.client_list = []
        self.db = DatabaseHandler(threading.Lock())
        self.client_messages = Queue.Queue()
        self.server_socket = network.network_base.NetworkBase()
        self.init_server_socket(int(SERVER_PORT))
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
            client_socket, client_addr = self.server_socket.accept()
            client = ClientHandler(client_socket, self.client_messages, self, client_addr, self.db)

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
            LOGGER.info("Client removed: {uid}".format(uid=client.uid))
            client.kill()
            self.client_list.remove(client)


class ClientHandler(threading.Thread):
    auth_statuses = {"auth_no": SERVER_CONFIG.getint("default", "auth_no"),
                     "auth_ok": SERVER_CONFIG.getint("default", "auth_ok"),
                     "auth_complete": SERVER_CONFIG.getint("default", "auth_complete")}
    client_types = {"unknown": -1, "hook": 0, "injector": 1, "admin": 2}

    def __init__(self, client_socket, client_messages, server, client_addr, db):
        super(ClientHandler, self).__init__()
        self.db = db
        self.client_socket = client_socket
        self.client_messages = client_messages
        self.server = server
        self.addr = client_addr
        self.authenticated = self.auth_statuses["auth_no"]
        self.status = None
        self.uid = self.addr
        self.client_info = {}
        self.client_type = self.client_types["unknown"]
        self.__kill = threading.Event()
        LOGGER.info("New client connected: {address}".format(address=self.addr))
        self.db.new_connection(self.client_type, self.addr, self.uid, datetime.datetime.now(), self.authenticated)

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

    def authenticate_admin(self, message):
        pass

    def authenticate(self, message):
        if self.authenticated == self.auth_statuses["auth_complete"]:
            return

        if not self.is_auth() and message.strip() in [CLIENT_HELLO, INJECTOR_HELLO, ADMIN_HELLO]:
            self.client_socket.send(SERVER_HELLO)
            self.authenticated = self.auth_statuses["auth_ok"]
            if message.strip() == CLIENT_HELLO:
                self.client_type = self.client_types["hook"]
            elif message.strip() == INJECTOR_HELLO:
                self.client_type = self.client_types["injector"]
            elif message.strip() == ADMIN_HELLO:
                self.client_type = self.client_types["admin"]
            return

        self.db.update_data("connections", ["userType", "authenticated"], [self.client_type, self.authenticated], "uId", self.uid)

        if self.client_type == self.client_types["admin"]:
            self.authenticate_admin()
            return

        if self.authenticated == self.auth_statuses["auth_ok"]:
            response = message.splitlines()
            self.status, uid = response[0].strip().split(" ")
            if int(self.status) != PROTOCOL_STATUS_CODES["authentication"]:
                return
            self.authenticated = self.auth_statuses["auth_complete"]
            self.db.update_data("connections", ["uId", "authenticated"], [uid,self.authenticated], "uId", self.uid)
            self.uid = uid
            # TODO: check if client already runs.

            response = response[1:]
            if len(response) > 0:
                for line in response:
                    key_end = line.find(":")
                    if key_end > 0:
                        key = response[0:key_end]
                        value = response[key_end + 1:].strip()
                        self.client_info[key] = value

            self.send(PROTOCOL_STATUS_CODES["ok"])
            LOGGER.info("{client_info} has authenticated".format(client_info=self))
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

    def send_rules(self):
        if self.client_type == self.client_types["hook"]:
            pass
        elif self.client_type == self.client_types["injector"]:
            self.send("{\"inject_to\":[\"notepad++.exe\", \"firefox.exe\"]}")
        elif self.client_type == self.client_types["admin"]:
            pass

    def __str__(self):
        if self.is_auth():
            return "{type} [Address: {address} | UID: {uid}]".format(address=self.addr, uid=self.uid,
                                                                     type=self.client_types.keys()[
                                                                         self.client_types.values().index(
                                                                                 self.client_type)])
        else:
            return "Client {address} is not authenticated".format(address=self.addr)


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
            if data.isnumeric():
                status = int(data)
            else:
                return
        else:
            status = int(data.split(" ")[0])

        if status == PROTOCOL_STATUS_CODES["incident_info"]:
            incident_information = json.loads(data[data.find(" "):])
            client.new_incident(incident_information)
            client.send(PROTOCOL_STATUS_CODES["ok"])
        elif status == PROTOCOL_STATUS_CODES["get_rules"]:
            client.send_rules()


if __name__ == "__main__":
    Server()
