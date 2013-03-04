..
.. Copyright John Reid 2012
..
.. This is a reStructuredText document. If you are reading this in text format, it can be 
.. converted into a more readable format by using Docutils_ tools such as rst2html.
..

.. _Docutils: http://docutils.sourceforge.net/docs/user/tools.html




========
 pybool
========
-------------------------------------------------------
 A Python package to infer Boolean regulatory networks
-------------------------------------------------------



Introduction
============

pybool is a Python package that infers Boolean regulatory networks from temporal expression
constraints.

Consider the common scenario of a biologist who is studying a particular regulatory network. 
They study a set of genes that are known to play a role in the network. They have some 
background knowledge of particular regulatory connections from his own studies or from the 
literature. In addition to this they have perturbation data that reveals the temporal order 
of expression of the genes under some conditions. For example, these could be derived from 
loss-of-function or over-expression experiments. The biologist would like to elucidate the 
entire network and to this end can perform various experiments to test particular regulatory 
connections. These experiments are costly and time-consuming. Which connections should they 
focus on? This is where the pybool package can help. By modelling candidate regulatory 
networks using Boolean logic, pybool evaluates which networks are consistent with the 
perturbation data and the known regulatory connections.

To install pybool please read the :doc:`installation instructions<installation>`.
 
To learn more about how to use pybool please read the section on :doc:`running pybool<run>`
and the section on :doc:`configuring pybool<configure>`.



License
=======

pybool is free for academic use. For commercial licenses please contact the author::

  John Reid,
  MRC Biostatistics Unit,
  Institute of Public Health,
  University Forvie Site,
  Robinson Way,
  Cambridge.
  CB2 0SR.



Contents
========

.. toctree::
   :maxdepth: 2

   installation
   run
   configure
   pybool modules<apidoc/modules>




Indices and tables
==================
 
* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`

