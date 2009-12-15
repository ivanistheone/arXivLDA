

col=[]
row=[]
data=[]

for line in cols:
    if len(line)>0:
        [i,j,d]= line.split()
        row.append(int(i)-1)
        col.append(int(j)-1)
        data.append(int(d))

msparse = sparse.csc_matrix( (data, (row,col)) )

m = msparse.toarray()

beta = 0.01;

m += beta;



