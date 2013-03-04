..
.. Copyright John Reid 2012
..
.. This is a reStructuredText document. If you are reading this in text format, it can be 
.. converted into a more readable format by using Docutils_ tools such as rst2html.
..

.. _Docutils: http://docutils.sourceforge.net/docs/user/tools.html




Installation
============

.. _installation instructions:

These installation instructions are for a general Linux system but should work with minor 
changes on other OSes (Windows, MacOS, etc..). They assume that most software will be
installed in ``$HOME/local``.

I find the easiest way to install Python packages is
into a virtual environment created by
virtualenv_. If you do not have administrator privileges or even if you do, then virtualenv_ is a good
way to manage what Python packages you install. These installation instructions are based 
on this method, although you could
easily adapt them to install straight into your machine's Python environment. If not using
virtualenv I recommend using pip_ or failing that easy_install_ or the standard Python
method of downloading a tarball, unpacking it and installing with::

    python setup.py install



Prerequisites
~~~~~~~~~~~~~

pybool relies on the following third-party softwares. Many of these can probably be installed using
your operating system's package manager which should be more straight-forward.

- Python_ 2.5 or newer. Development has been
  done on Python 2.6 but it should work on Python 2.5 and Python 2.7. It
  should not be difficult to get pybool working on Python 3 but it may not work
  out of the box. If installing on a Linux system with a package manager
  you will need the development package that contains the Python header files,
  for example the python-dev package on Ubuntu. 

- A tool to create virtual Python environments called virtualenv_. Please see
  the installation instructions on its website. You can install pybool without
  using virtualenv but you will need to adapt these instructions somewhat.
  
- A C++ compiler to compile the core C++ algorithm. All development has been
  done with GCC_ 4.4.3 and 4.6.3 but other versions and compilers should work.

- The `Boost C++ libraries`_: I have used version 1.45 although other versions should work.
  Following the commands_ given at the Boost website is straightforward, e.g.::
  
    ./bootstrap.sh --help
    ./bootstrap.sh --prefix=$HOME/local
    ./bjam --with-python --prefix=$HOME/local install release
  
  should install the boost Python library and the necessary headers. Once they are installed,
  you will need to update your ``LD_LIBRARY_PATH`` environment variable, e.g.::
  
    export LD_LIBRARY_PATH=$HOME/local/lib:$LD_LIBRARY_PATH
  
  so that the shared objects can be found at runtime.
        
- GraphViz_ tools: These are used to generate the network diagrams. In particular ``neato`` must
  be in your ``PATH``.


The following softwares are optional

- dot2tex_: This is used to create the publication quality network diagrams.

- LaTeX_: This is also used to generate the publication quality network diagrams. The preview style 
  and the tikz package should be installed.

- IPython_: This is used for the parallel implementation. It is not required
  if you do not wish to run pybool in parallel mode. You'll need to use IPython version
  0.11 or later.

.. _Python: http://www.python.org/
.. _numpy: http://numpy.scipy.org/
.. _matplotlib: http://matplotlib.sourceforge.net/
.. _networkx: http://networkx.lanl.gov/
.. _pygraphviz: http://networkx.lanl.gov/pygraphviz/
.. _virtualenv: http://www.virtualenv.org/
.. _dot2tex: http://www.fauskes.net/code/dot2tex/
.. _GCC: http://gcc.gnu.org/
.. _Boost C++ libraries: http://www.boost.org/
.. _commands: http://www.boost.org/doc/libs/1_45_0/more/getting_started/unix-variants.html#easy-build-and-install
.. _LaTeX: http://www.latex-project.org/
.. _GraphViz: http://www.graphviz.org/
.. _IPython: http://ipython.scipy.org/



Create the pybool environment
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Create a virtual environment to install pybool and its dependencies into::

    virtualenv --system-site-packages $HOME/local/pybool-env

Activate the environment (if you're not using a Posix system then have a look at the instructions at virtualenv_)::

    source $HOME/local/pybool-env/bin/activate
    
Every time you want to run pybool you will need to activate its environment in this way.

If you haven't already, download the
pybool `source distribution`_, unpack it somewhere and change into the top level directory. 



Configure, build, install
~~~~~~~~~~~~~~~~~~~~~~~~~

This Python package uses aksetup for installation, which means that
installation should be easy and quick. Try::
  
  ./configure.py --help

to examine the possible options. By the way, if a configuration option says ``several ok``,
then you may specify several values, separated by commas. Something like the
following should work::

  ./configure.py --boost-inc-dir=$HOME/local/include --boost-lib-dir=$HOME/local/lib
  python setup.py build
  python setup.py install
  
Configuration is obtained from files in this order::

  /etc/aksetup-defaults.py
  $HOME/.aksetup-defaults.py
  $PACKAGEDIR/siteconf.py

Once you've run configure, you can copy options from your ``siteconf.py`` file to
one of these files, and you won't ever have to configure them again manually.
In fact, you may pass the options ``--update-user`` and ``--update-global`` to
configure, and it will automatically update these files for you.

This is particularly handy if you want to perform an unattended or automatic
installation via easy_install_.

.. _easy_install: http://packages.python.org/distribute/easy_install.html
.. _pip: http://pypi.python.org/pypi/pip

To check that pybool has been successfully installed, change to an empty directory and
try running the following command::

  python -c "import pybool.cNetwork"

If you see any errors such as::

  Traceback (most recent call last):
    File "<string>", line 1, in <module>
  ImportError: libboost_python.so.1.45.0: cannot open shared object file: No such file or directory

you probably have not updated your ``LD_LIBRARY_PATH`` successfully.

The next step is to use the :doc:`tutorial<run>` on how to use pybool. Enjoy!


