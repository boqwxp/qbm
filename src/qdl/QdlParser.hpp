#ifndef QDLPARSER_HPP_INCLUDED
#define QDLPARSER_HPP_INCLUDED
#line 1 "QdlParser.ypp"

# include <istream>
  class Lib;
  class SVal;

#line 9 "QdlParser.hpp"
#include <string>
class QdlParser {
  typedef SVal YYSVal;
  class YYStack;
#line 7 "QdlParser.ypp"

  std::istream &m_in;
  Lib          &m_lib;

  //- Life Cycle ---------------------------------------------------------------
public:
  QdlParser(std::istream &in, Lib &lib) : m_in(in), m_lib(lib) { parse(); }
  ~QdlParser() {}

  //- Parser Interface Methods -------------------------------------------------
private:
  void error(std::string  msg);
  unsigned nextToken(YYSVal &sval);

  //- Private Parser Helpers ---------------------------------------------------
private:

  //- Usage Interface ----------------------------------------------------------
public:

#line 35 "QdlParser.hpp"
private:
  void parse();
public:
enum {
  POWER = 256,
  NUMBER = 257,
  IDENT = 258,
  TO = 259,
  LD = 260,
  COMPONENT = 261,
  CONSTANT = 262,
  CONFIG = 263,
  SIGNAL = 264,
  END = 265,
};
private:
enum { YYINTERN = 266 };
static unsigned short const  yyintern[];
static char const    *const  yyterms[];

private:
static unsigned short const  yylength[];
static unsigned short const  yylhs   [];
static char const    *const  yyrules [];

private:
static unsigned short const  yygoto  [][18];
static signed   short const  yyaction[][32];
};

#endif
