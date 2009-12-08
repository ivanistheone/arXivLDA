""" Simple example demonstrating usage of Dirichlet Forest """

from numpy import *

import DirichletForest as DF

# Model parameters (see paper for meanings)
alpha = 1
beta = .01
eta = 100

# Number of topics, size of vocab
(T,W) = (2,3)

# Vocabulary
vocab = ['apple','banana','motorcycle']

# Documents, can be:
# -List of Lists of integers
# docs = [[0,2],
#         [1,2],
#         [0,2],
#         [1,2]]
# -read in from SVMLight-style sparse text file
docs = DF.readDocs('example.docs')

# Build DF
df = DF.DirichletForest(alpha,beta,eta,T,W,vocab)

# Apply constraints

# Must-Link between apple and banana
df.merge('apple','banana')
# Note that any of these would be OK as well:
# df.merge('apple',[1])
# df.merge('apple',['banana'])
# df.merge(0,1)
# df.merge([0],[1])
# etc...

# Cannot-Link between apple and motorcycle
df.split('apple','motorcycle')

# Do inference on docs (take numsamp samples before estimating phi/theta)
(numsamp, randseed) = (50, 821945)
df.inference(docs,numsamp,randseed)

# Output results
print '\nTop 3 words from learned topics'
df.printTopics(N=3)

print '\nRaw inference output\n'
print 'phi = T-by-W NumPy matrix of topic-word probabilities' 
print str(df.phi) + '\n'

print 'theta = D-by-T NumPy matrix of doc-topic probabilities'
print str(df.theta) + '\n'

print 'zsamp = final z-sample'
print str(df.zsamp) + '\n'

print 'qsamp = final q-sample'
print str(df.qsamp) + '\n'
