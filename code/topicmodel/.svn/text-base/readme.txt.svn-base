readme.txt for topicmodel
=========================

David Newman
Department of Computer Science
University of California, Irvine
newman@uci.edu

Topic modeling scripts and code.
www.ics.uci.edu/~newman/code

This code is an implementation of the collapsed Gibbs sampler for
Latent Dirichlet Allocation (aka topic model) described in :
Griffiths, T., & Steyvers, M. (2004).  Finding Scientific
Topics. Proceedings of the National Academy of Sciences, 101
(suppl. 1), 5228-5235.

-------------------------------------------
INSTRUCTIONS:
-------------------------------------------
To run topic model from end-to-end, run "run.sh"
./run.sh

-------------------------------------------
Preprocessing scripts
-------------------------------------------
./Makewordstream.pl < docs.txt       > wordstream.txt
./Makevocab.pl      < wordstream.txt > vocab.txt
./Makedocword.pl    < wordstream.txt > docword.txt

-------------------------------------------
Topic model
-------------------------------------------
./topicmodel

-------------------------------------------
Postprocessing scripts
-------------------------------------------
./topics > topics.txt
(or matlab < topics.m)
./topicsindocs > topicsindocs.txt
(or matlab < topicsindocs.m)


0. Variables
------------
D = number of documents in collection
W = number of unique words in vocabulary
T = number of topics
N = total number of words in collection
 
 
1. Files
--------
docs.txt         (D lines)
wordstream.txt   (> N lines)
stopwords.txt
vocab.txt        (W lines)
freqwords.txt    (W lines)
docword.txt      (D.W sparse)
wp.txt           (W.T sparse)
dp.txt           (D.T sparse)
z.txt            (N lines)
topics.txt       (T lines)
topicsindocs.txt (D lines)
 
Sparse matrix format: 3 header lines, followed by i j cnt triples. E.g.
nrows
ncols
nnz
i j cnt
i j cnt
i j cnt
...

 
2. Create docs.txt
------------------
docs.txt contains the filenames of each document in the collection
The first line is docID=1, etc.
Note: docs.txt has D lines
 
 
3. Create wordstream.txt
------------------------
./Makewordstream.pl < docs.txt > wordstream.txt
INPUT: docs.txt (stdin)
OUTPUT: wordstream.txt (stdout)
Note: check wordstream.txt for correct tokenization, modify Makewordstream.pl as needed
 
 
4. Create vocab.txt
-------------------
./Makevocab.pl < wordstream.txt > vocab.txt
INPUT: wordstream.txt (stdin), stopwords.txt
OUTPUT: vocab.txt (stdout), freqwords.txt
Note: modify stopwords.txt and rerun if necessary
Note: vocab.txt has W lines
Note: Makevocab.pl also creates freqwords.txt
 
 
5. Create docword.txt
---------------------
./Makedocword.pl < wordstream.txt > docword.txt
INPUT: wordstream.txt (stdin), vocab.txt
OUPUT: docword.txt (stdout)
Note: docword.txt is D.W sparse matrix
Note: check docword.txt for correctness


6. Run the topic model
----------------------
./topicmodel 10 200 (T=10 topics, iter=200 iterations)
INPUT: docword.txt
OUTPUT: wp.txt, dp.txt, z.txt
Note: wp.txt is W.T sparse matrix
Note: dp.txt is D.T sparse matrix
Note: z.txt has N lines
 
 
7. Print words in each topic
----------------------------
./topics > topics.txt
-or-
matlab < topics.m
INPUT: wp.txt, vocab.txt
OUTPUT: topics.txt
Note: topics.txt has N lines


8. Print topics in each doc
---------------------------
./topicsindocs > topicsindocs.txt
-or-
matlab < topicsindocs.m
INPUT: dp.txt, docs.txt
OUTPUT: topcsindocs.txt
Note: topicsindocs.txt has D lines

