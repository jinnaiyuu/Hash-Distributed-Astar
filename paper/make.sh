#!/bin/bash

git commit paper.tex b.bib make.sh order/* eps/* others/* speedup/*

platex -shell-escape paper.tex
pbibtex paper.aux
platex -shell-escape paper.tex
dvipdfmx paper.dvi
evince paper.pdf
