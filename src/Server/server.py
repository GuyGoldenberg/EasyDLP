import datetime
import threading
import Queue
import json
import os
import thread
from server_config import *
import database_handler
import hashlib
import time


def read_log_file(filename):
    content = "Couldn't load log file"
    try:
        with open(filename, 'r') as thefile:
            content = thefile.read()
    except Exception as e:
        LOGGER.error("Couldn't read log file")
    finally:
        return content


class AdminAuthentication:
    def __init__(self, client, db):
        """
        Authenticates an administrator

        :param client:
        :type client: ClientHandler
        :param db:
        :type db: database_handler.DatabaseHandler
        """
        self.client = client
        self.credentials = None
        self.db = db

    def run(self, message):
        """
        Runs the appropriate sequence for admin authentication

        :param message: Admin message containing credentials
        """
        if self.parse_response(message):
            if self.validate_credentials():
                self.client.change_auth_status(self.client.auth_statuses["auth_complete"])
            else:
                LOGGER.warning("Username, password or unique id incorrect at client: {client}".format(
                    client=str(self.client)))
                return
        else:
            LOGGER.warning("Error receiving admin credentials: {client}".format(
                client=str(self.client)))
            return

    def parse_response(self, response):
        """
        Parses the client credentials into a status code and a json object.

        :param response: Full response from client (contains status code and json data)
        :type response: str
        :return: If response is ok and parsed successfully
        :rtype: bool
        """
        error_msg = str(PROTOCOL_STATUS_CODES["error"]) + " Error in authentication(protocol error), Try again."
        if response.find(" ") == -1:
            self.client.send(error_msg)
            return False

        status, credentials = response.split(" ", 1)
        if not status.isdigit() and int(status) == 1:
            self.client.send(error_msg)
            return False

        try:
            credentials_dict = json.loads(credentials)
        except Exception as e:
            self.client.send(error_msg)
            return False

        if "username" not in credentials_dict or "password" not in credentials_dict:
            self.client.send(error_msg)
            return False

        self.credentials = credentials_dict
        return True

    def validate_credentials(self):
        """
        Validates the admin credentials (uId, username, password) with the database

        :return: If credentials are matching the database
        :rtype: bool
        """
        admin_info = self.db.get_data("admins", "uId", self.credentials["uId"])
        response_text = "Username or password incorrect, try again."
        if admin_info is None or len(admin_info) == 0:
            response_text = "You cannot login as administrator from this computer. (unique id incorrect)"
            self.client.send(
                str(PROTOCOL_STATUS_CODES["error"]) + " Error in authentication, {message}".format(
                    message=response_text))
            return
        admin_info = admin_info[0]

        salt = admin_info["salt"]
        db_pass = admin_info["password"]
        db_username = admin_info["username"]
        password = self.credentials["password"]
        new_hashed = hashlib.sha512(password + salt)
        if db_pass == new_hashed.hexdigest() and db_username.lower() == self.credentials["username"].lower():
            self.client.send(str(PROTOCOL_STATUS_CODES["ok"]) + " Authenticated successfully.")
            self.client.uid = self.credentials["uId"]
            return True
        else:
            self.client.send(
                str(PROTOCOL_STATUS_CODES["error"]) + " Error in authentication, {message}".format(
                    message=response_text))
            return False


