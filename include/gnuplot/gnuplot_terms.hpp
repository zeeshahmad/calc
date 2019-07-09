#include <string>
#include <vector>

const std::vector<std::string> gnuplot_terms = { "","#begin term settings",
"if (chosen_term eq 'png') {",
"  set term png enhanced size 1200,1200 font 'Ubuntu,28' lw 2",
"  output_ext = '.png'",
"  line_lw = 2.6",
"  default_ps = 1.5",
"}",

"if (chosen_term eq 'epslatex') {",
"  set term epslatex size 15cm,16cm font ',20pt' lw 2",
"  output_ext = '.tex'",
"  line_lw = 4",
"  default_ps = 2.5",
"  set key samplen 5",
"  set key spacing 3",
"}",

"",
"#end term settings", "" };
