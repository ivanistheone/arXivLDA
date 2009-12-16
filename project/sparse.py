

#% purpose: print topics from topic model run
#% usage:   matlab> topics
#% inputs:  wp.txt, vocab.txt
#% outputs: topics.txt

#[wid,tid,cnt] = textread('wp.txt','%d%d%d','headerlines',3);
#wp = sparse(wid,tid,cnt);
col=[]
row=[]
datacol=[]
for line in cols:
    if len(line)>0:
        [i,j,d]= line.split()
        row.append(int(i)-1)
        col.append(int(j)-1)
        datacol.append(int(d))
msparse = sparse.csc_matrix( (datacol, (row,col)), dtype=float)
wp = msparse.toarray()


#[W,T] = size(wp);

[W,T] = m.shape

#beta = 0.01;
#wp = wp + beta;
beta = 0.01;
wp += beta;



#fac = 1 ./ sum(wp,1);
#prob_w_given_t = wp * diag(fac);

fac = 1/sum(wp,0)
prob_w_given_t = dot(wp, diag(fac) )


#prob_t = sum(wp,1);
#prob_t = prob_t / sum(prob_t);
#
#fid = fopen('topics2.txt','w');
#[word] = textread('vocab.txt','%s');
#if (length(word)~=W), error('bad size'); end;
#
#[xxsort,iisort] = sort(-prob_t);
#
#for tt=1:T
#  %t = iisort(tt);
#  t = tt;
#  prob = prob_w_given_t(:,t);
#  [xsort,isort] = sort(-prob);
#  %fprintf(fid,'[t%d] (prob=%.3f) ', t, prob_t(t));
#  fprintf(fid,'[t%d] ', t);
#  for w=1:12
#    fprintf(fid,'%s ', word{isort(w)});
#  end
#  fprintf(fid,'...\n');
#end
#
#fclose(fid);
#fprintf('wrote file topics2.txt\n');