class Server(object):
    """
    The main server, responsible connecting all of the peaces together.
    Responsible for accepting clients and sending them to the appropriate procedure.
    """

    def __init__(self):
        super(Server, self).__init__()
        self.client_list = []
        self.db = database_handler.DatabaseHandler(threading.Lock())
        self.client_messages = Queue.Queue()
        self.server_socket = network.network_base.NetworkBase()

        self.init_server_socket(int(SERVER_PORT))

        if os.path.isfile(ADMIN_LIST_FILENAME):
            pass

        accept_th = self.start_accepting()
        message_th = self.handle_messages()
        LOGGER.info("The server is running on port {port}".format(port=SERVER_PORT))
        accept_th.join()
        message_th.join()
        for client in self.client_list:
            client.join()

    def init_server_socket(self, port):
        """
        Binds the server to the desired port.

        :param port: desired port
        """
        self.server_socket.bind(str(port))
        self.server_socket.listen(5)

    def start_accepting(self):
        """
        Starts a thread for accepting clients

        :return: Thread which accept clients in a loop
        :rtype: threading.Thread
        """
        accept_th = threading.Thread(target=self.accept_clients)
        accept_th.daemon = True
        accept_th.start()
        return accept_th

    def accept_clients(self):
        """
        Accept new client (designed to run in a thread)
        """
        while True:
            client_socket, client_addr = self.server_socket.accept()
            client = ClientHandler(client_socket, self.client_messages, self, client_addr, self.db)

            self.client_list.append(client)
            client.daemon = True
            client.start()

    def handle_messages(self):
        """
        Starts a thread for client messages interpretation
        :return: Thread which interprets messages according to the protocol
        :rtype: threading.Thread
        """
        message_th = MessageHandler(self.client_messages, self)
        message_th.daemon = True
        message_th.start()
        return message_th

    def remove_client(self, client):
        """
        Remove client from the server
        :param client: Client to kill
        """
        if client in self.client_list:
            LOGGER.info("Client removed: {uid}".format(uid=client.uid))
            client.kill()
            self.client_list.remove(client)


