
# libLDA : topic model generator routines
#
# Ivan Savov 2009
#




class ProbabilityDistibution()



def kullbackLeibler(prob_p, prob_q):
    """ Computes the KL distance betwen two distributions represented as
        two arrays of same length

        lol... wowo that was impler that i thouthg ;)
        """
        p = prob_p
        q = prob_q
        return 0.5*sum(p*log(p/q)+q*log(q/p))



def runLDA():



class ProbabilityMatrix()


def TopicModel(

    N
    documents
    D
    vocab
    W
    phi
    theta

    topics



###


def hungarianAlgorithm(topics_A, topics_B):
    """ returns a mapping
        the optimal matching between the two distributions of the same size
        ( (0,3), (1,4), ... , )
        which means topicA0 is best matched to topicB3, etc...
        """


def hungarianAlgorithm(topics_A, topics_b):
    """ returns a mapping
        the optimal matching between the two distributions
        topics_A is more "course grained" than topics_b
        i.e. len(topics_A) < len(topics_b)

        I guess it will be pretty hard if you don't know the proportions...
        lets assume there is an order of magnitude difference ...


        exclusion:
        see if



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



