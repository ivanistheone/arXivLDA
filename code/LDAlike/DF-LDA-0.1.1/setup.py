from distutils.core import Extension, setup
import os

from numpy.distutils.misc_util import *

numpyincl = get_numpy_include_dirs()

cpp_mod = Extension('intLDA',
                    sources = ['intLDA.cpp','dtnode.cpp',
                               'multinode.cpp','node.cpp'],
                    include_dirs = [os.getcwd()] + numpyincl,
                    library_dirs = [],
                    libraries = [],
                    extra_compile_args = ['-O3','-Wall'],
                    extra_link_args = [])

py_mods = ['ConstraintCompiler','DirichletForest',
           'DirichletTreeNodes','PrefGraph','Reporter']

setup(name = 'DF-LDA',
      description = 'LDA with Dirichlet Forest Prior',
      version = '0.1.1',
      author = 'David Andrzejewski',
      author_email = 'andrzeje@cs.wisc.edu',
      license = 'GNU General Public License (Version 3 or later)',
      url = 'http://pages.cs.wisc.edu/~andrzeje/research/df_lda.html',
      ext_modules = [cpp_mod],
      py_modules = py_mods)


