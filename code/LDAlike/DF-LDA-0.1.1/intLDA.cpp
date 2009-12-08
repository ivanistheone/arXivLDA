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
#include "intLDA.h"

/**
 * This is the exposed method which is called from Python
 */
static PyObject * intLDA(PyObject *self, PyObject *args, PyObject* keywds)
{
  // Null-terminated list of arg keywords
  //
  static char* kwlist[] = {"docs","alpha","dirtree","leafmap","numsamp",
                                 "randseed","f","init","verbose","sampout",
                                 "burnin","interval","qinit",NULL};
  // Required args
  //
  PyObject* docs_arg; // List of Lists
  PyArrayObject* alpha; // NumPy Array
  PyObject* dirtree; // Tuple
  PyObject* leafmap; // List
  int numsamp;
  int randseed;
  // Optional args
  // 
  PyObject* f_arg = NULL; // List of f-labels foreach doc
  PyObject* init = NULL; // List of Lists to initialize Gibbs
  int verbose = 0; // 1 = verbose output (sampler progress)
  int sampout = 0; // 1 = output actual samples
  int burnin = 0; // Number of initial samples to throw away
  int interval = 1; // Number of samples between printed samples
  PyArrayObject* qinit = NULL; // NumPy Array

  // Parse function args
  //
  if(!PyArg_ParseTupleAndKeywords(args,keywds,"O!O!O!O!ii|O!O!iiiiO!",kwlist,
                                  &PyList_Type,&docs_arg,
                                  &PyArray_Type,&alpha,
                                  &PyTuple_Type,&dirtree,
                                  &PyList_Type,&leafmap,
                                  &numsamp,&randseed,
                                  &PyList_Type,&f_arg,
                                  &PyList_Type,&init,
                                  &verbose,&sampout,
                                  &burnin,&interval,
                                  &PyArray_Type,&qinit))
    // ERROR - bad args
    return NULL;

  // Init random number generator
  //
  node::set_randseed(static_cast<unsigned>(randseed));
 
  // Use args to populate structs
  // (also check for *validity*)
  //
  model_params* mp = new model_params;
  dataset* ds = new dataset;
  counts* c = new counts;
  if(BAD == convert_args(docs_arg,alpha,dirtree,
                         leafmap,f_arg,mp,ds,c))
    {
      // Args bad! Return to Python...error condition should be set
      delete mp;
      delete ds;
      delete c;
      return NULL;
    }
      
  // Initialize model, and make sure init went OK
  //
  int initstatus = init_model(mp,ds,c,randseed,init,qinit);
  if(initstatus == BAD)
    {
      // ERROR - something went wrong with our init            
      for(int ti=0; ti < mp->T; ti++)
        {
          delete c->topics[ti];
        }          
      delete c;

      Py_DECREF(mp->alphasum);      
      delete mp;

      delete ds;
      return NULL;
    }      
      
  // Do burn-in
  //
  for(int si=0; si < burnin; si++)
    {
      gibbs_chain(mp,ds,c);
    }
  
  // Do the requested number of Gibbs samples 
  //
  for(int si=0; si < numsamp; si++)
    {
      // In order to make it easier to test against deltaLDA
      node::set_randseed(static_cast<unsigned>(randseed+si));      

      if(verbose == 1)
        {
          printf("Gibbs sample %d of %d\n",si,numsamp);
          for(int ti=0; ti < mp->T; ti++)
            {
              printf("Topic %d q=",ti);
              (*(c->topics[ti])).qreport();
              printf("\n");
            }          
        }

      // Sample z values for each word, 
      // then sample y values for each topic
      //
      // (repeat <interval> times for good sampling practice)
      //
      for(int ii=0; ii < interval; ii++) 
        {
          gibbs_chain(mp,ds,c);
          for(int ti=0; ti < mp->T; ti++)
            {
              (*(c->topics[ti])).y_sampling();                       
            }          
        }

      // If sampout flag set, output 
      // -z-sample
      // -q-sample
      // -phi estimate 
      // 
      if(sampout == 1) 
        {
          // z-sample
          printf("z=");
          int d,i;
          for(d = 0; d < ds->D; d++) 
            {
              for(i = 0; i < ds->doclens[d]; i++) 
                {
                  printf("%d",ds->sample[d][i]);
                }
            }
          printf(" ");

          // q-sample
          printf("q=");
          for(int ti=0; ti < mp->T; ti++)
            {
              (*(c->topics[ti])).qreport();
              if(ti < (mp->T-1))
                printf(",");
            }
          printf("\n");

          // phi estimate
          PyArrayObject* phi = est_phi(mp, ds, c);
          printf("[");
          for(int pt = 0; pt < mp->T; pt++)
            {
              printf("[");
              for(int pw = 0; pw < ds->W; pw++)
                {
                  double val = *((double*)PyArray_GETPTR2(phi,pt,pw));
                  if(pw < (ds->W - 1))                
                    printf("%f,",val);
                  else
                    printf("%f",val);
                }
              if(pt < (mp->T -1))
                printf("],");
              else
                printf("]");
            }
          printf("]\n");
          Py_DECREF(phi);     
        }
    }
  
  // Estimate phi and theta
  //
  PyArrayObject* phi = est_phi(mp, ds, c);
  PyArrayObject* theta = est_theta(mp, ds, c);

  // Convert final z-sample back to List of Lists for return
  //
  PyObject* finalsamp = PyList_New(ds->D);
  for(int d = 0; d < ds->D; d++)
    {
      PyObject* docz = PyList_New(ds->doclens[d]);
      for(int i = 0; i < ds->doclens[d]; i++)
        PyList_SetItem(docz,i,PyInt_FromLong(ds->sample[d][i]));
      PyList_SetItem(finalsamp,d,docz);
    }
  
  // Get final q-sample
  //
  int P = ((*(c->topics[0])).get_multinodes()).size();
  npy_intp* qdims = new npy_intp[2];
  qdims[0] = mp->T;
  qdims[1] = P;
  PyArrayObject* qsamp = (PyArrayObject*)
    PyArray_ZEROS(2,qdims,PyArray_INT,0);
  delete[] qdims;
  for(int ti = 0; ti < mp->T; ti++)
    {
      vector<int> q = (*(c->topics[ti])).get_yvec();
      for(int yi = 0; yi < P; yi++) 
        {
          *((int*)PyArray_GETPTR2(qsamp,ti,yi)) = q[yi];
        }
    }

  // Package phi, theta, and final sample in tuple for return
  //
  PyObject* retval = PyTuple_New(4);
  PyTuple_SetItem(retval,0,(PyObject*) phi);
  PyTuple_SetItem(retval,1,(PyObject*) theta);
  PyTuple_SetItem(retval,2,finalsamp);
  PyTuple_SetItem(retval,3,(PyObject*) qsamp);

  // Do some final memory cleanup...
  //
 
  // Counts
  Py_DECREF(c->nd);
  for(int ti=0; ti < mp->T; ti++)
    {
      delete c->topics[ti];
    }          
  delete c;
  // Model params
  Py_DECREF(mp->alphasum);
  delete mp;
  // Dataset
  delete ds;

  return (PyObject*) retval;
}

