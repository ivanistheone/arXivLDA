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

"""
Contains DTNode, MLNode, MultiNode classes
(used by ConstraintCompiler to build DirichletForest data structure)
"""

class DTNode(object):
    """ Represent a Dirichlet Tree node """

    def __init__(self,edges,ichildren,maxind,leafstart):
        """
        How to do this exactly...
        """
        self.edges = edges
        self.ichildren = ichildren
        self.maxind = maxind
        self.leafstart = leafstart

    def numLeaves(self):
        """ Get the number of leaves below this node """
        n = sum([ic.numLeaves() for ic in self.ichildren])                
        return n + len(self.edges) - len(self.ichildren)
               
    def tupleConvert(self):
        """
        Given a Dirichlet Tree in node=List form,
        recursively convert to node=Tuple form
        """
        tchildren = [ic.tupleConvert() for ic in self.ichildren]
        return (self.edges,tchildren,self.maxind,self.leafstart)

    
class MLNode(DTNode):
    """ Represent a MustLink-node """

    def __init__(self,edges,ichildren,maxind,leafstart,words):
        self.edges = edges
        self.ichildren = ichildren
        self.maxind = maxind
        self.leafstart = leafstart
        self.words = words


class MultiNode(DTNode):
    """
    Represent a MultiNode
    (can take on different subtree structures)
    """

    def __init__(self,edges,ichildren,maxind,leafstart,words,variants):    
        self.edges = edges
        self.ichildren = ichildren
        self.maxind = maxind
        self.leafstart = leafstart
        self.words = words
        self.variants = variants
    
    def tupleConvert(self):
        """ For MultiNode, will need to also convert variants """
        tchildren = [ic.tupleConvert() for ic in self.ichildren]
        tvars = [(fakeroot.tupleConvert(),fake_leafmap)                 
                 for (fakeroot,fake_leafmap) in self.variants]
        return (None,tchildren,self.maxind,self.leafstart,tvars)

    def numLeaves(self):
        """ Can't take len(edges) b/c edges==None by convention """
        n = sum([ic.numLeaves() for ic in self.ichildren])
        return n + len(self.words)
