#include "filedat.hpp"

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

void ztd::printFormatException(ztd::format_error& exc)
{
  ztd::printErrorIndex(exc.data(), exc.where(), exc.what(), exc.origin());
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
    m_dataChunk = new ztd::chunkdat(m_data.c_str(), m_data.size(), 0, this);
  }
  catch(ztd::format_error& e)
  {
    throw ztd::format_error(e.what(), m_filePath, m_data, e.where());
  }
}

static std::string _getname(const char* in, const int in_size, int* start, int* val_size, int* end)
{
  int i=0;

  *start = in_size; //default no value
  *end = in_size; //default end
  *val_size=0; //default no value

  while(i<in_size)
  {
    if(i+1<in_size && in[i] == '/' && in[i+1] == '/')
    while(i<in_size && in[i] != '\n')
    i++;

    if(ztd::filedat::isRead(in[i]))
    break;

    i++;
  }
  if(i >= in_size) //ends without value
  return "";

  int j=i; //name start
  while(i<in_size && in[i] != '=') //skip to =
  i++;
  if(i >= in_size) //no =
  throw ztd::format_error("Tag has no value", "", std::string(in, in_size), j);

  if(i == j) //nothing preceding =
  throw ztd::format_error("Value has no tag", "", std::string(in, in_size), i);

  int k=i-1; //name end
  while( !ztd::filedat::isRead(in[k]) )
  k--;
  std::string name=std::string(in+j, k-j+1);

  i++;
  while(i < in_size && !ztd::filedat::isRead(in[i]))
  i++;
  if(i >= in_size) //no value
  {
    *start=i;
    *val_size=0;
    *end=i;
    return name;
  }
  if(in[i] == '\"') //"" val
  {
    *start=i; //value starts
    j=1; //size
    while(i+j < in_size && in[i+j]!='\"')
    {
      if(in[i+j]=='\\')
      j++;
      j++;
    }
    if(i+j >= in_size) // no closing "
    throw ztd::format_error("Double quote does not close", "", std::string(in, in_size), i-1);
    j++;
    *val_size=j;
    *end=i+j;
    return name;
  }
  if(in[i] == '\'') //"" val
  {
    *start=i; //value starts
    j=1; //size
    while(i+j < in_size && in[i+j]!='\'')
    {
      if(in[i+j]=='\\')
      j++;
      j++;
    }
    if(i+j >= in_size) // no closing '
    throw ztd::format_error("Single quote does not close", "", std::string(in, in_size), i-1);
    j++;
    *val_size=j;
    *end=i+j;
    return name;
  }
  if(in[i] == '{')
  {
    *start=i;
    j=1;
    int counter=0;
    while( i+j < in_size && !( counter == 0 && in[i+j]=='}') )
    {
      if(i+j+1<in_size && in[i+j] == '/' && in[i+j+1] == '/')
      while(i+j<in_size && in[i+j] != '\n')
      j++;
      if(in[i+j]=='\\')
      j++;
      if(in[i+j]=='{')
      counter++;
      if(in[i+j]=='}')
      counter--;
      j++;
    }
    if(i+j >= in_size) //reached end without closing
    throw ztd::format_error("Brace does not close", "", std::string(in, in_size), i);
    j++;
    *val_size=j;
    *end=i+j;
    return name;
  }
  if(in[i] == '[')
  {
    *start=i;
    j=1;
    int counter=0;
    while( i+j < in_size && !( counter == 0 && in[i+j]==']') )
    {
      if(i+j+1<in_size && in[i+j] == '/' && in[i+j+1] == '/')
      while(i+j<in_size && in[i+j] != '\n')
      j++;
      if(in[i+j]=='\\')
      j++;
      if(in[i+j]=='[')
      counter++;
      if(in[i+j]==']')
      counter--;
      j++;
    }
    if(i+j >= in_size) //reached end without closing
    throw ztd::format_error("Bracket does not close", "", std::string(in, in_size), i);
    j++;
    *val_size=j;
    *end=i+j;
    return name;
  }
  { // no encapsulation: go to end of line
    *start=i; //value starts
    j=0; //size
    while(i+j < in_size && in[i+j]!='\n')
    {
      if(in[i]+j=='\\')
      j++;
      j++;
    }
    while( !ztd::filedat::isRead(in[i+j]) )
    j--;
    *val_size=j+1;
    *end=i+j+1;
    return name;
  }

  return name;
}

