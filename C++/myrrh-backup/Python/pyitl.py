#
# Copyright John Reid 2010
#

"""
Python code for pyitl module.
"""

from _pyitl import *

def integral_interval_first(interval):
    "@return: The first element in the interval assuming integral type."
    if interval.is_left(closed_bounded):
        return interval.lower
    else:
        return interval.lower + 1


def integral_interval_last(interval):
    "@return: The last element in the interval assuming integral type."
    if interval.is_right(closed_bounded):
        return interval.upper
    else:
        return interval.upper - 1


def integral_interval_range(interval):
    "@return: A range that represents the integral interval."
    return range(interval.first, interval.last+1)


def integral_interval_xrange(interval):
    "@return: A xrange that represents the integral interval."
    return xrange(interval.first, interval.last+1)


def interval_gap(interval1, interval2):
    "@return: The gap between the 2 intervals."
    if interval2 in interval1 or interval1 in interval2:
        result = interval1.whole()
        result.clear()
        return result
    if interval2 < interval1:
        interval1, interval2 = interval2, interval1
    return interval1.hull(interval2).left_subtract(interval1).right_subtract(interval2)
