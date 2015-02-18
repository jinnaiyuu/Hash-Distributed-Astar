#!/bin/bash

echo "astar"
perf stat ./runastar.sh
echo "hdastar"
perf stat ./runhdastar.sh
echo "hdastar SZ"
perf stat ./runhdastarsz.sh