/**
 * Do an "online" init of Gibbs chain, adding one word
 * position at a time and then sampling for each new position
 */
static int init_model(model_params* mp, dataset* ds, counts* c,
                      int randseed,PyObject* zinit, PyArrayObject* qinit)
{   
  // Init y-values for each topic
  //
  int status = OK;
  if(qinit != NULL)
    {
      // Initialize from user-provided values
      // (checking these as we go)
      if(mp->T != PyArray_DIM(qinit,0)) 
        {
          // ERROR
          PyErr_SetString(PyExc_RuntimeError,
                          "q-init dimensionality mismatch - wrong # topics");
          return BAD;
        }

      for(int ti=0; ti < mp->T; ti++)
        {
          status = (*(c->topics[ti])).y_init(qinit,ti);
          if(status == BAD)
            {
              // ERROR - flag should already be set
              return BAD;
            }
        }
    }
  else
    {
      // Initialize by sampling proportionally
      //
      for(int ti=0; ti < mp->T; ti++)
        {
          (*(c->topics[ti])).y_sampling();
        }
    }

  // Optional report of initial topics
//   printf("Initial q-values\n");
//   for(int ti=0; ti < mp->T; ti++)  
//     {
//       printf("Topic %d = ",ti);
//       (*(c->topics[ti])).qreport();
//       printf("\n");
//     }       

  // Re-seed random number generator
  // !!! TESTING HACK TO ENSURE DELTA-LDA IDENTICAL RESULTS !!!
  //
  node::set_randseed(static_cast<unsigned>(randseed));

  // If z-sample provided, has correct number of docs
  if(zinit != NULL && ds->D != PyList_Size(zinit))
    {
      // ERROR
      PyErr_SetString(PyExc_RuntimeError,
                      "Number of docs / number of init samples mismatch");
      return BAD;
    }

  // Init ND count matrix 
  npy_intp* nddims = new npy_intp[2];
  nddims[0] = ds->D;
  nddims[1] = mp->T;
  c->nd =  (PyArrayObject*) PyArray_ZEROS(2,nddims,PyArray_INT,0);
  delete[] nddims;
  
  // Build init z sample, one word at a time
  //
  ds->sample.resize(ds->D);

  // Temporary array used for sampling  
  double* num = new double[mp->T];

  // For each doc in corpus
  for(int d = 0; d < ds->D; d++) 
    {
      // Create this sample
      vector<int> newsample;
      newsample.resize(ds->doclens[d]);
      ds->sample[d] = newsample;
      
      // Do we have a z-init or not?
      //
      if(zinit != NULL) 
        {
          //
          // INIT FROM Z-INIT
          //
          PyObject* docinit = PyList_GetItem(zinit,d);
          // verify that this is a List
          if(!PyList_Check(docinit))
            {
              // ERROR
              PyErr_SetString(PyExc_RuntimeError,
                              "Non-List element in initial sample");
              Py_DECREF(c->nd);
              delete[] num;
              return BAD;
            }
          // verify that it has the right length
          else if(ds->doclens[d] != PyList_Size(docinit))
            {
              // ERROR
              PyErr_SetString(PyExc_RuntimeError,
                              "Init sample/doc length mismatch");
              Py_DECREF(c->nd);
              delete[] num;
              return BAD;
            }      
          // Copy over values from List to our sample[]
          for(int i = 0; i < ds->doclens[d]; i++) 
            {
              // Get topic from init sample
              int zi = PyInt_AsLong(PyList_GetItem(docinit,i));
              if(zi < 0 || zi > (mp->T-1))
                {
                  // ERROR
                  PyErr_SetString(PyExc_RuntimeError,
                                  "Non-numeric or out of range init sample value");
                  Py_DECREF(c->nd);
                  delete[] num;
                  return BAD;
                }
 
              // Get the corresponding word
              int wi = ds->docs[d][i];

              // Store topic and increment counts
              ds->sample[d][i] = zi;
              (*((int*)PyArray_GETPTR2(c->nd,d,zi)))++;
              (*(c->topics[zi])).modify_count(1,mp->leafmap[wi]);
            }
      }
      else
        {
          //
          // INIT "ONLINE"
          //
          for(int i = 0; i < ds->doclens[d]; i++)
            {      
              int w_i = ds->docs[d][i];
	
              // For each topic, calculate numerators
              double norm_sum = 0;
              for(int j = 0; j < mp->T; j++) 
                {               
                  // Calculate numerator for this topic
                  // (NOTE: alpha denom omitted, since same for all topics) 
                  double alpha_j = *((double*)PyArray_GETPTR2(mp->alpha,ds->f[d],j));
                  double wordterm = (*(c->topics[j])).calc_wordterm(1,mp->leafmap[w_i]);
                  num[j] = wordterm * (*((int*)PyArray_GETPTR2(c->nd,d,j))+alpha_j);
                  norm_sum += num[j];
                }
	
              // Draw a sample
              //   
              int newz = node::mult_sample(num,norm_sum);
	
              // Update counts and initial sample vec
              //
              ds->sample[d][i] = newz;
              (*((int*)PyArray_GETPTR2(c->nd,d,newz)))++;          
              (*(c->topics[newz])).modify_count(1,mp->leafmap[w_i]);
            }
        }
    }
  // Cleanup and return
  //
  delete[] num;
  return OK;
}

