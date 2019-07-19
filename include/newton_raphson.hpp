#ifndef NEWTON_RAPHSON_H
#define NEWTON_RAPHSON_H

#include <vector>
#include <iostream>
#include <iomanip>
#include <complex>
#include <fstream>
#include <calc_util.hpp>
#include <plot_script.hpp>

class NewtonRaphson {
public:

  static int iterations;
  static double num_diff_step;
  static bool print_warnings;
  static bool record_history;

  struct result {
    std::complex<double> z;
    int iterations;
    std::vector<double> history_mod_f;
    std::vector<double> history_mod_df;
    std::vector<std::complex<double> > history_z;
    std::complex<double>(*f_zero)(std::complex<double>);
  };

  NewtonRaphson() { }

  
  result solve(std::complex<double>(*f_zero)(std::complex<double>), std::complex<double> guess_z, double epsabs, double epsrel) {
    std::complex<double> z(guess_z);
    std::complex<double> f;
    std::complex<double> df;
    
    result r;
    int i; 
    for (i = 0; i < iterations; i++) {
      f = f_zero(z); 
      df = num_diff(f_zero, z);
      if (record_history) {
        r.history_z.push_back(z);
        r.history_mod_f.push_back(abs(f));
        r.history_mod_df.push_back(abs(df));
      }

      if (abs(f/df) < epsabs + epsrel*abs(z)) break;
      if (abs(df) == 0.0) {
        print_warn("df is zero at "+to_string(z)+ " at iteration "+to_string(i));
        break;
      }
      if (!std::isfinite(abs(f))) {
        print_warn("f is badly behaved (Inf or NaN) at "+to_string(z) + " at iteration "+to_string(i));
        break;
      }

      z = z - f/df;
    }  
    r.iterations = i+1;
    r.z = z;
    r.f_zero = f_zero;

    return r;
  }


  void analyze(result r, std::string analysis_name, int skip_iterations = 0) {
    std::cout << analysis_name << "\n" << "-----------------------------" << "\n" << "iteratations: " << r.iterations << std::endl;
    calc_util::mkdir(analysis_name+"/");
    std::ofstream outfile;
    outfile.open(analysis_name+"/"+"history.data");
    double max_real = -1e300, min_real = 1e300, max_imag = -1e300, min_imag = 1e300;
    for (int i=0; i < r.iterations; i++) {
      outfile << r.history_mod_f.at(i) << "," 
        << r.history_mod_df.at(i) << "," 
        << r.history_mod_f.at(i)/r.history_mod_df.at(i) << ","
        << r.history_mod_f.at(i)/r.history_mod_df.at(i)/abs(r.history_z.at(i)) << ","
        << r.history_z.at(i).real() << "," 
        << r.history_z.at(i).imag() << "\n";
      if (r.history_z.at(i).real() > max_real) max_real = r.history_z.at(i).real();
      if (r.history_z.at(i).imag() > max_imag) max_imag = r.history_z.at(i).imag();
      if (r.history_z.at(i).real() < min_real) min_real = r.history_z.at(i).real();
      if (r.history_z.at(i).imag() < min_imag) min_imag = r.history_z.at(i).imag();
    }
    outfile.flush();
    outfile.close();
    std::string si = to_string(skip_iterations);

    PlotScript ps("error","png");
    ps.r("set xtics font 'Courier,17'");
    ps.r("set ytics font 'Courier,17'");
    ps.set_separator(",");
    ps.r("cd '"+analysis_name+"/'");
    ps.set_output("error");
    ps.r("unset title");
    ps.r("set multiplot layout 3,1");
    ps.r("plot 'history.data' skip "+to_string(skip_iterations)+" u ($0+"+si+"):1 w l title '(residual) |f|'");
    ps.r("plot 'history.data' skip "+to_string(skip_iterations)+" u ($0+"+si+"):3 w l title '(abs) |f|/|df|'");
    ps.r("plot 'history.data' skip "+to_string(skip_iterations)+" u ($0+"+si+"):4 w l title '(rel) |f|/|df|/|z|'");
    ps.r("unset multiplot");
    ps.end();

    PlotScript ps2("evolution","png");
    ps2.r("set xtics font 'Courier,17'");
    ps2.r("set ytics font 'Courier,17'");
    ps2.r("set label font 'Courier,17'");
    ps2.set_separator(",");
    ps2.r("cd '"+analysis_name+"/'");
    ps2.set_output("evolution");
    ps2.r("unset title");
    ps2.r("plot 'history.data' skip "+to_string(skip_iterations)+" u 5:6:($0+"+si+") w labels font 'Courier,12' notitle, "
    " 'history.data' u 5:6 w l title 'evolution'");
    ps2.end();

    outfile.open(analysis_name+"/"+"f_zero.data");
    double min_f=1e300, max_f=-1e300;
    double abs_f;
    for (double re = min_real; re <= max_real; re += (max_real-min_real)/10.) {
      for (double im = min_imag; im <= max_imag; im += (max_imag-min_imag)/10.) {
        abs_f = abs(r.f_zero(std::complex<double>(re,im)));
        outfile << re << "," << im << "," << abs_f << "\n";
        if (min_f > abs_f) min_f = abs_f;
        if (max_f < abs_f) max_f = abs_f;
      }
    }
    outfile.flush(); outfile.close();
    
    
    PlotScript ps3("function","png");
    ps3.r("set xtics font 'Courier,17'");
    ps3.r("set ytics font 'Courier,17'");
    ps3.r("set label font 'Courier,17'");
    ps3.set_separator(",");
    ps3.r("cd '"+analysis_name+"/'");
    ps3.set_output("function");
    ps3.r("unset title");
    ps3.r("set view map");
    ps3.r("set xtics norotate");
    ps3.r("set palette rgbformulae 10,13,33");
    ps3.r("set cbrange ["+to_string(min_f)+":"+to_string(max_f)+"]");
    ps3.r("splot 'f_zero.data' u 1:2:3 with image");
    ps3.end();

  }

protected:
private:

  std::string to_string(const std::complex<double>& z) {
    std::stringstream ss;
    ss << std::setprecision (4) << z.real();
    if (z.imag() != 0.0) ss  << " + " << z.imag() << "i ";
    return ss.str();
  }

  void print_warn(std::string warn) { if(print_warnings) std::cout << "nr: warning: " << warn << std::endl; }


  std::complex<double> num_diff(std::complex<double>(*f)(std::complex<double>), std::complex<double> z) {
    return (f(z+num_diff_step/2.) - f(z-num_diff_step/2.))/num_diff_step;
  }
};

int NewtonRaphson::iterations = 50;
bool NewtonRaphson::print_warnings = false;
bool NewtonRaphson::record_history = false;
double NewtonRaphson::num_diff_step = 1.0e-6;


#endif
