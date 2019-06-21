#ifndef NEWTON_RAPHSON_H
#define NEWTON_RAPHSON_H

#include "complex_double.h"
#include <vector>
#include <iostream>

class NewtonRaphson {
public:
  struct result {
    cd value;
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

  cd improve(cd(*f)(cd), cd guess_z, int num_iterations = -1) {
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

  cd improve_result(cd(*f)(cd), cd given_z, int iterations_left, result* r) {
    cd df = num_diff(f, given_z);
    cd improved_z = given_z - f(given_z)/df;

    /*std::cout << "given_z:" << given_z.string_form() << std::endl;
    std::cout << "improved_z:" << improved_z.string_form() << std::endl;*/


    r->value = improved_z;
    r->modf = abs(f(improved_z));

    r->iter_count++;


    if (r->iter_count > iters_excused && iters_excused > -1) {

      if (r->modf > modf_tol) {
        stop_warn(r, "|f("+improved_z.stringify()+")|="+std::to_string(r->modf)+", exceeded tolerance:"+std::to_string(modf_tol));
        return improved_z;
      }
      if (abs(df)==0.0) {
        stop_warn(r, "|df("+improved_z.stringify()+")| is zero, nr will diverge!"+df.stringify());
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

  cd num_diff(cd(*f)(cd), cd z) {
    cd f_up = f(z+num_diff_step/2.);
    cd f_down = f(z-num_diff_step/2.);
    return (f(z+num_diff_step/2.) - f(z-num_diff_step/2.))/num_diff_step;
  }
};

int NewtonRaphson::default_num_iters = 80;
bool NewtonRaphson::print_warnings = false;
int NewtonRaphson::iters_excused = 10;
double NewtonRaphson::num_diff_step = 1.0e-6;



#endif
