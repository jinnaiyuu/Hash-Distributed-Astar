#!/bin/bash

#############################
### Print into PDF
#############################

# TODO: output as coverage and node evaluations in table.

# syntax of the input
# <domain> <instance> <walltime> <expanded nodes> <solved> <stage>
# <domain> <instance> <walltime> <expanded nodes> <solved> <stage>
# <domain> <instance> <walltime> <expanded nodes> <solved> <stage>
# ....

# put this into a latex style and print into a pdf.

# syntax of the output
# Domain(#problems)   | solved   | expd(total)
# ----------------------------------------------
# <domain><#problems> | <solved> | <expd(total)>
# <domain><#problems> | <solved> | <expd(total)>
# .... 




#date=`date +%m%d%H%M`
#output_dir=$1

tex=""

tex_head="
\documentclass[latex]{article}

\renewcommand{\labelenumii}{\theenumii}
\renewcommand{\theenumii}{\theenumi.\arabic{enumii}.}


% Graphics
\usepackage{array}

% Utility
%\usepackage{comment}

\begin{document}
"

#parameter_text=`cat $2`
#echo $parameter_text

table_head="
\begin{table}[h]
	\caption{$date}
	\label{$date}
	\centering
	\begin{tabular}{l|rrr} \hline
		Domain(\\#problems) & solved & expands \\\\ \hline
"

tex_tail="
	\end{tabular}
\end{table}
\end{document}
"

tex+="$tex_head"
#tex+=`$parameter_text`
tex+="$table_head"

# output format
# <d>(<n_problems>) & <solved> & <expands> \\ 

tex+="`awk '{domains[$1]=$1; \
         n_problem[$1]+=1; \
         n_expd[$1]+=$4;   \
         n_solved[$1]+=$5;  \
         }
         END{\
         i=0;
         for (d in domains) {\
            printf("\t\t%s(%d) & %d & %d \\\\\\\\ \n" \
                    , d, n_problem[d], n_solved[d], n_expd[d]); \
         }
         }'| sort | 
         awk '{print;} (FNR%5)==0{printf("\t\t\\\\hline \n");}' `"

tex+="$tex_tail"

echo "$tex" > $1/summary.tex

latex $1/summary.tex 
dvipdf summary.dvi
mv summary.pdf $1
