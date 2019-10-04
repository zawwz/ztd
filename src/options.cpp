#include "options.hpp"

#include <string.h>
#include <stdlib.h>

ztd::option_error::option_error(error_type type, const std::string& option)
{
  opt=option;
  switch(type)
  {
    case unknown_option : msg = "Unkown option: " + opt; break;
    case takes_no_arg : msg = "Option " + opt + " doesn't take an argument"; break;
    case missing_arg : msg = "Option " + opt + " needs an argument"; break;
  }
}

std::vector<std::string> ztd::argVector(int argc, char** argv)
{
  std::vector<std::string> out;
  for(int i=1;i<argc; i++)
  {
    out.push_back(std::string(argv[i]));
  }
  return out;
}

ztd::option::option()
{
  shortDef=false;
  longDef=false;
  takesArgument=false;
  activated=false;
  charName=0;
}

ztd::option::option(char c, bool arg, std::string helptext, std::string argname)
{
  //char
  shortDef=true;
  charName=c;
  //string
  longDef=false;
  strName="";
  //arg
  takesArgument=arg;
  arg_name=argname;
  //help
  help_text=helptext;
  //init
  activated=false;
}

ztd::option::option(std::string const& str, bool arg, std::string helptext, std::string argname)
{
  //char
  shortDef=false;
  charName=0;
  //string
  longDef=true;
  strName=str;
  //arg
  takesArgument=arg;
  arg_name=argname;
  //help
  help_text=helptext;
  //init
  activated=false;
}

ztd::option::option(char c, std::string const& str, bool arg, std::string helptext, std::string argname)
{
  //char
  shortDef=true;
  charName=c;
  //string
  longDef=true;
  strName=str;
  //arg
  takesArgument=arg;
  arg_name=argname;
  //help
  help_text=helptext;
  //init
  activated=false;
}

ztd::option::option(const option& opt)
{
  //char
  shortDef=opt.shortDef;
  charName=opt.charName;
  //string
  longDef=opt.longDef;
  strName=opt.strName;
  //arg
  takesArgument=opt.takesArgument;
  arg_name=opt.arg_name;
  //help
  help_text=opt.help_text;
  //init
  activated=opt.activated;
  argument=opt.argument;
}

void ztd::option::print_help(int leftpad, int rightpad) const
{
  //prepadding
  printf("%*s", -1*leftpad, "");

  //short def
  if(this->shortDef)
  {
    printf("-%c ", this->charName);
    rightpad -= 3;
  }

  //longdef
  if(this->longDef)
  {
    printf("--%s ", this->strName.c_str());
    rightpad -= 3 + this->strName.size();
  }

  //argument
  if(this->takesArgument)
  {
    printf("<%s>", arg_name.c_str());
    rightpad -= arg_name.size() + 2;
  }

  printf("%*s%s", -1*rightpad, "", help_text.c_str());

  printf("\n");
}

void ztd::option_set::print_help(int leftpad, int rightpad) const
{
  for(auto it : this->option_vec)
    it.print_help(leftpad,rightpad);
}

ztd::option* ztd::option_set::find(char c)
{
  for( auto it=option_vec.begin() ; it!=option_vec.end() ; it++ )
  {
    if(it->shortDef && it->charName == c)
      return &*it;
  }
  return nullptr;
}

ztd::option* ztd::option_set::find(std::string const& str)
{
  for( auto it=option_vec.begin() ; it!=option_vec.end() ; it++ )
  {
    if(it->longDef && it->strName == str)
      return &*it;
  }
  return nullptr;
}

std::vector<std::string> ztd::option_set::process(std::vector<std::string> arguments, bool ignore_numbers)
{
  std::vector<std::string> out;
  unsigned int i=0;
  option_sequence.clear();
  for( auto it = arguments.begin(); it!=arguments.end() ; it++ )
  {
    if( (*it).size()>0 && (*it)[0]=='-' )
    {
      if((*it).size()>1 && (*it)[1]=='-')
      {
        std::size_t eqn=(*it).find('=');
        if(eqn == std::string::npos)
        {
          ztd::option* popt = this->find( (*it).substr(2,eqn-2) );
          if(popt == nullptr)
          {
            throw ztd::option_error(ztd::option_error::unknown_option, "--" + (*it).substr(2,eqn-2));
          }
          if(popt->takesArgument)
          {
            if( ++it == arguments.end() ) //finishes here
            {
              throw ztd::option_error(ztd::option_error::missing_arg, "--" + popt->strName);
            }
            popt->activated = true;
            popt->argument = (*it);
            option_sequence.push_back(*popt);
          }
          else
          {
            popt->activated = true;
            option_sequence.push_back(*popt);
          }
        }
        else
        {
          ztd::option* popt = this->find( (*it).substr(2,eqn-2) );
          if(popt == nullptr)
          {
            throw ztd::option_error(ztd::option_error::unknown_option, "--" +(*it).substr(2,eqn-2));
          }
          if(!popt->takesArgument)
          {
            throw ztd::option_error(ztd::option_error::takes_no_arg, "--" + popt->strName);
          }
          popt->activated = true;
          popt->argument = (*it).substr(eqn+1,(*it).size()-eqn-1 );
          option_sequence.push_back(*popt);
        }
      }
      else
      {
        i=1;
        if( ignore_numbers && (*it)[i] >= '0' && (*it)[i] <= '9')
        {
          while(it!=arguments.end() && (*it).size()>i)
            i++;
          out.push_back(*it);
        }
        else
        {
          ztd::option* popt=nullptr;
          bool tstop=false;
          while( !tstop && it!=arguments.end() && (*it).size()>i )
          {
            popt=this->find((*it)[i]);
            if(popt==nullptr) //not found: error
            {
              throw ztd::option_error(ztd::option_error::unknown_option, std::string("-") + (*it)[i] );
            }
            if(popt->takesArgument) //no argument
            {
              i++;
              if((*it).size()<=i) //finishes here
              {
                if( ++it == arguments.end() )
                {
                  throw ztd::option_error(ztd::option_error::missing_arg, std::string("-") +  popt->charName );
                }
                popt->activated = true;
                popt->argument = (*it);
                option_sequence.push_back(*popt);
                tstop = true;
              }
              else //continue
              {
                if( (*it)[i] == '=')
                  i++;
                popt->argument = (*it).substr(i , (*it).size()-i );
                popt->activated = true;
                option_sequence.push_back(*popt);
                tstop=true;
              }
            }
            else //no argument
            {
              popt->activated = true;
              option_sequence.push_back(*popt);
            }
            i++;
          } //while
        } // if not number
      }
    }
    else
    {
      out.push_back(*it);
    }
    if(it == arguments.end())
      break;
  }
  return out;
}
