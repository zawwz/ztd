#include "filedat.hpp"

#include <algorithm>

// Function code
bool ztd::filedat::isRead(char in)
{
  return in>=33 && in<=126;
}

static std::string repeatString(const std::string& str, const unsigned int n)
{
  std::string ret;
  for(unsigned int i=0 ; i<n ; i++)
    ret += str;
  return ret;
}

static std::string escape(std::string str, const char c)
{
  size_t pos = str.find(c);
  while(pos != std::string::npos)
  {
    str.insert(pos, "\\");
    pos += 2;
    pos = str.find(c, pos);
  }
  return str;
}

void ztd::printErrorIndex(const char* in, const int index, const std::string& message, const std::string& origin)
{
  int i=0, j=0; // j: last newline
  int line=1; //n: line #
  int in_size=strlen(in);
  if(index >= 0)
  {
    while(i < in_size && i < index)
    {
      if(in[i] == '\n')
      {
        line++;
        j=i+1;
      }
      i++;
    }
    while(i < in_size && in[i]!='\n')
    {
      i++;
    }
  }
  if(origin != "")
  {
    std::cerr << origin << ": Error\nLine " << line << " col " << index-j+1 << ": " << message << std::endl;
    std::cerr << std::string(in+j, i-j) << std::endl;
    std::cerr << repeatString(" ", index-j) << '^' << std::endl;
  }
  else
  {
    std::cerr << "Format Error: " << message << std::endl;
    if(index >= 0)
    {
      std::cerr << std::string(in, i) << std::endl;
      std::cerr << repeatString(" ", index-j) << '^' << std::endl;
    }
    else
    std::cerr << in << std::endl;
  }
}

std::string ztd::filedat::removeComments(std::string str)
{
  uint32_t i=0;
  while(i < str.size())
  {
    if( str[i] == '\\')
    {
      //skip checking char
      i++;
    }
    else if( str[i] == '"') // double quotes
    {
      uint32_t j=i;
      i++;
      while(i < str.size() && str[i]!='"') // until end of quote
      {
        if(i+1 < str.size() && str[i] == '\\' && str[i+1] == '"') //escaped quote
          i++; //ignore backslash

        i++; // add char and increment
      }
      if(i >= str.size()) // quote didn't end
        throw ztd::format_error("Double quote doesn't close", "", str, j);
      i++;
    }
    else if( str[i] == '\'') // single quotes
    {
      uint32_t j=i;
      i++;
      while(i < str.size() && str[i]!='\'') // until end of quote
      {
        if(i+1 < str.size() && str[i] == '\\' && str[i+1] == '\'') //escaped quote
          i++; //ignore backslash

        i++; // add char
      }
      if(i >= str.size()) // quote didn't end
        throw ztd::format_error("Single quote doesn't close", "", str, j);
      i++;
    }
    else if(str[i] == '#' || (i+1 < str.size() && str.substr(i,2) == "//")) // comment
    {
      uint32_t j=i;
      i = str.find('\n', i);
      str.erase(j,i-j);
      i=j;
    }
    i++;
  }
  return str;
}

ztd::filedat::filedat()
{
  m_dataChunk = new ztd::chunkdat();
}

ztd::filedat::filedat(std::string const& in)
{
  m_dataChunk = new ztd::chunkdat();
  m_filePath=in;
}

ztd::filedat::~filedat()
{
  if(m_dataChunk!=nullptr)
    delete m_dataChunk;
}

void ztd::filedat::clear()
{
  m_data="";
  if(m_dataChunk!=nullptr)
  {
    delete m_dataChunk;
    m_dataChunk = new ztd::chunkdat();
  }
}

bool ztd::filedat::readTest() const
{
  std::ifstream stream(m_filePath);
  if(!stream)
    return false;
  else
    return true;
}

void ztd::filedat::import_file(const std::string& path)
{
  if(path != "")
    m_filePath=path;
  std::ifstream st(m_filePath);
  if(!st)
    throw std::runtime_error("Cannot read file '" + m_filePath + '\'');

  this->clear();
  std::string line;

  while(st)
  {
    getline(st, line);
    m_data += (line + '\n');
  }
  this->generateChunk();
}

