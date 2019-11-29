#ifndef COLOR_HPP
#define COLOR_HPP

#include <iostream>

#define BOLD            "\033[0;02m"
#define ITALIC          "\033[0;03m"
#define UNDERLINE       "\033[0;04m"
#define BLINK           "\033[0;05m"
#define STRIKE          "\033[0;09m"

#define BOLD_ITALIC     "\033[1;03m"
#define BOLD_UNDERLINE  "\033[1;04m"
#define BOLD_BLINK      "\033[1;05m"
#define BOLD_STRIKE     "\033[1;09m"

#define COLOR_RESET     "\033[0;0m"

//! Number of predefined colors
#define COLOR_COUNT 19

/*! @file color.hpp
*  @brief Color management
*/

namespace ztd
{
  //! ANSI color management
  /*! C++ Class for easy ANSI color management and display
  */
  class color
  {
  public:
    //! Enumeration of preset colors
    /*!
    * Values: none,  white, gray, black, red, green, orange, blue, magenta, cyan,
    * b_white, b_lightgray, b_gray, b_red, b_green, b_yellow, b_blue, b_magenta, b_cyan
    *
    */
    enum color_name {
      none=0,
      white=1,
      gray=2,
      black=3,
      red=4,
      green=5,
      orange=6,
      blue=7,
      magenta=8,
      cyan=9,
      //bold
      b_white=10,
      b_lightgray=11,
      b_gray=12,
      b_red=13,
      b_green=14,
      b_yellow=15,
      b_blue=16,
      b_magenta=17,
      b_cyan=18
    };

    //! String color names
    /*!
    Values "none",  "white", "gray", "black", "red", "green", "orange", "blue", "magenta", "cyan",
    * "b_white", "b_lightgray", "b_gray", "b_red", "b_green", "b_yellow", "b_blue", "b_magenta", "b_cyan"
    */
    static const char* color_name_index[COLOR_COUNT];

    static const char* color_index[COLOR_COUNT];

    static const char* getColorCode(const unsigned int index);
    //!Get color code with of enum
    /*! @return Returns corresponding color code to the input enum color\n
    * Returns color none in case of failure
    @see color_name
    */
    static const char* getColorCode(color_name cm);
    //!Get color code with of string
    /*! @return Returns corresponding color code to the input string name\n
    * Returns color none in case of failure
    @see color_name_index
    */
    static const char* getColorCode(const std::string& str);

    //! @brief Constructor
    color();
    //! @brief Constructor with initial value
    color(color_name cm);
    //! @brief Constructor with initial value
    color(const std::string& name);

    //!Set color corresponding to enum value
    /*!
    Color set to none in case of failure
    @see color_name
    */
    void setColor(color_name cm);
    //!Set color with string name
    /*!
    Color set to none in case of failure
    @see color_name_index
    */
    void setColor(const std::string& name);

    //! Get color code
    /*! @return color in ANSI code
    */
    const char* code() const;

    //! Stored ansi color code
    std::string color_code;

    inline operator std::string() const { return color_code; };

  };

  //! Insert color code to stream
  inline std::ostream& operator<<(std::ostream& st, const ztd::color& c) { return st << c.code(); }
  //! Insert color code to stream
  inline std::ostream& operator<<(std::ostream& st, const ztd::color::color_name& c) { return st << ztd::color::color_index[c]; }
}

#endif //COLOR_HPP