static std::string _getlist(const char* in, const int in_size, int* start, int* end)
{
  int i=0;
  std::string ret;

  while(i<in_size)
  {
    if(i+1<in_size && in[i] == '/' && in[i+1] == '/')
    while(i<in_size && in[i] != '\n')
    i++;

    if(ztd::filedat::isRead(in[i]))
    break;

    i++;
  }

  *start=i;
  if(i >= in_size) //ends without value
  {
    *end = in_size;
    return "";
  }
  if(in[i] == ',') //value is empty
  {
    *end=i+1;
    return "";
  }

  int j=0;
  if(in[i] == '\"') //"" val
  {
    j=1; //size
    while(i+j < in_size && in[i+j]!='\"')
    {
      if(in[i+j]=='\\')
      j++;
      j++;
    }
    if(i+j >= in_size) // no closing "
    throw ztd::format_error("Double quote does not close", "", std::string(in, in_size), i-1);
    j++;
    ret = std::string(in+i, j);
    *end=i+j;
  }
  else if(in[i] == '\'') //"" val
  {
    j=1; //size
    while(i+j < in_size && in[i+j]!='\'')
    {
      if(in[i+j]=='\\')
      j++;
      j++;
    }
    if(i+j >= in_size) // no closing '
    throw ztd::format_error("Single quote does not close", "", std::string(in, in_size), i-1);
    j++;
    ret = std::string(in+i, j);
    *end=i+j;
  }
  else if(in[i] == '{')
  {
    j=1;
    int counter=0;
    while( i+j < in_size && !( counter == 0 && in[i+j]=='}') )
    {
      if(i+j+1<in_size && in[i+j] == '/' && in[i+j+1] == '/')
      while(i+j<in_size && in[i+j] != '\n')
      j++;
      if(in[i+j]=='\\')
      j++;
      if(in[i+j]=='{')
      counter++;
      if(in[i+j]=='}')
      counter--;
      j++;
    }
    if(i+j >= in_size) //reached end without closing
    throw ztd::format_error("Brace does not close", "", std::string(in, in_size), i);
    j++;
    ret = std::string(in+i, j);
    *end=i+j;
  }
  else if(in[i] == '[')
  {
    j=1;
    int counter=0;
    while( i+j < in_size && !( counter == 0 && in[i+j]==']') )
    {
      if(i+j+1<in_size && in[i+j] == '/' && in[i+j+1] == '/')
      while(i+j<in_size && in[i+j] != '\n')
      j++;
      if(in[i+j]=='\\')
      j++;
      if(in[i+j]=='[')
      counter++;
      if(in[i+j]==']')
      counter--;
      j++;
    }
    if(i+j >= in_size) //reached end without closing
    throw ztd::format_error("Bracket does not close", "", std::string(in, in_size), i);
    j++;
    ret = std::string(in+i, j);
    *end=i+j;
  }
  else // no encapsulation: go to next ,
  {
    j=0; //size
    while(i+j < in_size && in[i+j]!=',')
    {
      if(in[i+j]=='\\')
      j++;
      j++;
    }
    if(i+j < in_size)
    {
      while( !ztd::filedat::isRead(in[i+j]) )
      j--;
    }
    ret = std::string(in+i,j);
    *end=i+j;
  }

  i = *end;
  while(i < in_size && !ztd::filedat::isRead(in[i]))
  i++;
  if( i>= in_size ) //last char
  {
    *end=i;
    return ret;
  }
  else if(in[i] ==',') //comma as expected
  {
    *end=i+1;
    return ret;
  }
  else //Unexpected char
  throw ztd::format_error("Expecting comma", "", std::string(in, in_size), i);

}

