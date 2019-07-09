#ifndef NEWTON_RAPHSON_H
#define NEWTON_RAPHSON_H

#include <vector>
#include <iostream>
#include <complex>

class NewtonRaphson {
public:
  struct result {
    std::complex<double> value;
    int iter_count;
    double modf;
    int state;
  } last_result;

  const std::vector<std::string> state_labels = {"nothing happened", "success", "iterations stopped" };

  static int default_num_iters;
  static int iters_excused;
  double modf_tol = 0.0001;
  static double num_diff_step;
  static bool print_warnings;

  NewtonRaphson() { }

  std::complex<double> improve(std::complex<double>(*f)(std::complex<double>), std::complex<double> guess_z, int num_iterations = -1) {
    if (num_iterations == -1) num_iterations = default_num_iters;
    last_result.iter_count = 0; last_result.state = 0;
    return improve_result(f, guess_z, num_iterations, &last_result);
  }

protected:
private:

  void print_warn(std::string warn) { if(print_warnings) std::cout << "nr: warning: " << warn << std::endl; }
  void stop_warn(result *r, std::string message) {
    r->state = 2;
    print_warn(message+" ... iterations ("+std::to_string(r->iter_count)+") stopped");
  }

  std::complex<double> improve_result(std::complex<double>(*f)(std::complex<double>), std::complex<double> given_z, int iterations_left, result* r) {
    std::complex<double> df = num_diff(f, given_z);
    std::complex<double> improved_z = given_z - f(given_z)/df;

    r->value = improved_z;
    r->modf = abs(f(improved_z));

    r->iter_count++;


    if (r->iter_count > iters_excused && iters_excused > -1) {

      if (r->modf > modf_tol) {
        stop_warn(r, "|f("+to_string(improved_z)+")|="+std::to_string(r->modf)+", exceeded tolerance:"+std::to_string(modf_tol));
        return improved_z;
      }
      if (abs(df)==0.0) {
        stop_warn(r, "|df("+to_string(improved_z)+")| is zero, nr will diverge!"+to_string(df));
        return improved_z;
      }
      if (std::isnan(abs(improved_z.real()))) {
        stop_warn(r, "nan value reached!");
        return improved_z;
      }

    }

    if (iterations_left > 0) {
      improved_z = improve_result(f, improved_z, iterations_left-1, r);
    } else {
      r->state = 1;
    }

    return improved_z;
  }

  std::complex<double> num_diff(std::complex<double>(*f)(std::complex<double>), std::complex<double> z) {
    return (f(z+num_diff_step/2.) - f(z-num_diff_step/2.))/num_diff_step;
  }
};

int NewtonRaphson::default_num_iters = 80;
bool NewtonRaphson::print_warnings = false;
int NewtonRaphson::iters_excused = 5;
double NewtonRaphson::num_diff_step = 1.0e-6;



#endif
