#ifndef FILEDAT_HPP
#define FILEDAT_HPP

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <exception>

#include <cstring>


/*! @file filedat.hpp
*  @brief Storing and reading data
*
* Easily organize data in a JSON-like file format
*/

namespace ztd
{
  class filedat;
  class chunkdat;
  class format_error;

  //! @brief Abstract data storing object
  /*! Used for inheritance and type classing.
  <b> Not for external use </b>
  */
  class chunk_abstract
  {
  public:
    //! @brief Type of data
    /*! Values: none , string , map , list
    */
    enum typeEnum { none, string, map, list};

    //! @brief Get type of chunk
    typeEnum type();

    chunk_abstract();
    virtual ~chunk_abstract();

  protected:
    typeEnum m_type;
  };

  //! @brief String data storing class
  /*!
    Back-end of string data chunk.
    <b> Not for external use </b>
  */
  class chunk_string : public chunk_abstract
  {
  public:
    //! @brief String data
    std::string val;

    chunk_string();
    virtual ~chunk_string();
  };

  //! @brief Map data storing class
  /*!
    Back-end of map data chunk.
    <b> Not for external use </b>
  */
  class chunk_map : public chunk_abstract
  {
  public:
    //! @brief Mapped data
    std::map<std::string, chunkdat*> values;

    chunk_map();
    virtual ~chunk_map();

  };

  //! @brief String data storing class
  /*!
    Back-end of list data chunk.
    <b> Not for external use </b>
  */
  class chunk_list : public chunk_abstract
  {
  public:
    //! @brief List data
    std::vector<chunkdat*> list;

    chunk_list();
    virtual ~chunk_list();
  };

  //! @brief Chunk data object
  /*!
    Object containing a chunk of data\n
    Either string, map, or list
  */
  class chunkdat
  {
  public:
    //! @brief Constructor
    chunkdat();
    //! @brief Constructor with initial value
    chunkdat(const char* in);
    //! @brief Constructor with initial value
    chunkdat(std::string const& in);
    chunkdat(const char* in, const int in_size,  int offset=0, filedat* data=nullptr);
    chunkdat(std::string const& in, int offset=0, filedat* data=nullptr);
    //! @brief Constructor with copy
    chunkdat(chunkdat const& in);
    //dtor
    ~chunkdat();

    //! @brief Clear contents
    void clear();
    //! @brief Type of the stored data
    chunk_abstract::typeEnum type() const;
    //! @brief get pointer to chunk_abstract
    inline chunk_abstract* getp() const { return m_achunk; }
    //! @brief Size of list. -1 if not a list
    int listSize() const;
    //! @brief Get pointer to parent (debug)
    inline filedat* parent() const { return m_parent; }
    //! @brief Get data offset (debug)
    inline int offset() const { return m_offset; }


    //! @brief Set data
    /*!
    @param in C string data
    @param in_size Size of the string data
    @param offset Used for debugging
    @param data Used for debugging
    */
    void set(const char* in, const int in_size, int offset=0, filedat* parent=nullptr);
    //! @brief Set data
    /*!
    @param in String data
    @param offset Used for debugging
    @param data Used for debuggingy
    */
    inline void set(std::string const& in, int offset=0, filedat* data=nullptr) { this->set(in.c_str(), in.size(), offset, data); }

    //! @brief Copy chunk's data
    inline void set(chunkdat const& in) { this->set(in.strval(), in.offset(), in.parent()); } // TODO

    //! @brief Get string value of data
    /*!
    @param alignment Number of initial aligners
    @param aligner String to use for aligning sub-chunks
    @return String value of the whole chunk data
    */
    std::string strval(unsigned int alignment=0, std::string const& aligner="\t") const;

    void addToMap(std::string const& name, chunkdat const& val);
    void addToMap(std::vector<std::pair<std::string, chunkdat>> const& vec);
    void addToList(chunkdat const& val);
    void addToList(std::vector<chunkdat> const& vec);
    //! @brief Add keyed data to map chunk
    /*!
      @param name Key of the data
      @param val Data
    */
    inline void add(std::string const& name, chunkdat const& val) { addToMap(name, val); }
    //! @brief Add multiple keyed data to map chunk
    /*!
    @param vec Vector of pairs of keys and data
    */
    inline void add(std::vector<std::pair<std::string, chunkdat>> const& vec) { addToMap(vec); }
    //! @brief Append data to end of list
    /*!
    @param val Data to add
    */
    inline void add(chunkdat const& val) { addToList(val); }
    //! @brief Append list of data to end of list
    /*!
    @param vec Vector of data to add
    */
    inline void add(std::vector<chunkdat> const& vec) { addToList(vec); }
    //! @brief Concatenate chunks of data
    /*! Effective only if the two chunks are of the same type\n
        Map: combines two maps into a single map\n
        List: combines into a single list\n
        String: Concatenate strings
    */
    void concatenate(chunkdat const& chk);

