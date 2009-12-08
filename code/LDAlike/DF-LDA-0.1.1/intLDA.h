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
#include <vector>

#define PY_ARRAY_UNIQUE_SYMBOL PyArray_API_intLDA

#include <Python.h>
#include <numpy/arrayobject.h>

#include "dtnode.h"
#include "multinode.h"

#define OK 0
#define BAD 1

typedef struct {
  vector<dtnode*> topics;
  PyArrayObject* nd;
} counts;

typedef struct {
  PyArrayObject* alpha;
  PyArrayObject* alphasum;
  vector<int> leafmap;
  int T;
} model_params;

typedef struct {
  int D;
  int W;
  vector<int> doclens;
  vector<vector<int> > docs;
  vector<vector<int> > sample;
  vector<int> f;
} dataset;


static PyObject* intLDA(PyObject *self, PyObject *args, PyObject* keywds);

static int convert_args(PyObject* docs_arg, PyArrayObject* alpha,
                        PyObject* dirtree, PyObject* leafmap,
                        PyObject* f_arg, 
                        model_params* mp, dataset* ds, counts* c);

static int init_model(model_params* mp, dataset* ds,counts* c, 
                      int randseed, PyObject* zinit, PyArrayObject* qinit);

static void gibbs_chain(model_params* mp, dataset* ds, counts* c);

static PyArrayObject* est_phi(model_params* mp, dataset* ds, counts* c);
static PyArrayObject* est_theta(model_params* mp, dataset* ds, counts* c);
