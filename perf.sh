#!/bin/bash
#
# Tests the performance of the Uniform Reliable Broadcast application.
#
# usage: ./test_performance evaluation_time
#
# evaluation_time: Specifies the number of seconds the application
#                  should run.
#

evaluation_time=$3
init_time=2

# prepare input
if [ "$1" = "FIFO" ]; then
echo "writing FIFO input..."
 
echo "5
1 127.0.0.1 12001
2 127.0.0.1 12002
3 127.0.0.1 12003
4 127.0.0.1 12004
5 127.0.0.1 12005" > membership

else 
echo "writing LCausal input..."
    
echo "5
1 127.0.0.1 12001
2 127.0.0.1 12002
3 127.0.0.1 12003
4 127.0.0.1 12004
5 127.0.0.1 12005
1 4 5
2 1
3 1 2
4
5 3 4" > membership
fi

if [ ! -f Makefile ]; then
  echo "WARNING: Makefile not found! using default Makefile"
  if [ "$2" = "C" ]; then
    cp Makefile_c_example Makefile
  else
    cp Makefile_java_example Makefile
  fi
fi

# compile (should output: da_proc or Da_proc.class)
make clean
make

#start 5 processes
for i in `seq 1 5`
do
    if [ "$2" = "C" ]; then
      ./da_proc $i membership 100 &
    else
      java Da_proc $i membership 100 &
    fi
    da_proc_id[$i]=$!
done

#leave some time for process initialization
sleep $init_time

#start broadcasting
echo "Evaluating application for ${evaluation_time} seconds."
for i in `seq 1 5`
do
    if [ -n "${da_proc_id[$i]}" ]; then
	kill -USR2 "${da_proc_id[$i]}"
    fi
done

#let the processes do the work for some time
sleep $evaluation_time

#stop all processes
for i in `seq 1 5`
do
    if [ -n "${da_proc_id[$i]}" ]; then
	kill -TERM "${da_proc_id[$i]}"
    fi
done

#wait until all processes stop
for i in `seq 1 5`
do
	wait "${da_proc_id[$i]}"
done

#count delivered messages in the logs
#... (not implemented here)

total_count=0

for i in `seq 1 5`
do
    count=$(grep d da_proc_$i.out | wc -l)
    total_count=$((total_count + count))
    echo "Process $i received $count messages"
done

echo "Total count: $total_count messages"

echo "Performance test done."
