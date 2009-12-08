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
ConstraintCompiler module

Builds pairwise ML/CL constraints into DirichletForest
data structure (for passing along to C++ inference code)
"""
import operator as OP

from numpy import *

import PrefGraph as PG
from DirichletTreeNodes import *

def buildTree(mlcc,clcc,allowable,W,beta,eta):
    """
    Given:
    mlcc = must-link clusters of words (List of Lists)
    clcc = cannot-link clusters of must-link clusters and words (List of Tuples)
    allowable = within each clcc, the allowable combinations of mlcc/words
    W = total number of words (connect extra words directly to root)

    Return Dirichlet Forest data structure
    (Dirichlet Tree plus special multinodes for structure variants)

    PROCEDURE
    1) construct tree structure
    2) populate indices
    3) tuple-ify tree for pass to intLDA
    """

    # Build M-nodes for each mlcc (will have only leaf children)
    ml_nodes = [MLNode(eta*beta*ones((len(Mi))),[],[],None,Mi) for Mi in mlcc]

    # Build multinodes for each clcc
    multinodes = []
    for (cc,allow) in zip(clcc,allowable):

        # these are the true output connections of the multinode
        # ensure that ML-nodes always appear 1st
        icids = [ci for ci in cc if ci >= W]
        lcids = [ci for ci in cc if ci < W]
        fake_words = icids + lcids
        
        # construct a variation for each allowable set
        variations = []
        for aset in allow:
            # split btwn good/bad (allowable/not)
            good = aset
            bad = [fl for fl in fake_words if fl not in good]

            # scale the edges coming out of the likely internal node
            aedges = []
            for a in good:
                if(a >= W):
                    # M-node? --> beta * num leaves
                    aedges.append(beta * ml_nodes[a-W].numLeaves())
                else:
                    # Leaf? --> beta
                    aedges.append(beta)

            # scale the edges coming out of the fakeroot
            fedges = [eta * sum(aedges)]
            for b in bad:
                if(b >= W):
                    # M-node? --> beta * num leaves 
                    fedges.append(beta * ml_nodes[b-W].numLeaves())
                else:
                    # Leaf? --> beta
                    fedges.append(beta)

            # variant subtree
            likely_internal = DTNode(array(aedges),[],[],0)
            fakeroot = DTNode(array(fedges),[likely_internal],
                              [len(good)-1],
                              len(good))

            # fake leafmap permutation for this variant
            fake_wordmap = good + bad
            fake_leafmap = [fake_wordmap.index(wi)
                            for wi in fake_words]

            # save it
            variations.append((fakeroot,fake_leafmap))

        # build the multinode
        ichildren = [ml_nodes[ci-W] for ci in cc if ci >= W]
        lchildren = [ci for ci in cc if ci < W]
        multinodes.append(MultiNode(None,ichildren,[],0,lchildren,variations))

    # Create empty root node
    root = DTNode([],[],[],0)
                    
    # Start connecting things and populating indices
    #
    cur_ind = 0 # current leaf index
    wordmap = [] # maps leaf index --> word index

    # MultiNodes connected to root
    #
    for multinode in multinodes:
        # Map ML-node children
        for ml_child in multinode.ichildren:
            # Get word indices under this M-node
            wordmap = wordmap + ml_child.words
            # Save leafstart index for this M-node
            ml_child.leafstart = cur_ind
            # Record maxind in multinode
            cur_ind += len(ml_child.words)
            multinode.maxind.append(cur_ind-1)
        
        # Does this multinode connect directly to any leaves?
        if(len(multinode.words) > 0):
            # Record leaf start index
            multinode.leafstart = cur_ind
            cur_ind += len(multinode.words)
            # Record word indices
            wordmap = wordmap + multinode.words
            
        # Put this C-node and maxindex in root
        root.edges.append(beta*multinode.numLeaves())
        root.ichildren.append(multinode)
        root.maxind.append(cur_ind-1)

    # ML-nodes connected directly to root
    #
    for mln in ml_nodes:
        # Leaf start index == None --> has not been assigned yet
        if(mln.leafstart == None):
            # Set leaf start index for ML-node
            mln.leafstart = cur_ind
            cur_ind += mln.numLeaves()
            # Put ML-node and its maxind in root
            root.edges.append(beta*mln.numLeaves())
            root.ichildren.append(mln)
            root.maxind.append(cur_ind-1)
            # Record word indices under this M-node
            wordmap = wordmap + mln.words
            
    # Finally, rest of words must be connected directly to root
    root.edges = array(root.edges + [beta for i in range(W - cur_ind)])
    root.leafstart = cur_ind
    wordmap = wordmap + [wi for wi in range(W) if wi not in wordmap]

    # Convert everything to tuples
    root = root.tupleConvert()
    
    # Recover leafmap by inverting wordmap permutation
    leafmap = [wordmap.index(wi) for wi in range(W)]

    return (root,leafmap)            


def processPairwise(constr,W,vocab):
    # Unpack pairwise to construct graphs
    (mustlinks,cannotlinks) = constr

    # Get ML connected components
    mlcc = getML(mustlinks)

    # Invert this mapping
    if(len(mlcc) > 0):
        invmlcc = dict(reduce(OP.concat,[[(wi,mli+W) for wi in mlcc[mli]]
                                         for mli in range(len(mlcc))],[]))
    else:
        invmlcc = {}

    # Translate cannot-links between words to cannot-links
    # between ML components (when applicable)
    newcannotlinks = map(lambda x: tuple(map(lambda y: invmlcc.get(y,y),x)),
                         cannotlinks)

    # Check for conflicting constraints
    #
    # if 2 words are connected (transitively or directly) by must-links,
    # they will be in the same ML connected component
    #
    # if 2 words also have a pairwise cannot-link btwn them, then after
    # the mapping we will have a a pairwise cannot-link between an ML
    # connected component and *itself*
    #
    conflicts = [cl[0] for cl in newcannotlinks if cl[0]==cl[1]]
    if(len(conflicts) > 0):
        print ''
        # Identify the conflicting constraints
        print '!!! CONFLICTING CONSTRAINTS !!!'
        for conflict in conflicts:
            print 'Following pairs must *and* cannot-link:'
            mli = conflict - W
            mlw = mlcc[mli]
            for cl in cannotlinks:
                if(cl[0] in mlw and cl[1] in mlw):
                    # Print words if available, else word indices
                    if(vocab != None):
                        print '\t%s - %s' % (vocab[cl[0]],vocab[cl[1]])
                    else:
                        print '\t%d - %d' % (cl[0],cl[1])
        print ''
        raise RuntimeError('Conflicting constraints, see err msg')

    # Get CL graph connected components and allowable sets
    (clcc,allowable) = getCL(newcannotlinks)

    return (mlcc,clcc,allowable)


def getML(mustlinks):
    """
    Get must-link connected components from pairwise mustlink constraints
    """
    # Check if mustlinks is empty    
    if(len(mustlinks) == 0):
        return []
    
    # Get indices of all involved nodes
    nodes = reduce(lambda x,y: x.union(y),[set(ml) for ml in mustlinks],set([]))

    # Get must-link connected components
    return (PG.PrefGraph(nodes,mustlinks)).connected_components()



def getCL(cannotlinks):
    """
    Get cannot-link graph connected components, along with maximal independent
    sets for each (the 'allowable' word sets)
    """
    # Check if cannotlinks is empty
    if(len(cannotlinks) == 0):
        return ([],[])
    
    # First get indices of all involved nodes
    nodes = reduce(lambda x,y: x.union(y),
                   [set(cl) for cl in cannotlinks],set())
    
    # Get cannot-link connected components
    cl = PG.PrefGraph(nodes,cannotlinks)
    clcc = cl.connected_components()

    # For each connected component, enumerate all maximal independent sets
    # (equivalent to all maximal cliques in the complement graph)
    allowable = [((cl.subgraph(cc)).complement()).cliques()
                 for cc in clcc]
    
    return (clcc,allowable)
