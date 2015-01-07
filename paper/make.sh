#!/bin/bash

date=`date +%m/%d\ %H:%M`

git commit -m "autocommit $date" paper.tex b.bib make.sh
git push

platex -shell-escape paper.tex
platex -shell-escape paper.tex
pbibtex paper.aux
platex -shell-escape paper.tex
dvipdfmx paper.dvi
evince paper.pdf