void ztd::filedat::import_stdin()
{
  m_filePath="stdin";
  this->clear();
  std::string line;
  while(std::cin)
  {
    getline(std::cin, line);
    m_data += (line + '\n');
  }
  this->generateChunk();
}

void ztd::filedat::import_string(const std::string& data)
{
  this->clear();
  m_data=data;
  m_filePath="";
  this->generateChunk();
}

bool ztd::filedat::export_file(std::string const& path, std::string const& aligner) const
{
  std::ofstream stream;
  if(path=="")
    stream.open(m_filePath);
  else
    stream.open(path);
  if(!stream)
    return false;
  stream << this->strval(aligner);
  return true;
}

std::string ztd::filedat::strval(std::string const& aligner) const
{
  if(m_dataChunk == nullptr)
    return "";
  else
    return m_dataChunk->strval(0, aligner);
}

void ztd::filedat::generateChunk()
{
  try
  {
    if(m_dataChunk != nullptr)
      delete m_dataChunk;
    m_data = this->removeComments(m_data);
    m_dataChunk = new ztd::chunkdat(m_data, 0, nullptr);
  }
  catch(ztd::format_error& e)
  {
    m_dataChunk = nullptr;
    throw ztd::format_error(e.what(), m_filePath, m_data, e.where());
  }
}

std::pair<std::string, size_t> _skip(const std::string& str)
{
  size_t i=0;

  //skip to val start
  while( i < str.size() )
  {
    // start of value
    if(ztd::filedat::isRead(str[i]))
      break;
    i++;
  }
  std::string ret;
  if(i < str.size())
    ret = str.substr(i);
  else
  {
    i=str.size();
    ret="";
  }
  return std::make_pair(ret, i);
}


