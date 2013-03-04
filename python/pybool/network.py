#
# Copyright John Reid 2010, 2011, 2012, 2013
#


"""
Code to describe and generate boolean networks.
"""

import numpy as N, logging, copy, cNetwork



class BooleanNetwork(object):
    """
    A boolean regulatory network.
    """

    def __init__(self, meta_data):
        "Construct."

        self.meta_data = meta_data
        "Meta-data for this type of network."

        self.theta = N.zeros((self.meta_data.G,), dtype=int)
        "The constitutive (default) gene states."

        self.J = N.zeros((self.meta_data.G, self.meta_data.G), dtype=int)
        "The regulatory connections."

        self.input_parameters = [None] * self.meta_data.G
        "Parameters for external inputs."


    def __deepcopy__(self, memo):
        """
        Make a deep copy of the network but do not copy meta data.
        """
        result = BooleanNetwork(self.meta_data)
        result.theta = copy.deepcopy(self.theta, memo)
        result.J = copy.deepcopy(self.J, memo)
        result.input_parameters = copy.deepcopy(self.input_parameters, memo)
        return result


    def execute_time_step(self, X, t, external_inputs, output):
        "Use the current values of gene expression, X, to calculate the values at the next time point."
        # default activation is a result of transition matrix, J, with last X
        activation = N.dot(X, self.J).reshape((self.meta_data.G,))

        #logging.debug('Activation: %s', str(activation))
        for g, a in enumerate(activation):
            if 0 < a:
                output[g] = 1
            elif 0 == a:
                output[g] = self.theta[g]

        # check for external inputs. Use meta data defaults if none passed as arguments
        self._apply_external_inputs(output, t+1, external_inputs)


    def generate_realisation(self, condition=None, X=None):
        "Generate an entire realisation of the network."

        # Use default condition if none specified.
        if None == condition:
            condition = self.meta_data.default_condition

        # The condition's inputs merged with any external inputs
        merged_inputs = condition.merged_inputs

        # generate each time step of the realisation
        if None == X:
            X = N.empty((self.meta_data.T, self.meta_data.G), dtype=int)
        X[0] = self.meta_data.initial_states
        self._apply_external_inputs(X[0], 0, merged_inputs)
        for t in xrange(self.meta_data.T-1):
            #logging.debug('X[%2d]      : %s', t, str(X[t]))
            self.execute_time_step(X[t], t, merged_inputs, X[t+1])
        #logging.debug('X[%2d]      : %s', t+1, str(X[t+1]))

        return X


    def _apply_external_inputs(self, x, t, external_inputs):
        "Apply the external inputs to x."
        for g, input_fn in enumerate(external_inputs):
            if input_fn:
                x[g] = input_fn(t, self.input_parameters[g])




    
def network_to_features(network):
    """
    Maps the features of a network (theta, J, input parameters) into a sequence.
    """
    result = list(network.theta)
    result.extend(network.input_parameters)
    result.extend(network.J.flatten())
    return result

THETA=0
J=1
INPUT_PARAMETERS=2

def feature_string(feature):
    if THETA == feature:
        return 'theta'
    elif J == feature:
        return 'J'
    elif INPUT_PARAMETERS == feature:
        return 'input parameters'
    else:
        raise ValueError('Unknown feature.')
    
def which_network_feature(meta_data, u):
    """
    Identifies which feature in the network u refers to.
    """
    G = meta_data.G
    if u < G:
        return THETA, u
    u -= G
    if u < G:
        return INPUT_PARAMETERS, u
    u -= G
    return J, (u / G, u % G)

    

def py_execute_time_step(self, X, t, external_inputs, result):
    "Use the current values of gene expression, X, to calculate the values at the next time point."
    # default activation is a result of transition matrix, J, with last X
    activation = N.dot(X, self.J).reshape((self.meta_data.G,))

    #logging.debug('Activation: %s', str(activation))
    for g, a in enumerate(activation):
        if 0 < a:
            result[g] = 1
        elif 0 == a:
            result[g] = self.theta[g]

    # check for external inputs. Use meta data defaults if none passed as arguments
    if None == external_inputs:
        external_inputs = self.meta_data.external_inputs
    self._apply_external_inputs(result, t+1, external_inputs)


def c_execute_time_step(net, X, t, external_inputs, result):
    cNetwork.execute_time_step(
        X,
        net.J.T,
        net.theta,
        t,
        external_inputs,
        net.input_parameters,
        result
    )
    #py_result = py_execute_time_step(net, X, t, external_inputs)
    #assert (py_result == result).all()
    return result
BooleanNetwork.execute_time_step = c_execute_time_step



def evaluate_all_conditions(net):
    """
    Evaluate the network under all the conditions until one does not match.

    @return: The first condition that does not match (or None) and the number of mismatches as a tuple.
    """
    X = N.empty((net.meta_data.T, net.meta_data.G), dtype=int)
    assert net.meta_data.conditions # must have at least one
    for condition in net.meta_data.conditions:
        X, mismatches = evaluate_condition(net, condition, X)
        if mismatches > 0:
            break
    if not mismatches:
        return None, mismatches
    else:
        return condition, mismatches


