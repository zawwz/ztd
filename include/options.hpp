#ifndef ZTD_OPTIONS_HPP
#define ZTD_OPTIONS_HPP

//DUPLICATES NOT HANDLED: takes last one
//NO ORDER: no way to know options order
//RETURNS TRANSITIONAL STATE IF ERROR

#include <vector>
#include <string>
#include <iostream>

#include <exception>

/*! @file options.hpp
*  @brief CLI option reading and processing
*
* Easy managing of POSIX/GNU style options
*/


namespace ztd
{
  /*PROCESSING*/
  //! @brief behavior for option processing
  struct process_arguments {
    //! @brief -N to be considered as numbers instead of options
    bool ignore_numbers=false;
    //! @brief Stop processing on the first non-option argument
    bool stop_on_argument=false;
    //! @brief Ignore unknown options (added to return arguments)
    bool ignore_unknown=false;
    //! @brief Stop option processing on \-\-
    bool stop_on_doubledash=true;
    //! @brief Include \-\- in return arguments if it was encountered
    bool output_doubledash=false;
  };

  //! @brief Option exception
  /*!
    Thrown when errors are encountered during processing of options
  */
  class option_error : public std::exception
  {
  public:
    //! @brief option error types
    /*!
      Values:
        - unkown_option
        - takes_no_arg
        - missing_arg
    */
    enum error_type { unknown_option, takes_no_arg, missing_arg};

    //! @brief Conctructor
    option_error(error_type type, const std::string& option);

    //! @brief Type of error
    inline error_type type() { return errtype; }
    //! @brief Error message
    inline const char * what () const throw () {return msg.c_str();}
    //! @brief Subject of said error
    inline const char * option () const throw () {return opt.c_str();}
  private:
    error_type errtype;
    std::string opt;
    std::string msg;
  };
  //! @brief Convert argc/argv into vector<std::string>
  /*!
  Does not include argv[0]
  */
  std::vector<std::string> argVector(int argc, char** argv);

  //! @brief Single option object
  /*! Definition of a GNU/POSIX style option
  */
  class option
  {
  public:
    /* CTOR/DTOR */
    //ctors
    option();
    //! @brief Create a char defined option
    option(char c, bool arg=false, std::string helptext="", std::string argname="arg");
    //! @brief Create a string defined option
    option(const std::string& str, bool arg=false, std::string helptext="", std::string argname="arg");
    //! @brief Create a char and string defined option
    option(char c, const std::string& str, bool arg=false, std::string helptext="", std::string argname="arg");
    //! @brief Create copy
    option(const option& opt);

    /* FUNCTIONS */

    //! @brief Print option's help
    /*!
      @param leftpad Put @a leftpad spaces at start of the line
      @param rightpad Help text starts at column @a leftpad + @a rightpad
    */
    void print_help(int leftpad, int rightpad) const;

    /* PROPERTIES */

    bool shortDef;  // has a char definition
    char charName;

    bool longDef;   // has a string definition
    std::string strName;

    bool takesArgument;     // option takes an argument

    std::string help_text;  // text to display in print_help
    std::string arg_name;   // name of the argument to display in print_help

    /* PROCESSING STATUS */

    //! @brief Option was activated
    bool activated;         // option was activated

    //! @brief Option's argument
    std::string argument;   // argument of the option

    inline operator bool() const { return activated; }
    inline operator std::string() const { return argument; }

  };
  //! @brief Set of POSIX/GNU style options
  /*!
    Process arguments through it to extract options

    After processing:
    - for global option settings use find(), [], or parse through option_vec
    - for sequential option settings parse through option_sequence

  */
  class option_set
  {
  public:
    option_set() { ; }
    option_set(option opt) { add(opt); }
    option_set(std::vector<option> opts) { option_vec = opts; }
    /*CREATION FUNCTIONS*/
    //! @brief Add option to the set
    inline void add(option opt) { option_vec.push_back(opt); }
    //! @brief Vector call of add();
    inline void add(std::vector<option> opts) { for(auto it: opts) add(it); }
    //! @brief Variadic call of add()
    template<class... Args>
    void add(Args... args) { std::vector<option> rargs = { static_cast<option>(args)...}; for(auto it: rargs) add(it); }

    /*PRINT FUNCTIONS*/
    //! @brief Print help for the full option set
    /*!
      Output order follows option creation order
      @param leftpad Put @a leftpad spaces at start of the line
      @param rightpad Help text starts at column @a leftpad + @a rightpad
    */
    void print_help(int leftpad=2, int rightpad=25) const;

    /*QUERY FUNCTIONS*/
    //! @brief Find char option
    /*! @return nullptr if no option found
    */
    option* find(const char c);
    //! @brief Find string option
    /*! @return nullptr if no option found
    */
    option* find(const std::string& str);
    inline option* find(const char* str) { return this->find(std::string(str)); }

    static constexpr process_arguments default_process_args={};

    //! @brief Process arguments through the option set
    /*!
    If errors are encountered, exceptions option_error are thrown
    @see process_arguments
    @param arguments vector of string containing arguments and options
    @param behavior behavioral changes for option processing. Optional
    @return if @a behavior.stop_on_argument is specified, returns unprocessed arguments\n otherwise, returns leftover arguments that are not options\n
    */
    std::vector<std::string> process(std::vector<std::string> const& arguments, process_arguments const& behavior=default_process_args);
    //! @brief Process arguments through the option set
    /*!
    @see process(std::vector<std::string> const& arguments, process_arguments const& behavior)
    */
    inline std::vector<std::string> process(int argc, char** argv, process_arguments const& behavior=default_process_args) { return this->process(ztd::argVector(argc, argv), behavior); }

    //! @brief Get option with char name
    /*! @see option* find(char c)
    */
    option& operator[](const char c) { return *this->find(c); }
    //! @brief Get option with string name
    /*! @see option* find(const std::string& str);
    */
    option& operator[](const std::string& str) { return *this->find(str); }
    //! @brief Get nth option
    /*! Reads nth option from @a option_sequence
    */
    option& operator[](unsigned int n) { return option_sequence[n]; }

    //! @brief Options in the set
    std::vector<option> option_vec;
    //! @brief Ordered result of option processing
    std::vector<option> option_sequence;
  };

} //ztd

#endif //ZTD_OPTIONS_HPP