// value, rest, start of rest, start of value, delim found
static std::tuple<std::string, std::string, int, int, bool> _getstrval(const std::string& str, const char delim=0, const char altdelim=0)
{
  std::string val;

  //skip to start of val
  size_t i = _skip(str).second;
  int st = i;
  bool delim_found=false;

  // no value
  if(i >= str.size())
    return std::make_tuple("","",0,0,false);

  while(i < str.size())
  {
    if( str[i] == '"') // double quotes
    {
      uint32_t j=i;
      i++;
      while(i < str.size() && str[i]!='"') // until end of quote
      {
        if(i+1 < str.size() && str[i] == '\\' && str[i+1] == '"') //escaped quote
          i++; //ignore backslash

        val.push_back(str[i++]); // add char and increment
      }
      if(i >= str.size()) // quote didn't end
        throw ztd::format_error("Double quote doesn't close", "", str, j);
      i++;
    }
    else if( str[i] == '\'') // single quotes
    {
      uint32_t j=i;
      i++;
      while(i < str.size() && str[i]!='\'') // until end of quote
      {
        if(i+1 < str.size() && str[i] == '\\' && str[i+1] == '\'') //escaped quote
          i++; //ignore backslash

        val += str[i++]; // add char
      }
      if(i >= str.size()) // quote didn't end
        throw ztd::format_error("Single quote doesn't close", "", str, j);
      i++;
    }
    if(str[i] == '{') // {} map
    {
      uint32_t counter=0;
      uint32_t j=i;
      val += str[i++]; // add the bracket to the value
      while(i < str.size() && !(counter == 0 && str[i] == '}') )
      {
        if(str[i] == '}')
          counter--;
        else if(str[i] == '{')
          counter++;
        else if( str[i] == '"') // double quotes
        {
          uint32_t k=i;
          val += str[i++];
          while(i < str.size() && str[i]!='"') // until end of quote
          {
            if(i+1 < str.size() && str[i] == '\\' && str[i+1] == '"') //escaped quote
              val += str[i++];
            val += str[i++];
          }
          if(i >= str.size()) // quote didn't end
            throw ztd::format_error("Double quote does not close", "", str, k);
        }
        else if( str[i] == '\'') // single quotes
        {
          uint32_t k=i;
          val += str[i++];
          while(i < str.size() && str[i]!='\'') // until end of quote
          {
            if(i+1 < str.size() && str[i] == '\\' && str[i+1] == '\'') //escaped quote
              val += str[i++];
            val += str[i++];
          }
          if(i >= str.size()) // quote didn't end
            throw ztd::format_error("Single quote does not close", "", str, k);
        }
        val += str[i++];
      }

      if(i >= str.size()) //didn't close
        throw ztd::format_error("Brace does not close", "", str, j);

      val += str[i++]; // add brace
    }
    else if(str[i] == '[') // [] list
    {
      uint32_t counter=0;
      uint32_t j=i;
      val += str[i++]; // add the bracket to the value
      while(i < str.size() && !(counter == 0 && str[i] == ']') )
      {
        if(str[i] == '[')
          counter--;
        else if(str[i] == ']')
          counter++;
        else if( str[i] == '"') // double quotes
        {
          uint32_t k=i;
          val += str[i++];
          while(i < str.size() && str[i]!='"') // until end of quote
          {
            if(i+1 < str.size() && str[i] == '\\' && str[i+1] == '"') //escaped quote
              val += str[i++];
            val += str[i++];
          }
          if(i >= str.size()) // quote didn't end
            throw ztd::format_error("Double quote does not close", "", str, k);
        }
        else if( str[i] == '\'') // single quotes
        {
          uint32_t k=i;
          val += str[i++];
          while(i < str.size() && str[i]!='\'') // until end of quote
          {
            if(i+1 < str.size() && str[i] == '\\' && str[i+1] == '\'') //escaped quote
              val += str[i++];
            val += str[i++];
          }
          if(i >= str.size()) // quote didn't end
            throw ztd::format_error("Single quote does not close", "", str, k);
        }
        val += str[i++];
      }

      if(i >= str.size()) //didn't close
        throw ztd::format_error("Brace does not close", "", str, j);

      val += str[i++]; // add bracket
    }
    else if(!ztd::filedat::isRead(str[i])) // non read char
    {
      std::string tval="";
      if(delim == 0) // delim=0: stop at non read
        break;
      while(!ztd::filedat::isRead(str[i]) && !(str[i] == delim || str[i] == altdelim) ) // until read or delim
        tval += str[i++];
      if(str[i] == delim || str[i] == altdelim) // delim: stop
      {
        i++;
        break;
      }
      val += tval;
    }
    else // read char
    {
      if(str[i] == delim || str[i] == altdelim) // delim: stop without adding delim
      {
        i++;
        break;
      }

      val.push_back(str[i++]); //add char and increment
    }
  }
  std::string ret;
  if(i < str.size())
  {
    ret = str.substr(i);
    delim_found=true;
  }
  else
  {
    while( val.size() > 0 && !ztd::filedat::isRead(val[val.size()-1]) )
     val.pop_back();
    ret = "";
    delim_found=false;
    i=str.size();
  }
  return std::make_tuple(val, ret, i, st, delim_found); // return
}

