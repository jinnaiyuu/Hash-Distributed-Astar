#!/bin/bash

ulimit -v 2000000 -m 2000000

cat ../grid_instances/grid_1 | ./tiles grid hdastar 2 1
