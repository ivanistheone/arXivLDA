%----------------------------------------------------
% file:    topicsindocs.m
% purpose: print topics from topic model run
% usage:   matlab> topicsindocs
% inputs:  wp.txt, vocab.txt
% outputs: topicsindocs.txt
% version: 1.0
% author:  David Newman, newman@uci.edu
% date:    11/19/08
%----------------------------------------------------

[did,tid,cnt] = textread('dp.txt','%d%d%d','headerlines',3);
dp = sparse(did,tid,cnt);
[D,T] = size(dp);

%alpha = 0.1;
alpha = 0.0;
dp = dp + alpha;
fac = 1 ./ sum(dp,2);
for t=1:T
  dp(:,t) = dp(:,t) .* fac;
end
prob_t_given_d = dp';

fid = fopen('topicsindocs2.txt','w');
[doc] = textread('docs.txt','%s');
if (length(doc)~=D), error('bad size'); end;

for d=1:D
  prob = prob_t_given_d(:,d);
  [xsort,isort] = sort(-prob);
  fprintf(fid,'<%s> ', doc{d});
  for t=1:4
    tt = isort(t);
    if (prob(tt)>0.1)
      fprintf(fid,'t%d ', tt);
    end
  end
  fprintf(fid,'\n');
end

fclose(fid);
fprintf('wrote file topicsindocs2.txt\n');
