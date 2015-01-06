#!/bin/bash

platex -shell-espace paper.tex
pbibtex paper.aux
platex -shell-espace paper.tex
dvipdfmx paper.dvi
evince paper.pdf
