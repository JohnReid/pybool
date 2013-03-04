..
.. Copyright John Reid 2013
..
.. This is a reStructuredText document. If you are reading this in text format, it can be 
.. converted into a more readable format by using Docutils_ tools such as rst2html.
..

.. _Docutils: http://docutils.sourceforge.net/docs/user/tools.html




Configuring pybool
==================

.. _configuring:



In the running pybool section we saw how to run a pybool example that had already been configured. Here
we will look at how to configure our own set up. Let's imagine that we want to configure the example
already given but from scratch.

First we need to create a python module to hold our configuration. This can be in the current directory,
let's call the file ``nakajima.py``. We will fill in the details of the configuration step-by-step. We
import some functionality we will use later::

    import numpy as N, logging
    from pybool.constraints import gene_off, gene_on
    from pybool import network, constraints

The configuration module must define a class called `MetaData` that stores the configuration. This should inherit
from `pybool.constraints.MetaData`::

    class MetaData(constraints.MetaData):
        """
        Meta-data for drosophila neurogenesis regulatory 
        networks in Nakajima paper.
        
        All pybool configuration is done through attributes of this class.
        """

Most of the setup will be done when the `MetaData` class is initialised::

        def __init__(self):
            "Construct."
            
            #
            # Initialise the base class
            #
            super(MetaData, self).__init__()


Define the number of time steps to realise::

            self.T = 12
 
Add the genes to the configuration. Each gene can be just a normal gene that is part of the
network but some will be external inputs. `pybool.constraints.MetaData` defines methods to 
handle each of these two cases. In either case each gene should have a color associated with
it for graphical output and a position so that when a network is plotted the genes always
appear in the same place. The external inputs take a function (defined later) for the input
and a sequence of possible parameters to that function. The non-external inputs take an
initial state and a sequence of possible constitutive expression levels::

            SVP = self.add_external_input(
                'svp',
                color='purple',
                position=( 1, 1.5),
                input_function=svp_external_input,
                input_params=(None,),
            )
            HB = self.add_gene(
                'hb',
                initial_state=1,
                constitutive=(0, 1),
                color='green',
                position=( 0, 1.5),
            )
            KR = self.add_gene(
                'Kr',
                initial_state=0,
                constitutive=(0, 1),
                color='darkblue',
                position=(-1, 1  ),
            )
            PDM = self.add_gene(
                'pdm',
                initial_state=0,
                constitutive=(0, 1),
                color='deepskyblue',
                position=( 0,  .5),
            )
            CAS = self.add_gene(
                'cas',
                initial_state=0,
                constitutive=(0, 1),
                color='#DD0000',
                position=(-1, 0  ),
            )
            X = self.add_external_input(
                'X',
                color='#DD0000',
                position=( 1,  .5),
                input_function=X_external_input,
                input_params=N.arange(1, self.T),
            )
            

