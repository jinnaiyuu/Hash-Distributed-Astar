#!/bin/bash

date=`date +%m-%d`

cp paper.tex /home/yuu/Dropbox/
git commit -m "autocommit $date" paper.tex b.bib make.sh
#git push

platex -shell-escape paper.tex
makeindex paper
platex -shell-escape paper.tex
pbibtex paper.aux
platex -shell-escape paper.tex
dvipdfmx paper.dvi
cp paper.pdf /home/yuu/Dropbox/"2015_${date}_陣内佑_マルチコア環境における並列A*探索の探索オーバーヘッドの定性的な解析とアルゴリズムの再評価.pdf"


evince paper.pdf