// key, value, rest, start of rest, start of value, start of key
static std::tuple<std::string, std::string, std::string, int, int, int> _getkeyval(const std::string& in)
{
  std::string key, value, rstr;
  std::tuple<std::string, std::string, int, int, bool> tup;
  int dpos=-1, rpos=-1, valstart=-1, keystart=-1;
  bool eq_found=false;

  try // get key
  {
    tup=_getstrval(in, '=');
    key=std::get<0>(tup);
    rstr=std::get<1>(tup);
    dpos=std::get<2>(tup);
    keystart=std::get<3>(tup);
    eq_found=std::get<4>(tup);
  }
  catch(ztd::format_error& e)
  {
    ztd::format_error(e.what(), e.origin(), in, e.where());
    throw e;
  }
  if(key.size() > 0 && key[0] == ';') // ignore ; : new value
  {
    key.erase(key.begin());
    return std::make_tuple("", "", key+value+rstr, keystart+1, -1, -1);
  }
  if(key == "" && _skip(rstr).first != "") // key is empty
    throw ztd::format_error("Value has no Key", "", in, in.find(dpos) );
  if(key != "" && !eq_found) // no delim
    throw ztd::format_error("Key '"+key+"' has no value", "", in, keystart+key.size());

  try // get value
  {
    tup=_getstrval(rstr, ';', '\n');
    value=std::get<0>(tup);
    rstr=std::get<1>(tup);
    rpos=std::get<2>(tup);
    valstart = std::get<3>(tup);
  }
  catch(ztd::format_error& e)
  {
    ztd::format_error(e.what(), e.origin(), in, dpos+1+e.where());
    throw e;
  }

  int rr=-1;
  if(rpos >= 0)
    rr = dpos+rpos;
  else
    rr = in.size();

  return std::make_tuple(key, value, rstr, rr, valstart, keystart);

}

void ztd::chunkdat::set(const std::string& in, int offset, ztd::filedat* parent)
{
  this->clear();
  this->m_parent=parent;
  this->m_offset=offset;

  // isolate value
  auto tup = _getstrval(in); // any exception here is caught upwards
  std::string str = std::get<0>(tup);
  std::string rest = std::get<1>(tup);
  int i = std::get<2>(tup);
  int j = std::get<3>(tup);

  if(str == "") //empty: make an empty strval
  {
    ztd::chunk_string* cv = new ztd::chunk_string();
    m_achunk=cv;
    cv->val = "";
    return;
  }
  if ( str[0] == '{') // map
  {
    auto p = _skip(rest);
    if( p.first != "") //rest is not empty
      throw ztd::format_error("Unexpected char", "", in, i+p.second);

    str.erase(str.begin()); // remove first char '{'
    i = j+1;
    str.pop_back(); // remove last char '}'

    // create chunk
    ztd::chunk_map* tch = new ztd::chunk_map();
    m_achunk = tch;

    if(_skip(str).first == "") // empty map
      return;

    do
    {
      int keystart, valstart;
      std::string key, value;
      p = _skip(str);
      if(p.first[0] == '=')
        throw ztd::format_error("Value has no key", "", in, i+p.second);
      try // get one value
      {
        auto tup2 = _getkeyval(str);
        key=std::get<0>(tup2);              // key
        value=std::get<1>(tup2);            // value
        str=std::get<2>(tup2);              // rest
        valstart = i + std::get<4>(tup2);   // start of value
        keystart = i + std::get<5>(tup2);   // start of key
        i+=std::get<3>(tup2);               // add start of rest
      }
      catch(ztd::format_error& e)
      {
        throw ztd::format_error(e.what(), e.origin(), in, i+e.where());
      }
      if(key != "") // good value
      {
        try // insert value
        {
          ztd::chunkdat* chk = new ztd::chunkdat(value, offset + valstart, m_parent);
          if(!tch->values.insert( std::make_pair(key, chk ) ).second) // failed to insert
          {
            delete chk;
            throw ztd::format_error("Key '" + key + "' already present", "", in, keystart );
          }
        }
        catch(ztd::format_error& e)
        {
          throw ztd::format_error(e.what(), "", in, e.where() + valstart );
        }
      }
    }
    while(str != "");
  }
  else if ( str[0] == '[') // map
  {
    auto p = _skip(rest);
    if( p.first != "") //rest is not empty
      throw ztd::format_error("Unexpected char", "", in, i+p.second);

    str.erase(str.begin()); // remove first char '['
    i = j+1;
    str.pop_back(); // remove last char ']'

    // create chunk
    ztd::chunk_list* tch = new ztd::chunk_list();
    m_achunk = tch;

    if(_skip(str).first == "") // empty list
      return;

    do
    {
      int valstart;
      std::string value;
      try // get one value
      {
        auto tup2 = _getstrval(str, ',', ';');
        value=std::get<0>(tup2);            // value
        str=std::get<1>(tup2);              // rest
        valstart = i + std::get<3>(tup2);   // start of value
        i+=std::get<2>(tup2);               // add start of rest
      }
      catch(ztd::format_error& e)
      {
        throw ztd::format_error(e.what(), e.origin(), in, e.where() + i);
      }
      try
      {
        tch->list.push_back(new ztd::chunkdat(value, offset + valstart, m_parent) );
      }
      catch(ztd::format_error& e)
      {
        throw ztd::format_error(e.what(), e.origin(), in, e.where() + valstart );
      }
    }
    while(str != "");
  }
  else
  {
    ztd::chunk_string* cv = new ztd::chunk_string();
    m_achunk=cv;
    cv->val = in;
  }
}

