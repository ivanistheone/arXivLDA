all: topicmodel topics topicsindocs

topicmodel: topicmodel.o topiclib.o -lm
topics: topics.o topiclib.o -lm
topicsindocs: topicsindocs.o topiclib.o -lm

clean:
	rm -rf *.o topicmodel topics topicsindocs docword.txt dp.txt freqwords.txt topics.txt topicsindocs.txt vocab.txt wordstream.txt wp.txt z.txt

CC = gcc -Wall -O3
