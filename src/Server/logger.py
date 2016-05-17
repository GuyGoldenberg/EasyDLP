import logging
class Logger(logging.Logger):
    # TODO: build a logging library
    def __init__(self, name):
        logging.Logger.__init__(self, name)