    void erase(const std::string& key);

    void erase(const unsigned int index);


    //! @brief Create a copy of the chunk
    inline chunkdat copy() const { return chunkdat(*this); }
    //! @brief Create a pointed copy of the chunk
    inline chunkdat* pcopy() const { return new chunkdat(*this); }

    //! @brief Reference to subchunk of map
    /*!
      Throws format_error exception when operation fails
      @param a Key of the subchunk data
      @return Reference to the subchunk in question
    */
    chunkdat& subChunkRef(std::string const& a) const;
    //! @brief Reference to subchunk of list
    /*!
    Throws format_error exception when operation fails
    @param a Position of subchunk data in list
    @return Reference to the subchunk in question
    */
    chunkdat& subChunkRef(const unsigned int a) const;
    //! @brief Pointer to subchunk of map
    /*!
    @param a Key of the subchunk data
    @return Reference to the subchunk in question\n
    nullptr if operation failed
    */
    chunkdat* subChunkPtr(std::string const& a) const;
    //! @brief Pointer to subchunk of list
    /*!
    @param a Position of subchunk data in list
    @return Reference to the subchunk in question\n
    nullptr if operation failed
    */
    chunkdat* subChunkPtr(const unsigned int a) const;

    //! @brief Reference to subchunk of map.    @see subChunkRef(std::string const& a) const
    inline chunkdat& operator[](std::string const& a) const                             { return subChunkRef(a); }
    //! @brief Reference to subchunk of list.   @see subChunkRef(const unsigned int a) const
    inline chunkdat& operator[](const unsigned int a) const                             { return subChunkRef(a); }
    //! @brief Set chunk data and return *this. @see set(chunkdat const& in)
    inline chunkdat& operator=(chunkdat const& a)                                       { set(a); return *this; }
    //! @brief add and return *this.            @see add(std::string const& name, chunkdat const& val)
    inline chunkdat& operator+=(std::pair<std::string, chunkdat> const& a)              { add(a.first, a.second); return *this; }
    //! @brief add and return *this.            @see add(std::vector<std::pair<std::string, chunkdat>> const& vec)
    inline chunkdat& operator+=(std::vector<std::pair<std::string, chunkdat>> const& a) { add(a); return *this; }
    //! @brief add and return *this.            @see add(chunkdat const& val)
    inline chunkdat& operator+=(chunkdat const& a)                                      { add(a); return *this; }
    //! @brief add and return *this.            @see add(std::vector<chunkdat> const& vec)
    inline chunkdat& operator+=(std::vector<chunkdat> const& a)                         { add(a); return *this; }
    //! @brief concatenate and return *this.    @see concatenate(chunkdat const& chk)
    inline chunkdat& operator*=(chunkdat const& a)                                      { concatenate(a); return *this; }
    //! @brief remove and return *this.         @see remove(const std::string& key)
    inline chunkdat& operator-=(const std::string& key)     { erase(key); return *this; }
    //! @brief remove and return *this.         @see remove(const unsigned int index)
    inline chunkdat& operator-=(const unsigned int index)   { erase(index); return *this; }

    //add operator+ and operator*


  protected:
    filedat* m_parent;
    int m_offset;

    chunk_abstract* m_achunk;
  };

  //! @brief add
  inline chunkdat operator+(const chunkdat& a, const std::pair<std::string, chunkdat>& b)               { chunkdat ret(a); ret += b; return ret; }
  //! @brief add
  inline chunkdat operator+(const chunkdat& a, const std::vector<std::pair<std::string, chunkdat>>& b)  { chunkdat ret(a); ret += b; return ret; }
  //! @brief add
  inline chunkdat operator+(const chunkdat& a, const chunkdat& b)                                       { chunkdat ret(a); ret += b; return ret; }
  //! @brief add
  inline chunkdat operator+(const chunkdat& a, const std::vector<chunkdat>& b)                          { chunkdat ret(a); ret += b; return ret; }
  //! @brief concatenated chunk
  inline chunkdat operator*(const chunkdat& a, const chunkdat& b)     { chunkdat ret(a); ret *= b; return ret; }
  //! @brief substract
  inline chunkdat operator-(const chunkdat& a, const std::string& b)  { chunkdat ret(a); ret -= b; return ret; }
  //! @brief substract
  inline chunkdat operator-(const chunkdat& a, const unsigned int b)  { chunkdat ret(a); ret -= b; return ret; }

  //! @brief File data object
  /*!
  Object for importing, reading, altering and writing of file data\n
  */
  class filedat
  {
  public:
    //! @brief Constructor
    filedat();
    //! @brief Constructor with initial file path
    filedat(std::string const& in);
    virtual ~filedat();

