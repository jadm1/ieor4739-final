
How to run:

First run the server (configured for 2 task queues)
run_server.sh

Then submit the jobs to the server
submit_jobs.sh

Run as many worker as needed in parallel
run_worker.sh

Save the results:
save_results.sh


If you want to test parameters directly for the trade impact model use test_trade_impact_model.sh (or bin/tmworker-test directly)

If you want to input/output data from the server queues use test_server_push.sh/test_server_pop.sh (or bin/ntq-client-test directly)


How it works:

The ntqserver (network task queues) is made to run like a background service.
You need to set the number N of queues you want at run time. (0 <= queue IDs <= N - 1)
Once connected, a client can do 2 basic operations on a queue: 
PUSH that is upload arbitrary data of the specified size to a queue.
POP that is download data that was previously pushed to a queue.
It is up to the client to determine what the format of its data should be so that a communication is possible between a pusher and a popper.

In our case I used N=2 queues
the first queue (id 0) is used to list the jobs to be done with their parameters
the second queue (id 1) is used to save the job results.

So we have N+1=3 programs
The job supplier (tmjobsupplier) reads jobs to do from a file and pushes them to the first queue
The worker (tmworker) pops a job from the first queue, does it and then sends the results to the second queue.
It loops until no jobs are left in the first queue
As many workers can be working at the same time as wished.
The result saver program (tmresultsaver) pops results from the second queue and saves them to a file


