from ConfigParser import SafeConfigParser


class ConfigHandler(object, SafeConfigParser):
    CLIENT, SERVER, STRINGS = range(3)


    CLIENT_FILENAME = "client.ini"
    SERVER_FILENAME = "server.ini"
    STRINGS_FILENAME = "strings.ini"
    CONFIG_FILENAME = "config.ini"

    def __init__(self, conf_type=None, section=None):
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
        with open(ConfigHandler.CONFIG_FILENAME, 'w') as f:
            self.write(f)

    def get(self, section=None, option=None, raw=False, vars=None):
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
