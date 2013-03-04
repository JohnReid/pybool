#
# Copyright John Reid 2009
#

import _man as man, numpy as N, random


def get_index(a, idx):
    """
    Index an array by a sequence of indices.
    """
    for i in idx:
        a=a[i]
    return a


def array_test(x):
    "Test an array which has no zeros in it."
    assert (x != 0).all()

    # tag to call correct C++ functions
    tag = '%s_%d' % (x.dtype.name, len(x.shape))

    # test from python
    getattr(man, 'test_from_python_shape_%s' % tag)(x, x.shape)

    # test indexing
    index = tuple([random.randint(0, y-1) for y in x.shape])
    assert 0 != get_index(x, index)
    assert 0 != getattr(man, 'test_get_element_%s' % tag)(x, index)
    getattr(man, 'test_set_element_%s' % tag)(x, index, 0)
    assert 0 == get_index(x, index)
    assert 0 == getattr(man, 'test_get_element_%s' % tag)(x, index)



def test_dtype(dtype, num_dims):
    """
    Test arrays of a particular dtype and num_dims
    """
    shape = tuple((N.arange(0, num_dims, dtype=int)%2)+2)
    for order in ['C', 'F']:
        print dtype.name, dtype, num_dims, order

        # create and fill array with different non-zero values
        x = N.empty(shape, dtype=dtype, order=order)
        x.reshape(x.size, order=order)[:] = N.arange(1, x.size+1, dtype=x.dtype)

        array_test(x)

for typecode in man.exposed_typechars:
    dtype = N.dtype(N.typeDict[typecode])
    for num_dims in range(1, min(5, man.max_numpy_dimensions())):
        test_dtype(dtype, num_dims)