void ztd::chunkdat::set(const char* in, const int in_size, int offset, filedat* parent)
{
  this->clear(); //reset everything
  this->m_parent=parent;
  this->m_offset=offset;

  int i=0;

  while(i<in_size && !ztd::filedat::isRead(in[i])) //skip unread char
  i++;

  if(i >= in_size) //empty: make an empty strval
  {
    ztd::chunk_string* cv = new ztd::chunk_string();
    m_achunk=cv;
    cv->val = "";
    return;
  }
  else if( in[i] == '{')
  {
    i++;
    int val_end=in_size-1;
    while(!ztd::filedat::isRead(in[val_end])) //skip unread char
    val_end--;
    if(in[val_end] != '}')
    throw ztd::format_error("Expecting closing brace", "", std::string(in, in_size), val_end+1);

    ztd::chunk_map* tch = new ztd::chunk_map();
    m_achunk = tch;

    std::string name;
    std::string val;
    while(i < val_end)
    {
      int start=0;
      int _size=0;
      int end=0;

      while(!ztd::filedat::isRead(in[i]))
      i++;

      std::string newstr=std::string(in+i, val_end-i);
      try
      {
        name = _getname(newstr.c_str(), newstr.size(), &start, &_size, &end);
        val = newstr.substr(start, _size);
      }
      catch(ztd::format_error& e)
      {
        throw ztd::format_error(e.what(), "", std::string(in, in_size), e.where()+i);
      }

      if( name == "" ) //no more values
      break;

      try
      {
        ztd::chunkdat* chk = new ztd::chunkdat(val.c_str(),val.size(), offset + start+i, m_parent);
        if(!tch->values.insert( std::make_pair(name, chk ) ).second)
        {
          delete chk;
          throw ztd::format_error("Key '" + name + "' already present", "", std::string(in, in_size), 0 - start );
        }
      }
      catch(ztd::format_error& e)
      {
        throw ztd::format_error(e.what(), "", std::string(in, in_size), e.where() + start + i );
      }

      i += end;
    }

    return;

  }
  else if( in[i] == '[')
  {
    i++;
    int val_end=in_size-1;
    while(!ztd::filedat::isRead(in[val_end])) //skip unread char
    val_end--;
    if(in[val_end] != ']')
    throw ztd::format_error("Expecting closing bracket", "", std::string(in, in_size), val_end+1);

    ztd::chunk_list* tch = new ztd::chunk_list();
    m_achunk = tch;

    int end=0,start=0;
    while( i < val_end )
    {
      std::string val;
      std::string newstr=std::string(in+i, val_end-i);
      try
      {
        val = _getlist(newstr.c_str(), newstr.size(), &start, &end);
      }
      catch(ztd::format_error& e)
      {
        throw ztd::format_error(e.what(), "", std::string(in, in_size), e.where()+i);
      }

      try
      {
        tch->list.push_back(new ztd::chunkdat(val.c_str(),val.size(), offset + start+i, m_parent) );
      }
      catch(ztd::format_error& e)
      {
        throw ztd::format_error(e.what(), "", std::string(in, in_size), e.where() + start + i );
      }

      i+=end;
    }

    return;

  }
  else // string value
  {
    int val_end=in_size;
    val_end--;
    while(!ztd::filedat::isRead(in[val_end])) //skip unread char
    val_end--;

    ztd::chunk_string* tch = new ztd::chunk_string();
    m_achunk = tch;

    tch->val = std::string(in+i,val_end-i+1);

    return;

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
    std::string ret="{\n";
    for(auto it : cp->values)
    {
      ret += repeatString(aligner,alignment+1);
      ret += it.first;
      ret += " = ";
      if(it.second!=nullptr)
      ret += it.second->strval(alignment+1, aligner);
      ret += '\n';
    }
    ret += repeatString(aligner, alignment);
    ret += '}';
    return ret;
  }
  else if(this->type()==ztd::chunk_abstract::list)
  {
    ztd::chunk_list* lp = dynamic_cast<chunk_list*>(m_achunk);
    std::string ret="[\n";
    for(auto it : lp->list)
    {
      ret += repeatString(aligner, alignment+1);
      if(it!=nullptr)
      ret += it->strval(alignment+1, aligner);
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
    {
      throw ztd::format_error("chunkdat isn't a map", m_parent->filePath(), m_parent->im_data(), m_offset );
    }
    else
    {
      throw ztd::format_error("chunkdat isn't a map", "", this->strval(), -1);
    }
  }
  ztd::chunk_map* dc = dynamic_cast<chunk_map*>(m_achunk);
  auto fi = dc->values.find(in);
  if(fi == dc->values.end())
  {
    if(m_parent != nullptr)
    {
      throw ztd::format_error("Map doesn't have '" + in + "' flag", m_parent->filePath(), m_parent->im_data(), m_offset );
    }
    else
    {
      throw ztd::format_error("Map doesn't have '" + in + "' flag", "", this->strval(), -1);
    }
  }
  return *fi->second;
}

ztd::chunkdat& ztd::chunkdat::subChunkRef(const unsigned int a) const
{
  if(this->type()!=ztd::chunk_abstract::list)
  {
    if(m_parent != nullptr)
    {
      throw ztd::format_error("chunkdat isn't a list", m_parent->filePath(), m_parent->im_data(), m_offset );
    }
    else
    {
      throw ztd::format_error("chunkdat isn't a list", "", this->strval(), -1);
    }
  }
  ztd::chunk_list* cl = dynamic_cast<chunk_list*>(m_achunk);
  if(a >= cl->list.size())
  {
    if(m_parent != nullptr)
    {
      throw ztd::format_error("List size is below " + std::to_string(a), m_parent->filePath(), m_parent->im_data(), m_offset );
    }
    else
    {
      throw ztd::format_error("List size is below " + std::to_string(a), "", this->strval(), -1);
    }
  }
  return *cl->list[a];
}

ztd::chunkdat::chunkdat()
{
  m_achunk=nullptr;
}
ztd::chunkdat::chunkdat(const char* in)
{
  m_achunk=nullptr;
  set(in, strlen(in), 0, nullptr);
}
ztd::chunkdat::chunkdat(std::string const& in)
{
  m_achunk=nullptr;
  set(in, 0, nullptr);
}
ztd::chunkdat::chunkdat(const char* in, const int in_size,  int offset, filedat* data)
{
  m_achunk=nullptr;
  set(in, in_size, offset, data);
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
