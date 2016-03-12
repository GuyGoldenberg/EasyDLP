from ConfigParser import SafeConfigParser


class ConfigHandler(object, SafeConfigParser):
    CONFIG_FILENAME = "config.ini"
    STRINGS_FILENAME = "strings.ini"

    def __init__(self, section=None, strings=False):
        SafeConfigParser.__init__(self)
        self.section = section
        if strings:
            self.read(ConfigHandler.STRINGS_FILENAME)
        else:
            self.read(ConfigHandler.CONFIG_FILENAME)
        if section is not None:

            self.section_vars = dict(self.items(section))

    def save(self):
        with open(ConfigHandler.CONFIG_FILENAME, 'w') as f:
            self.write(f)

    def get(self, section=None, option=None, raw=False, vars=None):
        if section is None:
            return super(ConfigHandler, self).get(self.section, option)
        else:
            return super(ConfigHandler, self).get(section, option)

    def __getattribute__(self, item):
        """
        :rtype: str
        """
        if item.startswith("s_") and hasattr(self, "section_vars"):
            return self.section_vars.get(item, None)
        else:
            return super(ConfigHandler, self).__getattribute__(item)