/**
 * Run Gibbs chain to get a new full sample
 */
void gibbs_chain(model_params* mp, dataset* ds, counts* c)
{ 
  // Use Gibbs sampling to get a new z
  // sample, one position at a time
  //

  // Temporary array used for sampling
  double* num = new double[mp->T];

  // For each doc in corpus
  int d,j,i;
  for(d = 0; d < ds->D; d++) 
    {
      // For each word in doc
      for(i = 0; i < ds->doclens[d]; i++)
        {      
          // remove this w/z pair from all count/cache matrices 
          int z_i = ds->sample[d][i];
          int w_i = ds->docs[d][i];
          
          (*(c->topics[z_i])).modify_count(-1,mp->leafmap[w_i]);
          (*((int*)PyArray_GETPTR2(c->nd,d,z_i)))--;
      	
          // For each topic, calculate numerators
          double norm_sum = 0;
          for(j = 0; j < mp->T; j++) 
            { 
              // Calculate numerator for each topic
              // (NOTE: alpha denom omitted, since same for all topics)
              double alpha_j = *((double*)PyArray_GETPTR2(mp->alpha,ds->f[d],j));
              double wordterm = (*(c->topics[j])).calc_wordterm(1,mp->leafmap[w_i]);
              num[j] = wordterm * (*((int*)PyArray_GETPTR2(c->nd,d,j))+alpha_j);
              norm_sum += num[j];
            }
	
          // Draw a sample
          //
          j = node::mult_sample(num,norm_sum);
	
          // update count/cache matrices and sample vec
          //
          ds->sample[d][i] = j;
          (*((int*)PyArray_GETPTR2(c->nd,d,j)))++;
          (*(c->topics[j])).modify_count(1,mp->leafmap[w_i]);
        }
    }
  // Just cleanup and return 
  // (new sample will be returned in ds->sample
  //
  delete[] num;
  return;
}

