from strategy_common import LazyInit, lazy_init


class LazyInitTest(LazyInit):
    @lazy_init
    def method(self):
        pass

    def _init_impl(self, *_):
        pass


def test_lazy_init():
    obj = LazyInitTest()
    assert obj.initialized is False
    obj.method()
    assert obj.initialized is True
