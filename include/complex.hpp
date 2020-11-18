#ifndef ZTD_COMPLEX_HPP
#define ZTD_COMPLEX_HPP

#include <iostream>

#include <math.h>

/*! @file complex.hpp
*  @brief Complex numbers and related computation
*/

namespace ztd
{
  //! @brief Complex number
  class complex
  {
  public:
    //! @brief Contructor a+bi
    complex(const double& a=0, const double& b=0);
    //! @brief Contructor z
    complex(const complex& z);

    inline complex& operator=(const complex& z) { real=z.real; im=z.im; return *this;}

    //! @brief Complex Addition
    complex& operator+=(complex z);
    //! @brief Complex Substraction
    complex& operator-=(complex z);
    //! @brief Complex Multiplication
    complex& operator*=(complex z);
    //! @brief Complex Division
    complex& operator/=(complex z);


    //! @brief Real part
    double real;
    //! @brief Imaginary part
    double im;

    inline double mod() { return sqrt(real*real + im*im); }
    double arg() { return atan2(real, im); }

  };

  //! @brief Complex Addition
  inline complex operator+(complex z1, const complex& z2) { return z1 += z2; }
  //! @brief Complex Substraction
  inline complex operator-(complex z1, const complex& z2) { return z1 -= z2; }
  //! @brief Complex Multiplication
  inline complex operator*(complex z1, const complex& z2) { return z1 *= z2; }
  //! @brief Complex Division
  inline complex operator/(complex z1, const complex& z2) { return z1 /= z2; }

  //! @brief Output a+bi
  inline std::ostream& operator<<(std::ostream& stream, complex z) { stream << z.real << '+' << z.im << 'i'; return stream; }

  //! @brief Constant i definition
  extern const complex i;

}

#endif //ZTD_COMPLEX_HPP
