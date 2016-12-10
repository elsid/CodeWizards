from subprocess import Popen


class AutoKillProcess:
    def __init__(self, *args, **kwargs):
        self.__args = args
        self.__kwargs = kwargs

    def __enter__(self):
        self.__process = Popen(*self.__args, **self.__kwargs)
        return self.__process

    def __exit__(self, *_):
        if self.__process.returncode is None:
            self.__process.terminate()