void ztd::chunkdat::set(ztd::chunkdat const& in)
{
  // reset everything
  this->clear();
  // trace info
  m_offset=in.m_offset;
  m_parent=in.m_parent;

  // case copy
  if(in.type()==ztd::chunk_abstract::map) //map
  {
    ztd::chunk_map* cc = dynamic_cast<chunk_map*>(in.getp());
    ztd::chunk_map* tch = new ztd::chunk_map();
    for(auto it : cc->values)
    {
      tch->values.insert( std::make_pair(it.first, it.second->pcopy()) );
    }
    m_achunk=tch;
  }
  else if(in.type()==ztd::chunk_abstract::list) //list
  {
    ztd::chunk_list* cc = dynamic_cast<chunk_list*>(in.getp());
    ztd::chunk_list* tch = new ztd::chunk_list();
    for(auto it : cc->list)
    {
      tch->list.push_back(it->pcopy());
    }
    m_achunk=tch;
  }
  else if(in.type()==ztd::chunk_abstract::string) //string
  {
    ztd::chunk_string* cc = dynamic_cast<chunk_string*>(in.getp());
    ztd::chunk_string* tch = new ztd::chunk_string();
    tch->val = std::string(cc->val);
    m_achunk = tch;
  }
}

void ztd::chunkdat::addToMap(std::string const& name, chunkdat const& val)
{
  if(this->type()==ztd::chunk_abstract::map)
  {
    ztd::chunk_map* cp = dynamic_cast<chunk_map*>(m_achunk);
    ztd::chunkdat* chk = new ztd::chunkdat(val);
    if( !cp->values.insert( std::make_pair(name,chk) ).second )
    {
      delete chk;
      throw ztd::format_error("Key '" + name + "' already present", "", this->strval(), -1);
    }
  }
  else if(this->type() == ztd::chunk_abstract::none)
  {
    ztd::chunk_map* cp = new ztd::chunk_map();
    m_achunk=cp;
    cp->values.insert(std::make_pair(name , new ztd::chunkdat(val)));
  }
  else
  {
    throw ztd::format_error("Cannot add keys to non-map chunks", "", this->strval(), -1);
  }
}

void ztd::chunkdat::addToMap(std::vector<std::pair<std::string, chunkdat>> const& vec)
{
  for(auto it : vec)
    this->addToMap(it.first, it.second);
}

void ztd::chunkdat::addToList(chunkdat const& val)
{
  if(this->type()==ztd::chunk_abstract::list)
  {
    ztd::chunk_list* lp = dynamic_cast<chunk_list*>(m_achunk);
    lp->list.push_back(new ztd::chunkdat(val));
  }
  else if(this->type() == ztd::chunk_abstract::none)
  {
    ztd::chunk_list* lp = new ztd::chunk_list();
    m_achunk=lp;
    lp->list.push_back(new ztd::chunkdat(val));
  }
  else
  {
    throw ztd::format_error("Cannot add elements to non-list chunks", "", this->strval(), -1);
  }
}

void ztd::chunkdat::addToList(std::vector<chunkdat> const& vec)
{
  for(auto it : vec)
    this->addToList(it);
}

