#
# Copyright John Reid 2013
#


"""
Example to illustrate application of pybool. Based on regulatory network in paper
on robustness under functional constraint by Nakajima et al.
"""

import numpy as N, logging
from pybool.constraints import gene_off, gene_on
from pybool import network, constraints



class MetaData(constraints.MetaData):
    """
    Meta-data for drosophila neurogenesis regulatory 
    networks in Nakajima paper.
    
    All pybool configuration is done through attributes of this class.
    """

    def __init__(self):
        "Construct."
        
        #
        # Initialise the base class
        #
        super(MetaData, self).__init__()

        #
        # The number of time steps to realise.
        #
        self.T = 12
 
        #
        # Add the genes
        #
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
            color='black',
            position=( 1,  .5),
            input_function=X_external_input,
            input_params=N.arange(1, self.T),
        )
        
        #
        # Add each condition as a dict mapping genes to states and a constraints function
        #
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

        #
        # The condition to use if none specified.
        #
        self.default_condition = wt

        #
        # set up the possible regulatory connections
        #
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
        

def svp_external_input(t, p):
    "External input function for svp. svp is on when t = 1."
    return 1 == t and 1 or 0


