#
# Copyright John Reid 2010, 2011, 2012, 2013
#


"""
I/O for boolean regulatory networks.
"""


import numpy as N, matplotlib as M, pylab as P, logging, subprocess, os
from cookbook.pylab_utils import layout_sub_plot, pylab_ioff
from . import network, chow_liu_trees
from .analysis import aggregate_possible_regulations, aggregate_possible_thetas, aggregate_possible_inputs
from .analysis import count_possible_regulations, count_possible_thetas, count_possible_inputs
from .analysis import analyse_dependencies
import networkx as nx


def output_file(options, filename):
    "Return a path to the named output file."
    return os.path.join(options.output_dir, filename)



def ensure_dir_exists(dir):
    "Make sure a directory exists, making it and its parents if necessary."
    if not os.path.exists(dir):
        os.makedirs(dir)



def configure_matplotlib_for_tex():
    "Set up matplotlib parameters for plotting using TeX."
    raise ValueError('')
    fig_width_pt = 345.0  # Get this from LaTeX using \showthe\columnwidth
    inches_per_pt = 1.0/72.27               # Convert pt to inch
    golden_mean = (N.sqrt(5)-1.0)/2.0       # Aesthetic ratio
    fig_width = fig_width_pt*inches_per_pt  # width in inches
    fig_height = fig_width*golden_mean      # height in inches
    fig_size =  (fig_width,fig_height)
    params = {
        'backend'           : 'ps',
        'axes.labelsize'    : 6,
        'axes.titlesize'    : 6,
        'text.fontsize'     : 6,
        'legend.fontsize'   : 6,
        'xtick.labelsize'   : 4,
        'ytick.labelsize'   : 4,
        'xtick.direction'   : 'out',
        'ytick.direction'   : 'out',
        'xtick.major.size'  : 0,
        'xtick.minor.size'  : 0,
        'ytick.major.size'  : 0,
        'ytick.minor.size'  : 0,
        'text.usetex'       : True,
        'figure.figsize'    : fig_size
    }
    P.rcParams.update(params)


def rgb_as_string(rgb):
    "@return: A string representing the rgb colour."
    return '#%02X%02X%02X' % tuple((rgb * 255).astype(int))



class GraphBuilder(object):
    """
    Builds graphs representing the information in the meta data.
    """
    
    def __init__(self, meta_data, options):
        self.options = options
        
        try:
            import boost.graph as bgl
        except ImportError:
            logging.warning('Cannot import boost.graph python bindings. Will not create network of graph.')
            self.graph = None
            return
    
        self.graph = bgl.Digraph()
    
        # add vertices
        self.name_map = self.graph.add_vertex_property('node_id', 'string')
        self.position_map = self.graph.add_vertex_property('pos', 'string')
        if not options.black_and_white:
            self.color_map = self.graph.add_vertex_property('color', 'string')
        #self.fillcolor_map = self.graph.add_vertex_property('fillcolor', 'string')
        self.shape_map = self.graph.add_vertex_property('shape', 'string')
        self.style_map = self.graph.add_vertex_property('style', 'string')
        self.vertices = [self.graph.add_vertex() for g in xrange(meta_data.G)]
        for g, (v, gene) in enumerate(zip(self.vertices, meta_data.genes)):
            self.name_map[v] = gene
            if g in meta_data.graph_positions: # fix position of node if required
                self.position_map[v] = '%s,%s!' % meta_data.graph_positions[g]
            if not options.black_and_white:
                #self.fillcolor_map[v] = rgb_as_string(meta_data.colours[g])
                self.color_map[v] = rgb_as_string(meta_data.colours[g])
            if g in meta_data.external_inputs:
                self.shape_map[v] = "box" # external inputs are different shape
            else:
                self.shape_map[v] = "circle"
            #self.style_map[v] = "filled"
        
        self.arrowhead_map = self.graph.add_edge_property('arrowhead', 'string')
        self.arrowsize_map = self.graph.add_edge_property('arrowsize', 'float')
        self.edgestyle_map = self.graph.add_edge_property('style', 'string')

    
    def add_edge(self, src, dst, activatory):
        e = self.graph.add_edge(self.vertices[src], self.vertices[dst])
        if activatory:
            self.arrowhead_map[e] = 'normal'
            self.arrowsize_map[e] = 1.5
            self.edgestyle_map[e] = '-open triangle 90'
        else:
            self.arrowhead_map[e] = 'tee'
            self.arrowsize_map[e] = 1.5
            self.edgestyle_map[e] = '-triangle 90 reversed'
        return e





