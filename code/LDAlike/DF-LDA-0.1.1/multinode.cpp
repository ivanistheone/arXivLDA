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
#include "multinode.h"
#include "dtnode.h"


/**
 * Return the log of the weight for a given variant
 * (the weight is equal to the sum of all leaf weights
 * under the likely internal node for this variant)
 */
double multinode :: var_logweight(int given_y) 
{
  return variant_logweights[given_y];
}

/**
 * Calculate logpwz term for a given y value
 */
double multinode :: calc_logpwz(int given_y)
{
  dtnode* dtv = dynamic_cast<dtnode*>(variants[given_y]);
  return (*dtv).calc_logpwz();
}

/**
 * Set a new y value
 */
void multinode :: set_y(int given_y)
{
  y = given_y;
}

/**
 * Return y value (which variant is 'active') for this multinode
 */
int multinode :: get_y()
{
  return y;
}

/**
 * Return number of variants represented by this multinode
 */
int multinode :: num_variants()
{
  return variants.size();
}

/**
 * Calculate topic-word term of Gibbs sampling eqn
 */
double multinode :: calc_wordterm(double val, int leaf)
{ 
  // Is this leaf under one of our interior children?
  double newval;
  for(uint i = 0; i < ichildren.size(); i++)
    {
      if(leaf <= maxind[i])
        {
          // Update value with call to appropriate variant
          newval = (*(variants[y])).calc_wordterm(val,fake_leafmaps[y][i]);

          // Pass value down to actual child
          return (*(this->ichildren[i])).calc_wordterm(newval,leaf);
        }
    }

  // No, this leaf must be one of our leaves
  
  // Get leaf edge index
  int ei = ichildren.size() + (leaf - this->leafstart);
  // Update value with call to appropriate variant
  return (*(variants[y])).calc_wordterm(val,fake_leafmaps[y][ei]);
}

/**
 * Update all branch counts associated with a given leaf
 */
void multinode :: modify_count(double val, int leaf)
{
  // Is this leaf under one of our interior children?
  for(uint i = 0; i < ichildren.size(); i++)
    {
      if(leaf <= maxind[i])
        {
          // Update all variants
          for(uint v = 0; v < variants.size(); v++)
            {
              (*(variants[v])).modify_count(val,fake_leafmaps[v][i]);
            }

          // Recursively update child cts
          (*(ichildren[i])).modify_count(val,leaf);
          return;
        }
    }

  // No, this leaf must be one of our leaves
  
  // Get leaf edge index
  int ei = ichildren.size() + (leaf - leafstart);  
  // Update all variants
  for(uint v = 0; v < variants.size(); v++)
    {
      (*(variants[v])).modify_count(val,fake_leafmaps[v][ei]);
    }

  return;  
}

/**
 * Constructor
 */
multinode :: multinode(PyObject* arg_children, PyObject* arg_maxind, 
                       int arg_leafstart, PyObject* arg_variants)
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

  // Populate data members
  int nci = PyList_Size(arg_children);
  this->leafstart = arg_leafstart;
  
  // Build variants
  for(uint v = 0; v < PyList_Size(arg_variants); v++)
    {
      // Unpack variant dtnode and fake leafmap
      PyObject* newvar = PyList_GetItem(arg_variants,v);
      PyObject* vdtn = PyTuple_GetItem(newvar,0);
      PyObject* flm = PyTuple_GetItem(newvar,1);

      try {
        // Extract fake leaf map for this variant
        vector<int> cur_flm;
        for(uint fli = 0; fli < PyList_Size(flm); fli++)
          cur_flm.push_back(PyInt_AsLong(PyList_GetItem(flm,fli)));
        fake_leafmaps.push_back(cur_flm);

        // Unpack tuple contents
        PyArrayObject* vedges = (PyArrayObject*) PyTuple_GetItem(vdtn,0);
        PyObject* vchildren = PyTuple_GetItem(vdtn,1);
        PyObject* vmaxind = PyTuple_GetItem(vdtn,2);
        int vleafstart = PyInt_AsLong(PyTuple_GetItem(vdtn,3));
        
        // Build child dtnode
        dtnode* newvardt = new dtnode(vedges,vchildren,
                                      vmaxind,vleafstart); 
        variants.push_back(newvardt);

        // Calculate log of weighing term (for y-sampling)
        PyArrayObject* likely_edges = (PyArrayObject*) 
          PyTuple_GetItem(PyList_GetItem(vchildren,0),0);
        double le_sum = PyFloat_AsDouble(PyArray_Sum(likely_edges,NPY_MAXDIMS,
                                                     PyArray_DOUBLE,NULL));
        variant_logweights.push_back(log(le_sum));
      }

      catch (int err)
        {
          throw 1;
        }                             
    }

  // Init y to -1 
  // (will raise errors if not properly initialized elsewhere)
  y = -1;

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
            dtnode* newchild = new dtnode(cedges,cchildren,
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
            PyObject* cvariants = PyTuple_GetItem(pynode,4);

            // Build child multinode
            node* newchild = new multinode(cchildren,cmaxind,
                                           cleafstart,cvariants);
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
multinode :: ~multinode()
{
  //
  // Delete variants, ichildren will be 
  // deleted by base class destructor (node)
  //
  for(uint vi = 0; vi < variants.size(); vi++)
    delete variants[vi];
}