void ztd::chunkdat::concatenate(chunkdat const& chk)
{
  if(this->type() == ztd::chunk_abstract::none) //nothing: copy
  {
    this->set(chk);
  }
  else if(this->type()==ztd::chunk_abstract::map && chk.type()==ztd::chunk_abstract::map) //map
  {
    ztd::chunk_map* cc = dynamic_cast<chunk_map*>(chk.getp());
    for(auto it : cc->values)
    {
      this->add(it.first, *it.second);
    }
  }
  else if(this->type()==ztd::chunk_abstract::list && chk.type()==ztd::chunk_abstract::list) //list
  {
    ztd::chunk_list* cc = dynamic_cast<chunk_list*>(chk.getp());
    for(auto it : cc->list)
    {
      this->add(*it);
    }
  }
  else if(this->type()==ztd::chunk_abstract::string && chk.type()==ztd::chunk_abstract::string) //string
  {
    ztd::chunk_string* ci = dynamic_cast<chunk_string*>(chk.getp());
    ztd::chunk_string* cc = dynamic_cast<chunk_string*>(m_achunk);
    cc->val += ci->val;
  }
  else
  {
    throw ztd::format_error("Cannot concatenate chunks of different types", "", "", -1);
  }
}

// overwrite: chk replaces this when conflict
void ztd::chunkdat::merge(chunkdat const& chk, bool overwrite)
{
  if(this->type() == ztd::chunk_abstract::none) //nothing: copy
  {
    this->set(chk);
  }
  else if(this->type()==ztd::chunk_abstract::map && chk.type()==ztd::chunk_abstract::map) //map
  {
    ztd::chunk_map* ci = dynamic_cast<chunk_map*>(chk.getp());
    ztd::chunk_map* cc = dynamic_cast<chunk_map*>(m_achunk);
    for(auto it: ci->values) // iterate keys
    {
      auto fi = cc->values.find(it.first);
      if(fi == cc->values.end()) // new key
      {
        this->addToMap(it.first, *it.second);
      }
      else // key already present
      {
        fi->second->merge(*it.second, overwrite); // merge subchunks
      }
    }
    //map merge
  }
  else if(this->type()==ztd::chunk_abstract::list && chk.type()==ztd::chunk_abstract::list) //list
  {
    ztd::chunk_list* ci = dynamic_cast<chunk_list*>(chk.getp());
    for(auto it : ci->list)
    {
      this->add(*it);
    }
  }
  else if(this->type()==ztd::chunk_abstract::string && chk.type()==ztd::chunk_abstract::string) //string
  {
    ztd::chunk_string* cc = dynamic_cast<chunk_string*>(m_achunk);
    if(overwrite)
      cc->val = chk.str();
    else
      throw ztd::format_error("Cannot merge string chunks", "", "", -1);
  }
  else
  {
    if(overwrite)
      this->set(chk);
    else
      throw ztd::format_error("Cannot merge chunks of different types", "", "", -1);
  }
}


void ztd::chunkdat::erase(const std::string& key)
{
  if(this->type()==ztd::chunk_abstract::map)
  {
    ztd::chunk_map* cp = dynamic_cast<chunk_map*>(m_achunk);
    auto it = cp->values.find(key);
    if( it == cp->values.end() )
    {
      throw ztd::format_error("Key '" + key + "' not present", "", this->strval(), -1);
    }
    delete it->second;
    cp->values.erase(it);
  }
  else
  {
    throw ztd::format_error("Cannot erase element from non-map chunk", "", this->strval(), -1);
  }
}

void ztd::chunkdat::erase(const unsigned int index)
{
  if(this->type()==ztd::chunk_abstract::list)
  {
    if(index >= (unsigned int) this->listSize())
    {
      throw ztd::format_error("Cannot erase out of bonds: "+std::to_string(index)+" in size "+std::to_string(this->listSize()), "", this->strval(), -1);
    }
    ztd::chunk_list* lp = dynamic_cast<chunk_list*>(m_achunk);
    delete lp->list[index];
    lp->list.erase(lp->list.begin() + index);
  }
  else
  {
    throw ztd::format_error("Cannot erase element from non-list chunk", "", this->strval(), -1);
  }
}

