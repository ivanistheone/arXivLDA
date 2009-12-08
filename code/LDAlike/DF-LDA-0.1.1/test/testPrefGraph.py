import unittest
import pdb
import operator
import random
import time

import PrefGraph as PG
#import networkx as NX

def exists(cond, seq):
    """ For existential queries """
    for val in seq:
        if(cond(val)):
            return True
    return False

def forall(cond, seq):
    """ For universal queries """
    for val in seq:
        if(not cond(val)):
            return False
    return True

def listeq(seq1,seq2):
    """ Rough list equality operator """
    return (len(seq1) == len(seq2)
            and forall(lambda x: x in seq1,seq2))

def listcheck(results,targets):
    # Forall lists in targets,
    # exists a 'list-equal' list in results
    return forall(
        lambda y: exists(
        lambda x: listeq(x,y),
        results),
        targets)

# Size of star graph for complementary cliques test
STAR_SIZE = 100

class TestPrefGraph(unittest.TestCase):
    
    def setUp(self):
        """
        Set up base data/parameter values
        """
        nodes = range(10)
        edges = [(0,1),
                 (1,2),
                 (3,4),
                 (4,5),
                 (5,6),
                 (8,9)]
        self.pg = PG.PrefGraph(nodes,edges)

    def testConnectedComponents(self):
        """ Test connected_components """
        cc = self.pg.connected_components()
        target = [[0,1,2],[3,4,5,6],[7],[8,9]]
        
        # Forall connected components in target,
        # there exists a 'list-equal' component in cc
        self.assert_(listcheck(cc,target))

    def testSubgraphComplement(self):
        """ Test complement and subgraph """
        cc = self.pg.connected_components()
        for c in cc:
            csg = (self.pg.subgraph(c)).complement()
            if(0 in c):
                self.assert_(len(csg.adj.keys()) == 3 and
                             csg.connected(0,2) and
                             not csg.connected(0,1) and
                             not csg.connected(1,2))
            elif(4 in c):
                self.assert_(len(csg.adj.keys()) == 4 and
                             csg.connected(3,5) and
                             csg.connected(4,6) and
                             csg.connected(3,6) and
                             not csg.connected(3,4) and
                             not csg.connected(4,5) and
                             not csg.connected(5,6))
            elif(7 in c):
                self.assert_(len(csg.adj.keys()) == 1 and
                             len(csg.adj[7]) == 0)
            elif(8 in c):
                self.assert_(len(csg.adj.keys()) == 2 and                                
                             not csg.connected(8,9))
            else:
                # Should not reach here
                self.assert_(1 == 0)

    def testCliques(self):
        """ Test Bron-Kerbosh max clique algorithm """
        cc = self.pg.connected_components()
        for c in cc:
            if(0 in c):
                targets = [[0,2],[1]]
            elif(4 in c):
                targets = [[3,5],[3,6],[4,6]]
            elif(7 in c):
                targets = [[7]]
            elif(8 in c):
                targets = [[8],[9]]
            else:
                # Should not reach here
                self.assert_(1 == 0)
            # Find max-cliques of complement subgraph
            sgc = self.pg.subgraph(c).complement()
            cliques = sgc.cliques()
            # Forall maxcliques in target,
            # exists a 'list-equal' clique in cliques        
            self.assert_(listcheck(cliques,targets))

    def testLollipopClique(self):
        """ Test cliques on lollipop graph """
        nodes = range(5) + range(5,9)
        edges = [(0,1),(1,2),(3,4),
                 (4,5),(4,6),(4,7),(4,8),
                 (5,6),(5,7),(5,8),
                 (6,7),(6,8),
                 (7,8)]
        pg = PG.PrefGraph(nodes,edges)
        cliques = pg.cliques()
        targets = [[0,1],[1,2],[3,4],
                   [4,5,6,7,8]]
        self.assert_(listcheck(cliques,targets))

    def testStarClique(self):
        """ Test cliques on complement of star graph """
        start = time.clock()
        nodes = range(STAR_SIZE)
        edges = [(0,n) for n in range(1,STAR_SIZE)]
        cpg = (PG.PrefGraph(nodes,edges)).complement()
        cliques = cpg.cliques()
        targets = [[0],range(1,STAR_SIZE)]
        # Forall maxcliques in target,
        # exists a 'list-equal' clique in cliques        
        self.assert_(listcheck(cliques,targets))
        print '%f sec' % (time.clock() - start)

    def testConnectedClique(self):
        """ Test cliques on complete graph """
        csize = 10
        nodes = range(csize)
        edges = reduce(lambda x,y: x+y,
                       [[(i,j) for j in range(i+1,csize)]
                        for i in range(csize-1)])
        pg = PG.PrefGraph(nodes,edges)
        cliques = pg.cliques()
        targets = [range(csize)]
        # Forall maxcliques in target,
        # exists a 'list-equal' clique in cliques
        self.assert_(listcheck(cliques,targets))

    def testNoEdgeClique(self):
        """ Test cliques on edge-free graph """
        csize = 10
        nodes = range(csize)
        edges = []
        pg = PG.PrefGraph(nodes,edges)
        cliques = pg.cliques()
        targets = [[i] for i in range(csize)]
        # Forall maxcliques in target,
        # exists a 'list-equal' clique in cliques
        self.assert_(listcheck(cliques,targets))


#     def testStarCliqueNX(self):
#         """ Test for cliques on complement of star graph (NX) """
#         start = time.clock()
#         g = NX.Graph()
#         g.add_nodes_from(range(STAR_SIZE))
#         g.add_edges_from([(0,n) for n in range(1,STAR_SIZE)])
#         cliques = NX.find_cliques(NX.complement(g))
#         targets = [[0],range(1,STAR_SIZE)]
#         self.assert_(listcheck(cliques,targets)
#         print '%f sec' % (time.clock() - start)

    
# Run the unit tests!
if __name__ == '__main__':
    unittest.main()
