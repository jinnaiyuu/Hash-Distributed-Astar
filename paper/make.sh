#!/bin/bash

platex paper.tex
dvipdfmx paper.dvi
evince paper.pdf
