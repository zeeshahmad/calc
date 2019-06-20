#ifndef PLOT_SCRIPT_H
#define PLOT_SCRIPT_H

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>

#include "gnuplot/gnuplot.h"
#include "gnuplot/gnuplot_styles.h"
#include "gnuplot/gnuplot_terms.h"
#include "calc_util.h"
#include "complex_double.h"

using namespace std;

class PlotScript {
  public:
    static bool silent;
    GnuplotPipe gp;
    string term;
    string script_name;
    bool show_parameters = false;
    vector<string> history;
    vector<string> labels;

    int current_line_style=-1; int current_point_style=-1;
    const int line_styles[9] = {1,2,3,4,5,6,7,8,9};
    const int point_styles[9] = {10,11,12,13,14,15,16,17,18};

    string next_style(string type) {
      if (type == "line") {
        current_line_style = (current_line_style+1)%9;
        return "ls "+to_string(line_styles[current_line_style]);
      } else if (type == "point") {
        current_point_style = (current_point_style+1)%9;
        return "ls "+to_string(point_styles[current_point_style]);
      }
      return "invalid type of style:"+type;
    }

    PlotScript(string script_name_, string term = "png") {
      if (term == "png") show_parameters = true;
      script_name = script_name_;
      set_term(term);
      set_extra();
      set_styles();

    }

    void set_styles() {
      r_vec(gnuplot_styles);
    }

    void set_extra() {
      r("set encoding utf8");
      r("set key font 'Ubuntu,22'");
      string script_name_c = script_name; calc_util::findAndReplaceAll(script_name_c,"_"," ");
      if (term == "png") set("title '"+script_name_c+"'");
    }

    void set_term(string term) {
      this->term = term;
      r("chosen_term = '"+term+"'");
      r_vec(gnuplot_terms);
    }

    void set_output(string outfilename) {
      r("set output '" + outfilename + "'.output_ext");
    }

    void set_separator(string new_delimiter = " ") {
      r("set datafile separator '" + new_delimiter + "'");
    }

    vector<string> parameter_strings;

    void set_parameter_info()
    {
      string show_parameters_str = (show_parameters ? " 1":" 0");
      r("show_parameters ="+show_parameters_str);

      int bmargin = 4 + parameter_strings.size();
      stringstream final_text;
      final_text << "\\n\\n\\n\\n" << setprecision(3)<<scientific;
      for (int i = 0; i < parameter_strings.size(); i++) {
        final_text << parameter_strings.at(i) << "\\n";
      }
      r("if (show_parameters == 1) {");
      r("set bmargin "+to_string(bmargin));
      set_label(final_text.str(), "graph 0,graph 0",-1, "tc rgb c_light_gray");
      r("}");
    }

    void append_parameter_info(string param_string)
    {
      parameter_strings.push_back(param_string);
    }

    void set_label(string label_text, string position = "0,0", int label_index = -1, string extra = "") {
      if (label_index == -1) {
        label_index = labels.size();
        labels.push_back("");
      }
      labels[label_index] = r("set label "+to_string(label_index+1)+" at "+position+" \""+label_text+"\" "+extra);
    }

    void set(string the_rest) {
      r("set " + the_rest);
    }

    void end() {
      r("unset output");
      r("exit");
    }

    string r(string command, bool skip_history = false) {
      if (!silent) cout << "gnuplot: " << command << endl;
      if (!skip_history) history.push_back(command);
      gp.sendLine(command);
      return command;
    }

    void r_file(string filepath) {
      ifstream inFile(filepath);
      string line;
      while (getline(inFile, line)) {
        r(line);
      }
      inFile.close();
    }

    void r_vec(const vector<string> & commands) {
      for (int i = 0; i < commands.size(); i++)
        r(commands.at(i));
    }

    void export_script(string filepath) {
      ofstream outfile;
      outfile.open(filepath);
      for (int i = 0; i < history.size(); i++) {
        outfile << history.at(i) << "\n";
      }
      outfile.flush();
      outfile.close();
    }

  protected:
  private:

};

bool PlotScript::silent = false;

#endif
