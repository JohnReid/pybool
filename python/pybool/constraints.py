#
# Copyright John Reid 2010, 2011, 2012, 2013
#

"""
Code to help implement constraints in Boolean networks.
"""

import numpy as N, logging, network, matplotlib as M
from cookbook.cache_decorator import cached_method
from collections import namedtuple


def consecutive(iterable):
    "@return: Yield consecutive items from the iterable as a tuple."
    for x in iterable:
        try:
            yield last, x
        except NameError:
            pass
        last = x



def combinations(iterable, r):
    """
    combinations('ABCD', 2) --> AB AC AD BC BD CD
    combinations(range(4), 3) --> 012 013 023 123
    """
    pool = tuple(iterable)
    n = len(pool)
    if r > n:
        return
    indices = range(r)
    yield tuple(pool[i] for i in indices)
    while True:
        for i in reversed(range(r)):
            if indices[i] != i + n - r:
                break
        else:
            return
        indices[i] += 1
        for j in range(i + 1, r):
            indices[j] = indices[j - 1] + 1
        yield tuple(pool[i] for i in indices)


def gene_off(t, p):
    "External input function to turn a gene off."
    return 0


def gene_on(t, p):
    "External input function to turn a gene on."
    return 1


def combine(c1, c2, union=True):
    "Take 2 constraints and combine them. If union is False, then take the intersection."
    assert c1.shape == c2.shape
    result = N.empty_like(c1)
    for j, (i1, i2) in enumerate(zip(c1.ravel(), c2.ravel())):
        r = set(i1)
        if union:
            r = r.union(i2)
        else:
            r = r.intersection(i2)
        result.ravel()[j] = tuple(r)
    return result


Gene = namedtuple('Gene', 'name color position')
Condition = namedtuple('Condition', 'name inputs constraints merged_inputs')

class MetaData(object):
    "Base meta-data functionality for regulatory networks."

    def __init__(self, allow_X=True):
        "Construct."
        self.genes = []
        self.conditions = []
        self.possible_Js = N.empty((0,0), dtype=object)
        self.possible_thetas = N.empty((0,), dtype=object)
        self.initial_states = N.zeros((0,), dtype=int)
        self.external_inputs = {}
        self.possible_input_params = N.empty((0,), dtype=object)
        
    def _resize(self):
        self.possible_Js.resize((self.G, self.G))
        self.possible_thetas.resize((self.G,))
        self.initial_states.resize((self.G,))
        self.possible_input_params.resize((self.G,))
    
    G = property(lambda self: len(self.genes), "The number of genes in the network.")
    
    def add_external_input(self, name, color, position, input_function, input_params=(None,)):
        """Add an external input gene to the network.
        """
        g = self.G
        gene = Gene(name=name, color=M.colors.colorConverter.to_rgb(color), position=position)
        self.genes.append(gene)
        self._resize()
        self.initial_states[g] = 0
        self.possible_thetas[g] = (0,)
        self.external_inputs[g] = input_function
        self.possible_input_params[g] = input_params
        return g
        
    def add_gene(self, name, color, position, initial_state, constitutive):
        """Add a non-external input gene to the network.
        """
        g = self.G
        gene = Gene(name=name, color=M.colors.colorConverter.to_rgb(color), position=position)
        self.genes.append(gene)
        self._resize()
        self.initial_states[g] = initial_state
        self.possible_thetas[g] = constitutive
        self.possible_input_params[g] = (None,)
        return g
        
    def add_condition(self, name, inputs, constraints):
        """Add a condition to the network.
        """
        c = len(self.conditions)
        # merge the condition's inputs with any external inputs we have
        merged_inputs = [
            inputs.get(g, self.external_inputs.get(g)) 
            for g in xrange(self.G)
        ]
        condition = Condition(name=name, inputs=inputs, constraints=constraints, merged_inputs=merged_inputs)
        self.conditions.append(condition)
        return c
    
    def _tighten_constraints_on_inputs(self):
        "Check constraints on input genes."
        for g, gene in enumerate(self.genes):
            if g in self.external_inputs:
                if len(self.possible_thetas[g]) > 1:
                    logging.warning("Had more than one possible theta for external input gene, %7s, resetting.", gene)
                    self.possible_thetas[g] = (0,)
                for g2 in xrange(self.G):
                    if len(self.possible_Js[g2, g]) > 1:
                        logging.warning(
                            "Had more than one possible regulatory relationship from %7s to external input gene, %7s, resetting.",
                            self.genes[g2], gene
                        )
                        self.possible_Js[g2, g] = (0,)
            else:
                if len(self.possible_input_params[g]) > 1:
                    logging.warning("Had more than one possible input parameter for non-external input gene, %7s, resetting.", gene)
                    self.possible_input_params[g] = (None,)
                
    


