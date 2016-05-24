import database_handler
import re
import getpass
import random
import string
import hashlib
import binascii


def is_md5(uuid):
    return re.match(r"^[0-9a-f]{32}$", uuid) is not None


def get_admin_credentials():
    while True:
        result = raw_input("Would you like to register a new administrator? [y/n]: ").lower()
        if result == "n":
            print "Registration canceled."
            exit()
        elif result != "y":
            print "Please respond with 'y' (yes) or 'n' (no)."
            continue
        break

    while True:
        uuid = raw_input("Please enter the administrator PC unique id, The unique id can be found in the admin panel:")
        if not is_md5(uuid):
            print "Uuid structure is incorrect."
            continue
        break

    while True:
        username = raw_input("Please enter administrator username: ")
        if re.match(r"^[a-zA-Z]\B[a-zA-Z0-9]{2,18}[a-zA-Z0-9]\b$", username) is None:
            print "Username must contain at least 4 characters, must begin with a letter."
            continue
        break

    while True:
        password = getpass.getpass("Please enter a password (will not be visible): ")
        if re.match(r"^(?=.*\d)(?=.*[a-z])(?=.*[A-Z])(?=.*[a-zA-Z]).{8,}$", password) is None:
            print "Password ,ust contain at least 8 characters. At least one upper case, one lower case and one number."
            continue
        break

    return [uuid, username, password]


def generate_salt():
    alphabet = string.ascii_letters + string.digits + string.punctuation
    salt = ""
    for i in xrange(16):
        salt += random.choice(alphabet)
    return salt


def hash_password(password):
    salt = generate_salt()
    hashed = hashlib.sha512(password + salt).hexdigest()
    return [hashed, salt]


def main():
    db = database_handler.DatabaseHandler()
    uuid, username, password = get_admin_credentials()
    hashed_pass, salt = hash_password(password)
    db.insert_data("admins", ["uId", "username", "password", "salt"], [uuid, username, hashed_pass, salt], )


if __name__ == "__main__":
    main()
