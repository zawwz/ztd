#include "options.hpp"

#include <string.h>
#include <stdlib.h>

ztd::option_error::option_error(error_type type, const std::string& option)
{
  opt=option;
  errtype=type;
  switch(type)
  {
    case unknown_option : msg = "Unknown option: " + opt; break;
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

ztd::option::option(const std::string& str, bool arg, std::string helptext, std::string argname)
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

ztd::option::option(char c, const std::string& str, bool arg, std::string helptext, std::string argname)
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

ztd::option* ztd::option_set::find(const char c)
{
  for( auto it=option_vec.begin() ; it!=option_vec.end() ; it++ )
  {
    if(it->shortDef && it->charName == c)
      return &*it;
  }
  return nullptr;
}

ztd::option* ztd::option_set::find(const std::string& str)
{
  for( auto it=option_vec.begin() ; it!=option_vec.end() ; it++ )
  {
    if(it->longDef && it->strName == str)
      return &*it;
  }
  return nullptr;
}

std::vector<std::string> ztd::option_set::process(std::vector<std::string> arguments, bool ignore_numbers, bool stop_on_argument, bool ignore_unknown)
{
  std::vector<std::string> out;
  unsigned int i=0;
  option_sequence.clear();
  for( auto it = arguments.begin(); it!=arguments.end() ; it++ )
  {

    // arg is -- option and not empty
    if( it->size()>2 && it->substr(0,2) == "--" )
    {
      std::size_t eqn=it->find('=');
      if(eqn == std::string::npos) // no arg =
      {
        ztd::option* popt = this->find( it->substr(2,eqn-2) );
        if(popt == nullptr) // unknown -- opt
        {
          if(!ignore_unknown)
            throw ztd::option_error(ztd::option_error::unknown_option, "--" + it->substr(2,eqn-2));
          // add to ret if ignore
          out.push_back(*it);
        }
        else if(popt->takesArgument) // arg needed
        {
          if( ++it == arguments.end() ) // no arg given
            throw ztd::option_error(ztd::option_error::missing_arg, "--" + popt->strName);
          // take next arg as arg
          popt->activated = true;
          popt->argument = (*it);
          option_sequence.push_back(*popt);
        }
        else // no arg needed
        {
          popt->activated = true;
          option_sequence.push_back(*popt);
        }
      }
      else // there is =
      {
        ztd::option* popt = this->find( it->substr(2,eqn-2) ); // get option
        if(popt == nullptr) // unknown -- opt
        {
          if(!ignore_unknown)
            throw ztd::option_error(ztd::option_error::unknown_option, "--" +it->substr(2,eqn-2));
          // add to ret if ignore
          out.push_back(*it);
        }
        else
        {
          if(!popt->takesArgument) //option doesn't take arg
            throw ztd::option_error(ztd::option_error::takes_no_arg, "--" + popt->strName);
          // take = as arg
          popt->activated = true;
          popt->argument = it->substr(eqn+1,it->size()-eqn-1 );
          option_sequence.push_back(*popt);
        }
      }
    }
    //arg is - option and not empty
    else if(it->size()>1 && (*it)[0] == '-' && (*it)[1] != '-')
    {
      i=1;
      if( ignore_numbers && (*it)[i] >= '0' && (*it)[i] <= '9') // ignore numbers : add as ret arg
      {
        while(it!=arguments.end() && it->size()>i)
          i++;
        if(stop_on_argument)
          return std::vector<std::string>(it, arguments.end());
        out.push_back(*it);
      }
      else // opt
      {
        ztd::option* popt=nullptr;
        bool tstop=false;
        std::string ropt; // ignored opts
        while( !tstop && it!=arguments.end() && it->size()>i ) // iterate all chars of arg
        {
          popt=this->find((*it)[i]);
          if(popt==nullptr) // unknown opt
          {
            if(!ignore_unknown)
              throw ztd::option_error(ztd::option_error::unknown_option, std::string("-") + (*it)[i] );
          // add to ret if ignore
          ropt += (*it)[i];
          }
          else if(popt->takesArgument) // needs arg
          {
            i++;
            if(it->size()<=i) // finishes here
            {
              if( ++it == arguments.end() ) // no arg given
                throw ztd::option_error(ztd::option_error::missing_arg, std::string("-") +  popt->charName );
              // take next arg as arg
              popt->activated = true;
              popt->argument = (*it);
              option_sequence.push_back(*popt);
              tstop = true;
            }
            else // take rest as arg
            {
              if( (*it)[i] == '=') // ignore = char
                i++;
              popt->argument = it->substr(i , it->size()-i );
              popt->activated = true;
              option_sequence.push_back(*popt);
              tstop=true;
            }
          }
          else // needs no arg
          {
            popt->activated = true;
            option_sequence.push_back(*popt);
          }
          i++;
        } //while

        if(ropt.size() > 0) // ignored opts: append them to ret
          out.push_back("-" + ropt);

      } // if not number
    } // if opt
    else
    {
      if(stop_on_argument)
        return std::vector<std::string>(it, arguments.end());
      if( *it == "--" ) // empty -- : stop here
      {
        out.insert(out.end(), ++it, arguments.end());
        return out;
      }
      out.push_back(*it);
    }
    if(it == arguments.end())
      break;
  }
  return out;
}
