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

""" Contains Reporter class """

class Reporter:
    """ Implements various reporting functions """

    #
    # Basic LDA reporting
    #

    def printTopics(self,N=0):
        """
        Print Top N words by P(w|z) for each topic
        Also print out q-samples for each topic
        """        
        (T,W) = self.phi.shape
        (T,P) = self.qsamp.shape        
        N = N if N>0 else W # If no N arg, print all words foreach topic
        for ti in range(T):
            qstr = str([self.qsamp[ti,pi] for pi in range(P)])
            print '\nTopic %d (q=%s)' % (ti,qstr)
            top = range(W)
            top.sort(key=lambda wi:-self.phi[ti,wi])
            for wi in range(N):
                print '%s = %f' % (self.vocab[top[wi]],self.phi[ti,top[wi]])
        print ''

    def printParams(self):
        """ Print experimental parameters """
        print ''
        print 'Parameters'
        print '----------------------'
        print 'D = %d' % (self.D)
        print 'alpha = %f\nbeta = %f\neta = %f' % (self.alpha,self.beta,self.eta)
        print 'randseed = %d' % (self.randseed)
        print 'burnin = %d\ninterval = %d' % (self.burnin,self.interval)
        print 'numsamp = %d' % (self.numsamp)
        print ''


    #
    # Report details of Dirichlet Forest data structure, CL/ML graphs, etc
    #
    
    def reportTree(self):
        """
        Display the resulting Dirichlet Forest data structure which
        will be passed to the C++ inference code

        Schema:
        root = (edges,ichildren,maxind,leafstart)
        multinode = (None,ichildren,maxind,leafstart,variants)
        variant = (fakeroot,fakeleafmap)
        mustlink = (edges,[],[],leafstart)

        NOTE: fakeroot is a typical dtnode (like root)

        Permutation Mappings:
        leafmap[word_index] = leaf_index
        fakeleafmap[index_within_multinode] = fake_leaf_index (wrt *this* variant)

        That is, the procedure for querying a multinode is:
        1) Get leaf index from leafmap
        2) Find appropriate multinode
        3) Find appropriate leaf/internal child of multinode
        4) Call variant with the index of the leaf/internal (wrt *multinode*)
        5) Pass along to true internal if necessary

        """
        (root,leafmap,mlcc,W) = (self.root,self.leafmap,self.mlcc,self.W)
        # Get a leaf-mapper function for reporting
        # (will map leaf indices back to original word indices, or actual words)
        lm = getLeafMapper(leafmap)

        print '\nFINAL TREE STRUCTURE'
        print '----------------------\n'
        for (ichild,ind) in zip(root[1],range(len(root[1]))):
            if(ichild[0] == None):
                # This root child is a multinode
                print 'Root Edge %f' % (root[0][ind])
                print 'Root child %d - multinode' % (ind)

                # Print the must-link children
                for mlc in ichild[1]:
                    print '\tmust-link child'
                    (lstart,nch) = (mlc[3],len(mlc[0]))                
                    #print ('\t\tleafstart index = %d'
                    #       % (lstart))
                    #print ('\t\tnum children = %d'
                    #       % (nch))
                    print '\t\tML-edges = %s' % (str(mlc[0]))
                    print ('\t\twords = %s'
                           % (str(lm(range(lstart,lstart+nch)))))

                # Print the leaf children
                nlch = len(ichild[4][0][1]) - len(ichild[1])
                print '\tnum leaf children = %d' % (nlch)
                print ('\tleaf words = %s'
                       % str(lm(range(ichild[3],ichild[3]+nlch))))
                print ''

                # Enumerate agreeable sets for this multinode
                print '\tagreeable sets'
                for (fakeroot,fakeleafmap) in ichild[4]:
                    print '\tAgree Edge = %f' % (fakeroot[0][0])
                    # For this agreeable set, see which children
                    # lie under the 'likely' internal node
                    likely_maxind = fakeroot[2][0]
                    agreeable = [fakeleafmap.index(i)
                                 for i in range(likely_maxind+1)]                
                    aset = []
                    for ai in agreeable:
                        if(ai >= len(ichild[1])): 
                           # leaf child of multinode
                            aset.append(ichild[3]+ai-len(ichild[1]))
                        else:
                            # mustlink child of multinode
                            maxind = ichild[2][ai]
                            nch = len(ichild[1][ai][0])
                            aset.append(range((maxind+1-nch),maxind+1))
                    print '\tLeaf Edges = %s' % (str(fakeroot[0][1:]))
                    print '\t\tleaf indices = %s' % str(aset)
                    print '\t\tword indices = %s' % str(lm(aset))
                    print ''
            else:
                # This root child is a must-link node
                print 'Root Edge = %f' % (root[0][ind])
                print 'Root child %d - mustlink node' % (ind)
                (lstart,nch) = (ichild[3],len(ichild[0]))
                #print '\tleafstart = %d' % (lstart)
                print '\tML-edges = %s' % (str(ichild[0]))
                print '\tnum children = %d' % (nch)
                print ('\twords = %s'
                       % (str(lm(range(lstart,lstart+nch)))))


    def reportGraph(self):
        """
        Display some information about the graph implied
        by a given set of constraints
        """
        (mlcc,clcc,allowable,W) = (self.mlcc,self.clcc,self.allowable,self.W)
        
        print '\nCONSTRAINT GRAPH ANALYSIS'
        print '----------------------------\n'

        # must-link connected components
        print 'Must-Link Connected Components'
        print '----------' * 4
        for i in range(len(mlcc)):
            print 'MLCC %d = %s' % (W+i,str(self.vocMap(mlcc[i])))

        # get an expander function for expanding must-link components
        expander = self.getExpander(mlcc,W)

        # cannot-link connected components
        print '\nCannot-Link Connected Components'
        print '----------' * 4
        for (cc,allow) in zip(clcc,allowable):
            print 'CLCC = %s' % str(self.vocMap(expander(cc)))
            print 'Number of allowable sets = %d' % len(allow)
            for a in allow:
                print '\t%s' % str(self.vocMap(expander(a)))
            print ''


    def vocMap(self,val):
        """
        If we have vocab List, map a word index --> actual word
        """
        vocab = self.vocab
        if(vocab != None):
            if(type(val) == int):
                return vocab[val]
            elif(type(val) == list):
                return [self.vocMap(v) for v in val]
            else:
                raise RuntimeError('Bad vocMap() call, type = %s' %
                                   (str(type(val))))
        else:
            return val


    def getExpander(self):
        """
        Creates a function to expand indices into must-link components
        """
        (mlcc,W) = (self.mlcc,self.W)
        def expander(val):
            if(type(val) == list):
                return [expander(v) for v in val]
            elif(type(val) == int):
                if(val < W):
                    return val
                else:
                    return mlcc[val-W]
            else:
                raise RuntimeError('Bad expander() call, type = %s' %
                                   (str(type(val))))
        return expander

    def getLeafMapper(self):
        """
        Creates a function to do reverse leaf-mapping
        That is, map leaf indices -> word indices
        (or actual words if vocab available)
        """
        leafmap = self.leafmap
        def leafmapper(val):
            if(type(val) == list):
                return [leafmapper(v) for v in val]
            elif(type(val) == int):
                return self.vocMap(leafmap.index(val))
            else:
                raise RuntimeError('Bad leafmapper() call, type = %s' %
                                   (str(type(val))))
        return leafmapper        
