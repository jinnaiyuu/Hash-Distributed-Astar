#!/bin/bash


# Branching factor: $4/$3
# Rr:               $8/$3
# 


awk '{printf("%f %f", $4/$3)}'
