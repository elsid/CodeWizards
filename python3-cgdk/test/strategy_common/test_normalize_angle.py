from hamcrest import assert_that, equal_to
from math import pi

from strategy_common import normalize_angle


def test_for_greater_than_or_equal_minus_pi_and_less_than_or_equal_pi_returns_equal():
    result = normalize_angle(0.3 * pi)
    assert_that(result, equal_to(0.3 * pi))


def test_for_pi_returns_equal():
    result = normalize_angle(pi)
    assert_that(result, equal_to(pi))


def test_for_minus_pi_returns_equal():
    result = normalize_angle(-pi)
    assert_that(result, equal_to(-pi))


def test_for_2_pi_returns_0():
    result = normalize_angle(2 * pi)
    assert_that(result, equal_to(0))


def test_for_3_pi_returns_minus_pi():
    result = normalize_angle(3 * pi)
    assert_that(result, equal_to(-pi))


def test_for_minus_3_pi_returns_pi():
    result = normalize_angle(-3 * pi)
    assert_that(result, equal_to(pi))