ON, OFF = 0, 1

CHECK_DIFFERENT = 0
"Check that two genes do not have the same on/off pattern."

CHECK_CONSECUTIVE_DIFFERENT = 1
"Check that no consecutive genes in the list have the same on/off pattern."

CHECK_ALL_DIFFERENT = 2
"Check each pair on genes in the list have different on/off patterns."

CHECK_EXPRESSES_BEFORE = 3
"Check the first gene expresses before the second."

CHECK_ORDER_OF_EXPRESSION = 4
"Check the genes in the list are expressed in the same order they are given."

CHECK_NULL_EXPRESSION = 5
"Check there is no expression for the given gene."

CHECK_ON_TO_OFF_SWITCH = 6
"Check that the gene starts on, switches off and remains off."

CHECK_OFF_TO_ON_SWITCH = 7
"Check that the gene starts off, switches on and remains on."



class ChangePointChecker(object):
    """
    Given a set of change points, offers various functions to check 
    that genes are going on and off in the correct temporal order.
    """
    
    def __init__(self, R):
        "Construct with the given realisation."
        self.R = R
        "Realisation."
        
        self.cp = network.calculate_change_points(R)
        "Change points for a realisation."
    
    def __call__(self, constraint, genes):
        if constraint == CHECK_DIFFERENT:
            return self.check_different(*genes)
        
        elif constraint == CHECK_CONSECUTIVE_DIFFERENT:
            return self.check_consecutive_different(genes)
        
        elif constraint == CHECK_ALL_DIFFERENT:
            return self.check_all_different(genes)
        
        elif constraint == CHECK_EXPRESSES_BEFORE:
            return self.check_expresses_before(*genes)
        
        elif constraint == CHECK_ORDER_OF_EXPRESSION:
            return self.check_order_of_expression(genes)
        
        elif constraint == CHECK_NULL_EXPRESSION:
            return self.check_null_expression(genes)
        
        elif constraint == CHECK_ON_TO_OFF_SWITCH:
            return self.check_on_to_off_switch(genes)
        
        elif constraint == CHECK_OFF_TO_ON_SWITCH:
            return self.check_off_to_on_switch(genes)
        
        else:
            raise RuntimeError('Unknown constraint')
        
    def check_different(self, g1, g2):
        "Check that gene1 and gene2 do not have the same on/off pattern."
        return len(self.cp[g1]) != 2 or len(self.cp[g2]) != 2 or (self.cp[g1][ON] == self.cp[g2][ON] and self.cp[g1][OFF] == self.cp[g2][OFF])

    def check_consecutive_different(self, genes):
        "Check that no consecutive genes in the list have the same on/off pattern."
        return sum(self.check_different(g1, g2) for g1, g2 in consecutive(genes))

    def check_all_different(self, genes):
        "Check that no genes in the list have the same on/off pattern."
        return sum(self.check_different(g1, g2) for g1, g2 in combinations(genes, 2))

    def check_expresses_before(self, g1, g2):
        "Check gene1 is expressed before gene2."
        if len(self.cp[g1]) != 2: return 1
        if len(self.cp[g2]) != 2: return 1
        return (
            self.cp[g1][ON] > self.cp[g2][ON]   # g1 on before g2 on
            or
            self.cp[g1][OFF] > self.cp[g2][OFF]  # g1 off before g2 off
        )

    def check_order_of_expression(self, genes):
        "Check the genes in the list are expressed in order."
        return sum(self.check_expresses_before(g1, g2) for g1, g2 in consecutive(genes))

    def check_null_expression(self, g):
        "Check there is no expression for the given gene."
        return (self.R[:,g] != 0).any()
    
    def check_on_to_off_switch(self, g):
        "Check that the gene starts on, switches off and remains off."
        return not self.R[0,g] or self.R[-1,g] or len(self.cp[g]) != 2
    
    def check_off_to_on_switch(self, g):
        "Check that the gene starts off, switches on and remains on."
        return self.R[0,g] or not self.R[-1,g] or len(self.cp[g]) != 2
    


