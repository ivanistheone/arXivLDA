#log# Automatic Logger file. *** THIS MUST BE THE FIRST LINE ***
#log# DO NOT CHANGE THIS LINE OR THE TWO BELOW
#log# opts = Struct({'__allownew': True, 'logfile': 'ipython_log.py'})
#log# args = []
#log# It is safe to make manual edits below here.
#log#-----------------------------------------------------------------------
_ip.system("ls -F ")
import re
_ip.magic("logon ")
_ip.magic("logstart ")
_ip.magic("logstop ")
_ip.magic("clear ")
_ip.system("clear")
mutual_information=re.compile(r"I(.*;.*)" )
mutual_information.findall("I(a,b)alksj")
#[Out]# []
mutual_information.findall("I(a;b)alksj")
#[Out]# ['(a;b)alksj']
mutual_information.findall("I(a;b)alksjalskI(a;q332))")
#[Out]# ['(a;b)alksjalskI(a;q332))']
mutual_information
#[Out]# <_sre.SRE_Pattern object at 0xb7dacbf0>
print mutual_information
#mutual_information=re.compile(r"I(.*;.*)" )
mutual_information=re.compile("I(.*;.*)" )
mutual_information.findall("I(a,b)alksj")
#[Out]# []
mutual_information.findall("I(a;b)alksj")
#[Out]# ['(a;b)alksj']
mutual_information.findall("I(a;b)alksj")
#[Out]# ['(a;b)alksj']
mutual_information=re.compile("H([^)]*)" )
entropy=re.compile("H([^)]*)" )
entropy.findall("lkasjalskjHaabH(ab)alksjH(a|b)a;lsk")
#[Out]# ['aabH(ab', '(a|b']
entropy.findall("lkasjalskjHaabH(ab)alksjH(a|b)a;lsk")
#[Out]# ['aabH(ab', '(a|b']
entropy=re.compile("H\(.*\)" )
entropy.findall("lkasjalskjHaabH(ab)alksjH(a|b)a;lsk")
#[Out]# ['H(ab)alksjH(a|b)']
entropy.findall("lkasjalskjHaabH(ab)alksjH(a|b)a;lsk")
#[Out]# ['H(ab)alksjH(a|b)']
entropy=re.compile("H\(.*?\)" )
entropy=re.compile("H\(.*\)" )
entropy=re.compile("H\(.*?\)" )
entropy.findall("lkasjalskjHaabH(ab)alksjH(a|b)a;lsk")
#[Out]# ['H(ab)', 'H(a|b)']
paper = read("FQSW--0606225v1.txt")
paper = open("FQSW--0606225v1.txt").read()
len(paper)
#[Out]# 68044
entropy.findall(In [30]: entropy=re.compile("H\(.*?\)" )
In [31]: entropy.findall("lkasjalskjHaabH(ab)alksjH(a|b)a;lsk")
Out[31]: ['H(ab)', 'H(a|b)']
all_matches = entropy.findall(paper)
len(all_matches)
#[Out]# 106
number__of_matches = len(entropy.findall(paper)) 
all_matches.length
all_matches.__len__ 
#[Out]# <method-wrapper '__len__' of list object at 0xa2201ac>
all_matches.__len__()
#[Out]# 106
entropy.findall(paper)
#[Out]# ['H(A)', 'H(A)', 'H(B)', 'H(AB)', 'H(A|B)', 'H(AB)', 'H(B)', 'H(A|B)', 'H(A|B)', 'H(A)', 'H(R)', 'H(AR)', 'H(F )', 'H(F )', 'H(F )', 'H(F )', 'H(F )', 'H(F )', 'H(A)', 'H(A)', 'H(B)', 'H(A)', 'H(B)', 'H(AB)', 'H(B)', 'H(A)', 'H(B)', 'H(AB)', 'H(\xce\xbe R WA )', 'H(\xce\xbe R \xe2\x8a\x97 \xce\xbe WA )', 'H(B n )', 'H(CA )', 'H(B n CA )', 'H(WC AB)', 'H(WA Rn )', 'H(WA )', 'H(Rn )', 'H(An )', 'H(CA )', 'H(Rn )', 'H(CA )', 'H(An )', 'H(B n )', 'H(Rn )', 'H(An )', 'H(CA WA )', 'H(WA )', 'H(CA )', 'H(WA )', 'H(A)', 'H(B)', 'H(WA )', 'H(WB )', 'H(WA WB Rn )', 'H(WA WB )', 'H(Rn )', 'H(CA CB )', 'H(WA WB Rn )', 'H(WA )', 'H(WB )', 'H(Rn )', 'H(Rn )', 'H(AB)', 'H(CA CB )', 'H(WA )', 'H(WB )', 'H(AB)', 'H(A)', 'H(B)', 'H(AB)', 'H(WA )', 'H(WA R\xe2\x80\xb2 )', 'H(WB )', 'H(WB R\xe2\x80\xb2 )', 'H(WA WB )', 'H(WA WB R\xe2\x80\xb2 )', 'H(R\xe2\x80\xb2 )', 'H(A)', 'H(B)', 'H(A)', 'H(A)', 'H(A\xe2\x80\xb2\xe2\x80\xb2 )', 'H(A\xe2\x80\xb2 |A\xe2\x80\xb2\xe2\x80\xb2 )', 'H(A\xe2\x80\xb2\xe2\x80\xb2 )', 'H(A)', 'H(A1 )', 'H(A2 )', 'H(A2 )', 'H(A1 )', 'H(A2 )', 'H(A1 )', 'H(p)', 'H(p)', 'H(p)', 'H(p)', 'H(p)', 'H(p)', 'H(\xcf\x81)', 'H(p)', 'H(\xcf\x81)', 'H(\xcf\x81)', 'H(\xcf\x81)', 'H(\xcf\x81)', 'H(\xcf\x81)', 'H(A)', 'H(B)']
for i in entropy.findall(paper):
    print i
    
for i in entropy.findall(paper):
    i
    
#[Out]# 'H(A)'
#[Out]# 'H(A)'
#[Out]# 'H(B)'
#[Out]# 'H(AB)'
#[Out]# 'H(A|B)'
#[Out]# 'H(AB)'
#[Out]# 'H(B)'
#[Out]# 'H(A|B)'
#[Out]# 'H(A|B)'
#[Out]# 'H(A)'
#[Out]# 'H(R)'
#[Out]# 'H(AR)'
#[Out]# 'H(F )'
#[Out]# 'H(F )'
#[Out]# 'H(F )'
#[Out]# 'H(F )'
#[Out]# 'H(F )'
#[Out]# 'H(F )'
#[Out]# 'H(A)'
#[Out]# 'H(A)'
#[Out]# 'H(B)'
#[Out]# 'H(A)'
#[Out]# 'H(B)'
#[Out]# 'H(AB)'
#[Out]# 'H(B)'
#[Out]# 'H(A)'
#[Out]# 'H(B)'
#[Out]# 'H(AB)'
#[Out]# 'H(\xce\xbe R WA )'
#[Out]# 'H(\xce\xbe R \xe2\x8a\x97 \xce\xbe WA )'
#[Out]# 'H(B n )'
#[Out]# 'H(CA )'
#[Out]# 'H(B n CA )'
#[Out]# 'H(WC AB)'
#[Out]# 'H(WA Rn )'
#[Out]# 'H(WA )'
#[Out]# 'H(Rn )'
#[Out]# 'H(An )'
#[Out]# 'H(CA )'
#[Out]# 'H(Rn )'
#[Out]# 'H(CA )'
#[Out]# 'H(An )'
#[Out]# 'H(B n )'
#[Out]# 'H(Rn )'
#[Out]# 'H(An )'
#[Out]# 'H(CA WA )'
#[Out]# 'H(WA )'
#[Out]# 'H(CA )'
#[Out]# 'H(WA )'
#[Out]# 'H(A)'
#[Out]# 'H(B)'
#[Out]# 'H(WA )'
#[Out]# 'H(WB )'
#[Out]# 'H(WA WB Rn )'
#[Out]# 'H(WA WB )'
#[Out]# 'H(Rn )'
#[Out]# 'H(CA CB )'
#[Out]# 'H(WA WB Rn )'
#[Out]# 'H(WA )'
#[Out]# 'H(WB )'
#[Out]# 'H(Rn )'
#[Out]# 'H(Rn )'
#[Out]# 'H(AB)'
#[Out]# 'H(CA CB )'
#[Out]# 'H(WA )'
#[Out]# 'H(WB )'
#[Out]# 'H(AB)'
#[Out]# 'H(A)'
#[Out]# 'H(B)'
#[Out]# 'H(AB)'
#[Out]# 'H(WA )'
#[Out]# 'H(WA R\xe2\x80\xb2 )'
#[Out]# 'H(WB )'
#[Out]# 'H(WB R\xe2\x80\xb2 )'
#[Out]# 'H(WA WB )'
#[Out]# 'H(WA WB R\xe2\x80\xb2 )'
#[Out]# 'H(R\xe2\x80\xb2 )'
#[Out]# 'H(A)'
#[Out]# 'H(B)'
#[Out]# 'H(A)'
#[Out]# 'H(A)'
#[Out]# 'H(A\xe2\x80\xb2\xe2\x80\xb2 )'
#[Out]# 'H(A\xe2\x80\xb2 |A\xe2\x80\xb2\xe2\x80\xb2 )'
#[Out]# 'H(A\xe2\x80\xb2\xe2\x80\xb2 )'
#[Out]# 'H(A)'
#[Out]# 'H(A1 )'
#[Out]# 'H(A2 )'
#[Out]# 'H(A2 )'
#[Out]# 'H(A1 )'
#[Out]# 'H(A2 )'
#[Out]# 'H(A1 )'
#[Out]# 'H(p)'
#[Out]# 'H(p)'
#[Out]# 'H(p)'
#[Out]# 'H(p)'
#[Out]# 'H(p)'
#[Out]# 'H(p)'
#[Out]# 'H(\xcf\x81)'
#[Out]# 'H(p)'
#[Out]# 'H(\xcf\x81)'
#[Out]# 'H(\xcf\x81)'
#[Out]# 'H(\xcf\x81)'
#[Out]# 'H(\xcf\x81)'
#[Out]# 'H(\xcf\x81)'
#[Out]# 'H(A)'
#[Out]# 'H(B)'
for i in entropy.findall(paper):
    print i,
    
for i in entropy.findall(paper):
    print i + ", ",
    
