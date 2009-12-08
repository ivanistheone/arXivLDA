from distutils.core import setup, Extension
import os

from numpy.distutils.misc_util import *

numpyincl = get_numpy_include_dirs()

dldamodule = Extension("deltaLDA",
                    sources = ["deltaLDA.c"],
                    include_dirs = [os.getcwd()] + numpyincl,
                    library_dirs = [],
                    libraries = [],
                    extra_compile_args = ['-O3','-Wall'],
                    extra_link_args = [])

setup(name = 'deltaLDA',
      description = 'Delta LDA model',
      version = '0.1.1',
      author = 'David Andrzejewski',
      author_email = 'andrzeje@cs.wisc.edu',
      license = 'GNU General Public License (Version 3 or later)',
      url = 'http://pages.cs.wisc.edu/~andrzeje/research/df_lda.html',
      ext_modules = [dldamodule],
      py_modules = [])