std::vector<ztd::chunkdat*> ztd::chunkdat::getlist()
{
  if(this->type()!=ztd::chunk_abstract::list)
  {
    if(m_parent != nullptr)
      throw ztd::format_error("chunkdat isn't a list", m_parent->filePath(), m_parent->im_data(), m_offset );
    else
      throw ztd::format_error("chunkdat isn't a list", "", this->strval(), -1);
  }
  ztd::chunk_list* cl = dynamic_cast<chunk_list*>(m_achunk);
  return cl->list;
}
std::map<std::string, ztd::chunkdat*> ztd::chunkdat::getmap()
{
  if(this->type()!=ztd::chunk_abstract::map)
  {
    if(m_parent != nullptr)
      throw ztd::format_error("chunkdat isn't a map", m_parent->filePath(), m_parent->im_data(), m_offset );
    else
      throw ztd::format_error("chunkdat isn't a map", "", this->strval(), -1);
  }
  ztd::chunk_map* dc = dynamic_cast<chunk_map*>(m_achunk);
  return dc->values;
}

std::string ztd::chunkdat::strval(unsigned int alignment, std::string const& aligner) const
{
  if(this->type()==ztd::chunk_abstract::string)
  {
    ztd::chunk_string* vp = dynamic_cast<chunk_string*>(m_achunk);

    return vp->val;
  }
  else if(this->type()==ztd::chunk_abstract::map)
  {
    ztd::chunk_map* cp = dynamic_cast<chunk_map*>(m_achunk);
    if(cp->values.size() <= 0)
      return "{}";
    std::string ret="{\n";
    for(auto it : cp->values)
    {
      ret += repeatString(aligner,alignment+1);
      ret += it.first;
      ret += " = ";
      if(it.second!=nullptr)
      {
        if(it.second->type() == ztd::chunk_abstract::string)
        {
          ret += "\"" + escape(it.second->strval(), '"') + "\"";
        }
        else
        {
          ret += it.second->strval(alignment+1, aligner);
        }
      }
      ret += '\n';
    }
    ret += repeatString(aligner, alignment);
    ret += '}';
    return ret;
  }
  else if(this->type()==ztd::chunk_abstract::list)
  {
    ztd::chunk_list* lp = dynamic_cast<chunk_list*>(m_achunk);
    if(lp->list.size() <= 0)
      return "[]";
    std::string ret="[\n";
    for(auto it : lp->list)
    {
      ret += repeatString(aligner, alignment+1);
      if(it!=nullptr)
      {
        if(it->type() == ztd::chunk_abstract::string)
        {
          ret += '"' + escape(it->strval(), '"') + '"';
        }
        else
        {
          ret += it->strval(alignment+1, aligner);
        }
      }
      ret += ",\n";
    }
    ret.erase(ret.end()-2);
    ret += repeatString(aligner, alignment);
    ret += ']';
    return ret;
  }
  else
  return "";
}

int ztd::chunkdat::listSize() const
{
  if(this->type() != ztd::chunk_abstract::list)
  return -1;
  ztd::chunk_list* cl = dynamic_cast<chunk_list*>(m_achunk);
  return cl->list.size();
}

ztd::chunkdat* ztd::chunkdat::subChunkPtr(std::string const& in) const
{
  if(this->type()==ztd::chunk_abstract::map)
  {
    ztd::chunk_map* dc = dynamic_cast<chunk_map*>(m_achunk);
    auto fi = dc->values.find(in);
    if(fi == dc->values.end()) //none found
      return nullptr;
    return fi->second;
  }
  else //not a chunk
  {
    return nullptr;
  }
}

ztd::chunkdat* ztd::chunkdat::subChunkPtr(const unsigned int a) const
{
  if(this->type()==ztd::chunk_abstract::list)
  {
    ztd::chunk_list* cl = dynamic_cast<chunk_list*>(m_achunk);
    if(a >= cl->list.size()) //outside of range
    return nullptr;
    return cl->list[a];
  }
  else //not a list
  {
    return nullptr;
  }
}

