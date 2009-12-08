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
#include "dtnode.h"
#include "multinode.h"

/**
 * Return the vector of y-values for this topic
 */
vector<int> dtnode :: get_yvec()
{
  vector<node*> multi = get_multinodes();
  vector<int> yvals;
  for(uint mi = 0; mi < multi.size(); mi++)
    {
      multinode* mu = dynamic_cast<multinode*>(ichildren[mi]);
      yvals.push_back((*(mu)).get_y());
    }
  return yvals;
}

/**
 * Init from a given set of y-values 
 *
 */
int dtnode :: y_init(PyArrayObject* yinit, int ti)
{
  vector<node*> multi = get_multinodes();
  // Make sure dimensionality is correct 
  if(multi.size() != PyArray_DIM(yinit,1))
    {
      // ERROR 
      PyErr_SetString(PyExc_RuntimeError,
                      "q-init dimensionality mismatch - wrong # q-values");
      return BAD;
    }
  for(uint mi = 0; mi < multi.size(); mi++)
    {
      multinode* mu = dynamic_cast<multinode*>(ichildren[mi]);
      int yval = *((int*)PyArray_GETPTR2(yinit,ti,mi));
      //printf("initing w/ yval =%d\n",yval);
      // Make sure y-value in valid range [0,Q-1]
      if(yval < 0 || yval >= (*mu).num_variants())
        {
          // ERROR
          PyErr_SetString(PyExc_RuntimeError,
                          "Out-of-range value in q-init");
          return BAD;
        }
      else
        (*(mu)).set_y(yval);  
    }
  return OK;
}


/**
 * Do y-sampling for this topic
 */
void dtnode :: y_sampling()
{
  // Sample a new y-value for each multinode
  //
  vector<node*> multi = get_multinodes();
  for(uint mi = 0; mi < multi.size(); mi++)
    {
      multinode* mu = dynamic_cast<multinode*>(ichildren[mi]);
         
      // Get y-sampling values foreach variant
      //
      vector<double> vals;
      int numvar = (*(mu)).num_variants();
      for(int vi = 0; vi < numvar; vi++)
        {
          // Log-contribution of the agreeable subtree
          // +
          // Log-contribution of likely set weight
          //
          vals.push_back((*(mu)).calc_logpwz(vi) + 
                         (*(mu)).var_logweight(vi));
        }
            
      // Sample a new multinomial value from these log-likelihoods
      int y = node::log_mult_sample(vals);
      (*(mu)).set_y(y);
    }
}

/**
 * Do a simple q-report on multinodes
 */
void dtnode :: qreport()
{
  vector<node*> multi = get_multinodes();
  for(uint mi = 0; mi < multi.size(); mi++)
    {
      multinode* mu = dynamic_cast<multinode*>(ichildren[mi]);
      printf("%d",(*mu).get_y());
    }
}

/**
 * Do a report on multinodes
 */
void dtnode :: report()
{
  vector<node*> multi = get_multinodes();
  for(uint mi = 0; mi < multi.size(); mi++)
    {
      multinode* mu = dynamic_cast<multinode*>(ichildren[mi]);

      double e = *((double*)PyArray_GETPTR1(edges,mi));
      printf("Edge = %f\n",e);
      printf("Multinode %d, y = %d (of %d variants)\n",
             mi,(*mu).get_y(),(*mu).num_variants());
    }
}


/**
 * Calculate the log of the (uncollapsed) P(w|z) expression
 * (used for the y-sampling step)
 */
double dtnode :: calc_logpwz()
{ 
  // Calculate for this node
  double logpwz = lgamma(orig_edgesum) - lgamma(edgesum);
  for(uint ei = 0; ei < PyArray_DIM(edges,0); ei++)
    {
      logpwz += lgamma(*((double*)PyArray_GETPTR1(edges,ei))) -
        lgamma(*((double*)PyArray_GETPTR1(orig_edges,ei)));
    }
  // Calculate for all interior children
  for(uint i = 0; i < ichildren.size(); i++)
    {
      
      dtnode* child = dynamic_cast<dtnode*>(this->ichildren[i]);
      logpwz += (*child).calc_logpwz();
    }
  return logpwz;
}


/**
 * Calculate topic-word term of Gibbs sampling eqn
 */
double dtnode :: calc_wordterm(double val, int leaf)
{
  // Is this leaf under one of our interior children?
  double newval;
  for(uint i = 0; i < ichildren.size(); i++)
    {
      if(leaf <= maxind[i])
        {
          // Update value
          newval = ((*((double*)PyArray_GETPTR1(this->edges,i)))
                    / this->edgesum);
          // Recurisively mult value by child
          return (*(this->ichildren[i])).calc_wordterm(newval*val,leaf);
        }
    }

  // No, this leaf must be one of our leaves
  
  // Get leaf edge index
  int ei = ichildren.size() + (leaf - this->leafstart);
  // Update value
  newval = (*((double*)PyArray_GETPTR1(this->edges,ei))) / this->edgesum;
  return (val * newval);
}

/**
 * Update all branch counts associated with a given leaf
 */
