"""
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
"""

""" Contains the DirichletForest class """

from numpy import *

from Reporter import Reporter
import ConstraintCompiler as CC
from intLDA import intLDA

def readDocs(doc_fn):
    """ Read docs in sparse SVMLight-style format """
    docs = []
    for line in open(doc_fn):
        doc = []
        pairs = line.split()
        for pair in pairs:
            (wi,ct) = [int(v) for v in pair.split(':')]
            doc.extend([wi] * ct)
        docs.append(doc)
    return docs

def combine(*constraints):
    """
    Given Lists of constraints, combine into a single set

    constraints = varargs Tuple of Lists of constraints
    each constraint = 2-element Tuples
    constraint[0] = List of 2-element must-link constraint Tuples
    constraint[1] = List of 2-element cannot-link constraint Tuples
    """
    combo = (set([]),set([]))
    for constr in constraints:
        combo[0].update(constr[0])
        combo[1].update(constr[1])
    return combo

class DirichletForest(Reporter):
    """ Dirichlet Forest Prior for Topic Modeling  """

    def __init__(self,alpha,beta,eta,T,W,vocab=None):
        """ Constructor """
        # Basic parameters
        self.vocab = vocab
        (self.T,self.W) = (T,W)
        self.alpha = alpha * ones((1,self.T))
        self.beta = beta
        self.eta = eta
        # Constraints and DF structures
        self.constr = (set(),set())
        (self.root,self.leafmap,
         self.mlcc,self.clcc,self.allowable) = 5 * (None,)
        # Outputs of inference    
        (self.zsamp,self.qsamp,
         self.phi,self.theta) = 4 * (None,)
        # Docs
        self.docs = None
        
    def inference(self,docs,numsamp,randseed,**kwargs):
        """
        Do inference via collapsed Gibbs sampling,
        estimate phi/theta from final sample
        """
        # Compile preferences, if we haven't already
        if(self.root == None):
            self.compile()
        # Save numsamp and randseed arguments
        (self.numsamp,self.randseed) = (numsamp,randseed)
        # Do inference, initializing sampler if applicable
        if(self.zsamp != None and self.qsamp != None):
            (self.phi,self.theta,self.zsamp,self.qsamp) = intLDA(
                docs,self.alpha,self.root,
                self.leafmap,numsamp,randseed,
                init=self.zsamp,qinit=self.qsamp,
                **kwargs)
        elif(self.zsamp != None):
            (self.phi,self.theta,self.zsamp,self.qsamp) = intLDA(
                docs,self.alpha,self.root,
                self.leafmap,numsamp,randseed,
                init=self.zsamp,
                **kwargs)
        elif(self.qsamp != None):
            (self.phi,self.theta,self.zsamp,self.qsamp) = intLDA(
                docs,self.alpha,self.root,
                self.leafmap,numsamp,randseed,
                qinit=self.qsamp,
                **kwargs)
        else:
            (self.phi,self.theta,self.zsamp,self.qsamp) = intLDA(
                docs,self.alpha,self.root,
                self.leafmap,numsamp,randseed,
                **kwargs)
        
    def compile(self):
        """ Compile constraints into Dirichlet Forest data structure """
        (self.mlcc,self.clcc,self.allowable) = CC.processPairwise(
            self.constr,self.W,self.vocab)
        (self.root,self.leafmap) = CC.buildTree(self.mlcc,self.clcc,
                                                self.allowable,self.W,
                                                self.beta,self.eta)
                
    #
    # For tricky descriptor-related reasons, these decorators
    # MUST be applied to functions in the current order (!)
    #

    class Listify(object):
        """
        Decorator to wrap constraint operation functions, wraps non-List
        arguments as 1-element Lists if necessary
        """        
        def __init__(self,fcn):
            self.fcn = fcn
            self.instance = None
        def __get__(self,instance,cls):
            """ Need this to 'save' instance reference (self) """
            self.instance = instance
            return self
        def __call__(self,*args):
            """ Prepend instance ref (self) to args, pass to inner calls """
            newargs = (self.instance,) + tuple([arg if type(arg)==list
                                                else [arg] for arg in args])
            return self.fcn(*newargs)
        
    class VocabConvert(object):
        """
        Decorator to wrap constraint operation functions, converts List of
        words to List of integers (using vocab) if neccesary        
        """
        def __init__(self,fcn):
            self.fcn = fcn
        def __call__(self,*args):
            """ Assumes 0th arg is instance reference (self) """
            newargs = (args[0],)  + tuple([[args[0].vocab.index(w)
                                            if type(w) != int else w
                                            for w in a] for a in args[1:]])
            return self.fcn(*newargs) 

    #
    # These functions build up the preferences for this Dirichlet Forest
    #

    def ML(self,w0,w1):
        """ Add a simple must-link between single words w0 and w1 """
        self.constr = combine(self.constr,self.merge(w0,w1))

    def CL(self,w0,w1):
        """ Add a simple cannot-link between single words w0 and w1 """
        self.constr = combine(self.constr,self.split(w0,w1))
    
    @Listify
    @VocabConvert
    def merge(self,w0,w1):
        """
        Merge wordsets w0 and w1
        (place Must-Links among and between)
        """
        newML0 = [(w0[wi],w0[wi+1]) for wi in range(len(w0)-1)]
        newML1 = [(w1[wi],w1[wi+1]) for wi in range(len(w1)-1)]
        newML2 = [(w0[0],w1[0])]

        self.constr = combine(self.constr,(newML0 + newML1 + newML2,[]))
    
    @Listify
    @VocabConvert
    def split(self,*wsets):
        """
        Split all wordsets in wsets
        (place Must-Links among, Cannot-Links between)
        """
        # Must-Link within wordsets
        newML = reduce(lambda x,y: x+y,
                       [[(ws[wi],ws[wi+1])
                         for wi in range(len(ws)-1)]
                        for ws in wsets])
        # Cannot-Link across all pairs of wordsets
        newCL = []
        for i in range(len(wsets)):
            for j in range(i+1,len(wsets)):
                newCL.append((wsets[i][0],wsets[j][0]))

        self.constr = combine(self.constr,(newML, newCL))
        
    @Listify
    @VocabConvert
    def stopword1(self,*args):
        """
        Args is a Tuple of Lists, final List are stopwords by convention
        -mustlink existing topics
        -mustlink stopwords
        -cannotlink stopwords vs all existing topics
        """
        topics = args[:-1]
        stop = args[-1]

        newML = []
        newCL = []
        for t in topics:
            (tml,tcl) = split(t,stop)
            newML = newML + tml
            newCL = newCL + tcl
        self.constr = combine(self.constr,(newML,newCL))
    
    @Listify
    @VocabConvert
    def stopword2(self,*args):
        """
        Args is a Tuple of Lists, final List are stopwords by convention
        -mustlink stopwords
        -cannotlink stopwords vs all existing topics
        """
        topics = args[:-1]
        stop = args[-1]

        newML = [(stop[wi],stop[wi+1]) for wi in range(len(stop)-1)]
        newCL = []

        alltopicwords = list(set(reduce(lambda x,y: x+y,topics)))

        for ws in stop:
            for wt in alltopicwords:
                newCL.append((ws,wt))

        self.constr = combine(self.constr,(newML,newCL))
