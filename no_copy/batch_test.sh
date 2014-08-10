#!/bin/bash

iter_time=$1
# function num is dependent by the code
# 0->sendfile
# 1->mmap+write
# 2->splice
# 3->read+write
fun_num=4
stat_fn=time_stat

# clean
if [[ $1 == *clean* ]]; then
    rm bigfile bigfile.back time.* time_stat
    exit 0
fi

# make test file
seq 100000000 | awk 'NR%3==1' > bigfile

# compile
g++ -O2 ./no_copy_sys_call.cpp || { echo 'gen bin failed' && exit 1; }

# run
for ((j=0;j<$fun_num;++j)); do
    > time.$j
done
> time.cp
> time.cat
for ((i=0;i<$iter_time;++i)); do 
    (time cp bigfile bigfile.back) 2>&1 | grep '^real' >> time.cp && rm bigfile.back
    (time cat bigfile > bigfile.back) 2>&1 | grep '^real' >> time.cat && rm bigfile.back
    for ((j=0;j<$fun_num;++j)); do
        (time ./a.out $j bigfile bigfile.back) 2>&1 | grep '^real' >> time.$j && rm bigfile.back
    done
done

# calc average time
> $stat_fn
for e in time.* ; do
    file_post=$(echo ${e##*.})
    aver_time=$(awk '{second=substr($NF,3,5);sum+=second}END{print sum/NR}' $e)
    echo "$file_post=$aver_time" >> $stat_fn
done
