#!/bin/sh
cd runs
# test a set of parameters
../bin/ntq-client-test -i -n 1 -q 0 -a localhost -p 12345