class ClientHandler(threading.Thread):
    """
    Created for every client, responsible for every communication with the client, authentication and more.
    """
    auth_statuses = {"auth_no": SERVER_CONFIG.getint("default", "auth_no"),
                     "auth_ok": SERVER_CONFIG.getint("default", "auth_ok"),
                     "auth_complete": SERVER_CONFIG.getint("default", "auth_complete")}
    client_types = {"unknown": -1, "hook": 0, "injector": 1, "admin": 2}

    def __init__(self, client_socket, client_messages, server, client_addr, db):
        """
        :param client_socket: The socket of the client as accepted
        :type client_socket: network.network_base.NetworkBase
        :param client_messages: A queue of messages to push messages to
        :type client_messages: Queue.Queue
        :param server: The server object
        :type server: server.Server
        :param client_addr: Client full address [ip:port]
        :type client_addr: str
        :param db: Database object
        :type db: database_handler.DatabaseHandler
        """
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
        """
        Receive new message and put it in the stack
        """
        while True:
            if self.__kill.is_set():
                thread.exit()
                return
            else:
                data = self.client_socket.recv(MAX_RECV)

                data = self.receive_full_data(data)
                if data is not None:
                    self.client_messages.put([self, data])
        return

    def receive_full_data(self, data):
        """
        Designed to follow the protocol standard and receive messages from the client correctly.
        This method helps splitting the received data into different client messages using the `content-length`

        :param data: Message from the client, this message may be a some messages in the same variable
        :type data: str
        :return: Last message to push to the stack
        """
        length = None
        if data.find(" ") == -1:
            if not self.closed_socket(data):
                return data

        possible_length = data.split(" ")[0]
        try:
            possible_length_loaded = json.loads(possible_length)
            if "content-length" in possible_length_loaded:
                length = possible_length_loaded["content-length"]
                data = data[len(possible_length) + 1:]
        except:  # Don't care about the exception, probably ValueError.
            return data

        if length == len(data):
            return data

        if length < len(data):
            return_data = data[0:length]
            data = data[length:]
            self.client_messages.put([self, return_data])
            return self.receive_full_data(data)
        if length > len(data):
            data += self.client_socket.recv(length - len(data))
            return data
        return None  # If client disconnected

    def closed_socket(self, data):
        """
        Remove client from server if it has disconnected
        :param data:
        :return:
        """
        if data == SOCKET_CLOSE_DATA:
            self.server.remove_client(self)
            return True
        return False

    def send(self, data):
        """
        Send data to the client.

        :param data: Data to send
        """
        try:
            self.client_socket.send(str(data))
        except Exception as e:
            raise
            LOGGER.error("Error sending data to client [{error}]".format(e.args))
            return

    def is_auth(self):
        """
        Check if client has fully authenticated
        :return: Weather the client is authenticated
        :rtype: bool
        """
        return self.authenticated == self.auth_statuses["auth_complete"]

    def kill(self):
        """
        Kill client
        """
        self.__kill.set()

    def change_auth_status(self, new_status):
        """
        Change the client authentication step
        :param new_status: The new authentication step
        """
        LOGGER.info("Client {client} authentication status has changed to state: {state}".format(client=str(self),
                                                                                                 state=new_status))
        self.authenticated = new_status
        self.db.update_data("connections", ["userType", "authenticated"], [self.client_type, self.authenticated], "uId",
                            self.uid)
        if new_status == self.auth_statuses["auth_complete"]:
            LOGGER.info(
                "Client {client} has authenticated successfully".format(client=str(self)))

    def authenticate(self, message):
        """
        Main function for authenticating a client. Same function for all client types.
        Calls the appropriate function for authentication.
        :param message: Client message (authentication step)
        :rtype: None
        """
        if self.authenticated == self.auth_statuses["auth_complete"]:
            return

        if not self.is_auth() and message.strip() in [CLIENT_HELLO, INJECTOR_HELLO, ADMIN_HELLO]:
            self.change_auth_status(self.auth_statuses["auth_ok"])
            self.send(SERVER_HELLO)
            if message.strip() == CLIENT_HELLO:
                self.client_type = self.client_types["hook"]
            elif message.strip() == INJECTOR_HELLO:
                self.client_type = self.client_types["injector"]
            elif message.strip() == ADMIN_HELLO:
                self.client_type = self.client_types["admin"]
            return

        if self.client_type == self.client_types["admin"]:
            admin = AdminAuthentication(self, self.db)
            admin.run(message)
            return

        if self.authenticated == self.auth_statuses["auth_ok"]:
            response = message.splitlines()
            self.status, uid = response[0].strip().split(" ")
            if int(self.status) != (PROTOCOL_STATUS_CODES["authentication"]):
                return

            self.change_auth_status(self.auth_statuses["auth_complete"])
            self.uid = uid

            self.send(str(PROTOCOL_STATUS_CODES["ok"]) + " Authentication complete")
            LOGGER.info("{client_info} has authenticated".format(client_info=self))
            return
        return

    def new_incident(self, incident):
        """
        Called when the client perform an illegal action.
        :param incident: incident info
        :type incident: dict
        """
        try:
            incident_json = json.loads(incident)
        except Exception as e:
            LOGGER.error("Error loading incident information[{errmsg}], data received:{data}".format(errmsg=str(e.args),
                                                                                                     data=incident))
            self.send(str(PROTOCOL_STATUS_CODES["error"]) + " Incident structure incorrect. Try again")
            return

        self.db.add_incident(incident_json, self.uid)
        LOGGER.info("New incident at " + self.addr)
        self.send(str(PROTOCOL_STATUS_CODES["ok"]) + " Incident added successfully")
        # TODO: Alert admin

    def send_rules(self):
        """
        Send the rules requested by the client
        """
        if self.client_type == self.client_types["hook"]:
            self.send(str(PROTOCOL_STATUS_CODES["ok"]) + " " + json.dumps(self.db.get_rules()))
        elif self.client_type == self.client_types["injector"]:
            processes_black_list = list(set([rule["processName"] for rule in self.db.get_rules()]))
            black_list_by_proto = {"inject_to": processes_black_list}
            self.send(str(PROTOCOL_STATUS_CODES["ok"]) + " " + json.dumps(black_list_by_proto))
        elif self.client_type == self.client_types["admin"]:
            self.send(str(PROTOCOL_STATUS_CODES["ok"]) + " " + json.dumps(self.db.get_rules()))
        LOGGER.info("Rules sent successfully to client: {client}".format(client=str(self)))

    def validate_rule(self, rule, must_fields=[]):
        if self.client_type != self.client_types["admin"]:
            LOGGER.warning(
                "Client tried to perform operation without the appropriate permissions: {client}".format(str(self)))
            return False
        try:
            rule = json.loads(rule)
        except Exception as e:
            LOGGER.error("Error with with rule structure, problem loading json: {msg}".format(e.message))
            self.send(str(PROTOCOL_STATUS_CODES["error"]) + " RULE STRUCTURE INCORRECT")
            return False
        for item in must_fields:
            if item not in rule:
                LOGGER.error("Rule structure is incorrect {client}".format())
                self.send(str(PROTOCOL_STATUS_CODES["error"]) + " RULE STRUCTURE INCORRECT")
                return False
        return True

    def add_rule(self, rule):
        """
        Adds a new rule to the database

        :param rule: rule data in JSON format
        :type rule: str
        """
        if not self.validate_rule(rule, ["processName", "ruleType", "actionToTake", "ruleContent"]):
            return
        try:
            rule = json.loads(rule)
        except ValueError as e:
            self.send(str(PROTOCOL_STATUS_CODES["error"]) + " Error parsing rule. Try again.")
            LOGGER.error("Error parsing rule: {errmsg}".format(errmsg=str(e.args)))
            return
        except Exception as e:
            self.send(str(PROTOCOL_STATUS_CODES["error"]) + " Error parsing rule. Try again.")
            LOGGER.error("Unknown error while parsing rule: {errmsg}".format(errmsg=str(e.args)))
            return

        self.db.add_rule(rule)
        LOGGER.info("New rule added to database by {client}.".format(client=str(self)))
        self.send(str(PROTOCOL_STATUS_CODES["ok"]) + " Rule added successfully")

    def protocol_error(self, msg=""):
        """
        Notify the client if a protocol error has occurred.
        :param msg: Error information to send
        """
        self.send(
            str(PROTOCOL_STATUS_CODES["error"]) + " Error interpreting request (protocol error){msg}".format(
                msg=": " + msg))

    def update_rule(self, rule):
        """
        Update a rule in the database by the rule id
        :param rule: A rule represented as a json dictionary
        :type rule: str
        """
        if not self.validate_rule(rule, ["processName", "ruleType", "actionToTake", "ruleContent", "id"]):
            return

        try:
            rule = json.loads(rule)
        except ValueError as e:
            self.send(str(PROTOCOL_STATUS_CODES["error"]) + " Error parsing rule. Try again.")
            LOGGER.error("Error parsing rule: {errmsg}".format(errmsg=str(e.args)))
            return
        except Exception as e:
            self.send(str(PROTOCOL_STATUS_CODES["error"]) + " Error parsing rule. Try again.")
            LOGGER.error("Unknown error while parsing rule: {errmsg}".format(errmsg=str(e.args)))
            return

        self.db.modify_rule(rule["id"], rule)
        LOGGER.info("Rule #{id} modified successfully by {client}.".format(id=rule["id"], client=str(self)))
        self.send(str(PROTOCOL_STATUS_CODES["ok"]) + " Rule #{id} modified successfully".format(id=rule["id"]))

    def send_log(self, info):
        """
        Send the log of the network module or the server to a client, must be an admin.

        :param info: Which logging file the admin wants
        :return:
        """
        if self.client_type != self.client_types["admin"]:
            LOGGER.warning(
                "Client tried to perform operation without the appropriate permissions: {client}".format(str(self)))
            self.send(str(PROTOCOL_STATUS_CODES["error"]) + " Unauthorized action")
            return False
        log_files = {"server": "server.log", "network": "network.log"}
        if info not in log_files.keys():
            LOGGER.error("Log file requested does not exist! ({file})".format(file=info))
            self.send(str(PROTOCOL_STATUS_CODES["error"]) + " The log file requested doesn't exist")
            return
        log_content = read_log_file(log_files[info])
        data_to_send = str(PROTOCOL_STATUS_CODES["ok"]) + " " + log_content
        LOGGER.info(
            "Log file ({file}) sent to client successfully -> {client}".format(file=log_files[info], client=str(self)))
        self.send(data_to_send)

    def remove_rule(self, rule):
        """
        Remove a rule from the database based on the rule id
        :param rule: A rule represented as a json dictionary
        :type rule: str
        """
        if not self.validate_rule(rule, ["id"]):
            return
        try:
            rule = json.loads(rule)
        except ValueError as e:
            self.send(str(PROTOCOL_STATUS_CODES["error"]) + " Error parsing rule. Try again.")
            LOGGER.error("Error parsing rule: {errmsg}".format(errmsg=str(e.args)))
            return
        except Exception as e:
            self.send(str(PROTOCOL_STATUS_CODES["error"]) + " Error parsing rule. Try again.")
            LOGGER.error("Unknown error while parsing rule: {errmsg}".format(errmsg=str(e.args)))
            return

        self.db.delete_rule(rule["id"])
        LOGGER.info("Rule #{id} removed successfully by {client}.".format(id=rule["id"], client=str(self)))
        self.send(str(PROTOCOL_STATUS_CODES["ok"]) + " Rule #{id} removed successfully".format(id=rule["id"]))

    def send_connection_history(self):
        """
        Send all of the table of connections history
        """
        if self.client_type != self.client_types["admin"]:
            LOGGER.warning(
                "Client tried to perform operation without the appropriate permissions: {client}".format(str(self)))
            return
        LOGGER.info("Connections history sent successfully to client: {client}".format(client=str(self)))
        self.send(str(PROTOCOL_STATUS_CODES["ok"]) + " " + json.dumps(self.db.get_connections()))

    def send_incidents_history(self):
        """
        Send all of the table of incidents history
        """
        if self.client_type != self.client_types["admin"]:
            LOGGER.warning(
                "Client tried to perform operation without the appropriate permissions: {client}".format(str(self)))
            return
        LOGGER.info("Incidents history sent successfully to client: {client}".format(client=str(self)))
        self.send(str(PROTOCOL_STATUS_CODES["ok"]) + " " + json.dumps(self.db.get_incidents()))

    def __str__(self):
        if self.uid != self.addr:
            return "{type} | {address} | {uid}".format(address=self.addr, uid=self.uid,
                                                       type=self.client_types.keys()[
                                                           self.client_types.values().index(
                                                               self.client_type)])
        else:
            return "{type} | {address}".format(address=self.addr,
                                               type=self.client_types.keys()[
                                                   self.client_types.values().index(
                                                       self.client_type)])


