CC = g++
INCDIRS = /usr/local/lib/g++-include
INCFLAGS = $(INCDIRS:%=-I%)
CFLAGS = -O6 #-pg -Wall
HTMM_BASE = htmm.o fast_restricted_hmm.o document.o EM.o sentence.o \
            fast_restricted_viterbi.o
WEB_HTMM_OBJS = $(HTMM_BASE) web_em.o web_htmm_driver.o check_prob.o

all: web_htmm

htmm: $(HTMM_OBJS)
	$(CC) $(CFLAGS) -o htmm \
	$(HTMM_OBJS) $(LDFLAGS)

web_htmm: $(WEB_HTMM_OBJS)
	$(CC) $(CFLAGS) -o web_htmm \
	$(WEB_HTMM_OBJS) $(LDFLAGS)

# built in rule for c++:

.cc.o:
	$(CC) $(CFLAGS) $(INCFLAGS) -c $<

clean:
	-rm *.o 
	-rm htmm web_htmm
	-rm core

depend:
	makedepend -- $(CFLAGS) -- *.cc

# DO NOT DELETE

EM.o: /usr/include/stdlib.h /usr/include/features.h /usr/include/sys/cdefs.h
EM.o: /usr/include/gnu/stubs.h /usr/include/sys/types.h
EM.o: /usr/include/bits/types.h /usr/include/bits/wordsize.h
EM.o: /usr/include/bits/typesizes.h /usr/include/time.h /usr/include/endian.h
EM.o: /usr/include/bits/endian.h /usr/include/sys/select.h
EM.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
EM.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
EM.o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sched.h
EM.o: /usr/include/alloca.h /usr/include/math.h /usr/include/bits/huge_val.h
EM.o: /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h EM.h htmm.h
EM.o: document.h sentence.h fast_restricted_hmm.h error.h
EM.o: fast_restricted_viterbi.h
document.o: document.h sentence.h error.h
fast_restricted_hmm.o: /usr/include/math.h /usr/include/features.h
fast_restricted_hmm.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
fast_restricted_hmm.o: /usr/include/bits/huge_val.h
fast_restricted_hmm.o: /usr/include/bits/mathdef.h
fast_restricted_hmm.o: /usr/include/bits/mathcalls.h fast_restricted_hmm.h
fast_restricted_hmm.o: /usr/include/assert.h
fast_restricted_viterbi.o: fast_restricted_viterbi.h
htmm.o: htmm.h document.h sentence.h error.h
htmm_driver.o: /usr/include/stdlib.h /usr/include/features.h
htmm_driver.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
htmm_driver.o: /usr/include/sys/types.h /usr/include/bits/types.h
htmm_driver.o: /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h
htmm_driver.o: /usr/include/time.h /usr/include/endian.h
htmm_driver.o: /usr/include/bits/endian.h /usr/include/sys/select.h
htmm_driver.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
htmm_driver.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
htmm_driver.o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sched.h
htmm_driver.o: /usr/include/alloca.h EM.h htmm.h document.h sentence.h
sentence.o: sentence.h error.h
show_top_words.o: error.h