class NetworkXGraphBuilder(object):
    """
    Builds graphs representing the information in the meta data using Python package networkx.
    """
    
    def __init__(self, meta_data, options):
        self.options = options
        
        self.graph = nx.MultiDiGraph()
    
        # add vertices
        for g, gene in enumerate(meta_data.genes):
            attributes = {
                'label' : gene.name,
                # 'style' : 'filled',
            }
            if gene.position: # fix position of node if required
                attributes['pos'] = '%s,%s!' % gene.position
            if not options.black_and_white:
                attributes['color'] = rgb_as_string(N.asarray(gene.color))
            if g in meta_data.external_inputs:
                attributes['shape'] = "box" # external inputs are different shape
            else:
                attributes['shape'] = "circle"
            self.graph.add_node(g, **attributes)

    
    def add_edge(self, src, dst, activatory, dashed):
        if activatory:
            attributes = NetworkXGraphBuilder._activatory_attributes.copy()
        else:
            attributes = NetworkXGraphBuilder._repressive_attributes.copy()
        if not self.options.use_latex:
            attributes['style'] = '' # only use style for latex output
        if dashed:
            attributes['style'] += ',dashed'
        self.graph.add_edge(src, dst, **attributes)

    _activatory_attributes = {
        'arrowhead' : 'normal',
        'arrowsize' : 1.5,
        'style'     : '-open triangle 90'
    }

    _repressive_attributes = {
        'arrowhead' : 'tee',
        'arrowsize' : 1.5,
        'style'     : '-triangle 90 reversed'
    }



def graph_network(net, options):
    "Create a BGL graph of the network."
    
    builder = NetworkXGraphBuilder(net.meta_data, options)

    if builder.graph:
        # add edge
        for src, dst in zip(*N.asarray(net.J).nonzero()):
            builder.add_edge(src, dst, net.J[src,dst] > 0, False)

    return builder.graph




def graph_restrictions(meta_data, options, possible_Js=None):
    "Create a BGL graph of the possible networks."
    
    if None == possible_Js:
        possible_Js = meta_data.possible_Js
    
    builder = NetworkXGraphBuilder(meta_data, options)

    # add edges
    if builder.graph:
        for src in xrange(meta_data.G):
            for dst in xrange(meta_data.G):
                Js = possible_Js[src,dst]
                dashed = 0 in Js
                
                # add edges
                for J in Js:
                    if 0 != J:
                        activatory = J > 0
                        builder.add_edge(src, dst, activatory, dashed)
                
    return builder.graph


_dot2tex_cmd = 'dot2tex --autosize --crop --prog=neato -ftikz --nodeoptions "ultra thick,minimum size=1cm" --figonly --tikzedgelabels'
_neato_cmd = 'neato -Nfontsize=16 -Nwidth=1 -Nheight=1 -Nfixedsize=true -Npenwidth=3 -s.4'

def write_graph(graph, name, options):
    "Write the graph as a DOT file and a SVG file."
    import networkx as nx
    dot_file = '%s.dot' % name
    nx.write_dot(graph, dot_file)
    if options.use_latex:
        logging.info('Plotting figures using dot2tex and LaTeX tikz package.')
        subprocess.check_call('%s %s > %s.tex' % (_dot2tex_cmd, dot_file, name), shell=True)
    for format in options.formats:
        subprocess.check_call('%s -T%s %s > %s.%s' % (_neato_cmd, format, dot_file, name, format), shell=True)