class MessageHandler(threading.Thread):
    def __init__(self, client_messages, server):
        super(MessageHandler, self).__init__()
        self.client_messages = client_messages

    def run(self):
        """
        Get a message from the client messages queue and send it to interpretation.
        """
        while True:
            self.interpret_message(self.client_messages.get())

    @staticmethod
    def interpret_message(message):
        """
        Interprets a message according to the protocol status code
        :param message: Message to interpret
        :type message: str
        """
        client, data = message
        if not client.is_auth():
            client.authenticate(data)
            return

        if data.find(" ") == -1:
            if data.isdigit():
                status = int(data)
            else:
                return
        else:
            try:
                status, info = data.split(" ", 1)
                info = info.strip()
                if status.isdigit():
                    status = int(status)
                else:
                    LOGGER.warning(
                        "Protocol error in client {client}. Message: {msg}".format(client=str(client), msg=data))
                    client.protocol_error("Couldn't find request status code")

                    return
            except Exception as e:
                LOGGER.warning("Protocol error in client {client}. Message: {msg}".format(client=str(client), msg=data))
                client.protocol_error(str(e.args))
                return

        # TODO: Transform this to a dictionary with status code as key and method pointer as value
        if status == PROTOCOL_STATUS_CODES["incident_info"]:
            client.new_incident(info)
        elif status == PROTOCOL_STATUS_CODES["get_rules"]:
            client.send_rules()
        elif status == PROTOCOL_STATUS_CODES["add_rule"]:
            client.add_rule(info)
        elif status == PROTOCOL_STATUS_CODES["update_rule"]:
            client.update_rule(info)
        elif status == PROTOCOL_STATUS_CODES["delete_rule"]:
            client.remove_rule(info)
        elif status == PROTOCOL_STATUS_CODES["get_log"]:
            client.send_log(info)
        elif status == PROTOCOL_STATUS_CODES["get_connection_history"]:
            client.send_connection_history()
        elif status == PROTOCOL_STATUS_CODES["get_incident_history"]:
            client.send_incidents_history()