    //! @brief Get current file path
    inline std::string filePath() const { return m_filePath; }
    //! @brief Set file path
    inline void setFilePath(std::string const& in) { m_filePath=in; }

    //! @brief Test wether file can be read
    bool readTest() const;

    //! @brief Import file data
    /*!
    Throws format_error exceptions if errors are encountered while reading
    @param path Will set this as file path if not empty
    */
    void import_file(const std::string& path="");
    //! @brief Import data from stdin
    /*!
    Throws format_error exceptions if errors are encountered while reading
    */
    void import_stdin();
    //! @brief Import data from string
    /*!
    Throws format_error exceptions if errors are encountered while reading
    */
    void import_string(const std::string& data);
    //! @brief Export data to file
    /*!
    @param path Will set this as file path if not empty
    @param aligner String used to align subchunks
    */
    bool export_file(std::string const& path="", std::string const& aligner="\t") const;

    //! @brief Clear contents of data
    void clear();

    //! @brief Get string value of data
    /*!
    @param aligner String used to align subchunks
    */
    std::string strval(std::string const& aligner="\t") const;

    //! @brief Get reference to chunk data
    inline chunkdat& data() const { return *m_dataChunk; }
    //! @brief Get pointer to chunk data
    inline chunkdat* pdata() const { return m_dataChunk; }

    //! @brief Set data
    /*!
    Throws format_error exceptions if errors are encountered
    @see chunkdat::set(chunkdat const& a)
    */
    inline void set_data(chunkdat const& in) { m_dataChunk->set(in); }

    //! @brief Imported data as is. Used for debugging
    inline const std::string& im_data() const { return m_data; }
    //! @brief Imported data as is. Used for debugging
    inline const char* im_c_data() const { return m_data.c_str(); }

    //! @brief Reference to subchunk
    //! @see chunkdat::operator[](std::string const &a) const
    inline chunkdat& operator[](const std::string& index) { return m_dataChunk->subChunkRef(index); }
    //! @brief Reference to subchunk
    //! @see chunkdat::operator[](const unsigned int a) const
    inline chunkdat& operator[](const unsigned int index) { return m_dataChunk->subChunkRef(index); }

    //! @brief add data and return *this
    //! @see chunkdat::operator+=(std::pair<std::string, chunkdat> const& a)
    inline filedat& operator+=(std::pair<std::string, chunkdat> const& a)              { *m_dataChunk += a; return *this; }
    //! @brief add data and return *this
    //! @see chunkdat::operator+=(std::vector<std::pair<std::string, chunkdat>> const& a)
    inline filedat& operator+=(std::vector<std::pair<std::string, chunkdat>> const& a) { *m_dataChunk += a; return *this; }
    //! @brief add data and return *this
    //! @see chunkdat::operator+=(chunkdat const& a)
    inline filedat& operator+=(chunkdat const& a)                                      { *m_dataChunk += a; return *this; }
    //! @brief add() and return *this
    //! @see chunkdat::operator+=(std::vector<chunkdat> const& a)
    inline filedat& operator+=(std::vector<chunkdat> const& a)                         { *m_dataChunk += a; return *this; }

    //! @brief set_data() and return *this
    //! @see chunkdat::operator+=(std::vector<chunkdat> const& a)
    inline filedat& operator=(chunkdat const& a)                                       { set_data(a); return *this; }

    //! @brief Is a read char
    static bool isRead(char in);

  private:
    //functions
    void generateChunk();
    //attributes
    std::string m_filePath;
    std::string m_data;
    chunkdat* m_dataChunk;
  };

  //! @brief Data format exception
  /*!
    Thrown when errors are encountered when manipulating data chunks
  */
  class format_error : public std::exception
  {
  public:
    //! @brief Conctructor
    inline format_error(const std::string& what, const std::string& origin, const std::string& data, int where)  { desc=what; index=where; filename=origin; sdat=data; }

    //! @brief Error message
    inline const char * what () const throw () {return desc.c_str();}
    //! @brief Origin of the data, name of imported file, otherwise empty if generated
    inline const char * origin() const throw () {return filename.c_str();}
    //! @brief Data causing the exception
    inline const char * data() const throw () {return sdat.c_str();}
    //! @brief Where the error is located in the data
    inline const int where () const throw () {return index;}
  private:
    std::string desc;
    int index;
    std::string filename;
    std::string sdat;
  };

  inline std::ostream& operator<<(std::ostream& stream, chunkdat const& a)  { return stream << a.strval(); }
  inline std::ostream& operator<<(std::ostream& stream, filedat const& a)   { return stream << a.strval(); }

  void printErrorIndex(const char* in, const int index, const std::string& message, const std::string& origin);
  //! @brief Print exception to console
  /*!
    If origin is known, displays location and discriminating line\n
    If origin is unknown, displays whole data up to discriminating line
  */
  void printFormatException(format_error& exc);

}


#endif //FILEDAT_HPP