def plot_network_realisation(net, X, xlabel=False, ylabel=False):
    "Plot the realisation of the network."
    colours = N.ones((X.shape[0], net.meta_data.G, 3))
    for g, (x_col, gene) in enumerate(zip(X.T, net.meta_data.genes)):
        for t, x in enumerate(x_col):
            if x:
                colours[t,g] = gene.color
    P.imshow(colours, interpolation='nearest')
#    linewidth = .2
#    for t in xrange(X.shape[0]+1):
#        P.axhline(y=t-.5, xmin=0, xmax=X.shape[1], color='white', lw=linewidth)
#    for g in xrange(X.shape[1]+1):
#        P.axvline(x=g-.5, ymin=0, ymax=X.shape[0], color='white', lw=linewidth)
    P.xlim((-.5, X.shape[1]-.5))
    P.ylim((X.shape[0]-.5, -.5))
    axes = P.gca()
    if xlabel:
        P.xticks(range(net.meta_data.G), [gene.name for gene in net.meta_data.genes])
        P.setp(P.gca().get_xticklabels(), rotation=45, horizontalalignment='right', fontsize=7)
        for line in axes.get_xticklines():
            line.set_visible(False)
    else:
        P.xticks([], [])
    if ylabel:
        P.ylabel('time', fontsize=8)
        P.setp(P.gca().get_yticklabels(), fontsize=8)
        for line in axes.get_yticklines():
            line.set_visible(False)
    else:
        P.yticks([], [])


def regulation_as_str(r):
    "Convert a regulatory value to a string."
    if -5 == r:
        return '-'
    elif 0 == r:
        return '0'
    elif 1 == r:
        return '+'
    else:
        raise ValueError('Unknown regulation value.')


def centre_string(s, width):
    "Pad and centre a string to the given width."
    length = len(s)
    pre = (width - length) / 2
    post = width - pre - length
    return '%s%s%s' % (' ' * pre, s, ' ' * post)


_matrix_entry_width = 6


def regulatory_matrix_as_string(possible_regulations):
    str_possible_regulations = N.empty(possible_regulations.shape, dtype=object)
    for g1 in xrange(possible_regulations.shape[0]):
        for g2 in xrange(possible_regulations.shape[1]):
            pr = list(possible_regulations[g1,g2])
            pr.sort()
            str_possible_regulations[g1,g2] = centre_string(
                '%s' % '/'.join(map(regulation_as_str, pr)),
                _matrix_entry_width
            )
    return str_possible_regulations


def add_gene_headers_to_matrix(genes, matrix, width=5):
    "Takes a matrix and creates a new one with headers for rows and columns."
    result = N.empty((matrix.shape[0]+1, matrix.shape[1]+1), dtype=object)
    result[1:,1:] = matrix
    genes = [centre_string(g.name, _matrix_entry_width) for g in genes]
    result[0,0] = ' ' * _matrix_entry_width
    result[0,1:] = genes
    result[1:,0] = genes
    return result


def summarise_meta_data(meta_data):
    "Log some information about the network constraint meta data."
    logging.info('Have %d genes called: %s', meta_data.G, ','.join(gene.name for gene in meta_data.genes))
    logging.info(
        'The possible regulatory relationships are:\n%s', 
        str(add_gene_headers_to_matrix(meta_data.genes, regulatory_matrix_as_string(meta_data.possible_Js.T)))
    )
    for g, (gene, thetas, initial_state) in enumerate(zip(meta_data.genes, meta_data.possible_thetas, meta_data.initial_states)):
        if g not in meta_data.external_inputs:
            logging.info(
                'Gene %7s : initial state %s : constitutive expression %7s',
                gene.name, initial_state, thetas
            )
    for g, gene in enumerate(meta_data.genes):
        if g in meta_data.external_inputs:
            logging.info(
                '%7s is an external input with possible input parameters: %s', 
                gene.name, ','.join(map(str, meta_data.possible_input_params[g]))
            )
    logging.info('The conditions to test are: %s', ', '.join(c.name for c in meta_data.conditions))




