
# libLDA : topic model generator routines
#
# Ivan Savov 2009
#

# == GOALS ==
# Using liblda, one should be able to quickly train LDA
# To make an easy to use wrapper for D. Newman's Gibbs Sampling LDA code
# and other...
#
# == Data storage ==
# What do we store in RAM and what do we store on disk?
#  - first pass which does pre-processing and generates word counts --> on disk
#    in each of the folders specified as input dirs create .wordcounts hidden dir
#  - second pass which is dictionary aware can generate list in RAM (long term objective)
#    for now we will create a rundir and create the docword.txt in there
#  - (long term objective) save progress dp.txt and wp.txt and z.txt after different NITER
#
#  - after LDA the topic extraction and document-topic extraction depends on building
#    of prob_w_given_t and prob_t_given_d which are built from file on filesystem
#    would be good to have some sort of "cache" mechanism
#           --> mylibs/memcache.py, but I have to make a wrapper that splits the
#               numpy arrays into chunks of size < 1MB
#               (low priority)
#
#  - later we can Query an LDA model with some documentQuery set
#    this has to wait for TODO 1
#
#





# TODO:
#  1 - read through the topicmodel code + libs
#      and wrap it as a python extension
#  2 - try D. Andrzejewski's cvbLDA code
#  3 -
#


class ProbabilityDistibution(object):
    """ thought i might use this but no..."""
    pass


def kullbackLeibler(prob_p, prob_q):
    """ Computes the KL distance betwen two distributions represented as
        two arrays of same length

        lol... wowo that was impler that i thouthg ;)
        """
    p = prob_p
    q = prob_q
    return 0.5*sum(p*log(p/q)+q*log(q/p))



class Document(object):
    """Base class for two kinds of documents
         PDFDoc
         TextDoc
         DBDoc


        methods:
            __len__(vocab=None)
                return total number of words (only count words in vocab if not None)
            get_wordcounts(vocab)
                return the wc vector according to vocab
            get_id
                returns a unique identifier dbid, filesystem path or ?
            get_metadata ?

    """

class DocumentCollection(object):
    """ A base class for
            DirListDC
            FileListDC
            DatabaseDC

            a Document is basically an id and a wordcount vector

        attibutes:
            N
            D

        methods:
            get_doc_list()
            get_doc_id(int n \in 0 .. D-1)
            get_wc
            get_metadata?

    """

    def __init__(self):
        self.N=None
        self.D=None





class TopicModel(object):
    """
    This is the the main class for the LDA topic model functionnality

    attrinbutes:
        N           : total number of words in documet collection
        T           : number of topics in model
        documents   : reference to a DocumentColleciton object
        D           : len(documents)
        vocab       : ref to Vacabulary object
        W           : len(vocab)

        phi         : topic-word distribution
                      size: WxT
                      alias: prob_w_given_t
        theta       : topic-document distibution
                      size: TxD
                      alias: prob_t_given_d
        z           : the topic assignment
                      size: ?
        topics      : array of size T with short descriptions for each of the topics
                      ['physics', 'chemistry', ... , 'general science']


    methods:
        __init__ constuctor ... see below
        train(method)
        query(queryDocCol, method)

    """

    def __init__(self) #, documents=None, T=None, vocab=None):
        """ must give a DocumentCollection object upon creation and T # topics """

        self.T=None
        self.documents = None


        # trained / non-trained
        self.is_trained = False


    def fromDocumentCollection(self, documents=None, T=None, vocab=None):

    def loadFromPickledDict(self, fileobj):
        """Loads the TopicModel from a cPickle file-like object """
        pass

    def loadFromJson=None):
        """Loads the TopicModel from a json file"""
        pass


    def train(self, method):
        """ run the training method on the TopicModel
            allowed values are:
            NewmanGibbs
            (soon) AndrzejewskiCVB
            """
        pass




def hungarianAlgorithm(topics_A, topics_B):
    """ returns a mapping
        the optimal matching between the two distributions of the same size
        ( (0,3), (1,4), ... , )
        which means topicA0 is best matched to topicB3, etc...
        """
    pass


def genHungarianAlgorithm(topics_A, topics_b):
    """ returns a mapping
        the optimal matching between the two distributions
        topics_A is more "course grained" than topics_b
        i.e. len(topics_A) < len(topics_b)

        I guess it will be pretty hard if you don't know the proportions...
        lets assume there is an order of magnitude difference ...


        exclusion:
        see if
        """
    pass




def getCostMatrix(topicmodel1, topicmodel2):
    """ computes the KL distance between each of the probs in model1
        and model2

        return matrix is of size model1.T x model2.T
        """

    cost =zeros()
    for t1 in arange(tpm1[1]):
        for t2 in arange(tpm2[1]):
            cost[ t1, t2 ] = kl(tpm1[3][:,t1],  tpm2[3][:,t2]   )

    return cost



