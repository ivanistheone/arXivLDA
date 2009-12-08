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

""" Contains PrefGraph class """

class PrefGraph:
    """
    Graph represented as adjacency list
    (self.adj: node index --> List of neighbors' indices)
    """

    def __init__(self,nodes,edges):
        """
        nodes = List of (hashable) nodes
        edges = List of tuples (order doesn't matter, this is undirected graph)
        """
        self.adj = dict([(n,set([(e[0] if n==e[1] else e[1])
                                 for e in edges if n in e]))
                         for n in nodes])

    def connected_components(self):
        """ Connected components as List of Lists """
        (remaining,visited) = (set(self.adj.keys()),set([]))
        cc = []
        while(remaining):
            nbd = set([remaining.pop()])
            while(nbd):
                cur = nbd.pop()
                visited.add(cur)
                nbd = (set(self.adj[cur]) | nbd) - visited            
            cc.append(list(visited))
            remaining = remaining - visited
            visited = set([])
        return cc
    
    def subgraph(self,sg_nodes):
        """ Subgraph """
        return PrefGraph(sg_nodes,
                         set(reduce(lambda x,y: x+y,
                                    [[(n,a) for a in self.adj[n]
                                      if a in sg_nodes]
                                     for n in sg_nodes])))

    def complement(self):
        """ Graph complement """
        return PrefGraph(self.adj.keys(),
                         set(reduce(lambda x,y: x+y,
                                    [[(n,a) for a in self.adj.keys()
                                      if a != n and a not in self.adj[n]]
                                     for n in self.adj.keys()])))

    def cliques(self):
        """
        Bron-Kerbosch maximal clique detection algorithm
        (implementation based on ACM Algorithm 457 note)
        """
        return self._extend([],self.adj.keys(),[])

    def _extend(self,compsub,candlist,notlist):
        """ Recursive clique extenstion fcn for branch-and-bound """
        # Check recursive base cases
        if((not candlist) and (not notlist)):
            # Max clique condition
            return [compsub]
        elif([n for n in notlist
              if self.connToAll(n,candlist)]):
            # Bound (stop) condition
            return []
        # Otherwise recursively extend
        cliques = []
        # Fix point for B&K w/ "clever" cand select
        #        
        fixpt = self.get_fixpt(candlist,notlist)
        while(candlist):
            # Clever candidate selection
            #
            if(fixpt in candlist):
                cand = fixpt
                candlist.remove(fixpt)                
            else:
                disconnected_candidates = [c for c in candlist
                                           if not self.connected(c,fixpt)]
                if(disconnected_candidates):
                    cand = disconnected_candidates[0]
                    candlist.remove(cand)
                else:
                    # If all remaining candidates are connected
                    # to the fixedpt in notlist, all recursive extensions
                    # will hit bound/terminate condition and return
                    return cliques
            # Naive candidate selection
            #
            #cand = candlist.pop()

            # Construct updated lists
            r_compsub = compsub + [cand]
            r_candlist = [n for n in candlist
                          if self.connected(n,cand)]
            r_notlist = [n for n in notlist
                         if self.connected(n,cand)]                            
            # Recursive extension call
            cliques.extend(self._extend(r_compsub,r_candlist,r_notlist))
            # Add extended candidate to notlist
            notlist.append(cand)            
        return cliques

    def get_fixpt(self,candlist,notlist):
        """
        For Bron & Kerbosch Version 2 extension

        Find candidate/notlist point with *fewest*
        disconnections in candidate set

        Picking pts disconnected to this fixed point
        will get us to bound/termination condition fastest        
        """
        cscores = [len([c for c in candlist
                       if not self.connected(fp,c)])
                  for fp in candlist]
        # Note: have to give notlist nodes an extra -1 
        # (nodes in candlist will have to be chosen before
        # starting to decrement their counters)
        nscores = [len([c for c in candlist
                        if not self.connected(fp,c)]) - 1
                   for fp in notlist]
        # Find all points w/ minimal score, arbitrarily
        # choose final point in this list
        fixes = [fp for (fp,s) in zip(candlist+notlist,cscores+nscores)
                 if s==min(cscores+nscores)]
        return fixes[-1]                                
        
    def connToAll(self,node,rest):
        """ Is node connected to all nodes in rest? """
        if(rest):            
            return all([self.connected(node,other)
                        for other in rest])
        else:
            return True
           
    def connected(self,a,b):
        """ Are nodes A and B connected (or the same)? """
        return b in self.adj[a] or b is a