void dtnode :: modify_count(double val, int leaf)
{
  // Is this leaf under one of our interior children?
  for(uint i = 0; i < ichildren.size(); i++)
    {
      if(leaf <= maxind[i])
        {
          // Update edge and sum
          (*((double*)PyArray_GETPTR1(this->edges,i))) += val;
          edgesum += val;
          // Recursively update child cts
          (*(ichildren[i])).modify_count(val,leaf);
          return;
        }
    }

  // No, this leaf must be one of our leaves
  
  // Get leaf edge index
  int ei = ichildren.size() + (leaf - leafstart);
  // Update edge and sum
  (*((double*)PyArray_GETPTR1(edges,ei))) += val;
  edgesum += val;
  return;  
}

/**
 * Constructor
 */
dtnode :: dtnode(PyArrayObject* arg_edges, PyObject* arg_children,
                 PyObject* arg_maxind, int arg_leafstart)
{       
  // Do some argument checking
  if(!PyList_Check(arg_children) || !PyList_Check(arg_maxind))
    {    
      // ERROR
      PyErr_SetString(PyExc_RuntimeError,
                      "Non-List children/maxind element in dirtree node");
      throw 1;
    }
  if(arg_leafstart < 0) 
    {
      // ERROR
      PyErr_SetString(PyExc_RuntimeError,
                      "Negative leafstart value in dirtree node");
      throw 1;
    }
  if(PyList_Size(arg_children) != PyList_Size(arg_maxind))
    {
      // ERROR
      PyErr_SetString(PyExc_RuntimeError,
                      "Different sizes for children/maxind in dirtree node");
      throw 1;
    }    
  double edgemin = PyFloat_AsDouble(PyArray_Min(arg_edges,NPY_MAXDIMS,NULL));    
  if(edgemin <= 0) 
    {
      // ERROR
      PyErr_SetString(PyExc_RuntimeError,
                      "Negative/zero edge value in dirtree node");
      throw 1;
    }

  // Populate data members
  int nci = PyList_Size(arg_children);
  this->leafstart = arg_leafstart;
  
  // Get edge values and sum
  // (note that this *must* be a copy!)
  npy_intp* edims = new npy_intp[1];
  edims[0] = PyArray_DIM(arg_edges,0);
  this->edges = (PyArrayObject*) PyArray_ZEROS(1,edims,PyArray_DOUBLE,0);
  PyArray_CopyInto(this->edges,arg_edges);
  this->edgesum = PyFloat_AsDouble(PyArray_Sum(this->edges,NPY_MAXDIMS,
                                               PyArray_DOUBLE,NULL));   
  // Also make a copy of *original* edge values 
  // (not augmented by counts)
  this->orig_edges = (PyArrayObject*) PyArray_ZEROS(1,edims,PyArray_DOUBLE,0);
  PyArray_CopyInto(this->orig_edges,arg_edges);
  this->orig_edgesum = PyFloat_AsDouble(PyArray_Sum(this->orig_edges,NPY_MAXDIMS,
                                                    PyArray_DOUBLE,NULL));   
  delete[] edims;

  // Recursively build children
  // 
  int c;
  for(c = 0; c < nci; c++)
    {    
      // Get max leaf index under this child, check it
      maxind.push_back(PyInt_AsLong(PyList_GetItem(arg_maxind,c)));
      if(maxind[c] < 0)
        {
          // ERROR
          PyErr_SetString(PyExc_RuntimeError,
                          "Negative maxind value in dirtree node");
          throw 1;
        }
      // exploiting boolean short-circuit here...
      if(c > 0 && maxind[c] <= maxind[c-1])
        {
          // ERROR
          PyErr_SetString(PyExc_RuntimeError,
                          "Non-increasing maxind value in dirtree node");
          throw 1;
        }
     
      // Recursively build child
      //
      try{
        // Check that node tuple size is correct
        PyObject* pynode = PyList_GetItem(arg_children,c);
        // Is child a dtnode, or a multinode?
        //
        if(4 == PyTuple_Size(pynode))
          {
            // dtnode

            // Unpack tuple contents
            PyArrayObject* cedges = (PyArrayObject*) PyTuple_GetItem(pynode,0);
            PyObject* cchildren = PyTuple_GetItem(pynode,1);
            PyObject* cmaxind = PyTuple_GetItem(pynode,2);
            int cleafstart = PyInt_AsLong(PyTuple_GetItem(pynode,3));
        
            // Build child dtnode
            node* newchild = new dtnode(cedges,cchildren,
                                        cmaxind,cleafstart);
            ichildren.push_back(newchild);
          }
        else if(5 == PyTuple_Size(pynode))
          {
            // multinode

            // Unpack tuple contents        
            PyObject* cchildren = PyTuple_GetItem(pynode,1);
            PyObject* cmaxind = PyTuple_GetItem(pynode,2);
            int cleafstart = PyInt_AsLong(PyTuple_GetItem(pynode,3));
            PyObject* variants = PyTuple_GetItem(pynode,4);

            // Build child multinode
            node* newchild = new multinode(cchildren,cmaxind,
                                           cleafstart,variants);
            ichildren.push_back(newchild);
          }
        else{
          // ERROR
          PyErr_SetString(PyExc_RuntimeError,
                          "Node has wrong number of elements");
          throw 1;
        }
      }
      catch (int err)
        {
          throw 1;
        }
    }
}

/**
 * Destuctor
 */
dtnode :: ~dtnode()
{
  //
  // Decrease ref cts on NumPy arrays, ichildren will be 
  // deleted by base class destructor (node)
  //
  Py_DECREF(edges);
  Py_DECREF(orig_edges);
}
