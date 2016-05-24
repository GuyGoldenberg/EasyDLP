import logging

def logger_generator(filename):
    # debug, info, warning, error, critical
    """
    Generate logger object.
    Create logger object define logging level and logging file and structure.
    :returns logger: the logger object
    """
    LOGGING_FILE = filename

    logging.basicConfig(level=logging.DEBUG)
    logger = logging.getLogger(__name__)
    LEVELS = {'debug': logging.DEBUG,
              'info': logging.INFO,
              'warning': logging.WARNING,
              'error': logging.ERROR,
              'critical': logging.CRITICAL,
              }
    file_logging_level = logging.DEBUG

    level = logging.INFO

    logger.setLevel(logging.DEBUG)

    handler = logging.FileHandler(LOGGING_FILE)
    formatter = logging.Formatter('%(asctime)s - %(levelname)s - %(message)s')
    handler.setFormatter(formatter)
    consoleHandler = logging.StreamHandler()
    consoleFormatter = logging.Formatter('%(levelname)s - %(message)s')
    consoleHandler.setFormatter(consoleFormatter)
    consoleHandler.setLevel(level)
    handler.setLevel(file_logging_level)
    logger.addHandler(consoleHandler)
    logger.addHandler(handler)
    logger.propagate = False

    return logger
