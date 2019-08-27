#ifndef COMPLEX_HPP
#define COMPLEX_HPP

#include <iostream>

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

    //! @brief Complex Addition
    complex& operator+=(const complex& z);
    //! @brief Complex Substraction
    complex& operator-=(const complex& z);
    //! @brief Complex Multiplication
    complex& operator*=(const complex& z);
    //! @brief Complex Division
    complex& operator/=(const complex& z);

    //! @brief Real part
    double real;
    //! @brief Imaginary part
    double im;

  };

  //! @brief Complex Addition
  inline complex operator+(const complex& z1, const complex& z2) { complex z3(z1); return z3 += z1; }
  //! @brief Complex Substraction
  inline complex operator-(const complex& z1, const complex& z2) { complex z3(z1); return z3 -= z1; }
  //! @brief Complex Multiplication
  inline complex operator*(const complex& z1, const complex& z2) { complex z3(z1); return z3 *= z1; }
  //! @brief Complex Division
  inline complex operator/(const complex& z1, const complex& z2) { complex z3(z1); return z3 /= z1; }

  //! @brief Output a+bi
  inline std::ostream& operator<<(std::ostream& stream, complex z) { stream << z.real << '+' << z.im << 'i'; return stream; }

  //! @brief Constant i definition
  extern const complex i;

}

#endif //COMPLEX_HPP
