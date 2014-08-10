test some linux sys call to eliminate the copy
===

0->sendfile

1->mmap+write

2->splice

3->read+write

they are almost the same speed when the computer is free
run:
bash batch_test.sh [iter_times]
can see the result
