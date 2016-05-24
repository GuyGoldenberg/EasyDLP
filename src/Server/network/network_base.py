# coding=utf-8

import ctypes
import logger
from network import DLL_PATH, STRINGS

LOGGER = None

class NetworkError(Exception):
    def __init__(self, id, name):
        self.id = id
        self.name = name

    def __str__(self):
        return str("{0} [{1}]".format(self.name, self.id))


class NetworkBase:
    def __init__(self, p_network_obj=None):

        self.network = ctypes.CDLL(DLL_PATH)
        self.__kill = False
        if p_network_obj is not None:
            self.p_network_obj = p_network_obj
        else:
            self.p_network_obj = self.network.New_NetworkBase()
        return_codes_list = ["wsastrartup_error", "getaddrinfo_error", "socket_create_error", "connect_error",
                             "send_error", "bind_error", "listen_error", "accept_error", "nullptr_error",
                             "settimeout_error"]
        self.return_codes = dict((id + 1, item) for id, item in enumerate(return_codes_list))

    def check_for_errors(self, return_code):
        return_name = self.return_codes.get(return_code)
        if return_name is not None and return_name.endswith("_error"):
            raise NetworkError(return_code, return_name)

    @staticmethod
    def format_error(template, e):
        """
        :type template: str
        :type e: NetworkError
        :return: formatted string
        """
        if template is not None:
            return template.format(error_name=e.name, error_id=e.id)
        else:
            return "Error displaying error ♥. If you got here you should quit programming!"

    def connect(self, ip, port):
        ip = ctypes.cast(ip, ctypes.c_char_p)
        port = ctypes.c_char_p(str(port))
        try:
            result = self.network.net_connect(self.p_network_obj, ip, port)
            self.check_for_errors(result)
        except NetworkError as e:
            LOGGER.error(self.format_error(STRINGS.connect_error, e))
            raise e
        except Exception as e:
            LOGGER.error(str(e))
            raise e
        return True

    def bind(self, port):
        port = ctypes.c_char_p(str(port))
        try:
            result = self.network.net_bind(self.p_network_obj, port)
            self.check_for_errors(result)
        except NetworkError as e:
            LOGGER.error(self.format_error(STRINGS.bind_error, e))
            raise e
        except Exception as e:
            LOGGER.error(str(e))
            raise e
        return True

    def listen(self, backlog):
        backlog = ctypes.c_int(backlog)
        try:
            result = self.network.net_listen(self.p_network_obj, backlog)
            self.check_for_errors(result)
        except NetworkError as e:
            LOGGER.error(self.format_error(STRINGS.listen_error, e))
            raise e
        except Exception as e:
            LOGGER.error(str(e))
            raise e
        return True

    def accept(self):
        try:
            result = self.network.net_accept(self.p_network_obj)
            self.check_for_errors(result)
            if result == 0:
                raise NetworkError(-1, STRINGS.nullptr)
        except NetworkError as e:
            if e.id != -1:
                LOGGER.error(self.format_error(STRINGS.accept_error, e))
            else:
                LOGGER.error(STRINGS.nullptr.format(error_name="accept"))
            raise e
        except Exception as e:
            LOGGER.error(str(e))
            raise e
        client = NetworkBase(p_network_obj=result)
        return [client, client.get_info()]

    def get_info(self):
        return ctypes.cast(self.network.net_getinfo(self.p_network_obj), ctypes.c_char_p).value

    def recv(self, buffer_size, client=None):
        buffer_size = ctypes.c_int(buffer_size)
        result = None
        try:
            if client is None:
                result = self.network.net_recv(self.p_network_obj, buffer_size)
            else:
                result = self.network.net_recv(client, buffer_size)
            if result == 0:
                raise NetworkError(-1, STRINGS.nullptr.format(error_name="recv"))
        except NetworkError as e:
            if e.id != -1:
                LOGGER.error(self.format_error(STRINGS.recv_error, e))
            else:
                LOGGER.error(STRINGS.nullptr.format(error_name="recv"))
            raise e
        except Exception as e:
            LOGGER.error(str(e))
            raise e
        if result > 0:
            return ctypes.cast(result, ctypes.c_char_p).value

    def send(self, data, client=None):
        data = ctypes.cast(data, ctypes.c_char_p)
        try:
            if client is None:
                result = self.network.net_send(self.p_network_obj, data)
            else:
                result = self.network.net_send(client, data)
        except NetworkError as e:
            LOGGER.error(self.format_error(STRINGS.send_error, e))
            raise e
        except Exception as e:
            LOGGER.error(str(e))
            raise e
        return True

    def settimeout(self, timeout):
        timeout = ctypes.c_int(timeout)
        try:
            result = self.network.net_settimeout(self.p_network_obj, timeout)
            self.check_for_errors(result)
        except NetworkError as e:
            LOGGER.error(self.format_error(STRINGS.settimeout_error, e))
            raise e
        except Exception as e:
            LOGGER.error(str(e))
            raise e