ztd::chunkdat& ztd::chunkdat::subChunkRef(std::string const& in) const
{
  if(this->type()!=ztd::chunk_abstract::map)
  {
    if(m_parent != nullptr)
      throw ztd::format_error("chunkdat isn't a map", m_parent->filePath(), m_parent->im_data(), m_offset );
    else
      throw ztd::format_error("chunkdat isn't a map", "", this->strval(), -1);
  }
  ztd::chunk_map* dc = dynamic_cast<chunk_map*>(m_achunk);
  auto fi = dc->values.find(in);
  if(fi == dc->values.end())
  {
    if(m_parent != nullptr)
      throw ztd::format_error("Map doesn't have '" + in + "' flag", m_parent->filePath(), m_parent->im_data(), m_offset );
    else
      throw ztd::format_error("Map doesn't have '" + in + "' flag", "", this->strval(), -1);
  }
  return *fi->second;
}

ztd::chunkdat& ztd::chunkdat::subChunkRef(const unsigned int a) const
{
  if(this->type()!=ztd::chunk_abstract::list)
  {
    if(m_parent != nullptr)
      throw ztd::format_error("chunkdat isn't a list", m_parent->filePath(), m_parent->im_data(), m_offset );
    else
      throw ztd::format_error("chunkdat isn't a list", "", this->strval(), -1);
  }
  ztd::chunk_list* cl = dynamic_cast<chunk_list*>(m_achunk);
  if(a >= cl->list.size())
  {
    if(m_parent != nullptr)
      throw ztd::format_error("List size is below " + std::to_string(a), m_parent->filePath(), m_parent->im_data(), m_offset );
    else
      throw ztd::format_error("List size is below " + std::to_string(a), "", this->strval(), -1);
  }
  return *cl->list[a];
}

ztd::chunkdat::chunkdat()
{
  m_achunk=nullptr;
  m_parent=nullptr;
  m_offset=0;
}
ztd::chunkdat::chunkdat(const char* in)
{
  m_achunk=nullptr;
  set(in, strlen(in), 0, nullptr);
}
ztd::chunkdat::chunkdat(std::string const& in, int offset, filedat* parent)
{
  m_achunk=nullptr;
  set(in, offset, parent);
}
ztd::chunkdat::chunkdat(const char* in, const int in_size, int offset, filedat* parent)
{
  m_achunk=nullptr;
  set(in, in_size, offset, parent);
}
ztd::chunkdat::chunkdat(chunkdat const& in)
{
  m_achunk=nullptr;
  set(in);
}
ztd::chunkdat::~chunkdat()
{
  clear();
}

void ztd::chunkdat::clear()
{
  if(m_achunk!=nullptr)
    delete m_achunk;
  m_achunk=nullptr;
}

ztd::chunk_abstract::typeEnum ztd::chunkdat::type() const
{
  if(m_achunk!=nullptr)
    return m_achunk->type();
  else
    return ztd::chunk_abstract::none;
}

ztd::chunk_abstract::chunk_abstract()
{
  m_type=ztd::chunk_abstract::none;
}
ztd::chunk_abstract::typeEnum ztd::chunk_abstract::type()
{
  return m_type;
}
ztd::chunk_abstract::~chunk_abstract()
{

}

ztd::chunk_string::chunk_string()
{
  m_type=ztd::chunk_abstract::string;
}
ztd::chunk_string::~chunk_string()
{

}

ztd::chunk_map::chunk_map()
{
  m_type=ztd::chunk_abstract::map;
}
ztd::chunk_map::~chunk_map()
{
  for(auto it : values)
  {
    if(it.second != nullptr)
    delete it.second;
  }
}

ztd::chunk_list::chunk_list()
{
  m_type=ztd::chunk_abstract::list;
}
ztd::chunk_list::~chunk_list()
{
  for(auto it : list)
  {
    if(it!=nullptr)
    delete it;
  }
}
