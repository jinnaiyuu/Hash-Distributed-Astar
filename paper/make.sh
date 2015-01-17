#!/bin/bash

date=`date +%m%d`

git commit -m "autocommit $date" paper.tex b.bib make.sh
#git push

uplatex -shell-escape paper.tex
makeindex paper
pbibtex paper.aux
uplatex -shell-escape paper.tex
uplatex -shell-escape paper.tex
dvipdfmx paper.dvi
cp paper.pdf "2015${date}_陣内佑_マルチコア環境における並列A*探索の探索オーバーヘッドの定性的な解析とアルゴリズムの再評価.pdf"

cp ../paper/* ~/Dropbox/my_papers/paper

evince paper.pdf