def evaluate_condition(net, condition, X=None):
    """Evaluate the network under one condition.
    """
    from pybool import constraints
    X = net.generate_realisation(condition, X=X)
    checker = constraints.ChangePointChecker(X)
    mismatches = sum(checker(constraint, genes) for constraint, genes in condition.constraints)
    return X, mismatches


def calculate_gene_change_points(X):
    "Inspect the expression of a gene to find when it turns off and on."
    on = False
    for t, x in enumerate(X):
        if bool(x) != on:
            yield t
            on = bool(x)
    if on:
        yield t


def calculate_change_points(X):
    "Inspect a realisation of a network to calculate when genes are on and off."
    change_points = [list(calculate_gene_change_points(gene_expression)) for gene_expression in X.T]
    return change_points

def c_calculate_change_points(X):
    return cNetwork.calculate_change_points(X.T)
calculate_change_points = c_calculate_change_points


class BooleanNetworkGenerator(object):
    "Generates boolean networks with all possible parameters."

    def __init__(self, meta_data):
        "Construct."
        self.meta_data = meta_data
        self._check_consistency()
    
    
    def _check_consistency(self):
        "Check that the meta data is consistent."
        for g, gene in enumerate(self.meta_data.genes):
            if g in self.meta_data.external_inputs:
                if len(self.meta_data.possible_thetas[g]) > 1:
                    raise RuntimeError("""
Does not make sense to have more than one constitutive expression level for gene %s which is an external input.
                        """ % gene.name)
                for reg_g in xrange(self.meta_data.G):
                    if len(self.meta_data.possible_Js[reg_g, g]) > 1:
                        raise RuntimeError("""
Does not make sense to have more than one possible regulatory relationship for an external input. 
%s regulates %s in more than one way.
                        """ % (self.meta_data.genes[reg_g].name, gene.name))
            elif len(self.meta_data.possible_input_params[g]) > 1:
                raise RuntimeError("""
Does not make sense to have more than one input parameter for gene %s which is not an external input.
                    """ % gene.name)
            


    def __call__(self):
        "Yield one network after another."

        net = BooleanNetwork(self.meta_data)
        G = self.meta_data.G

        num_networks = (
            number_possible_combinations(net.meta_data.possible_thetas)
            *
            number_possible_combinations(net.meta_data.possible_input_params)
            *
            number_possible_combinations(net.meta_data.possible_Js.reshape(G**2))
        )
        logging.info('Will generate %d different networks.', num_networks)

        # possible thetas
        logging.debug('Generating possible combinations of thetas.')
        possible_thetas = [theta.copy() for theta in combinations(net.meta_data.possible_thetas, N.empty((G,), dtype=int))]

        # possible input parameters
        logging.debug('Generating possible combinations of input parameters.')
        possible_input_params = [copy.copy(ip) for ip in combinations(net.meta_data.possible_input_params, [None]*G)]

        # for each setting of J
        logging.debug('Generating possible combinations of J.')
        indexes = list(N.ndindex(G, G))
        for J in combinations(net.meta_data.possible_Js.reshape(G**2), net.J.reshape(G**2)):
            for input_params in possible_input_params:
                net.input_parameters[:] = input_params
                for theta in possible_thetas:
                    net.theta[:] = theta
                    yield net



def number_possible_combinations(possible_values):
    "@return: The number of possible combinations of the values."
    return reduce(int.__mul__, map(len, possible_values))



def combinations(possible_values, X=None):
    """
    Yield each possible combination of the values given.

    @arg possible_values: Should be a sequence of sequences of possible values.
    @arg X: Object to store combinations in. If not given, a numpy array is used.

    For example:

    >>> possible_values = (
    ...     (-5, 0, 1),
    ...     (0, 1),
    ... )
    >>> for X in combinations(possible_values):
    ...     print X
    [-5  0]
    [0 0]
    [1 0]
    [-5  1]
    [0 1]
    [1 1]
    """
    num_combs = number_possible_combinations(possible_values)
    logging.debug('There are %d possible combinations.', num_combs)
    n = len(possible_values)
    num_values = N.array(map(len, possible_values))
    idx = N.zeros(n, dtype=int)
    if None == X:
        X = N.empty((n,), dtype=int)
    for i, values in enumerate(possible_values):
        X[i] = values[0]
    while True:
        yield X
        for i, (num, values) in enumerate(zip(num_values, possible_values)):
            idx[i] += 1
            if num == idx[i]:
                idx[i] = 0
                X[i] = values[0]
            else:
                X[i] = values[idx[i]]
                break
        else:
            break



def filter_unique_Js(iterable):
    "Filter those networks from the iterable that have the same J."
    last = None
    for net in iterable:
        if None != last and (last.J != net.J).any():
            yield net
        last = net



if '__main__' == __name__:
    import doctest
    doctest.testmod()