We need to add the conditions for which we have expression constraints. ``wt``
is the wild-type condition, ``hb-`` is the hb knock-out condition and ``Kr++`` the
Kr over-expression condition. For each condition we define which genes are
knocked-out (`gene_off`) or over-expressed (`gene_on`) as a dictionary mapping
genes to states. Each condition can have multiple knock-outs and/or over-expressed genes.
The constraints are defined as sequences of conditions to check. Which constraints
are available and what they do is documented as part of the
`pybool.constraints.ChangePointChecker` class::

            wt = self.add_condition(
                'wt',
                { },
                constraints=(
                    (constraints.CHECK_ORDER_OF_EXPRESSION, (HB, KR, PDM, CAS)),
                    (constraints.CHECK_CONSECUTIVE_DIFFERENT, (HB, KR, PDM, CAS)),
                    (constraints.CHECK_ON_TO_OFF_SWITCH, X),
                )
            )
            hb_ko = self.add_condition(
                'hb-',
                inputs={  HB : gene_off },
                constraints=(
                    (constraints.CHECK_ORDER_OF_EXPRESSION, (KR, PDM, CAS)),
                    (constraints.CHECK_CONSECUTIVE_DIFFERENT, (KR, PDM, CAS)),
                    (constraints.CHECK_ON_TO_OFF_SWITCH, X),
                )
            )
            kr_ko = self.add_condition(
                'Kr-',
                inputs={  KR : gene_off },
                constraints=(
                    (constraints.CHECK_ORDER_OF_EXPRESSION, (HB, PDM, CAS)),
                    (constraints.CHECK_CONSECUTIVE_DIFFERENT, (HB, PDM, CAS)),
                    (constraints.CHECK_ON_TO_OFF_SWITCH, X),
                )
            )
            pdm_ko = self.add_condition(
                'pdm-',
                inputs={ PDM : gene_off },
                constraints=(
                    (constraints.CHECK_ORDER_OF_EXPRESSION, (HB, KR, CAS)),
                    (constraints.CHECK_CONSECUTIVE_DIFFERENT, (HB, KR, CAS)),
                    (constraints.CHECK_ON_TO_OFF_SWITCH, X),
                )
            )
            cas_ko = self.add_condition(
                'cas-',
                inputs={ CAS : gene_off },
                constraints=(
                    (constraints.CHECK_ORDER_OF_EXPRESSION, (HB, KR, PDM)),
                    (constraints.CHECK_CONSECUTIVE_DIFFERENT, (HB, KR, PDM)),
                    (constraints.CHECK_ON_TO_OFF_SWITCH, X),
                )
            )
            hb_oe = self.add_condition(
                'hb++',
                inputs={  HB : gene_on },
                constraints=(
                    (constraints.CHECK_ORDER_OF_EXPRESSION, (HB, KR)),
                    (constraints.CHECK_NULL_EXPRESSION, PDM),
                    (constraints.CHECK_NULL_EXPRESSION, CAS),
                    (constraints.CHECK_CONSECUTIVE_DIFFERENT, (HB, KR)),
                    (constraints.CHECK_ON_TO_OFF_SWITCH, X),
                )
            )
            kr_oe = self.add_condition(
                'Kr++' ,
                inputs={  KR : gene_on },
                constraints=(
                    (constraints.CHECK_ORDER_OF_EXPRESSION, (HB, KR, PDM)),
                    (constraints.CHECK_NULL_EXPRESSION, CAS),
                    (constraints.CHECK_CONSECUTIVE_DIFFERENT, (HB, KR, PDM)),
                    (constraints.CHECK_ON_TO_OFF_SWITCH, X),
                )
            )
            pdm_oe = self.add_condition(
                'pdm++',
                inputs={ PDM : gene_on },
                constraints=(
                    (constraints.CHECK_ORDER_OF_EXPRESSION, (HB, PDM, CAS)),
                    (constraints.CHECK_NULL_EXPRESSION, KR),
                    (constraints.CHECK_CONSECUTIVE_DIFFERENT, (HB, PDM, CAS)),
                    (constraints.CHECK_ON_TO_OFF_SWITCH, X),
                )
            )
            cas_oe = self.add_condition(
                'cas++',
                inputs={ CAS : gene_on },
                constraints=(
                    (constraints.CHECK_ORDER_OF_EXPRESSION, (HB, KR)),
                    (constraints.CHECK_NULL_EXPRESSION, PDM),
                    (constraints.CHECK_CONSECUTIVE_DIFFERENT, (HB, KR, CAS)),
                    (constraints.CHECK_ON_TO_OFF_SWITCH, X),
                )
            )
    

We define a default condition. This is usually the wild-type::

            self.default_condition = wt


Now we define the possible regulatory connections (the matrix J in our
recurrence relation). These are the
restrictions based on expert knowledge::

            unconstrained = (-5, 0, 1)
            represses_or_none = (-5, 0)
            activates = (1,)
            represses = (-5,)
            no_regulation = (0,)
            
            # initialise all connections to no_regulation
            for g1 in xrange(self.G):
                for g2 in xrange(self.G):
                    self.possible_Js[g1, g2] = no_regulation
            
            # X can regulate any of HB, KR, PDM and CAS
            self.possible_Js[  X, HB] = unconstrained
            self.possible_Js[  X, KR] = unconstrained
            self.possible_Js[  X,PDM] = unconstrained
            self.possible_Js[  X,CAS] = unconstrained
            
            # from Figure 1 in Nakajima paper
            self.possible_Js[SVP, HB] = represses
            self.possible_Js[ HB, KR] = activates
            self.possible_Js[ HB,PDM] = represses
            self.possible_Js[ HB,CAS] = represses_or_none
            self.possible_Js[ KR,PDM] = activates
            self.possible_Js[ KR,CAS] = represses
            self.possible_Js[PDM, KR] = represses
            self.possible_Js[PDM,CAS] = activates
            self.possible_Js[CAS,PDM] = represses


We define the functions for the external input genes::        

    def svp_external_input(t, p):
        "External input function for svp. svp is on when t = 1."
        return 1 == t and 1 or 0
    
    
    def X_external_input(t, p):
        "External input function for X. X is on when t < p."
        return int(t < p)
