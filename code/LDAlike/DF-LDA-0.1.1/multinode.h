/**
   This file is a part of

   DF-LDA - Implementation of the Dirichlet Forest (DF) prior
   for the Latent Dirichlet Allocation (LDA) model

   Copyright (C) 2009 David Andrzejewski (andrzeje@cs.wisc.edu)
 
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <vector>

#include <Python.h>

#define PY_ARRAY_UNIQUE_SYMBOL PyArray_API_intLDA
#define NO_IMPORT_ARRAY

#include <numpy/arrayobject.h>

#include "node.h"

#ifndef unif
#define unif() ((double) rand()) / ((double) RAND_MAX)
#endif

using namespace std;

#ifndef H_MULTINODE
#define H_MULTINODE

/**
 * Multinode
 */
class multinode: public node
{

 protected:
  vector<node*> variants;
  vector<vector<int> > fake_leafmaps;
  vector<double> variant_logweights;
  int y;

 public:
  multinode(PyObject* children, PyObject* maxind, 
            int leafstart, PyObject* variants);
  virtual ~multinode();
  virtual void modify_count(double val, int leaf);
  virtual double calc_wordterm(double val, int leaf);  
  int num_variants();
  int get_y();
  double var_logweight(int given_y);
  void set_y(int given_y);
  double calc_logpwz(int given_y);
};
#endif
