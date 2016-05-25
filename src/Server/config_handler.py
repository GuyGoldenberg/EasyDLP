from ConfigParser import SafeConfigParser


class ConfigHandler(object, SafeConfigParser):
    """
    A config parser wrapper. Makes it easier to fetch config values

    """
    CLIENT, SERVER, STRINGS = range(3)


    CLIENT_FILENAME = "client.ini"
    SERVER_FILENAME = "server.ini"
    STRINGS_FILENAME = "strings.ini"
    CONFIG_FILENAME = "config.ini"

    def __init__(self, conf_type=None, section=None):
        """

        :param conf_type: Which config file to get data from
        :param section:  Which section in the file to get data from
        """
        SafeConfigParser.__init__(self)
        self.section = section
        if conf_type == ConfigHandler.CLIENT:
            self.read(ConfigHandler.CLIENT_FILENAME)
        elif conf_type == ConfigHandler.SERVER:
            self.read(ConfigHandler.SERVER_FILENAME)
        elif conf_type == ConfigHandler.STRINGS:
            self.read(ConfigHandler.STRINGS_FILENAME)
        else:
            self.read(ConfigHandler.CONFIG_FILENAME)

        if section is not None:
            self.section_vars = dict(self.items(section))
        else:
            self.section_vars = None

    def save(self):
        """
        Save the configuration changed to the file
        """
        with open(ConfigHandler.CONFIG_FILENAME, 'w') as f:
            self.write(f)

    def get(self, section=None, option=None, raw=False, vars=None):
        """
        Get a value from the config file
        :param section: Section to get data from
        :param option: Which key to get data from
        :return: Value of the key in the given section

        """
        if section is None:
            return super(ConfigHandler, self).get(self.section, option)
        else:
            return super(ConfigHandler, self).get(section, option)

    def __getattr__(self, item):
        """
        :rtype: str
        """
        if self.section_vars is not None and item in self.section_vars:
            return self.section_vars.get(item, None)
        else:
            return super(ConfigHandler, self).__getattribute__(item)
