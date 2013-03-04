#
# Copyright John Reid 2011
#

"""
Chow-Liu Trees
==============

`Chow-Liu trees`_ were originally defined in Chow, C. K.; Liu, C. N. (1968),
"Approximating discrete probability distributions with dependence trees",
IEEE Transactions on Information Theory IT-14 (3): 462-467.

.. _Chow-Liu trees: http://en.wikipedia.org/wiki/Chow-Liu_tree



In this module, each data point is presented as a sequence of discrete-valued features. For example suppose we have data, X = {X},
where each x has n=4 features.

>>> X = [
...     'AACC',
...     'AAGC',
...     'AAGC',
...     'GCTC',
...     'ACTC',
... ]
>>> n = len(X[0])


We can calculate the marginal distribution of each feature

>>> import pybool.chow_liu_trees as CLT
>>> for u in xrange(n):
...     print CLT.marginal_distribution(X, u)
defaultdict(<type 'float'>, {'A': 0.80000000000000004, 'G': 0.20000000000000001})
defaultdict(<type 'float'>, {'A': 0.60000000000000009, 'C': 0.40000000000000002})
defaultdict(<type 'float'>, {'C': 0.20000000000000001, 'T': 0.40000000000000002, 'G': 0.40000000000000002})
defaultdict(<type 'float'>, {'C': 1.0})

and also the marginal distribution of a pair of features

>>> print CLT.marginal_pair_distribution(X, 0, 1)
defaultdict(<type 'float'>, {('A', 'A'): 0.60000000000000009, ('G', 'C'): 0.20000000000000001, ('A', 'C'): 0.20000000000000001})

>>> print CLT.marginal_pair_distribution(X, 1, 2)
defaultdict(<type 'float'>, {('A', 'G'): 0.40000000000000002, ('C', 'T'): 0.40000000000000002, ('A', 'C'): 0.20000000000000001})


We can calculate the mutual infomation between all pairs of features

>>> for v in xrange(n):
...     for u in xrange(v):
...         print u, v, CLT.calculate_mutual_information(X, u, v)
0 1 0.223143551314
0 2 0.223143551314
1 2 0.673011667009
0 3 0.0
1 3 0.0
2 3 0.0

        

Finally we can build a Chow-Liu tree

>>> T = CLT.build_chow_liu_tree(X, n)
>>> print T.edges(data=True)
[(0, 1, {'weight': -0.22314355131420974}), (0, 3, {'weight': -0}), (1, 2, {'weight': -0.6730116670092563})]
"""


import numpy as N, networkx as nx
from collections import defaultdict



def marginal_distribution(X, u):
    """
    Return the marginal distribution for the u'th features of the data points, X.
    """
    values = defaultdict(float)
    s = 1. / len(X)
    for x in X:
        values[x[u]] += s
    return values



def marginal_pair_distribution(X, u, v):
    """
    Return the marginal distribution for the u'th and v'th features of the data points, X.
    """
    if u > v:
        u, v = v, u
    values = defaultdict(float)
    s = 1. / len(X)
    for x in X:
        values[(x[u], x[v])] += s
    return values



def calculate_mutual_information(X, u, v):
    """
    X are the data points.
    u and v are the indices of the features to calculate the mutual information for.
    """
    if u > v:
        u, v = v, u
    marginal_u = marginal_distribution(X, u)
    marginal_v = marginal_distribution(X, v)
    marginal_uv = marginal_pair_distribution(X, u, v)
    I = 0.
    for x_u, p_x_u in marginal_u.iteritems():
        for x_v, p_x_v in marginal_v.iteritems():
            if (x_u, x_v) in marginal_uv:
                p_x_uv = marginal_uv[(x_u, x_v)]
                I += p_x_uv * (N.log(p_x_uv) - N.log(p_x_u) - N.log(p_x_v))
    return I


def build_chow_liu_tree(X, n):
    """
    Build a Chow-Liu tree from the data, X. n is the number of features. The weight on each edge is
    the negative of the mutual information between those features. The tree is returned as a networkx
    object.
    """
    G = nx.Graph()
    for v in xrange(n):
        G.add_node(v)
        for u in xrange(v):
            G.add_edge(u, v, weight=-calculate_mutual_information(X, u, v))
    T = nx.minimum_spanning_tree(G)
    return T
            


if '__main__' == __name__:
    import doctest
    doctest.testmod()
