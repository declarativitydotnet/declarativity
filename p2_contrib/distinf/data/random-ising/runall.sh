#./lbp.sh residual 30000 10x10_5/*.mat
#./lbp.sh asynchronous 300 10x10_5/*.mat
#./lbp.sh synchronous 300 10x10_5/*.mat
#./schiff.sh 0.3 300 10x10_5/*.mat
#./schiff.sh 1 300 10x10_5/*.mat

./exponential.sh 3 30000 10x10_5/*.mat
./exponential.sh 3 10000 10x10_1/*.mat