def summarise_possible_networks(meta_data, networks):
    "Log some information about the possible networks."
    
    #
    # regulations
    #
    possible_regulations = aggregate_possible_regulations(meta_data, networks)
    str_possible_regulations = regulatory_matrix_as_string(possible_regulations)
    logging.info(
        'Consistent regulatory relationships in the networks are:\n%s',
        str(add_gene_headers_to_matrix(meta_data.genes, str_possible_regulations.T))
    )
    
    regulation_counts = count_possible_regulations(meta_data, networks)
    for regulatee, regulatee_counts in zip(meta_data.genes, regulation_counts.T):
        for regulator, counts in zip(meta_data.genes, regulatee_counts):
            if len(counts) > 1:
                logging.info(
                    'How often %7s regulates %7s     %s',
                    regulator.name,
                    regulatee.name,
                    '     '.join('%s : %2d%%' % (regulation_as_str(r), 100*c/len(networks)) for r, c in counts.iteritems())
                )
    
    #
    # thetas
    #
    possible_thetas = aggregate_possible_thetas(meta_data, networks)
    theta_counts = count_possible_thetas(meta_data, networks)
    for gene, thetas in zip(meta_data.genes, possible_thetas):
        logging.info('Consistent constitutive expression levels for %7s are %s', gene.name, ','.join(map(str, thetas)))
    for gene, counts in zip(meta_data.genes, theta_counts):
        if len(counts) > 1:
            logging.info(
                'Counts for constitutive expression levels for %7s are    %s',
                gene.name,
                '     '.join('%s : %2d%%' % (t, 100*c/len(networks)) for t, c in counts.iteritems())
            )

    #
    # input parameters
    #
    possible_inputs = aggregate_possible_inputs(meta_data, networks)
    input_counts = count_possible_inputs(meta_data, networks)
    for g, input_fn in meta_data.external_inputs.iteritems():
        gene = meta_data.genes[g]
        logging.info(
            'Consistent input parameters for the external input of %7s are: %s', 
            gene.name, ','.join(map(str, possible_inputs[g]))
        )
    for g, input_fn in meta_data.external_inputs.iteritems():
        if len(input_counts[g]) > 1:
            gene = meta_data.genes[g]
            logging.info(
                'Counts for input parameters for the external input of %7s are    %s', 
                gene.name, 
                '     '.join('%s : %2d%%' % (i, 100*c/len(networks)) for i, c in input_counts[g].iteritems())
            )
    
    #
    # Dependencies where mutual information is positive
    #
    networks_as_features, T, edges = analyse_dependencies(networks)
    for u, v, data in edges:
        I = -data['weight']
        if I > 0.:
            feature1, x1 = network.which_network_feature(meta_data, u)
            feature2, x2 = network.which_network_feature(meta_data, v)
            logging.info(
                'Mutual information between %s:%s and %s:%s is %.2f',
                network.feature_string(feature1), x1, network.feature_string(feature2), x2, I
            )
            logging.debug(chow_liu_trees.marginal_pair_distribution(networks_as_features, u, v))
        else:
            break
        
    return possible_regulations




@pylab_ioff
def plot_network_realisations(net):
    "Plot the network realisations over all the conditions."
    num_cols = len(net.meta_data.conditions)
    fig = P.figure(figsize=(num_cols + 1, 3))
    for i, condition in enumerate(net.meta_data.conditions):
        P.subplot(1, num_cols, i+1)
        P.title(condition.name, fontsize=10)
        X, mismatches = network.evaluate_condition(net, condition)
        logging.debug('Condition: %6s; mismatches=%2d', condition, mismatches)
        plot_network_realisation(net, X, xlabel=True, ylabel=0==i)
    #P.subplots_adjust(left=0, bottom=0, right=1, top=1, wspace=None, hspace=None)
    P.tight_layout()
    return fig
