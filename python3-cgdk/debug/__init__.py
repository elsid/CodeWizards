def log(**kwargs):
    log_dict(kwargs)


def log_dict(value):
    from json import dumps
    print(dumps(value))
