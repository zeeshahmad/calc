#ifndef COMPLEX_DOUBLE_H
#define COMPLEX_DOUBLE_H

#include <complex>
#include <vector>
#include <iostream>
#include <string>

using namespace std;

//order of the following matters
typedef complex<double> cd_raw;
class cd;
typedef vector<cd> cd_vector;

class cd: public cd_raw {
public:
  using cd_raw::cd_raw;

  cd(int i):cd_raw((double) i, 0.0) { }

  cd(cd_raw z):cd_raw(z) { }


  string string_form() {
    return to_string(real())+(imag()==0.0 ? "":" + "+to_string(imag())+"i");
  }

  static cd_vector linspace(cd start, cd end, int num_real, int num_imag = 1);

};

void print(const cd& z, string name) {
  cout << scientific << name << ": " << z.real() << " + "<<z.imag() <<"i "<<endl;
}


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

#endif
