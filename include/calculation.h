#ifndef CALCULATION_H
#define CALCULATION_H

#define EXPORT_DELIMITER " "

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>

#include "complex_double.h"
#include "plot_script.h"
#include "calc_util.h"


using namespace std;

class Parameter: public cd {
public:
  string units_label;
  string name_label;
  Parameter(cd value, string name_label, string units_label)
  :cd(value), units_label(units_label), name_label(name_label)
  {  }
  Parameter& operator=(double & x) {
    cd::operator=(x);
    return *this;
  }
  string name_with_units() { return name_label+" ("+units_label+")"; }
  string value_as_string() {
    stringstream ss; ss << setprecision(3)<<scientific;
    ss << this->real() << " + " << this->imag() << "i ";
    return ss.str();
  }

  string nice_string(bool include_name = true, bool include_units = true) {
    stringstream ss; if (include_name) ss << name_label << ": ";
    ss << setprecision (4) << real();
    if (imag() != 0.0) ss  << " + " << imag() << "i ";
    if (include_units) ss << " (" << units_label << ")";
    return ss.str();
  }
};
typedef vector<Parameter*> ParameterList;

class Variable: public Parameter {
public:
  cd_vector points;
  Variable(cd ls_from, cd ls_to, int ls_real_points, int ls_imag_points, string name_label, string units_label)
    : Parameter(ls_from, units_label, name_label)
    {
      points = cd::linspace(ls_from, ls_to, ls_real_points, ls_imag_points);
    }
  Variable(cd ls_from, cd ls_to, int ls_real_points, string name_label, string units_label)
    : Variable(ls_from, ls_to, ls_real_points, 1, units_label, name_label)
    {  }


};
typedef vector<Variable*> VariableList;
typedef vector<double> RowResult;

class Calculation {

public:

  static bool nowork;
  static string calc_path;
  string name;
  ParameterList parameters;
  PlotScript* ps;
  vector< vector<double> > data;
  vector< string > headers;


  Calculation(string name):name(name)
  {
    calc_util::mkdir(calc_path);
    calc_util::mkdir(calc_path+name+"/");
  }

  void work(VariableList variables, vector<double> (*iteration_func)(),
      bool export_live = true)
  {
    if (nowork) {
      print_log("skip work.");
    } else {
      print_log("begin work");
      list_parameters();
      ofstream outfile; if (export_live) {
        outfile.open(get_data_filepath());
        outfile << concat_strings(headers, EXPORT_DELIMITER) << "\n";
      }
      iterate_recurse(iteration_func, variables, &outfile, export_live);

      if (export_live) {
        outfile.flush();
        outfile.close();
      }
      print_log("end work");
    }
  }


  void plot(vector<string> (*plot_coms)(Calculation*), string term = "png",
   bool export_script= false, string export_name = "") {
    bool silent_state = PlotScript::silent; PlotScript::silent = true;
    ps = new PlotScript(name, term);
    ps->r("cd '"+calc_path+name+"/'", true);
    if (export_name=="") ps->set_output(name);
    else ps->set_output(export_name);
    ps->set_separator(EXPORT_DELIMITER);
    for (int i = 0; i < parameters.size(); i++)
      ps->append_parameter_info(parameters.at(i)->nice_string(true));
    ps->set_parameter_info();
    if (plot_coms != NULL) {
      vector<string> comms = plot_coms(this);
      for (int i = 0; i<comms.size(); i++) {
        parse_header_names(comms.at(i));
        parse_data_file_path(comms.at(i));
        parse_styles(comms.at(i));
        ps->r(comms.at(i));
      }
    }
    PlotScript::silent = silent_state;
    ps->end();
    if (export_script) ps->export_script(calc_path+name+"/"+name+".plt");
    delete(ps);
  }

  void export_data() {
    ofstream outfile(get_data_filepath());
    outfile << concat_strings(headers, EXPORT_DELIMITER) << "\n";
    for (int i = 0; i < data.size(); i++) {
      for (int j = 0; j < data.at(i).size(); j++) {
        outfile << data.at(i).at(j) << EXPORT_DELIMITER;
      }
      outfile << "\n";
    }
    outfile.flush();
    outfile.close();
  }

  string get_data_filepath() { return calc_path+name+"/"+name+".data"; }

  void list_parameters()  {
    for (int i =0; i< parameters.size(); i++) {
      print_log("param: "+parameters.at(i)->name_with_units()
        +" -> "+parameters.at(i)->value_as_string(), false);
    }

    cout.flush();
  }

  static void set_flags(int argc,char* argv[]) {
    for (int i = 0; i < argc; ++i)
      if ((string) argv[i]=="nowork") { nowork = true; break; }
  }

protected:
private:

  static string next_style_point(PlotScript* ps) {return "p "+ps->next_style("point"); }
  static string next_style_line(PlotScript* ps)  {return "l "+ps->next_style("line"); }

  void parse_styles(string & data) {
    findAndReplaceAll_func(data,"<p_style>",next_style_point);
    findAndReplaceAll_func(data,"<l_style>",next_style_line);
  }

  void findAndReplaceAll_func(string & data, string toSearch, string (*replace_func)(PlotScript* )) {
    size_t pos = data.find(toSearch);
    while( pos != string::npos)	{
      string replaceStr = replace_func(ps);
      data.replace(pos, toSearch.size(), replaceStr);
      pos =data.find(toSearch, pos + replaceStr.size());
    }
  }

  void parse_header_names(string & data) {
    for (int j = 0; j < headers.size(); j++) {
      calc_util::findAndReplaceAll(data, "<"+headers.at(j)+">",
        to_string(find_header_index_by_name(headers.at(j))+1) );
    }
  }

  void parse_data_file_path(string & data) {
    calc_util::findAndReplaceAll(data,"<data_file_path>",name+".data");
  }


  int find_header_index_by_name(string header_name) {
    for (int i = 0; i < headers.size(); i++) {
      if (headers.at(i)==header_name) return i;
    }
    print_log("could not find header by name:" + header_name);
    return -1;
  }

  void print_log(string saywhat, bool flush = true)
  {
    cout << "calc \'" << name << "\': " << saywhat << "\n";
    if (flush) cout.flush();
  }

  void iterate_recurse(vector<double> (*f)(),
    VariableList &variables, ofstream* file_handle,
    bool export_live, int depth=0)
  {
    if (depth < variables.size())
    {
      for (int i = 0; i < variables.at(depth)->points.size(); i++)
      {
        variables.at(depth)->real(variables.at(depth)->points.at(i).real());
        variables.at(depth)->imag(variables.at(depth)->points.at(i).imag());
        iterate_recurse(f, variables, file_handle, export_live, depth+1);
      }
    }
    else
    {

      RowResult results_row = f();
      data.push_back(results_row);
      if (export_live) {
        for (int i = 0; i < results_row.size(); i++) *file_handle << results_row.at(i) << EXPORT_DELIMITER;
        if (results_row.size()>0) *file_handle << "\n";
      }
    }
  }

  string concat_strings(vector<string> list, string delimiter) {
    string s;
    for (int i = 0; i < list.size(); i++) {
      s = s + list.at(i);
      if (i < list.size()-1) s = s + delimiter;
    }
    return s;
  }

};

bool Calculation::nowork = false;
string Calculation::calc_path = "calculations_output/";


#endif
