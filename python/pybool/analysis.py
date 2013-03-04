#
# Copyright John Reid 2011
#


"""
Code to analyse the consistent networks.
"""

import numpy as N
from . import network, chow_liu_trees
from collections import defaultdict


def aggregate_possible_regulations(meta_data, networks):
    "Aggregate the possible regulatory relationships in the networks."
    possible_regulations = N.empty((meta_data.G, meta_data.G), dtype=object)
    for net in networks:
        for g1 in xrange(meta_data.G):
            for g2 in xrange(meta_data.G):
                if None == possible_regulations[g1,g2]:
                    possible_regulations[g1,g2] = set()
                possible_regulations[g1,g2].add(net.J[g1,g2])
    return possible_regulations




def count_possible_regulations(meta_data, networks):
    "Count the possible regulatory relationships in the networks."
    possible_regulations = N.empty((meta_data.G, meta_data.G), dtype=object)
    for net in networks:
        for g1 in xrange(meta_data.G):
            for g2 in xrange(meta_data.G):
                if None == possible_regulations[g1,g2]:
                    possible_regulations[g1,g2] = defaultdict(int)
                possible_regulations[g1,g2][net.J[g1,g2]] += 1
    return possible_regulations




def aggregate_possible_thetas(meta_data, networks):
    "Aggregate the possible theta parameters."
    possible_thetas = N.empty((meta_data.G,), dtype=object)
    for net in networks:
        for g in xrange(meta_data.G):
            if None == possible_thetas[g]:
                possible_thetas[g] = set()
            possible_thetas[g].add(net.theta[g])
    return possible_thetas




def count_possible_thetas(meta_data, networks):
    "Count the possible theta parameters."
    possible_thetas = N.empty((meta_data.G,), dtype=object)
    for net in networks:
        for g in xrange(meta_data.G):
            if None == possible_thetas[g]:
                possible_thetas[g] = defaultdict(int)
            possible_thetas[g][net.theta[g]] += 1
    return possible_thetas




def aggregate_possible_inputs(meta_data, networks):
    "Aggregate the possible input parameters."
    possible_inputs = [set() for g in xrange(meta_data.G)]
    for net in networks:
        for g in xrange(meta_data.G):
            possible_inputs[g].add(net.input_parameters[g])
    return possible_inputs




def count_possible_inputs(meta_data, networks):
    "Count how often each possible input parameters occurs."
    input_counts = [defaultdict(int) for g in xrange(meta_data.G)]
    for net in networks:
        for g in xrange(meta_data.G):
            input_counts[g][net.input_parameters[g]] += 1
    return input_counts



def analyse_dependencies(networks):
    """
    Analyse the dependencies between pairs of network features.
    """
    if networks:
        networks_as_features = map(network.network_to_features, networks)
        n = len(networks_as_features[0])
        T = chow_liu_trees.build_chow_liu_tree(networks_as_features, n)
        edges = T.edges(data=True)
        edges.sort(key=lambda x:x[2]['weight'])
        return networks_as_features, T, edges


