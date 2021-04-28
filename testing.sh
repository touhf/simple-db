# testing simple-db

make

# create test

./main test.db c
amount=$(ls -dp *test.db* | wc -l)
if [ $amount -eq 1 ]
then
	echo "CREATE: successfull"
else
	echo "CREATE: failed"
fi

# set/get test

./main test.db s 0 name111 emailll
get_rec=$(./main test.db g 0)
ex_res="0 name111 emailll"
if [ "$get_rec" = "$ex_res" ]
then
	echo "SET/GET: successfull"
else
	echo "SET/GET: failed"
fi

# delete test

del_rec=$(./main test.db d 0)
if [ "$del_rec" = "" ]
then
	echo "DELETE: successfull"
else
	echo "DELETE: failed"
fi

rm -f test.db

