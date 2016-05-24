import sqlite3
from server_config import *
import json

class DatabaseHandler:
    """
    Used to perform operations on the database in a thread secure way

    :param lock: thread lock, used to solve concurrency problems.
    :type lock: threading.Lock
    """

    def __init__(self, lock=None):

        self.db = sqlite3.connect(DATABASE_NAME, check_same_thread=False)
        self.create_tables()
        self.lock = lock
        self.db.row_factory = self.dict_factory
        self.db.text_factory = str


    def create_tables(self):
        """
        Create all the needed tables for server operations. Database structure taken from a json source.
        """
        try:
            with open(DATABASE_STRUCTURE_FILENAME) as f:
                structure = json.load(f)

        except:
            LOGGER.error("Couldn't open database structure file: {file}".format(file=DATABASE_STRUCTURE_FILENAME))
            exit()

        for table in structure:
            query = "CREATE TABLE IF NOT EXISTS {table} ({structure})"
            structure_query = []
            for key, type in structure[table].iteritems():
                structure_query.append("{key} {type}".format(key=key, type=type))
            query = query.format(table=table, structure=",".join(structure_query))
            self.db.execute(query)

    def insert_data(self, table, fields, values):
        """
        Global function to insert data to a given table
        :param table: table to insert data to
        :param fields: fields names to insert
        :param values: value to set
        """
        if self.lock is not None:
            self.lock.acquire()
        query = "INSERT INTO {table}({fields}) VALUES ({questions})".format(table=table, fields=",".join(fields),
                                                                            questions=",".join(
                                                                                "?" for x in xrange(len(values))))
        cursor = self.db.cursor()
        cursor.execute(query, values)
        self.db.commit()
        if self.lock is not None:
            self.lock.release()

    def update_data(self, table, fields, values, id_name, id_value):
        """
        Global function to update data in a given table in the database
        :param table: table to update data in
        :param fields: fields to update
        :param values: values to set
        :param id_name: identifier to match
        :param id_value: value to match identifier to
        """
        if self.lock is not None:
            self.lock.acquire()

        set_data_string = ",".join("{key}=?".format(key=item) for item in fields)

        query = "UPDATE {table} set {data} WHERE {idname} = ?".format(table=table, data=set_data_string,
                                                                      idname=id_name)
        cursor = self.db.cursor()
        cursor.execute(query, values + [id_value])
        self.db.commit()
        if self.lock is not None:
            self.lock.release()

    def delete_data(self, table, id_name, id_value):
        """
        Global function to delete data from a given table in the database

        :param table: Table to delete from
        :param id_name: identifier to delete row by
        :param id_value: value the identifier should be
        """
        if self.lock is not None:
            self.lock.acquire()
        query = "DELETE FROM {table} WHERE {idname} = ?".format(table=table, idname=id_name)
        cursor = self.db.cursor()
        cursor.execute(query, [id_value])
        self.db.commit()
        if self.lock is not None:
            self.lock.release()

    def get_data(self, table, id_name=1, id_value=1):
        """
        Global function to get data from the database at a given table

        :param table: a table to fetch data from
        :type table: str
        :return: dictionary containing the table data
        :rtype: dict
        """
        if self.lock is not None:
            self.lock.acquire()
        cur = self.db.cursor()
        query = "SELECT * FROM {table} WHERE {idname} = ?".format(table=table, idname=id_name)
        print repr(id_value)
        cur.execute(query, [id_value])
        if self.lock is not None:
            self.lock.release()
        return cur.fetchall()

    def add_incident(self, incident_info, uid):
        """
        Add new incident to the database
        :param incident_info: Information about the incident
        :type incident_info: dict
        :param uid: Client unique identifier
        :type uid: int
        """
        self.insert_data("incidents",
                         ["uId", "appTitle", "fileTried", "incidentTime", "appPath", "actionTaken", "appPid",
                          "currentUser"],
                         [uid, incident_info.get("appTitle", None), incident_info.get("fileTried", None),
                          incident_info.get("incidentTime", None),
                          incident_info.get("appPath", None), incident_info.get("actionTaken", None),
                          int(incident_info.get("appPID", None)), incident_info.get("currentUser", None)]
                         )

    def new_connection(self, type, addr, uid, time, auth_status):
        """
        Log a new client connection in the database

        :param type: Client type [hook, injector etc..]
        :param addr: Client address
        :param uid: Client unique identifier
        :param time: Client connection time
        :param auth_status: status in the client authentication
        """
        self.insert_data("connections", ["uId", "userType", "address", "connectionTime", "authenticated"],
                         [uid, type, addr, time, auth_status])

    def set_connection_auth_state(self, uid, state):
        """
        Change the client authentication step. Called during authentication.

        :param uid: Client unique identifier
        :param state: The new auth state
        """
        self.update_data("connections", ["authenticated"], [state], "uId", uid)

    def add_rule(self, rule_info):
        """
        Add new rule to the database
        :param rule_info: Information about the rule
        """
        self.insert_data("rules", ["processName", "ruleType", "actionToTake", "ruleContent"],
                         [rule_info.get("processName"), rule_info.get("ruleType"), rule_info.get("actionToTake"),
                          rule_info.get("ruleContent")])

    def delete_rule(self, rule_id):
        """
        Delete a rule in the database.

        :param rule_id: rule id to delete
        """
        self.delete_data("rules", "id", rule_id)

    def modify_rule(self, rule_id, rule_info):
        """
        Edit a rule in the database

        :param rule_id: id of the rule in the database
        :type rule_id: int
        :param rule_info: information about the rule
        :type rule_info: dict
        """
        self.update_data("rules", ["processName", "ruleType", "actionToTake", "ruleContent"],
                         [rule_info.get("processName"), rule_info.get("ruleType"), rule_info.get("actionToTake"),
                          rule_info.get("ruleContent")],
                         "id", rule_id)

    def dict_factory(self, cursor, row):
        """
        Fetch database results as dictionary

        :return: result dictionary
        :rtype: dict
        """
        d = {}
        for idx, col in enumerate(cursor.description):
            d[col[0]] = row[idx]
        return d

    def get_incidents(self):
        """
        Get all incidents in the database
        :return:
        """
        return self.get_data("incidents")

    def get_rules(self):
        """
        Get all rules in the database
        :return:
        """
        return self.get_data("rules")

    def get_connections(self):
        """
        Get all connections in the database
        :return:
        """
        return self.get_data("connections")
