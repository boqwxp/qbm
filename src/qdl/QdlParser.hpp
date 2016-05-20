#ifndef QDLPARSER_HPP_INCLUDED
#define QDLPARSER_HPP_INCLUDED
#line 1 "QdlParser.ypp"

# include <memory>
# include <stack>
# include <map>
# include <istream>
  class Lib;
  class SVal;

#line 12 "QdlParser.hpp"
#include <string>
class QdlParser {
  typedef SVal YYSVal;
  class YYStack;
#line 10 "QdlParser.ypp"

  Lib  &m_lib;
  bool  m_newline;

  class StreamDeleter;
  std::stack<std::unique_ptr<std::istream, StreamDeleter>>  m_sources;
  std::map<std::string, std::string>                        m_defines;

  //- Life Cycle ---------------------------------------------------------------
public:
  QdlParser(std::istream &in, Lib &lib);
  ~QdlParser();

  //- Parser Interface Methods -------------------------------------------------
private:
  void error(std::string  msg);
  unsigned nextToken(YYSVal &sval);

#line 36 "QdlParser.hpp"
private:
  void parse();
public:
enum {
  POWER = 256,
  NUMBER = 257,
  IDENT = 258,
  CHOOSE = 259,
  COMPONENT = 260,
  CONFIG = 261,
  CONSTANT = 262,
  END = 263,
  FOR = 264,
  GENERATE = 265,
  LD = 266,
  MAPSTO = 267,
  SIGNAL = 268,
  THROUGH = 269,
};
private:
enum { YYINTERN = 270 };
static unsigned short const  yyintern[];
static char const    *const  yyterms[];

private:
static unsigned short const  yylength[];
static unsigned short const  yylhs   [];
static char const    *const  yyrules [];

private:
static unsigned short const  yygoto  [][19];
static signed   short const  yyaction[][36];
};

#endif
