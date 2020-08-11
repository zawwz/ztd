#include "color.hpp"

const char* ztd::color::color_name_index[COLOR_COUNT] = {
  "none",
  "white",
  "gray",
  "black",
  "red",
  "green",
  "orange",
  "blue",
  "magenta",
  "cyan",
  //bold
  "b_white",
  "b_lightgray",
  "b_gray",
  "b_red",
  "b_green",
  "b_yellow",
  "b_blue",
  "b_magenta",
  "b_cyan"
  //format
  "bold",
  "italic",
  "underline",
  "blink",
  "strike"
};

const char* ztd::color::color_index[COLOR_COUNT] = {
  COLOR_RESET, //none
  "\033[37m", //white
  "\033[90m", //gray
  "\033[30m", //black
  "\033[31m", //red
  "\033[32m", //green
  "\033[33m", //orange
  "\033[34m", //blue
  "\033[35m", //magenta
  "\033[36m", //cyan
  // bold
  "\033[1;37m", //bold white
  "\033[1;02m", //bold light gray
  "\033[1;30m", //bold gray
  "\033[1;31m", //bold red
  "\033[1;32m", //bold green
  "\033[1;33m", //bold yellow
  "\033[1;34m", //bold blue
  "\033[1;35m", //bold magenta
  "\033[1;36m", //bold cyan
  // format
  BOLD,
  ITALIC,
  UNDERLINE,
  BLINK,
  STRIKE
};

const char* ztd::color::getColorCode(const unsigned int index)
{
  if( index < 0 || index >= COLOR_COUNT)
    return COLOR_RESET;
  else
    return ztd::color::color_index[index];
}

const char* ztd::color::getColorCode(ztd::color::color_name cm)
{
  return ztd::color::color_index[(int) cm];
}

const char* ztd::color::getColorCode(const std::string& str)
{
  for(unsigned int i=0; i<COLOR_COUNT ; i++)
  {
    if(color_name_index[i] == str)
    {
      return getColorCode(i);
    }
  }
  return COLOR_RESET;
}

ztd::color::color()
{
  setColor(none);
}

ztd::color::color(ztd::color::color_name cm)
{
  setColor(cm);
}

ztd::color::color(const std::string& name)
{
  setColor(name);
}

void ztd::color::setColor(ztd::color::color_name cm)
{
  color_code=getColorCode(cm);
}

void ztd::color::setColor(const std::string& name)
{
  color_code=getColorCode(name);
}

const char* ztd::color::code() const
{
  return color_code.c_str();
}