/**
 * Use final sample to estimate theta = P(z|d)
 */
PyArrayObject* est_theta(model_params* mp, dataset* ds, counts* c)
{ 
  npy_intp* tdims = new npy_intp[2];
  tdims[0] = ds->D;
  tdims[1] = mp->T;
  PyArrayObject* theta = (PyArrayObject*) PyArray_ZEROS(2,tdims,PyArray_DOUBLE,0);
  delete[] tdims;

  PyArrayObject* rowsums = (PyArrayObject*) PyArray_Sum(c->nd,1,PyArray_DOUBLE,NULL);

  int d,t;
  for(d = 0; d < ds->D; d++) 
    {
      double rowsum = *((double*)PyArray_GETPTR1(rowsums,d));
      int f = ds->f[d];
      double alphasum = *((double*)PyArray_GETPTR1(mp->alphasum,f));
      for(t = 0; t < mp->T; t++)
        {
          double alpha_t = *((double*)PyArray_GETPTR2(mp->alpha,f,t));
          int ndct = *((int*)PyArray_GETPTR2(c->nd,d,t));

          // Calc and assign theta entry
          double newval = (ndct + alpha_t) / (rowsum+alphasum);
          *((double*)PyArray_GETPTR2(theta,d,t)) = newval;
        }
    }
  return theta;
}

