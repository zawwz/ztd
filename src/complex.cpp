#include "complex.hpp"

const ztd::complex ztd::i(0,1);

ztd::complex::complex(const double& a, const double& b)
{
  real=a;
  im=b;
}
ztd::complex::complex(const ztd::complex& z)
{
  real=z.real;
  im=z.im;
}

ztd::complex& ztd::complex::operator+=(ztd::complex z)
{
  real += z.real;
  im += z.im;
  return *this;
}
ztd::complex& ztd::complex::operator-=(ztd::complex z)
{
  real -= z.real;
  im -= z.im;
  return *this;
}
ztd::complex& ztd::complex::operator*=(ztd::complex z)
{
  double r=real;
  real = real*z.real - im*z.im;
  im = r*z.im + im*z.real;
  return *this;
}
ztd::complex& ztd::complex::operator/=(ztd::complex z)
{
  double r=real;
  double div = z.real*z.real + z.im*z.im;
  real = (real*z.real + im*z.im)/div;
  im   = (im*z.real - r*z.im)/div;
  return *this;
}
