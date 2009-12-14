%----------------------------------------------------
% file:    topics.m
% purpose: print topics from topic model run
% usage:   matlab> topics
% inputs:  wp.txt, vocab.txt
% outputs: topics.txt
% version: 1.0
% author:  David Newman, newman@uci.edu
% date:    11/19/08
%----------------------------------------------------

[wid,tid,cnt] = textread('wp.txt','%d%d%d','headerlines',3);
wp = sparse(wid,tid,cnt);
[W,T] = size(wp);

beta = 0.01;
wp = wp + beta;

fac = 1 ./ sum(wp,1);
prob_w_given_t = wp * diag(fac);
  
prob_t = sum(wp,1);
prob_t = prob_t / sum(prob_t);

fid = fopen('topics2.txt','w');
[word] = textread('vocab.txt','%s');
if (length(word)~=W), error('bad size'); end;

[xxsort,iisort] = sort(-prob_t);

for tt=1:T
  %t = iisort(tt);
  t = tt;
  prob = prob_w_given_t(:,t);
  [xsort,isort] = sort(-prob);
  %fprintf(fid,'[t%d] (prob=%.3f) ', t, prob_t(t));
  fprintf(fid,'[t%d] ', t);
  for w=1:12
    fprintf(fid,'%s ', word{isort(w)});
  end
  fprintf(fid,'...\n');
end

fclose(fid);
fprintf('wrote file topics2.txt\n');
