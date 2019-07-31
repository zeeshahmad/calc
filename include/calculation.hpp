#ifndef CALCULATION_H
#define CALCULATION_H

#define EXPORT_DELIMITER " "

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <complex>

#include "plot_script.hpp"
#include "calc_util.hpp"

typedef std::complex<double> cd;

std::string to_string(const cd& z) {
  std::stringstream ss;
  ss << std::setprecision (4) << z.real();
  if (z.imag() != 0.0) ss  << " + " << z.imag() << "i ";
  return ss.str();
}

const cd I(0.,1.);


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
    final_string += to_string(*this);
    if (include_units) final_string += " (" + units_label + ")";
    return final_string;
  }
};
typedef std::vector<Parameter*> ParameterList;

class Variable: public Parameter {
public:
  std::vector<cd> points;
  Variable(std::string name_label, std::string units_label)
    : Parameter(0, name_label, units_label)
    {

    }

  Variable& operator=(double  x) {
    cd::operator=(x);
    return *this;
  }
  
  Variable& operator=(cd  x) {
    cd::operator=(x);
    return *this;
  }


  std::string stringify(bool include_name = true, bool include_units = true) {
    return Parameter::stringify(include_name, include_units) + " ["+to_string(points.front())+"->"+to_string(points.back())+"]";
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

  void work(VariableList variables, std::vector<double> (*iteration_func)())
  {
    if (nowork) {
      print_log("skip work.");
    } else {
      print_log("begin work");
      list_parameters();
      std::ofstream outfile; 
      outfile.open(get_data_filepath());
      outfile << concat_strings(headers, EXPORT_DELIMITER) << "\n";
      iterate_recurse(iteration_func, variables, &outfile);

      outfile.flush();
      outfile.close();
      print_log("end work");
    }
  }


  void plot(PlotCommands (*plot_coms)(), std::string term = "png",
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
      PlotCommands comms = plot_coms();
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

  /*
  bool data_contains(std::vector<double> data_row) {
    for (int i=0; i<data.size(); i++) if (data.at(i)==data_row) return true;
    return false;
  }
*/

  void iterate_recurse(std::vector<double> (*f)(),
    VariableList &variables, std::ofstream* file_handle,
    int depth=0)
  {
    if (depth < variables.size())
    {
      for (int i = 0; i < variables.at(depth)->points.size(); i++)
      {
        variables.at(depth)->real(variables.at(depth)->points.at(i).real());
        variables.at(depth)->imag(variables.at(depth)->points.at(i).imag());
        iterate_recurse(f, variables, file_handle, depth+1);
      }
    }
    else
    {

      RowResult results_row = f();
      data.push_back(results_row);
      //export data
      for (int i = 0; i < results_row.size(); i++) *file_handle << results_row.at(i) << EXPORT_DELIMITER;
      if (results_row.size()>0) *file_handle << "\n";
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

std::vector<cd> linspace(cd start, cd end, int num_real, int num_imag = 1)
{
  std::vector<cd> linspaced;
  int num_real_spaces = (num_real < 2) ? 1 : num_real-1;
  int num_imag_spaces = (num_imag < 2) ? 1 : num_imag-1;

  for (int r = 0; r < num_real; r++)
  {
    double real_spaced = std::real(start+(double)r*((end-start)/(double)num_real_spaces));
    for (int i = 0; i < num_imag; i++)
    {
      double imag_spaced = std::imag(start+(double)i*((end-start)/(double)num_imag_spaces));
      linspaced.push_back(real_spaced + I*imag_spaced);
    }
  }
  return linspaced;
}

std::vector<cd> logspace(cd start, cd end, int num_real, int num_imag = 1)
{
  std::vector<cd> logspaced;
  std::vector<cd> real_pows = linspace(log10(start.real()), log10(end.real()), num_real);
  std::vector<cd> imag_pows = linspace(log10(start.imag()), log10(end.imag()), num_imag);

  for (int r = 0; r < num_real; r++)
  {
    for (int i = 0; i < num_imag; i++)
    {
      cd imag_part = pow(10., imag_pows.at(i));
      if (num_imag == 1 && start.imag() == 0.0) imag_part = 0.0;
      logspaced.push_back(pow(10.,real_pows.at(r))+I*imag_part );
    }
  }

  return logspaced;
}


#endif
