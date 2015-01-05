#!/bin/bash

awk 'BEGIN{n=1} /24 Puzzle/{printf("%d ", n); n++} $3=="wall"{printf("%f ", $5)} $4=="expanded"{printf("%d\n", $5)} /PBS/{printf("\n")}'
