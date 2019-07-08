#ifndef CALCULATION_H
#define CALCULATION_H

#define EXPORT_DELIMITER " "

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <iomanip>

#include "complex_double.h"
#include "plot_script.h"
#include "calc_util.h"


class Parameter: public cd {
public:
  std::string units_label;
  std::string name_label;
  Parameter(cd value, std::string name_label, std::string units_label)
  :cd(value), units_label(units_label), name_label(name_label)
  {  }
  Parameter& operator=(double  x) {
    cd::operator=(x);
    return *this;
  }

  std::string stringify(bool include_name = true, bool include_units = true) {
    std::string final_string = "";
    if (include_name) final_string += name_label + ": ";
    final_string += cd::stringify();
    if (include_units) final_string += " (" + units_label + ")";
    return final_string;
  }
};
typedef std::vector<Parameter*> ParameterList;

class Variable: public Parameter {
public:
  cd_vector points;
  Variable(/*cd ls_from, cd ls_to, int ls_real_points, int ls_imag_points,*/ std::string name_label, std::string units_label)
    : Parameter(0, units_label, name_label)
    {
      //points = cd::linspace(ls_from, ls_to, ls_real_points, ls_imag_points);
    }
  /*Variable(cd ls_from, cd ls_to, int ls_real_points, std::string name_label, std::string units_label)
    : Variable(ls_from, ls_to, ls_real_points, 1, units_label, name_label)
    {  }*/
  Variable& operator=(double  x) {
    cd::operator=(x);
    return *this;
  }

  std::string stringify(bool include_name = true, bool include_units = true) {
    return Parameter::stringify(include_name, include_units) + " ["+points.front().stringify()+"->"+points.back().stringify()+"]";
  }

};
typedef std::vector<Variable*> VariableList;
typedef std::vector<double> RowResult;
typedef std::vector<std::string> PlotCommands;

class Calculation {

public:

  static bool nowork;
  static std::string calc_path;
  std::string name;
  ParameterList parameters;
  PlotScript* ps;
  std::vector< std::vector<double> > data;
  std::vector< std::string > headers;


  Calculation(std::string name):name(name)
  {
    calc_util::mkdir(calc_path);
    calc_util::mkdir(calc_path+name+"/");
  }

  void work(VariableList variables, std::vector<double> (*iteration_func)(),
      bool export_live = true)
  {
    if (nowork) {
      print_log("skip work.");
    } else {
      print_log("begin work");
      list_parameters();
      std::ofstream outfile; if (export_live) {
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


  void plot(PlotCommands (*plot_coms)(Calculation*), std::string term = "png",
   bool export_script= false, std::string export_name = "") {
    bool silent_state = PlotScript::silent; PlotScript::silent = true;
    ps = new PlotScript(name, term);
    ps->r("cd '"+calc_path+name+"/'", true);
    if (export_name=="") ps->set_output(name);
    else ps->set_output(export_name);
    ps->set_separator(EXPORT_DELIMITER);
    for (int i = 0; i < parameters.size(); i++)
      ps->append_parameter_info(parameters.at(i)->stringify(true));
    ps->set_parameter_info();
    if (plot_coms != NULL) {
      PlotCommands comms = plot_coms(this);
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
    std::ofstream outfile(get_data_filepath());
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

  std::string get_data_filepath() { return calc_path+name+"/"+name+".data"; }

  void list_parameters()  {
    for (int i =0; i< parameters.size(); i++) {
      print_log("param: "+parameters.at(i)->stringify(), false);
    }

    std::cout.flush();
  }

  static void set_flags(int argc,char* argv[]) {
    for (int i = 0; i < argc; ++i)
      if ((std::string) argv[i]=="nowork") { nowork = true; break; }
  }

protected:
private:

  static std::string next_style_point(PlotScript* ps) {return "p "+ps->next_style("point"); }
  static std::string next_style_line(PlotScript* ps)  {return "l "+ps->next_style("line"); }

  void parse_styles(std::string & data) {
    findAndReplaceAll_func(data,"<p_style>",next_style_point);
    findAndReplaceAll_func(data,"<l_style>",next_style_line);
  }

  void findAndReplaceAll_func(std::string & data, std::string toSearch, std::string (*replace_func)(PlotScript* )) {
    size_t pos = data.find(toSearch);
    while( pos != std::string::npos)	{
      std::string replaceStr = replace_func(ps);
      data.replace(pos, toSearch.size(), replaceStr);
      pos =data.find(toSearch, pos + replaceStr.size());
    }
  }

  void parse_header_names(std::string & data) {
    for (int j = 0; j < headers.size(); j++) {
      calc_util::findAndReplaceAll(data, "<"+headers.at(j)+">",
        std::to_string(find_header_index_by_name(headers.at(j))+1) );
    }
  }

  void parse_data_file_path(std::string & data) {
    calc_util::findAndReplaceAll(data,"<data_file_path>",name+".data");
  }


  int find_header_index_by_name(std::string header_name) {
    for (int i = 0; i < headers.size(); i++) {
      if (headers.at(i)==header_name) return i;
    }
    print_log("could not find header by name:" + header_name);
    return -1;
  }

  void print_log(std::string saywhat, bool flush = true)
  {
    std::cout << "calc \'" << name << "\': " << saywhat << "\n";
    if (flush) std::cout.flush();
  }

  void iterate_recurse(std::vector<double> (*f)(),
    VariableList &variables, std::ofstream* file_handle,
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

  std::string concat_strings(std::vector<std::string> list, std::string delimiter) {
    std::string s;
    for (int i = 0; i < list.size(); i++) {
      s = s + list.at(i);
      if (i < list.size()-1) s = s + delimiter;
    }
    return s;
  }

};

bool Calculation::nowork = false;
std::string Calculation::calc_path = "calculations_output/";


#endif
