import unittest
import pdb
import operator
import random
import time

from numpy import *

import DirichletForest as DF

# If deltaLDA extension module present,
# we will sanity check against it
hasDelta = True
try:
    from deltaLDA import deltaLDA
except:
    hasDelta = False

class TestDF(unittest.TestCase):
    
    def setUp(self):
        """ Set up base data/parameter values """
        self.T = 3
        (self.alpha,self.beta,self.eta) = (.1,.1,10000)
        self.ldaalpha = self.alpha * ones((1,self.T))
        self.docs = [[1,1,2],
                     [1,1,1,1,2],
                     [3,3,3,4],
                     [3,3,4,4,3,3],
                     [0,0,0,0,0],
                     [0,0,0,0]]
        self.init = [[0 for w in doc] for doc in self.docs]
        self.f = [0 for doc in self.docs]
        self.W = 7
        self.numsamp = 50
        self.randseed = 821945

        # Build DF object
        self.df = DF.DirichletForest(self.alpha,self.beta,self.eta,
                                     self.T,self.W)
               
        # Default constraint set will create a 'full' tree
        # (ie, exhibits all variety of substructures)
        self.df.split([1,2],[3])
        self.df.split([0],[3])
        self.df.merge([4],[5])        

        # equality tolerance for testing
        self.tol = 1e-6


    def matAgree(self,mat1,mat2):
        """
        Given 2 NumPy matrices,
        check that all values agree within self.tol         
        """    
        return (abs(mat1-mat2)).sum(axis=None) < self.tol


    def matProb(self,mat):
        """
        Given a NumPy matrix,
        check that all values >= 0, sum to 1 (valid prob dist)
        """
        sumto1 = all([abs(val - float(1)) < self.tol
                      for val in mat.sum(axis=1)])
        geq0 = all([val >= 0 for val in
                    mat.reshape(mat.size)])
        return sumto1 and geq0
        

    #
    # These are to test *correct* operation
    #

    def testCC(self):
        """ Test basic ConstraintCompiler operation """
        W = 8
        df = DF.DirichletForest(self.alpha,self.beta,self.eta,
                                self.T,W)
        df.split([0,1,2,3],[4,5])
        df.split([0,1,2,3],[6])
        df.split([4,5],[7])
        df.compile()
     
        # ML clusters should be: 0123, 45
        self.assert_(len(df.mlcc) == 2)
        self.assert_(set(df.mlcc[0]) == set([0,1,2,3]))
        self.assert_(set(df.mlcc[1]) == set([4,5]))

        # CL contention set should be: 8,9,6,7
        self.assert_(len(df.clcc) == 1)
        self.assert_(set(df.clcc[0]) == set([8,9,6,7]))

        # Agreeable sets should be: [0123]7,[45]6,67
        self.assert_(len(df.allowable) == 1)
        allowsets = [set(a) for a in df.allowable[0]]
        correctsets = [set([8,7]),set([9,6]),set([6,7])]
        matching = [a in correctsets for a in allowsets]
        self.assert_(reduce(operator.__or__,matching))


    def testSanity1(self):
        """ Test no constraints vs deltaLDA implementation """
        # Don't even try unless deltaLDA module present
        if(not hasDelta):
            return
        
        # Randomly generated docs
        docs = [[random.randint(self.W) for i in range(1000)]
                for j in range(100)]

        # Set beta for standard LDA
        ldabeta = self.df.beta * ones((self.T,self.W))
        
        # Run standard LDA
        (sphi,stheta,ssample) = deltaLDA(docs,self.ldaalpha,
                                         ldabeta,self.numsamp,self.randseed)

        # Run Interactive LDA with empty constraint set
	df = DF.DirichletForest(self.alpha,self.beta,self.eta,
                                self.T,self.W)
        df.inference(docs,self.numsamp,self.randseed)

        # Assert matrix agreement, valid prob dists
        self.assert_(self.matAgree(df.phi,sphi))
        self.assert_(self.matProb(df.phi))
        self.assert_(self.matProb(sphi))

        self.assert_(self.matAgree(df.theta,stheta))
        self.assert_(self.matProb(df.theta))
        self.assert_(self.matProb(stheta))
        
        
                                
    def testSanity2(self):
        """ Test eta=1 against deltaLDA implementation """
        # Don't even try unless deltaLDA module present
        if(not hasDelta):
            return

        # Randomly generated docs
        docs = [[random.randint(self.W) for i in range(1000)]
                for j in range(100)]
        
        # Set beta for standard LDA
        ldabeta = self.beta * ones((self.T,self.W))
        
        # Run standard LDA
        start = time.time()
        (sphi,stheta,ssample) = deltaLDA(docs,self.ldaalpha,
                                         ldabeta,self.numsamp,self.randseed)
        ldatime = time.time() - start

        # Run Interactive LDA with the baseline (full-tree) constraints,
        # but temporarily set constraint strength to 1 to build tree
        eta = 1
        df = DF.DirichletForest(self.alpha,self.beta,eta,
                                self.T,self.W)
        df.split([1,2],[3])
        df.split([0],[3])
        df.merge([4],[5])        
        df.inference(docs,self.numsamp,self.randseed)

        # Assert matrix agreement, valid prob dists
        self.assert_(self.matAgree(df.phi,sphi))
        self.assert_(self.matProb(df.phi))
        self.assert_(self.matProb(sphi))

        self.assert_(self.matAgree(df.theta,stheta))
        self.assert_(self.matProb(df.theta))
        self.assert_(self.matProb(stheta))

     

    def testSanity3(self):
        """ Test beta*eta=X vs deltaLDA with beta=X """
        # Don't even try unless deltaLDA module present
        if(not hasDelta):
            return

        # 'magic' X parameter
        X = 50
        
        # Randomly generated docs
        W = 2
        docs = [[random.randint(W) for i in range(1000)]
                for j in range(100)]
        
        # Set beta for standard LDA
        ldabeta = X * ones((self.T,W))
        
        # Run standard LDA
        (sphi,stheta,ssample) = deltaLDA(docs,self.ldaalpha,
                                         ldabeta,self.numsamp,self.randseed)
        
        # Run Interactive LDA with the baseline (full-tree) constraints,
        # but temporarily set constraint strength to 1 to build tree
        eta = X
        beta = 1
        df = DF.DirichletForest(self.alpha,beta,eta,
                                self.T,W)        
        df.merge([0],[1])
        df.inference(docs,self.numsamp,self.randseed)

        # Assert matrix agreement, valid prob dists
        self.assert_(self.matAgree(df.phi,sphi))
        self.assert_(self.matProb(df.phi))
        self.assert_(self.matProb(sphi))

        self.assert_(self.matAgree(df.theta,stheta))
        self.assert_(self.matProb(df.theta))
        self.assert_(self.matProb(stheta))


    def testBalance(self):
        """ Test the 'balance' properties of constructed DF """
        # Use baseline (full-tree) constraints, but temporarily set
        # constraint strength eta=1 to build tree
        eta = 1
        df = DF.DirichletForest(self.alpha,self.beta,eta,
                                self.T,self.W)
        df.split([1,2],[3])
        df.split([0],[3])
        df.merge([4],[5])        
        df.compile()

        # Test balance condition for all internal edges
        for (edge,ich) in zip(df.root[0],df.root[1]):
            if(ich[0] == None):
                # Is child a multinode?
                for (fakeroot,fake_leafmap) in ich[4]:
                    self.assert_(edge == sum(fakeroot[0]))
                    # Check likely internal node
                    likeint = fakeroot[1][0]
                    self.assert_(fakeroot[0][0] == sum(likeint[0]))
                    # Check all ML-node children
                    for (iich,i) in zip(ich[1],range(len(ich[1]))):
                        fi = fake_leafmap[i]
                        # Under likely internal node, or not?
                        if(fi >= len(likeint[0])):
                            tmpi = fi - len(likeint[0]) + 1
                            self.assert_(fakeroot[0][tmpi] == sum(iich[0]))
                        else:
                            self.assert_(likeint[0][fi] == sum(iich[0]))
            else:
                # Otherwise it's an ML-node
                self.assert_(edge == sum(ich[0]))


    def testConstrained(self):
        """ Test *split* constraint """
        # True topics = [0,1,2,3],[4,5]        
        docs = [[0,1,2,3,0,1,2,3,0,1,2,3,4,5],
                [0,1,2,3,0,1,2,3,0,1,2,3,4,5],
                [0,1,2,3,0,1,2,3,0,1,2,3,4,5],
                [4,5,4,5,4,5,4,5,4,5,4,5,0,1,2,3],
                [4,5,4,5,4,5,4,5,4,5,4,5,0,1,2,3],
                [0,1,2,3,0,1,2,3,0,1,2,3,4,5],
                [0,1,2,3,0,1,2,3,0,1,2,3,4,5],
                [0,1,2,3,0,1,2,3,0,1,2,3,4,5],
                [4,5,4,5,4,5,4,5,4,5,4,5,0,1,2,3],
                [4,5,4,5,4,5,4,5,4,5,4,5,0,1,2,3],
                [0,1,2,3,0,1,2,3,0,1,2,3,4,5],
                [0,1,2,3,0,1,2,3,0,1,2,3,4,5],
                [0,1,2,3,0,1,2,3,0,1,2,3,4,5],
                [4,5,4,5,4,5,4,5,4,5,4,5,0,1,2,3],
                [4,5,4,5,4,5,4,5,4,5,4,5,0,1,2,3]]                

        # Now we want to 'force' recovery of
        # 2 distinct topics [0,1] and [2,3]

        # Build constraints
        (T,W) = (2,6)
        df = DF.DirichletForest(self.alpha,self.beta,self.eta,T,W)
        df.split([0,1],[2,3])
        randseed = 1
        numsamp = 200
        df.inference(docs,numsamp,randseed)

        # Verify that we find:
        # 1) a topics with high [0,1] and low [2,3]
        # 2) a topics with low [0,1] and high [2,3]
        (hit1,hit2) = (False,False)
        phimax = df.phi.max(axis=1)
        for t in range(df.phi.shape[0]):
            tmax = phimax[t]
            if(df.phi[t][0] >= .5*tmax and
               df.phi[t][1] >= .5*tmax and
               df.phi[t][2] < .5*tmax and
               df.phi[t][3] < .5*tmax):
                hit1 = True
            elif(df.phi[t][2] >= .5*tmax and
                 df.phi[t][3] >= .5*tmax and
                 df.phi[t][0] < .5*tmax and
                 df.phi[t][1] < .5*tmax):
                hit2 = True
        self.assert_(hit1 and hit2)

        # Check that phi/theta form valid probability distributions
        self.assert_(self.matProb(df.phi))
        self.assert_(self.matProb(df.theta))

        
    def testStandard(self):
        """ Test no constraints mode """
        # Don't even try unless deltaLDA module present
        if(not hasDelta):
            return

        # Temporarily shrink vocab
        W = 5

        # Set beta for standard LDA
        ldabeta = self.beta * ones((self.T,W))

        # Run standard LDA
        (sphi,stheta,ssample) = deltaLDA(self.docs,self.ldaalpha,ldabeta,
                                         self.numsamp,self.randseed)

        # Run Interactive LDA with empty constraint set
        df = DF.DirichletForest(self.alpha,self.beta,self.eta,
                                self.T,W)
        df.inference(self.docs,self.numsamp,self.randseed)

        #
        # First, validate correctness of recovered topics
        #
        
        # theta should clust docs [0,1], [2,3], [4,5]
        maxtheta = argmax(df.theta,axis=1)
        self.assert_(maxtheta[0] == maxtheta[1])
        self.assert_(maxtheta[2] == maxtheta[3])
        self.assert_(maxtheta[4] == maxtheta[5])

        # corresponding phi should emph [1,2], [3,4], [0]
        maxphi = argmax(df.phi,axis=1)
        self.assert_(maxphi[maxtheta[0]] == 1)
        self.assert_(maxphi[maxtheta[2]] == 3)
        self.assert_(maxphi[maxtheta[4]] == 0)

        # Assert matrix agreement, valid prob dists
        self.assert_(self.matAgree(df.phi,sphi))
        self.assert_(self.matProb(df.phi))
        self.assert_(self.matProb(sphi))

        self.assert_(self.matAgree(df.theta,stheta))
        self.assert_(self.matProb(df.theta))
        self.assert_(self.matProb(stheta))

    #
    # These test that the program fails correctly on *bad input*
    #


    #
    # Bad constraints
    #
    
    def testConstraintConflict(self):
        """ Test conflicting constraints """
        df = DF.DirichletForest(self.alpha,self.beta,self.eta,
                                self.T,self.W)
        df.split([0,1],[2,3])
        df.merge(1,2)
        self.assertRaises(RuntimeError,df.compile)

    #
    # Bad data
    #    

    def testNonListDoc(self):
        """ Test non-list doc """
        self.docs[0] = None
        self.assertRaises(RuntimeError,self.df.inference,self.docs,
                          self.numsamp,self.randseed)
    def testNegWord(self):
        """ Test bad word (negative) """
        self.docs[0][-1] = -1
        self.assertRaises(RuntimeError,self.df.inference,self.docs,
                          self.numsamp,self.randseed)
    def testBigWord(self):
        """ Test bad word (too big) """
        self.docs[0][-1] = self.W # should fail b/c of zero-based indexing
        self.assertRaises(RuntimeError,self.df.inference,self.docs,
                          self.numsamp,self.randseed)
    def testNonNumWord(self):
        """ Test bad word (non-numeric) """
        self.docs[0][-1] = ''
        self.assertRaises(RuntimeError,self.df.inference,self.docs,
                          self.numsamp,self.randseed)

    #
    # Bad parameters
    #
    
    def testBadAlpha(self):
        """ Test neg alpha """
        self.df.alpha[0,1] = -1;
        self.assertRaises(RuntimeError,self.df.inference,self.docs,
                          self.numsamp,self.randseed)


    #
    # Bad q-init samples
    #
    #     !!! q-init MUST be constructed w/ dtype=int32 !!!
    #
    
    def testQInitTooManyT(self):
        """ Test q-init w/ too many topics """
        self.df.qsamp = zeros((self.T+1,1),int32)
        self.assertRaises(RuntimeError,self.df.inference,self.docs,
                          self.numsamp,self.randseed)     
    def testQInitTooFewT(self):
        """ Test q-init w/ too few topics """
        self.df.qsamp = zeros((self.T-1,1),int32)
        self.assertRaises(RuntimeError,self.df.inference,self.docs,
                          self.numsamp,self.randseed)
    def testQInitTooManyQ(self):
        """ Test q-init w/ too many multinode values """
        self.df.qsamp = zeros((self.T,2),int32)
        self.assertRaises(RuntimeError,self.df.inference,self.docs,
                          self.numsamp,self.randseed)
    def testQInitTooFewQ(self):
        """ Test q-init w/ too few multinode values """
        self.df.qsamp = zeros((self.T,0),int32)
        self.assertRaises(RuntimeError,self.df.inference,self.docs,
                          self.numsamp,self.randseed)
    def testQInitBadVal(self):
        """ Test out of range q-init """
        # < 0
        self.df.qsamp = zeros((self.T,1),int32)
        self.df.qsamp[0,0] = -1
        self.assertRaises(RuntimeError,self.df.inference,self.docs,
                          self.numsamp,self.randseed)
        # > P
        self.df.qsamp = zeros((self.T,1),int32)
        self.df.qsamp[0,0] = 2
        self.assertRaises(RuntimeError,self.df.inference,self.docs,
                          self.numsamp,self.randseed)
        
    #
    # Bad z-init samples
    #

    def testTooFewInit(self):
        """ Test z-init w/ too few docs """
        self.init = self.init[:-1]
        self.df.zsamp = self.init
        self.assertRaises(RuntimeError,self.df.inference,self.docs,
                          self.numsamp,self.randseed)
    def testTooManyInit(self):
        """ Test z-init w/ too many docs """
        self.init = self.init + [[]]
        self.df.zsamp = self.init
        self.assertRaises(RuntimeError,self.df.inference,self.docs,
                          self.numsamp,self.randseed)
    def testLenMisInit(self):
        """ Test z-init w/ doc len mismatch """
        self.init[2].append(0)
        self.df.zsamp = self.init
        self.assertRaises(RuntimeError,self.df.inference,self.docs,
                          self.numsamp,self.randseed)
    def testNonListInit(self):
        """ Test z-init w/ non-List doc """
        self.init[0] = None
        self.df.zsamp = self.init
        self.assertRaises(RuntimeError,self.df.inference,self.docs,
                          self.numsamp,self.randseed)
    def testNegInit(self):
        """ Test z-init w/ neg value """
        self.init[0][0] = -2
        self.df.zsamp = self.init
        self.assertRaises(RuntimeError,self.df.inference,self.docs,
                          self.numsamp,self.randseed)
    def testBigInit(self):
        """ Test z-init w/ too large value """
        self.init[0][0] = 3
        self.df.zsamp = self.init
        self.assertRaises(RuntimeError,self.df.inference,self.docs,
                          self.numsamp,self.randseed)
    def testNonNumInit(self):
        """ Test z-init w/ non-numeric value """
        self.init[0][0] = ''
        self.df.zsamp = self.init
        self.assertRaises(RuntimeError,self.df.inference,self.docs,
                          self.numsamp,self.randseed)
    
# Run the unit tests!
if __name__ == '__main__':
    unittest.main()
