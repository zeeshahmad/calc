#ifndef COMPLEX_DOUBLE_H
#define COMPLEX_DOUBLE_H

#include <complex>
#include <vector>
#include <iostream>
#include <string>
#include <sstream>

typedef std::complex<double> cd_raw;
class cd;
typedef std::vector<cd> cd_vector;

class cd: public cd_raw {
public:
  using cd_raw::cd_raw;
  cd():cd_raw(0.0) { }
  //cd(int i):cd_raw((double) i, 0.0) { }

  cd(cd_raw z):cd_raw(z) { }


  std::string stringify() {
    std::stringstream ss;
    ss << std::setprecision (4) << real();
    if (imag() != 0.0) ss  << " + " << imag() << "i ";
    return ss.str();
  }

  static cd_vector linspace(cd start, cd end, int num_real, int num_imag = 1);
  static cd_vector logspace(cd start, cd end, int num_real, int num_imag = 1);

};

cd operator*(const double& x, const cd& z)
{
  return cd(x*z.real(),x*z.imag());
}

cd operator+(const double& x, const cd& z)
{
  return cd(x+z.real(),z.imag());
}

cd operator-(const double& x, const cd& z)
{
  return cd(x-z.real(),-z.imag());
}

cd operator/(const cd& z, const double& x) {
  return cd(z.real()/x,z.imag()/x);
}

const cd I(0.,1.);


cd_vector cd::linspace(cd start, cd end, int num_real, int num_imag)
{
  cd_vector linspaced;
  for (int r = 0; r < num_real; r++)
  {
    double real_spaced = std::real(start+r*((end-start)/num_real));
    for (int i = 0; i < num_imag; i++)
    {
      double imag_spaced = std::imag(start+i*((end-start)/num_real));
      linspaced.push_back(real_spaced + I*imag_spaced);
    }
  }
  return linspaced;
}

cd_vector cd::logspace(cd start, cd end, int num_real, int num_imag)
{
  cd_vector logspaced;
  cd_vector real_pows = linspace(log10(start.real()), log10(end.real()), num_real);
  cd_vector imag_pows = linspace(log10(start.imag()), log10(end.imag()), num_imag);
  for (int r = 0; r < num_real; r++)
  {
    for (int i = 0; i < num_imag; i++)
    {
      logspaced.push_back(pow(10.,real_pows.at(r))+I*pow(10., imag_pows.at(i)) );
    }
  }
  return logspaced;
}

#endif
