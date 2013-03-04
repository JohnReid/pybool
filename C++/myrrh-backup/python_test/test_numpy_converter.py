#
# Copyright John Reid 2009
#

from test_python_numpy_converter import *
import numpy as N

assert check_numpy_conversion_multi_array(N.ones((4,6))).sum() == 4. * 6.