/**
 * Use final sample to estimate phi = P(w|z)
 */
PyArrayObject* est_phi(model_params* mp, dataset* ds, counts* c)
{   
  npy_intp* pdims = new npy_intp[2];
  pdims[0] = mp->T;
  pdims[1] = ds->W;
  PyArrayObject* phi = (PyArrayObject*) PyArray_ZEROS(2,pdims,PyArray_DOUBLE,0);
  delete[] pdims;

  int t,w;
  for(t = 0; t < mp->T; t++) 
    {
      for(w = 0; w < ds->W; w++) 
        {
          *((double*)PyArray_GETPTR2(phi,t,w)) = 
            (*(c->topics[t])).calc_wordterm(1,mp->leafmap[w]);
        }
    }
  return phi;
}


/**
 * Simultaneously check args and populate structs
 */
static int convert_args(PyObject* docs_arg, PyArrayObject* alpha,
                        PyObject* dirtree, PyObject* leafmap,
                        PyObject* f_arg, 
                        model_params* mp, dataset* ds, counts* c)
  {
    int i;
    int fmax = 0; // Will need to ensure alpha dim is this big
    int D = PyList_Size(docs_arg);
    ds->D = D;

    // If f-labels were not specified, just set all to zero
    if(f_arg == NULL)
      {
        ds->f.assign(D,0);
      }
    // Otherwise convert and check
    else
      {
        if(D != PyList_Size(f_arg)) 
          {
            // ERROR
            PyErr_SetString(PyExc_RuntimeError,
                            "f and docs have different lengths");
            return BAD;            
          }
        for(i = 0; i < D; i++)
          {            
            ds->f.push_back(PyInt_AsLong(PyList_GetItem(f_arg,i)));
            // If PyInt_AsLong fails, it returns -1
            // (also should not be getting neg f-values anyway)
            if(ds->f[i] < 0)
              {
                // ERROR
                PyErr_SetString(PyExc_RuntimeError,
                                "Non-numeric or negative f-value");
                return BAD;
              }
            else if(ds->f[i] > fmax)
              {
                fmax = ds->f[i];
              }
          }
      }
  
    // Get info from parameters
    int W = PyList_Size(leafmap);
    ds->W = W;
    int F = PyArray_DIM(alpha,0);
    int T = PyArray_DIM(alpha,1);
    mp->alpha = alpha;
    mp->T = T;
    mp->alphasum = (PyArrayObject*) PyArray_Sum(alpha,1,PyArray_DOUBLE,NULL);

    // Check max F
    if(fmax != (F - 1))
      {
        // ERROR
        Py_DECREF(mp->alphasum);
        PyErr_SetString(PyExc_RuntimeError,
                        "Alpha/f dimensionality mismatch");
        return BAD;
      }

    // Check that all alpha values are non-negative
    //
    double alphamin = PyFloat_AsDouble(PyArray_Min(alpha,NPY_MAXDIMS,NULL));
    if(alphamin < 0)       
      {
        // ERROR
        Py_DECREF(mp->alphasum);      
        PyErr_SetString(PyExc_RuntimeError,
                        "Negative Alpha value");
        return BAD;
      }

    // Read in and check leafmap (must be a valid permutation)
    //
    int* valid = new int[W];
    for(i = 0; i < W; i++)
      valid[i] = 0;
    mp->leafmap.reserve(W);
    for(i = 0; i < W; i++)
      {
        mp->leafmap[i] = PyInt_AsLong(PyList_GetItem(leafmap,i));
        if(mp->leafmap[i] < 0 || mp->leafmap[i] > (W-1))
          {
            // ERROR
            Py_DECREF(mp->alphasum);      
            PyErr_SetString(PyExc_RuntimeError,
                            "Out-of-range value in leafmap");
            delete[] valid;
            return BAD;
          }
        valid[mp->leafmap[i]] = 1;
      }    
    int sum = 0;
    for(i = 0; i < W; i++)
      sum += valid[i];
    if(sum != W)
      {
        // ERROR
        Py_DECREF(mp->alphasum);      
        PyErr_SetString(PyExc_RuntimeError,
                        "Out-of-range value in leafmap");
        delete[] valid;
        return BAD;
      }
    delete[] valid;

    // Use dirtree arg to build Dirichlet Tree 
    // data structures for each topic
    // 
    
    // Populate counts struct

    // Build Dirichlet Tree for each topic
    for(int ti = 0; ti < T; ti++)
      {
        try{
          if(4 == PyTuple_Size(dirtree))
            {
              // Unpack tuple contents
              PyArrayObject* cedges = (PyArrayObject*) 
                PyTuple_GetItem(dirtree,0);
              PyObject* cchildren = PyTuple_GetItem(dirtree,1);
              PyObject* cmaxind = PyTuple_GetItem(dirtree,2);
              int cleafstart = PyInt_AsLong(PyTuple_GetItem(dirtree,3));
        
              // Build root node of Dirichlet Tree
              dtnode* newtopic = new dtnode(cedges,cchildren,
                                            cmaxind,cleafstart);
              c->topics.push_back(newtopic);
            }
          else{
            // ERROR
            Py_DECREF(mp->alphasum);      
            PyErr_SetString(PyExc_RuntimeError,
                            "Node has wrong number of elements");
            return BAD;
          }
        }
        catch (int err)
          {           
            // ERROR - called fcn has set err condition
            Py_DECREF(mp->alphasum);      
            return BAD;
          }        
      }

    // Convert documents from PyObject* to vector<int>
    //
    int d;
    ds->doclens.resize(D);
    ds->docs.resize(D);
    for(d = 0; d < D; d++)
      {
        PyObject* doc = PyList_GetItem(docs_arg,d);
        if(!PyList_Check(doc))
          {
            // ERROR
            for(int ti=0; ti < mp->T; ti++)
              {
                delete c->topics[ti];
              }          
            Py_DECREF(mp->alphasum);      
            PyErr_SetString(PyExc_RuntimeError,
                            "Non-List element in docs List");
            return BAD;
          }
        ds->doclens[d] = PyList_Size(doc);
        vector<int> newdoc;
        newdoc.resize(ds->doclens[d]);
        ds->docs[d] = newdoc;
        for(i = 0; i < ds->doclens[d]; i++)
          {
            ds->docs[d][i] = PyInt_AsLong(PyList_GetItem(doc,i));            
            if(ds->docs[d][i] < 0 || ds->docs[d][i] > (W - 1))
              {
                // ERROR
                for(int ti=0; ti < mp->T; ti++)
                  {
                    delete c->topics[ti];
                  }          
                Py_DECREF(mp->alphasum);      
                PyErr_SetString(PyExc_RuntimeError,
                                "Non-numeric or out of range word");
                return BAD;
              }
          }
      }

    // Everything went fine, return OK code
    //
    return OK;
  }
 

//
// PYTHON EXTENSION BOILERPLATE BELOW
//

// Defines the module method table
PyMethodDef methods[] = 
  {
    {"intLDA", (PyCFunction) intLDA, 
     METH_VARARGS | METH_KEYWORDS, "Run Interactive LDA"},
    {NULL, NULL, 0, NULL}  // Is a 'sentinel' (?)
  };

// This is a macro that does stuff for us (linkage, declaration, etc)
PyMODINIT_FUNC 
initintLDA() // Passes method table to init our module
{
  (void) Py_InitModule("intLDA", methods); 
  import_array(); // Must do this to satisfy NumPy (!)
}
