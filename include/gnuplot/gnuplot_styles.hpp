#include <string>
#include <vector>

const std::vector<std::string> gnuplot_styles = { "","#begin styles",
"c_blue = '#0088aa'",
"c_dgreen = '#325a0a'",
"c_orange = '#ff6600'",
"c_brown = '#9b745c'",
"c_green = '#30ae30'",
"c_purple = '#6600ff'",
"c_gold = '#bf9800'",
"c_maroon = '#8d2727'",
"c_light_gray = '#a3a3a3'",
"c_dark_gray = '#424242'",
"c_pink = '#ff0066'",
//# first 9 styles for 'using lines'
"set style line 1 lw line_lw lc rgb c_blue",
"set style line 2 lw line_lw lc rgb c_purple",
"set style line 3 lw line_lw lc rgb c_orange",
"set style line 4 lw line_lw lc rgb c_brown",
"set style line 5 lw line_lw lc rgb c_green",
"set style line 6 lw line_lw lc rgb c_pink",
"set style line 7 lw line_lw lc rgb c_gold",
"set style line 8 lw line_lw lc rgb c_maroon",
"set style line 9 lw line_lw lc rgb c_light_gray",
//"# next 9 styles for 'using lines'
"point_lw = 1.3",
"set style line 10 lw point_lw ps default_ps lc rgb c_maroon",
"set style line 11 lw point_lw ps default_ps lc rgb c_blue",
"set style line 12 lw point_lw ps default_ps lc rgb c_dgreen",
"set style line 13 lw point_lw ps default_ps lc rgb c_orange",
"set style line 14 lw point_lw ps default_ps lc rgb c_brown",
"set style line 15 lw point_lw ps default_ps lc rgb c_green",
"set style line 16 lw point_lw ps default_ps lc rgb c_purple",
"set style line 17 lw point_lw ps default_ps lc rgb c_gold",
"set style line 18 lw point_lw ps default_ps lc rgb c_maroon",
"#end styles", "" };
