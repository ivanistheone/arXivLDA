import unittest

from numpy import *
from deltaLDA import deltaLDA

class TestDeltaLDA(unittest.TestCase):
    
    def setUp(self):
        """
        Set up base data/parameter values
        """
        self.alpha = .1 * ones((1,3))
        self.beta = ones((3,5))
        self.docs = [[1,1,2],
                     [1,1,1,1,2],
                     [3,3,3,4],
                     [3,3,4,4,3,3],
                     [0,0,0,0,0],
                     [0,0,0,0]]
        self.f = [0 for doc in self.docs]
        self.init = [[0 for w in doc] for doc in self.docs]
        self.numsamp = 50
        self.randseed = 821945

        # equality tolerance for testing
        self.tol = 1e-6

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

    def testStandard(self):
        """
        Test standard LDA with base data/params
        """
        (phi,theta,sample) = deltaLDA(self.docs,self.alpha,self.beta,
                                      self.numsamp,self.randseed)
        # theta should clust docs [0,1], [2,3], [4,5]
        maxtheta = argmax(theta,axis=1)
        self.assert_(maxtheta[0] == maxtheta[1])
        self.assert_(maxtheta[2] == maxtheta[3])
        self.assert_(maxtheta[4] == maxtheta[5])
        # theta valid prob matrix
        self.assert_(self.matProb(theta))

        # corresponding phi should emph [1,2], [3,4], [0]
        maxphi = argmax(phi,axis=1)
        self.assert_(maxphi[maxtheta[0]] == 1)
        self.assert_(maxphi[maxtheta[2]] == 3)
        self.assert_(maxphi[maxtheta[4]] == 0)
        # phi valid prob matrix
        self.assert_(self.matProb(phi))


    def testDelta(self):
        """
        Test DeltaLDA with base data/params + f-values
        """
        our_f = [0, 0, 0, 0, 1, 1]
        alpha = array([[.1, .1, 0],[.1, .1, .1]])
        
        (phi,theta,sample) = deltaLDA(self.docs,alpha,self.beta,
                                      self.numsamp,self.randseed,f=our_f)

        # theta should assign special topic to docs [4,5]
        maxtheta = argmax(theta,axis=1)
        self.assert_(maxtheta[4] == 2)
        self.assert_(maxtheta[5] == 2)
        # theta valid prob matrix
        self.assert_(self.matProb(theta))

        # theta rows should sum to 1
        self.assert_(reduce(lambda x,y: x and y,
                            [abs(val - float(1)) < self.tol
                             for val in theta.sum(axis=1)]))

        # phi for special topic should emph [0]
        maxphi = argmax(phi,axis=1)
        self.assert_(maxphi[2] == 0)
        # phi valid prob matrix
        self.assert_(self.matProb(phi))
        

    def testInit(self):
        """
        Test standard LDA with init from previous sample
        (this doesn't test how the init could affect behavior,
        just checks that using an init doesn't fail completely...)
        """
        # Give stupid init
        (phi,theta,sample) = deltaLDA(self.docs,self.alpha,self.beta,
                                      self.numsamp,self.randseed,init=self.init)

        # theta should clust docs [0,1], [2,3], [4,5]
        maxtheta = argmax(theta,axis=1)
        self.assert_(maxtheta[0] == maxtheta[1])
        self.assert_(maxtheta[2] == maxtheta[3])
        self.assert_(maxtheta[4] == maxtheta[5])

        # corresponding phi should emph [1,2], [3,4], [0]
        maxphi = argmax(phi,axis=1)
        self.assert_(maxphi[maxtheta[0]] == 1)
        self.assert_(maxphi[maxtheta[2]] == 3)
        self.assert_(maxphi[maxtheta[4]] == 0)

    #
    # These test that the program fails correctly on *bad input*
    #    

    """
    Test bad document data
    """
    
    def testNonListDoc(self):
        """  Non-list doc """
        self.docs[0] = None
        self.assertRaises(RuntimeError,deltaLDA,self.docs,self.alpha,
                          self.beta,
                          self.numsamp,self.randseed)
    def testNegWord(self):
        """  Bad word (negative) """
        self.docs[0][-1] = -1
        self.assertRaises(RuntimeError,deltaLDA,self.docs,self.alpha,
                          self.beta,       
                          self.numsamp,self.randseed)
    def testBigWord(self):
        """  Bad word (too big)                 """
        self.docs[0][-1] = 5
        self.assertRaises(RuntimeError,deltaLDA,self.docs,self.alpha,
                          self.beta,       
                          self.numsamp,self.randseed)
    def testNonNumWord(self):
        """  Bad word (non-numeric) """
        self.docs[0][-1] = ''
        self.assertRaises(RuntimeError,deltaLDA,self.docs,self.alpha,
                          self.beta,       
                          self.numsamp,self.randseed)

    """
    Test bad alpha/beta values
    """

    def testNegAlpha(self):
        """  Negative alpha """
        self.alpha[0,1] = -1
        self.assertRaises(RuntimeError,deltaLDA,self.docs,self.alpha,self.beta,
                          self.numsamp,self.randseed)
    def testNegBeta(self):
        """  Negative beta """
        self.beta[1,2] = -1
        self.assertRaises(RuntimeError,deltaLDA,self.docs,self.alpha,self.beta,
                          self.numsamp,self.randseed)
    def testAlphaBetaDim(self):
        """  Alpha/Beta dim mismatch """
        self.alpha = .1 * ones((1,4))
        self.beta = ones((3,5))
        self.assertRaises(RuntimeError,deltaLDA,self.docs,self.alpha,self.beta,
                          self.numsamp,self.randseed)

    """
    Test bad init samples
    """

    def testTooFewInit(self):
        """  Too few docs """
        self.init = self.init[:-1]
        self.assertRaises(RuntimeError,deltaLDA,self.docs,self.alpha,
                          self.beta,
                          self.numsamp,self.randseed,init=self.init)
    def testTooManyInit(self):
        """  Too many docs """
        self.init = self.init + [[]]
        self.assertRaises(RuntimeError,deltaLDA,self.docs,self.alpha,
                          self.beta,
                          self.numsamp,self.randseed,init=self.init)
    def testLenMisInit(self):
        """  Doc length mismatch """
        self.init[2].append(0)
        self.assertRaises(RuntimeError,deltaLDA,self.docs,self.alpha,
                          self.beta,
                          self.numsamp,self.randseed,init=self.init)
    def testNonListInit(self):
        """  Non-list doc sample """
        self.init[0] = None
        self.assertRaises(RuntimeError,deltaLDA,self.docs,self.alpha,
                          self.beta,
                          self.numsamp,self.randseed,init=self.init)
    def testNegInit(self):
        """  Out-of-range topic value (negative) """
        self.init[0][0] = -2
        self.assertRaises(RuntimeError,deltaLDA,self.docs,self.alpha,
                          self.beta,
                          self.numsamp,self.randseed,init=self.init)
    def testBigInit(self):
        """  Out-of-range topic value (too big) """
        self.init[0][0] = 3
        self.assertRaises(RuntimeError,deltaLDA,self.docs,self.alpha,
                          self.beta,
                          self.numsamp,self.randseed,init=self.init)
    def testNonNumInit(self):
        """  Out-of-range topic value (non-numeric) """
        self.init[0][0] = ''
        self.assertRaises(RuntimeError,deltaLDA,self.docs,self.alpha,
                          self.beta,
                          self.numsamp,self.randseed,init=self.init)

    
"""  Run the unit tests! """
if __name__ == '__main__':
    unittest.main()
