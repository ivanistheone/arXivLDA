from numpy import *
from deltaLDA import deltaLDA

# alpha and beta *must* be NumPy Array objects
#
# Their dimensionalities implicitly specify:
# -the number of topics T
# -the vocabulary size W
# -number of f-label values F (just 1 for 'standard' LDA)
#
# alpha = F x T
# beta = T x W
#
alpha = .1 * ones((1,3))
beta = ones((3,5))

# docs must be a Python List of Python lists
# Each individual List contains the indices of word tokens
# (That is, docs[0] = "w1 w1 w2", docs[1] = "w1 w1 w1 w1 w2", etc) 
#
docs = [[1,1,2],
        [1,1,1,1,2],
        [3,3,3,4],
        [3,3,4,4,3,3],
        [0,0,0,0,0],
        [0,0,0,0]]

# numsamp specifies how many samples to take from the Gibbs sampler
numsamp = 50

# randseed is used to initialize the Gibbs sampler random number generator
randseed = 194582

# This command will run the standard LDA model
#
(phi,theta,sample) = deltaLDA(docs,alpha,beta,numsamp,randseed)

# This command will initialize the Gibbs sampler from a user-supplied sample
#
#(phi,theta,sample) = deltaLDA(docs,alpha,beta,numsamp,randseed,init=sample)

# This command will run standard LDA, but show Gibbs sampler output
# ("Gibbs sample X of Y")
#
#(phi,theta,sample) = deltaLDA(docs,alpha,beta,numsamp,randseed,verbose=1)

# These commands will run deltaLDA
# (use different alpha vectors for different docs, depending on value of f)
#
#delta_f = [0, 0, 0, 0, 1, 1]
#delta_alpha = array([[.1, .1, 0],[.1, .1, .1]])
#(phi,theta,sample) = deltaLDA(docs,delta_alpha,beta,numsamp,randseed,f=delta_f)




# theta is the matrix of document-topic probabilities
# (estimated from final sample)
# 
# theta = D x T
# theta[di,zj] = P(z=zj | d=di)
#
print ''
print 'Theta - P(z|d)'
print str(theta)
print ''

# phi is the matrix of topic-word probabilities 
# (estimated from final sample)
# 
# phi = T x W
# phi[zj,wi] = P(w=wi | z=zj)
#
print ''
print 'Phi - P(w|z)'
print str(phi)
print ''

# Since the simple documents we created and fed into deltaLDA exhibit such
# clearly divided word usage patterns, the resulting phi and theta
# should reflect these patterns nicely
