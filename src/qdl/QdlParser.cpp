#line 31 "QdlParser.ypp"

# include "QdlParser.hpp"

# include "Expression.hpp"
# include "Statement.hpp"
# include "CompDecl.hpp"
# include "Lib.hpp"
# include <cctype>
# include <functional>
# include <memory>
# include <limits>
# include <fstream>
# include <sstream>

  QdlParser::QdlParser(std::istream &in, Lib &lib)
    : m_lib(lib), m_newline(true) {
    m_sources.emplace(&in, [](std::istream*){});
    parse();
  }
  QdlParser::~QdlParser() {}

  void QdlParser::error(std::string  msg) {
    if(!m_sources.empty()) {
      std::string  line;
      getline(*m_sources.top(), line);
      msg += " before \"" + line + '"';
    }
    throw  msg;
  }

  class SVal {
    class Box {
    protected:
      Box() {}
      ~Box() {}
    };

    class Name : public Box {
      std::string const  m_val;
    public:
      Name(std::string const &val) : m_val(val) {}
      ~Name() {}
    public:
      std::string const& value() const { return  m_val; }
    };

    class Number : public Box {
      unsigned const  m_val;
    public:
      Number(unsigned const  val) : m_val(val) {}
      ~Number() {}
    public:
      unsigned value() const { return  m_val; }
    };

    class Expr : public Box {
      std::shared_ptr<Expression const> const  m_val;
    public:
      Expr(std::shared_ptr<Expression const> const &val) : m_val(val) {}
      ~Expr() {}
    public:
      std::shared_ptr<Expression const> const& value() const { return  m_val; }
    };

    class Bus : public Name {
      std::shared_ptr<Expression const> const  m_width;
    public:
      Bus(std::string const &name,
	  std::shared_ptr<Expression const> const &width)
	: Name(name), m_width(width) {}
      ~Bus() {}
    public:
      std::shared_ptr<Expression const> const& width() const { return  m_width; }
    };

    class Stmt : public Box {
      std::shared_ptr<Statement> const  m_val;
    public:
      Stmt(std::shared_ptr<Statement> const &val) : m_val(val) {}
      ~Stmt() {}
    public:
      std::shared_ptr<Statement> const& value() const { return  m_val; }
    };

    class Comp : public Box {
      CompDecl &m_val;
    public:
      Comp(CompDecl &val) : m_val(val) {}
      ~Comp() {}
    public:
      CompDecl& value() { return  m_val; }
    };

    std::shared_ptr<Box>  contents;

  public:
    SVal() {}
    ~SVal() {}

    // Setting
  public:
    SVal& operator=(std::string const& val) {
      contents.reset(new Name(val));
      return *this;
    }
    SVal& operator=(unsigned const  val) {
      contents.reset(new Number(val));
      return *this;
    }
    SVal& operator=(std::shared_ptr<Expression const> const &val) {
      contents.reset(new Expr(val));
      return *this;
    }
    SVal& operator=(std::shared_ptr<Statement> const &val) {
      contents.reset(new Stmt(val));
      return *this;
    }
    SVal& operator=(CompDecl &val) {
      contents.reset(new Comp(val));
      return *this;
    }
    SVal& makeBus(std::string                       const &name,
		  std::shared_ptr<Expression const> const &width) {
      contents.reset(new Bus(name, width));
      return *this;
    }

    // Querying
  public:
    std::string const& name() const {
      return  static_cast<Name&>(*contents).value();
    }
    unsigned number() const {
      return  static_cast<Number&>(*contents).value();
    }
    std::shared_ptr<Expression const> const& expr() const {
      return  static_cast<Expr&>(*contents).value();
    }
    std::shared_ptr<Statement> const& stmt() const {
      return  static_cast<Stmt&>(*contents).value();
    }
    CompDecl& comp() const {
      return  static_cast<Comp&>(*contents).value();
    }
    std::shared_ptr<Expression const> const& width() const {
      return  static_cast<Bus&>(*contents).width();
    }
  };

  unsigned QdlParser::nextToken(YYSVal &sval) {
    bool  newline = m_newline;
    m_newline = false;

    std::istream *in = m_sources.top().get();
    while(true) {
      int const  c = in->get();

      // Filter out Operators
      switch(c) {
      case EOF:
	m_sources.pop();
	if(m_sources.empty())  return  0;
	in = m_sources.top().get();
	continue;

      case '.':
	if(in->peek() == '.') {
	  in->ignore();
	  return  THROUGH;
	}
	goto  err;
	
      case '-':
	if(in->peek() == '>') {
	  in->ignore();
	  return  MAPSTO;
	}
	return  c;

      case '*':
	if(in->peek() == '*') {
	  in->ignore();
	  return  POWER;
	}
	return  c;

      case '/':
	if(in->peek() == '/') {
	  in->ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	  newline = true;
	  continue;
	}
	return  c;

      case '<':
      case '>':
      case '(':
      case ')':
      case '[':
      case ']':
      case '=':
      case ',':
      case ';':
      case '~':
      case '&':
      case '|':
      case '^':
      case '+':
      case '%':
      case ':':
      case '?':
      case '#':
	return  c;

      case '\'': // check for directives
	if(!newline)  goto  err;
	std::string   line;
	getline(*in, line, '\n');

	if(line.compare(0, 7, "include") == 0) {
	  std::unique_ptr<char[]>  filename(new char[line.size()]);
	  int  end = -1;

	  sscanf(line.c_str()+7, " \"%[^\"]\" %n", filename.get(), &end);
	  if((size_t)(end+7) == line.size()) {
	    m_sources.emplace(new std::ifstream(filename.get()), std::default_delete<std::istream>());
	    in = m_sources.top().get();
	    continue;
	  }
	}
	else if(line.compare(0, 6, "define") == 0) {
	  std::unique_ptr<char[]>  ident(new char[line.size()]);
	  std::unique_ptr<char[]>  value(new char[line.size()]);
	  int end = -1;

	  sscanf(line.c_str()+6, " %s %s %n", ident.get(), value.get(), &end);
		 if((size_t)(end+6) == line.size()) {
	    m_defines.emplace(ident.get(), value.get());
	    continue;
	  }
	}
	else if(line.compare(0, 5, "undef") == 0) {
	  std::unique_ptr<char[]>  ident(new char[line.size()]);
	  int end = -1;

	  sscanf(line.c_str()+5, " %s %n", ident.get(), &end);
	  if((size_t)(end+5) == line.size()) {
	    m_defines.erase(ident.get());
	    continue;
	  }
	}
	throw "Malformed directive: " + line;
      }

      // Skip Space
      if(isspace(c)) {
	newline = c == '\n';
	continue;
      }

      // Keywords and Identifiers
      if(isalpha(c) || (c == '_')) {
	std::string  w(1, (char)c);
	while(true) {
	  int const  cc = in->peek();
	  if(!isalnum(cc) && (cc != '_'))  break;
	  w += (char)in->get();
	}
	if(w == "component")  return  COMPONENT;
	if(w == "constant")   return  CONSTANT;
	if(w == "config")     return  CONFIG;
	if(w == "CHOOSE")     return  CHOOSE;
	if(w == "end")        return  END;
	if(w == "for")        return  FOR;
	if(w == "generate")   return  GENERATE;
	if(w == "ld")	      return  LD;
	if(w == "signal")     return  SIGNAL;
	auto const  it = m_defines.find(w);
	if(it != m_defines.end()) {
	  // Temporarily remove this macro definition to counter recursions
	  std::pair<std::string, std::string>  cap = *it;
	  m_sources.emplace(new std::stringstream(it->second),
			    [this, cap](std::istream *is) {
			      m_defines.emplace(cap.first, cap.second);
			      delete  is;
			    });
	  m_defines.erase(it);
	  in = m_sources.top().get();
	  continue;
	}

	sval = w;
	return  IDENT;
      }

      // Numbers
      if(isdigit(c)) {
	unsigned  v = c - '0';
	while(true) {
	  if(!isdigit(in->peek()))  break;
	  v = 10*v + (in->get() - '0');
	}
	sval = v;
	return  NUMBER;
      }

    err:
      error(std::string("Illegal Character: '") + (char)c + "'");
    }
  } // nextToken()

#line 313 "QdlParser.cpp"
#include <vector>
class QdlParser::YYStack {
  class Ele {
  public:
    unsigned short  state;
    YYSVal          sval;

  public:
    Ele(unsigned short _state, YYSVal const& _sval)
     : state(_state), sval(_sval) {}
    Ele(unsigned short _state)
     : state(_state) {}
  };
  typedef std::vector<Ele>  Stack;
  Stack  stack;

public:
  YYStack() {}
  ~YYStack() {}

public:
  void push(unsigned short state) {
    stack.push_back(Ele(state));
  }
  void push(unsigned short state, YYSVal const& sval) {
    stack.push_back(Ele(state, sval));
  }
  void pop(unsigned cnt) {
    Stack::iterator  it = stack.end();
    stack.erase(it-cnt, it);
  }

  YYSVal& operator[](unsigned idx) { return  (stack.rbegin()+idx)->sval; }
  unsigned short operator*() const { return  stack.back().state; }
};

char const *const  QdlParser::yyterms[] = { "EOF", 
"POWER", "NUMBER", "IDENT", "CHOOSE", "COMPONENT", "CONFIG", "CONSTANT", "END",
"FOR", "GENERATE", "LD", "MAPSTO", "SIGNAL", "THROUGH", "'<'", "','",
"'('", "'>'", "')'", "';'", "'='", "':'", "'['", "']'",
"'~'", "'+'", "'-'", "'#'", "'*'", "'/'", "'%'", "'&'",
"'|'", "'^'", "'?'", };
unsigned short const  QdlParser::yyintern[] = {
     0,    270,    270,    270,    270,    270,    270,    270,
   270,    270,    270,    270,    270,    270,    270,    270,
   270,    270,    270,    270,    270,    270,    270,    270,
   270,    270,    270,    270,    270,    270,    270,    270,
   270,    270,    270,     28,    270,     31,     32,    270,
    17,     19,     29,     26,     16,     27,    270,     30,
   270,    270,    270,    270,    270,    270,    270,    270,
   270,    270,     22,     20,     15,     21,     18,     35,
   270,    270,    270,    270,    270,    270,    270,    270,
   270,    270,    270,    270,    270,    270,    270,    270,
   270,    270,    270,    270,    270,    270,    270,    270,
   270,    270,    270,     23,    270,     24,     34,    270,
   270,    270,    270,    270,    270,    270,    270,    270,
   270,    270,    270,    270,    270,    270,    270,    270,
   270,    270,    270,    270,    270,    270,    270,    270,
   270,    270,    270,    270,     33,    270,     25,    270,
   270,    270,    270,    270,    270,    270,    270,    270,
   270,    270,    270,    270,    270,    270,    270,    270,
   270,    270,    270,    270,    270,    270,    270,    270,
   270,    270,    270,    270,    270,    270,    270,    270,
   270,    270,    270,    270,    270,    270,    270,    270,
   270,    270,    270,    270,    270,    270,    270,    270,
   270,    270,    270,    270,    270,    270,    270,    270,
   270,    270,    270,    270,    270,    270,    270,    270,
   270,    270,    270,    270,    270,    270,    270,    270,
   270,    270,    270,    270,    270,    270,    270,    270,
   270,    270,    270,    270,    270,    270,    270,    270,
   270,    270,    270,    270,    270,    270,    270,    270,
   270,    270,    270,    270,    270,    270,    270,    270,
   270,    270,    270,    270,    270,    270,    270,    270,
   270,    270,    270,    270,    270,    270,    270,    270,
   270,    270,    270,    270,    270,    270,    270,    270,
     1,      2,      3,      4,      5,      6,      7,      8,
     9,     10,     11,     12,     13,     14, };

#ifdef TRACE
char const *const  QdlParser::yyrules[] = {
"   0: [ 0] $        -> comp_decl",
"   1: [ 2] comp_dec -> COMPONENT IDENT",
"   2: [ 2] comp_dec -> comp_decl_start '<' IDENT",
"   3: [ 2] comp_dec -> comp_decl_gen ',' IDENT",
"   4: [ 0] comp_dec -> comp_decl_start '(' bus",
"   5: [ 0] comp_dec -> comp_decl_gen '>' '(' bus",
"   6: [ 0] comp_dec -> comp_decl_in ',' bus",
"   7: [ 3] comp_dec -> comp_decl_in MAPSTO bus",
"   8: [ 0] comp_dec -> comp_decl_out ',' bus",
"   9: [ 0] comp_dec -> comp_decl_out ')'",
"  10: [ 0] comp_dec -> comp_decl_body stmt ';'",
"  11: [ 0] comp_dec -> comp_decl_body END ';'",
"  12: [ 0] comp_dec -> comp_decl comp_decl_body END ';'",
"  13: [ 0] stmt     -> CONSTANT IDENT '=' expr",
"  14: [ 3] stmt     -> CONFIG bus",
"  15: [ 3] stmt     -> SIGNAL bus",
"  16: [ 0] stmt     -> expr '=' expr",
"  17: [ 0] stmt     -> inst_out ')'",
"  18: [ 3] stmt     -> generate END",
"  19: [ 2] inst_sta -> IDENT ':' IDENT",
"  20: [ 0] inst_gen -> inst_start '<' expr",
"  21: [ 0] inst_gen -> inst_gen ',' expr",
"  22: [ 0] inst_in  -> inst_start '(' expr",
"  23: [ 0] inst_in  -> inst_gen '>' '(' expr",
"  24: [ 0] inst_in  -> inst_in ',' expr",
"  25: [ 3] inst_out -> inst_in MAPSTO expr",
"  26: [ 0] inst_out -> inst_out ',' expr",
"  27: [ 3] generate -> FOR IDENT '=' expr THROUGH expr GENERATE",
"  28: [ 0] generate -> generate stmt ';'",
"  29: [ 2] bus      -> IDENT",
"  30: [ 0] bus      -> IDENT '[' expr ']'",
"  31: [ 2] ex_atom  -> NUMBER",
"  32: [ 2] ex_atom  -> IDENT",
"  33: [ 0] ex_atom  -> '~' ex_atom",
"  34: [ 0] ex_atom  -> '+' ex_atom",
"  35: [ 0] ex_atom  -> '-' ex_atom",
"  36: [ 3] ex_atom  -> LD ex_atom",
"  37: [ 0] ex_atom  -> CHOOSE '<' expr '>' '(' expr ')'",
"  38: [ 0] ex_atom  -> '(' expr ')'",
"  39: [ 0] ex_atom  -> ex_atom '[' expr ']'",
"  40: [ 0] ex_atom  -> ex_atom '[' expr ':' expr ']'",
"  41: [ 1] ex_atom  -> ex_atom POWER ex_atom",
"  42: [ 0] ex_cat   -> ex_atom",
"  43: [ 0] ex_cat   -> ex_cat '#' ex_atom",
"  44: [ 0] ex_mult  -> ex_cat",
"  45: [ 0] ex_mult  -> ex_mult '*' ex_cat",
"  46: [ 0] ex_mult  -> ex_mult '/' ex_cat",
"  47: [ 0] ex_mult  -> ex_mult '%' ex_cat",
"  48: [ 0] ex_logic -> ex_mult",
"  49: [ 0] ex_logic -> ex_logic '&' ex_mult",
"  50: [ 0] ex_logic -> ex_logic '|' ex_mult",
"  51: [ 0] ex_logic -> ex_logic '^' ex_mult",
"  52: [ 0] ex_add   -> ex_logic",
"  53: [ 0] ex_add   -> ex_add '+' ex_logic",
"  54: [ 0] ex_add   -> ex_add '-' ex_logic",
"  55: [ 0] expr     -> ex_add",
"  56: [ 0] expr     -> ex_add '?' ex_add ':' expr",
};
#endif
unsigned short const QdlParser::yylength[] = {
     1,      2,      3,      3,      3,      4,      3,      3,
     3,      2,      3,      3,      4,      4,      2,      2,
     3,      2,      2,      3,      3,      3,      3,      4,
     3,      3,      3,      7,      3,      1,      4,      1,
     1,      2,      2,      2,      2,      7,      3,      4,
     6,      3,      1,      3,      1,      3,      3,      3,
     1,      3,      3,      3,      1,      3,      3,      1,
     5, };
unsigned short const QdlParser::yylhs   [] = {
   (unsigned short)~0u,      1,      2,      2,      3,      3,      3,      5,
     5,      6,      6,      0,      0,      7,      7,      7,
     7,      7,      7,     11,     12,     12,     13,     13,
    13,      9,      9,     10,     10,      4,      4,     14,
    14,     14,     14,     14,     14,     14,     14,     14,
    14,     14,     15,     15,     16,     16,     16,     16,
    17,     17,     17,     17,     18,     18,     18,      8,
     8, };

unsigned short const  QdlParser::yygoto  [][19] = {
{      6,      5,      4,      3,      0,      2,      1,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,     21,     20,     19,     18,     17,     16,     15,     14,     13,     12,     11,     10,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      5,      4,      3,      0,      2,      9,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,     21,     20,     19,     18,     17,     16,     15,     14,     13,     12,     11,     10,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,     80,     20,     19,     18,     17,     16,     15,     14,     13,     12,     11,     10,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,     65,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,     54,      0,      0,      0,      0,  },
{      0,      0,      0,      0,     49,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,     47,      0,      0,      0,      0,      0,     14,     13,     12,     11,     10,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,     46,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,     45,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,     35,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,     44,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,     39,      0,      0,      0,      0,      0,     14,     13,     12,     11,     10,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,     42,      0,      0,      0,      0,      0,     14,     13,     12,     11,     10,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,     52,      0,      0,      0,      0,      0,     14,     13,     12,     11,     10,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,     57,      0,      0,      0,      0,      0,     14,     13,     12,     11,     10,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,     59,      0,      0,      0,      0,      0,     14,     13,     12,     11,     10,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,     64,      0,      0,      0,      0,      0,     14,     13,     12,     11,     10,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,     67,      0,      0,      0,      0,      0,     14,     13,     12,     11,     10,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,     70,      0,      0,      0,      0,      0,     14,     13,     12,     11,     10,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,     76,      0,      0,      0,      0,      0,     14,     13,     12,     11,     10,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,     79,      0,      0,      0,      0,      0,     14,     13,     12,     11,     10,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,     86,      0,      0,      0,      0,      0,     14,     13,     12,     11,     10,  },
{      0,      0,      0,      0,      0,      0,      0,      0,     85,      0,      0,      0,      0,      0,     14,     13,     12,     11,     10,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,     91,      0,      0,      0,      0,      0,     14,     13,     12,     11,     10,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,     90,      0,      0,      0,      0,      0,     14,     13,     12,     11,     10,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,     95,      0,      0,      0,      0,      0,     14,     13,     12,     11,     10,  },
{      0,      0,      0,      0,      0,      0,      0,      0,     94,      0,      0,      0,      0,      0,     14,     13,     12,     11,     10,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,     97,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,     14,    103,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,     14,    102,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,     14,    101,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,     14,     13,    109,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,     14,     13,    108,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,     14,     13,    107,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,     14,     13,     12,    117,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,     14,     13,     12,    116,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,     14,     13,     12,     11,    113,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,    115,      0,      0,      0,      0,      0,     14,     13,     12,     11,     10,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,    120,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,    125,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,    130,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,    129,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,    133,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
};

signed short const  QdlParser::yyaction[][36] = {
{      0,      0,      0,      0,      0,      7,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,     22,     23,     24,      0,     25,     26,    134,     28,      0,     29,      0,     30,      0,      0,      0,     31,      0,      0,      0,      0,      0,      0,      0,     32,     33,     34,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,    131,      0,      0,    132,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,    127,      0,      0,      0,    128,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,    122,      0,    123,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,    118,      0,    119,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      1,      0,      0,      0,      0,      7,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      8,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,     -1,      0,     -1,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,     22,     23,     24,      0,     25,     26,     27,     28,      0,     29,      0,     30,      0,      0,      0,     31,      0,      0,      0,      0,      0,      0,      0,     32,     33,     34,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,    -55,      0,    -55,      0,    -55,      0,    -55,      0,    -55,    -55,    -55,    -55,    -55,      0,    -55,      0,    110,    111,      0,      0,      0,      0,      0,      0,      0,    112,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,    -52,      0,    -52,      0,    -52,      0,    -52,      0,    -52,    -52,    -52,    -52,    -52,      0,    -52,      0,    -52,    -52,      0,      0,      0,      0,    104,    105,    106,    -52,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,    -48,      0,    -48,      0,    -48,      0,    -48,      0,    -48,    -48,    -48,    -48,    -48,      0,    -48,      0,    -48,    -48,      0,     98,     99,    100,    -48,    -48,    -48,    -48,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,    -44,      0,    -44,      0,    -44,      0,    -44,      0,    -44,    -44,    -44,    -44,    -44,      0,    -44,      0,    -44,    -44,     96,    -44,    -44,    -44,    -44,    -44,    -44,    -44,  },
{      0,     37,      0,      0,      0,      0,      0,      0,      0,      0,    -42,      0,    -42,      0,    -42,      0,    -42,      0,    -42,    -42,    -42,    -42,    -42,     38,    -42,      0,    -42,    -42,    -42,    -42,    -42,    -42,    -42,    -42,    -42,    -42,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,     92,      0,      0,      0,     93,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,     87,      0,     88,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,     83,      0,     84,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,     22,     23,     24,      0,     25,     26,     81,     28,      0,     29,      0,     30,      0,      0,      0,     31,      0,      0,      0,      0,      0,      0,      0,     32,     33,     34,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,     77,      0,      0,     78,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,     75,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,     74,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,    -31,      0,      0,      0,      0,      0,      0,      0,      0,    -31,      0,    -31,      0,    -31,      0,    -31,      0,    -31,    -31,    -31,    -31,    -31,    -31,    -31,      0,    -31,    -31,    -31,    -31,    -31,    -31,    -31,    -31,    -31,    -31,  },
{      0,    -32,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,    -32,     72,    -32,      0,      0,    -32,    -32,    -32,    -32,    -32,    -32,    -32,    -32,    -32,    -32,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,     66,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,     50,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,     62,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,     61,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,     55,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,     22,     36,     24,      0,      0,      0,      0,      0,      0,     29,      0,      0,      0,      0,      0,     31,      0,      0,      0,      0,      0,      0,      0,     32,     33,     34,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,     50,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,     22,     36,     24,      0,      0,      0,      0,      0,      0,     29,      0,      0,      0,      0,      0,     31,      0,      0,      0,      0,      0,      0,      0,     32,     33,     34,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,     22,     36,     24,      0,      0,      0,      0,      0,      0,     29,      0,      0,      0,      0,      0,     31,      0,      0,      0,      0,      0,      0,      0,     32,     33,     34,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,     22,     36,     24,      0,      0,      0,      0,      0,      0,     29,      0,      0,      0,      0,      0,     31,      0,      0,      0,      0,      0,      0,      0,     32,     33,     34,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,     22,     36,     24,      0,      0,      0,      0,      0,      0,     29,      0,      0,      0,      0,      0,     31,      0,      0,      0,      0,      0,      0,      0,     32,     33,     34,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,     37,      0,      0,      0,      0,      0,      0,      0,      0,    -35,      0,    -35,      0,    -35,      0,    -35,      0,    -35,    -35,    -35,    -35,    -35,     38,    -35,      0,    -35,    -35,    -35,    -35,    -35,    -35,    -35,    -35,    -35,    -35,  },
{      0,    -32,      0,      0,      0,      0,      0,      0,      0,      0,    -32,      0,    -32,      0,    -32,      0,    -32,      0,    -32,    -32,    -32,    -32,    -32,    -32,    -32,      0,    -32,    -32,    -32,    -32,    -32,    -32,    -32,    -32,    -32,    -32,  },
{      0,      0,     22,     36,     24,      0,      0,      0,      0,      0,      0,     29,      0,      0,      0,      0,      0,     31,      0,      0,      0,      0,      0,      0,      0,     32,     33,     34,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,     22,     36,     24,      0,      0,      0,      0,      0,      0,     29,      0,      0,      0,      0,      0,     31,      0,      0,      0,      0,      0,      0,      0,     32,     33,     34,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,     40,      0,     41,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,     22,     36,     24,      0,      0,      0,      0,      0,      0,     29,      0,      0,      0,      0,      0,     31,      0,      0,      0,      0,      0,      0,      0,     32,     33,     34,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,    -39,      0,      0,      0,      0,      0,      0,      0,      0,    -39,      0,    -39,      0,    -39,      0,    -39,      0,    -39,    -39,    -39,    -39,    -39,    -39,    -39,      0,    -39,    -39,    -39,    -39,    -39,    -39,    -39,    -39,    -39,    -39,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,     43,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,    -40,      0,      0,      0,      0,      0,      0,      0,      0,    -40,      0,    -40,      0,    -40,      0,    -40,      0,    -40,    -40,    -40,    -40,    -40,    -40,    -40,      0,    -40,    -40,    -40,    -40,    -40,    -40,    -40,    -40,    -40,    -40,  },
{      0,    -41,      0,      0,      0,      0,      0,      0,      0,      0,    -41,      0,    -41,      0,    -41,      0,    -41,      0,    -41,    -41,    -41,    -41,    -41,     38,    -41,      0,    -41,    -41,    -41,    -41,    -41,    -41,    -41,    -41,    -41,    -41,  },
{      0,     37,      0,      0,      0,      0,      0,      0,      0,      0,    -34,      0,    -34,      0,    -34,      0,    -34,      0,    -34,    -34,    -34,    -34,    -34,     38,    -34,      0,    -34,    -34,    -34,    -34,    -34,    -34,    -34,    -34,    -34,    -34,  },
{      0,     37,      0,      0,      0,      0,      0,      0,      0,      0,    -33,      0,    -33,      0,    -33,      0,    -33,      0,    -33,    -33,    -33,    -33,    -33,     38,    -33,      0,    -33,    -33,    -33,    -33,    -33,    -33,    -33,    -33,    -33,    -33,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,     48,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,    -38,      0,      0,      0,      0,      0,      0,      0,      0,    -38,      0,    -38,      0,    -38,      0,    -38,      0,    -38,    -38,    -38,    -38,    -38,    -38,    -38,      0,    -38,    -38,    -38,    -38,    -38,    -38,    -38,    -38,    -38,    -38,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,    -15,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,    -29,      0,      0,      0,    -29,      0,      0,    -29,    -29,      0,      0,     51,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,     22,     36,     24,      0,      0,      0,      0,      0,      0,     29,      0,      0,      0,      0,      0,     31,      0,      0,      0,      0,      0,      0,      0,     32,     33,     34,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,     53,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,    -30,      0,      0,      0,    -30,      0,      0,    -30,    -30,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,    -36,      0,      0,      0,      0,      0,      0,      0,      0,    -36,      0,    -36,      0,    -36,      0,    -36,      0,    -36,    -36,    -36,    -36,    -36,     38,    -36,      0,    -36,    -36,    -36,    -36,    -36,    -36,    -36,    -36,    -36,    -36,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,     56,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,     22,     36,     24,      0,      0,      0,      0,      0,      0,     29,      0,      0,      0,      0,      0,     31,      0,      0,      0,      0,      0,      0,      0,     32,     33,     34,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,     58,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,     22,     36,     24,      0,      0,      0,      0,      0,      0,     29,      0,      0,      0,      0,      0,     31,      0,      0,      0,      0,      0,      0,      0,     32,     33,     34,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,     60,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,    -27,    -27,    -27,      0,    -27,    -27,    -27,    -27,      0,    -27,      0,    -27,      0,      0,      0,    -27,      0,      0,      0,      0,      0,      0,      0,    -27,    -27,    -27,      0,      0,      0,      0,      0,      0,      0,      0,  },
{    -12,      0,      0,      0,      0,    -12,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,     63,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,     22,     36,     24,      0,      0,      0,      0,      0,      0,     29,      0,      0,      0,      0,      0,     31,      0,      0,      0,      0,      0,      0,      0,     32,     33,     34,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,    -13,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,    -14,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,     22,     36,     24,      0,      0,      0,      0,      0,      0,     29,      0,      0,      0,      0,      0,     31,      0,      0,      0,      0,      0,      0,      0,     32,     33,     34,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,     68,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,     69,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,     22,     36,     24,      0,      0,      0,      0,      0,      0,     29,      0,      0,      0,      0,      0,     31,      0,      0,      0,      0,      0,      0,      0,     32,     33,     34,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,     71,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,    -37,      0,      0,      0,      0,      0,      0,      0,      0,    -37,      0,    -37,      0,    -37,      0,    -37,      0,    -37,    -37,    -37,    -37,    -37,    -37,    -37,      0,    -37,    -37,    -37,    -37,    -37,    -37,    -37,    -37,    -37,    -37,  },
{      0,      0,      0,     73,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,    -19,      0,    -19,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,    -10,    -10,    -10,      0,    -10,    -10,    -10,    -10,      0,    -10,      0,    -10,      0,      0,      0,    -10,      0,      0,      0,      0,      0,      0,      0,    -10,    -10,    -10,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,     22,     36,     24,      0,      0,      0,      0,      0,      0,     29,      0,      0,      0,      0,      0,     31,      0,      0,      0,      0,      0,      0,      0,     32,     33,     34,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,    -16,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,     22,     36,     24,      0,      0,      0,      0,      0,      0,     29,      0,      0,      0,      0,      0,     31,      0,      0,      0,      0,      0,      0,      0,     32,     33,     34,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,    -17,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,    -26,      0,      0,    -26,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,     82,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,    -18,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,    -28,    -28,    -28,      0,    -28,    -28,    -28,    -28,      0,    -28,      0,    -28,      0,      0,      0,    -28,      0,      0,      0,      0,      0,      0,      0,    -28,    -28,    -28,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,     22,     36,     24,      0,      0,      0,      0,      0,      0,     29,      0,      0,      0,      0,      0,     31,      0,      0,      0,      0,      0,      0,      0,     32,     33,     34,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,     22,     36,     24,      0,      0,      0,      0,      0,      0,     29,      0,      0,      0,      0,      0,     31,      0,      0,      0,      0,      0,      0,      0,     32,     33,     34,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,    -22,      0,      0,      0,    -22,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,    -20,      0,    -20,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,     22,     36,     24,      0,      0,      0,      0,      0,      0,     29,      0,      0,      0,      0,      0,     31,      0,      0,      0,      0,      0,      0,      0,     32,     33,     34,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,     89,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,     22,     36,     24,      0,      0,      0,      0,      0,      0,     29,      0,      0,      0,      0,      0,     31,      0,      0,      0,      0,      0,      0,      0,     32,     33,     34,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,    -23,      0,      0,      0,    -23,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,    -21,      0,    -21,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,     22,     36,     24,      0,      0,      0,      0,      0,      0,     29,      0,      0,      0,      0,      0,     31,      0,      0,      0,      0,      0,      0,      0,     32,     33,     34,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,     22,     36,     24,      0,      0,      0,      0,      0,      0,     29,      0,      0,      0,      0,      0,     31,      0,      0,      0,      0,      0,      0,      0,     32,     33,     34,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,    -24,      0,      0,      0,    -24,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,    -25,      0,      0,    -25,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,     22,     36,     24,      0,      0,      0,      0,      0,      0,     29,      0,      0,      0,      0,      0,     31,      0,      0,      0,      0,      0,      0,      0,     32,     33,     34,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,     37,      0,      0,      0,      0,      0,      0,      0,      0,    -43,      0,    -43,      0,    -43,      0,    -43,      0,    -43,    -43,    -43,    -43,    -43,     38,    -43,      0,    -43,    -43,    -43,    -43,    -43,    -43,    -43,    -43,    -43,    -43,  },
{      0,      0,     22,     36,     24,      0,      0,      0,      0,      0,      0,     29,      0,      0,      0,      0,      0,     31,      0,      0,      0,      0,      0,      0,      0,     32,     33,     34,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,     22,     36,     24,      0,      0,      0,      0,      0,      0,     29,      0,      0,      0,      0,      0,     31,      0,      0,      0,      0,      0,      0,      0,     32,     33,     34,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,     22,     36,     24,      0,      0,      0,      0,      0,      0,     29,      0,      0,      0,      0,      0,     31,      0,      0,      0,      0,      0,      0,      0,     32,     33,     34,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,    -47,      0,    -47,      0,    -47,      0,    -47,      0,    -47,    -47,    -47,    -47,    -47,      0,    -47,      0,    -47,    -47,     96,    -47,    -47,    -47,    -47,    -47,    -47,    -47,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,    -46,      0,    -46,      0,    -46,      0,    -46,      0,    -46,    -46,    -46,    -46,    -46,      0,    -46,      0,    -46,    -46,     96,    -46,    -46,    -46,    -46,    -46,    -46,    -46,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,    -45,      0,    -45,      0,    -45,      0,    -45,      0,    -45,    -45,    -45,    -45,    -45,      0,    -45,      0,    -45,    -45,     96,    -45,    -45,    -45,    -45,    -45,    -45,    -45,  },
{      0,      0,     22,     36,     24,      0,      0,      0,      0,      0,      0,     29,      0,      0,      0,      0,      0,     31,      0,      0,      0,      0,      0,      0,      0,     32,     33,     34,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,     22,     36,     24,      0,      0,      0,      0,      0,      0,     29,      0,      0,      0,      0,      0,     31,      0,      0,      0,      0,      0,      0,      0,     32,     33,     34,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,     22,     36,     24,      0,      0,      0,      0,      0,      0,     29,      0,      0,      0,      0,      0,     31,      0,      0,      0,      0,      0,      0,      0,     32,     33,     34,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,    -51,      0,    -51,      0,    -51,      0,    -51,      0,    -51,    -51,    -51,    -51,    -51,      0,    -51,      0,    -51,    -51,      0,     98,     99,    100,    -51,    -51,    -51,    -51,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,    -50,      0,    -50,      0,    -50,      0,    -50,      0,    -50,    -50,    -50,    -50,    -50,      0,    -50,      0,    -50,    -50,      0,     98,     99,    100,    -50,    -50,    -50,    -50,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,    -49,      0,    -49,      0,    -49,      0,    -49,      0,    -49,    -49,    -49,    -49,    -49,      0,    -49,      0,    -49,    -49,      0,     98,     99,    100,    -49,    -49,    -49,    -49,  },
{      0,      0,     22,     36,     24,      0,      0,      0,      0,      0,      0,     29,      0,      0,      0,      0,      0,     31,      0,      0,      0,      0,      0,      0,      0,     32,     33,     34,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,     22,     36,     24,      0,      0,      0,      0,      0,      0,     29,      0,      0,      0,      0,      0,     31,      0,      0,      0,      0,      0,      0,      0,     32,     33,     34,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,     22,     36,     24,      0,      0,      0,      0,      0,      0,     29,      0,      0,      0,      0,      0,     31,      0,      0,      0,      0,      0,      0,      0,     32,     33,     34,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,    114,      0,      0,      0,    110,    111,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,     22,     36,     24,      0,      0,      0,      0,      0,      0,     29,      0,      0,      0,      0,      0,     31,      0,      0,      0,      0,      0,      0,      0,     32,     33,     34,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,    -56,      0,    -56,      0,    -56,      0,    -56,      0,    -56,    -56,    -56,    -56,    -56,      0,    -56,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,    -54,      0,    -54,      0,    -54,      0,    -54,      0,    -54,    -54,    -54,    -54,    -54,      0,    -54,      0,    -54,    -54,      0,      0,      0,      0,    104,    105,    106,    -54,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,    -53,      0,    -53,      0,    -53,      0,    -53,      0,    -53,    -53,    -53,    -53,    -53,      0,    -53,      0,    -53,    -53,      0,      0,      0,      0,    104,    105,    106,    -53,  },
{      0,      0,      0,    121,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,     50,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,     -4,      0,      0,      0,     -4,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,     -2,      0,     -2,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,    126,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,    124,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,     50,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,     -5,      0,      0,      0,     -5,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,     -3,      0,     -3,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,     50,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,     50,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,     -6,      0,      0,      0,     -6,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,     -7,      0,      0,     -7,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,     50,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,     -9,     -9,     -9,      0,     -9,     -9,     -9,     -9,      0,     -9,      0,     -9,      0,      0,      0,     -9,      0,      0,      0,      0,      0,      0,      0,     -9,     -9,     -9,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,     -8,      0,      0,     -8,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,    135,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
{    -11,      0,      0,      0,      0,    -11,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,  },
};

void QdlParser::parse() {
  YYStack  yystack;
  yystack.push(0);

  // Fetch until error (throw) or accept (return)
  while(true) {
    // Current lookahead
    YYSVal          yysval;
    unsigned short  yytok = nextToken(yysval);

    if(yytok <  YYINTERN)  yytok = yyintern[yytok];
    if(yytok >= YYINTERN)  error("Unknown Token");
#ifdef TRACE
    std::cerr << "Read " << yyterms[yytok] << std::endl;
#endif

    // Reduce until shift
    while(true) {
      signed short const  yyact = yyaction[*yystack][yytok];
      if(yyact == 0) {
        std::string                yymsg("Expecting (");
        signed short const *const  yyrow = yyaction[*yystack];
        for(unsigned  i = 0; i < 36; i++) {
          if(yyrow[i])  yymsg.append(yyterms[i]) += '|';
        }
        *yymsg.rbegin() = ')';
        error(yymsg.append(" instead of ").append(yyterms[yytok]));
        return;
      }
      if(yyact >  1) { // shift
#ifdef TRACE
        std::cerr << "Push " << yyterms[yytok] << std::endl;
#endif
        yystack.push(yyact, yysval);
        break;
      }
      else {           // reduce (includes accept)
        YYSVal                yylval;
        unsigned short const  yyrno = (yyact < 0)? -yyact : 0;
        unsigned short const  yylen = yylength[yyrno];
        
#ifdef TRACE
        std::cerr << "Reduce by " << yyrules[yyrno] << std::endl;
#endif
        switch(yyrno) { // do semantic actions
        case 0:         // accept
          return;
case 1: {
#line 352 "QdlParser.ypp"

                    yylval = m_lib.declareComponent(yystack[yylen - 2].name());
                  
#line 802 "QdlParser.cpp"
break;
}
case 2: {
#line 356 "QdlParser.ypp"

                  yystack[yylen - 1].comp().addParameter(yystack[yylen - 3].name());
		  yylval = yystack[yylen - 1];
                
#line 811 "QdlParser.cpp"
break;
}
case 3: {
#line 360 "QdlParser.ypp"

                  yystack[yylen - 1].comp().addParameter(yystack[yylen - 3].name());
		  yylval = yystack[yylen - 1];
                
#line 820 "QdlParser.cpp"
break;
}
case 4: {
#line 365 "QdlParser.ypp"

                  yystack[yylen - 1].comp().addPort(PortDecl::Direction::in, yystack[yylen - 3].name(), yystack[yylen - 3].width());
		  yylval = yystack[yylen - 1];
                
#line 829 "QdlParser.cpp"
break;
}
case 5: {
#line 369 "QdlParser.ypp"

                  yystack[yylen - 1].comp().addPort(PortDecl::Direction::in, yystack[yylen - 4].name(), yystack[yylen - 4].width());
		  yylval = yystack[yylen - 1];
                
#line 838 "QdlParser.cpp"
break;
}
case 6: {
#line 373 "QdlParser.ypp"

 		  yystack[yylen - 1].comp().addPort(PortDecl::Direction::in, yystack[yylen - 3].name(), yystack[yylen - 3].width());
		  yylval = yystack[yylen - 1];
                
#line 847 "QdlParser.cpp"
break;
}
case 7: {
#line 377 "QdlParser.ypp"

                  yystack[yylen - 1].comp().addPort(PortDecl::Direction::out, yystack[yylen - 3].name(), yystack[yylen - 3].width());
		  yylval = yystack[yylen - 1];
                
#line 856 "QdlParser.cpp"
break;
}
case 8: {
#line 381 "QdlParser.ypp"

 		  yystack[yylen - 1].comp().addPort(PortDecl::Direction::out, yystack[yylen - 3].name(), yystack[yylen - 3].width());
		  yylval = yystack[yylen - 1];
		
#line 865 "QdlParser.cpp"
break;
}
case 9: {
#line 385 "QdlParser.ypp"
 yylval = yystack[yylen - 1]; 
#line 871 "QdlParser.cpp"
break;
}
case 10: {
#line 386 "QdlParser.ypp"

		  yystack[yylen - 1].comp().addStatement(yystack[yylen - 2].stmt());
		  yylval = yystack[yylen - 1];
                
#line 880 "QdlParser.cpp"
break;
}
case 13: {
#line 394 "QdlParser.ypp"

            yylval = std::make_shared<ConstDecl>(yystack[yylen - 2].name(), yystack[yylen - 4].expr());
          
#line 888 "QdlParser.cpp"
break;
}
case 14: {
#line 397 "QdlParser.ypp"
 yylval = std::make_shared<ConfigDecl>(yystack[yylen - 2].name(), yystack[yylen - 2].width()); 
#line 894 "QdlParser.cpp"
break;
}
case 15: {
#line 398 "QdlParser.ypp"
 yylval = std::make_shared<SignalDecl>(yystack[yylen - 2].name(), yystack[yylen - 2].width()); 
#line 900 "QdlParser.cpp"
break;
}
case 16: {
#line 399 "QdlParser.ypp"
 yylval = std::make_shared<Equation>(yystack[yylen - 1].expr(), yystack[yylen - 3].expr()); 
#line 906 "QdlParser.cpp"
break;
}
case 17: {
#line 400 "QdlParser.ypp"
 yylval = yystack[yylen - 1]; 
#line 912 "QdlParser.cpp"
break;
}
case 18: {
#line 401 "QdlParser.ypp"
 yylval = yystack[yylen - 1]; 
#line 918 "QdlParser.cpp"
break;
}
case 19: {
#line 403 "QdlParser.ypp"

               yylval = std::make_shared<Instantiation>(yystack[yylen - 1].name(), m_lib.resolveComponent(yystack[yylen - 3].name()));
             
#line 926 "QdlParser.cpp"
break;
}
case 20: {
#line 406 "QdlParser.ypp"

               static_cast<Instantiation&>(*yystack[yylen - 1].stmt()).addParameter(yystack[yylen - 3].expr());
	       yylval = yystack[yylen - 1];
             
#line 935 "QdlParser.cpp"
break;
}
case 21: {
#line 410 "QdlParser.ypp"

               static_cast<Instantiation&>(*yystack[yylen - 1].stmt()).addParameter(yystack[yylen - 3].expr());
	       yylval = yystack[yylen - 1];
             
#line 944 "QdlParser.cpp"
break;
}
case 22: {
#line 414 "QdlParser.ypp"

               static_cast<Instantiation&>(*yystack[yylen - 1].stmt()).addConnection(yystack[yylen - 3].expr());
	       yylval = yystack[yylen - 1];
             
#line 953 "QdlParser.cpp"
break;
}
case 23: {
#line 418 "QdlParser.ypp"

               static_cast<Instantiation&>(*yystack[yylen - 1].stmt()).addConnection(yystack[yylen - 4].expr());
	       yylval = yystack[yylen - 1];
             
#line 962 "QdlParser.cpp"
break;
}
case 24: {
#line 422 "QdlParser.ypp"

               static_cast<Instantiation&>(*yystack[yylen - 1].stmt()).addConnection(yystack[yylen - 3].expr());
	       yylval = yystack[yylen - 1];
             
#line 971 "QdlParser.cpp"
break;
}
case 25: {
#line 427 "QdlParser.ypp"

               static_cast<Instantiation&>(*yystack[yylen - 1].stmt()).addConnection(yystack[yylen - 3].expr());
	       yylval = yystack[yylen - 1];
             
#line 980 "QdlParser.cpp"
break;
}
case 26: {
#line 431 "QdlParser.ypp"

               static_cast<Instantiation&>(*yystack[yylen - 1].stmt()).addConnection(yystack[yylen - 3].expr());
	       yylval = yystack[yylen - 1];
             
#line 989 "QdlParser.cpp"
break;
}
case 27: {
#line 436 "QdlParser.ypp"

	       yylval = std::make_shared<Generate>(yystack[yylen - 2].name(), yystack[yylen - 4].expr(), yystack[yylen - 6].expr());
	     
#line 997 "QdlParser.cpp"
break;
}
case 28: {
#line 439 "QdlParser.ypp"

	       static_cast<Generate&>(*yystack[yylen - 1].stmt()).addStatement(yystack[yylen - 2].stmt());
	       yylval = yystack[yylen - 1];
	     
#line 1006 "QdlParser.cpp"
break;
}
case 29: {
#line 445 "QdlParser.ypp"

            yylval.makeBus(yystack[yylen - 1].name(), std::make_shared<ConstExpression>(1));
          
#line 1014 "QdlParser.cpp"
break;
}
case 30: {
#line 448 "QdlParser.ypp"

	    yylval.makeBus(yystack[yylen - 1].name(), yystack[yylen - 3].expr());
	  
#line 1022 "QdlParser.cpp"
break;
}
case 31: {
#line 453 "QdlParser.ypp"

            yylval = std::make_shared<ConstExpression>(yystack[yylen - 1].number());
          
#line 1030 "QdlParser.cpp"
break;
}
case 32: {
#line 456 "QdlParser.ypp"

            yylval = std::make_shared<NameExpression>(yystack[yylen - 1].name());
          
#line 1038 "QdlParser.cpp"
break;
}
case 33: {
#line 459 "QdlParser.ypp"

	    yylval = std::make_shared<UniExpression>(UniExpression::Op::NOT, yystack[yylen - 2].expr());
          
#line 1046 "QdlParser.cpp"
break;
}
case 34: {
#line 462 "QdlParser.ypp"

	    yylval = yystack[yylen - 2];
	  
#line 1054 "QdlParser.cpp"
break;
}
case 35: {
#line 465 "QdlParser.ypp"

	    yylval = std::make_shared<UniExpression>(UniExpression::Op::NEG, yystack[yylen - 2].expr());
          
#line 1062 "QdlParser.cpp"
break;
}
case 36: {
#line 468 "QdlParser.ypp"

	    yylval = std::make_shared<UniExpression>(UniExpression::Op::LD, yystack[yylen - 2].expr());
          
#line 1070 "QdlParser.cpp"
break;
}
case 37: {
#line 471 "QdlParser.ypp"

	    yylval = std::make_shared<ChooseExpression>(yystack[yylen - 3].expr(), yystack[yylen - 6].expr());
	  
#line 1078 "QdlParser.cpp"
break;
}
case 38: {
#line 474 "QdlParser.ypp"

	    yylval = yystack[yylen - 2];
	  
#line 1086 "QdlParser.cpp"
break;
}
case 39: {
#line 477 "QdlParser.ypp"

	    yylval = std::make_shared<BiExpression>(BiExpression::Op::SEL, yystack[yylen - 1].expr(), yystack[yylen - 3].expr());
          
#line 1094 "QdlParser.cpp"
break;
}
case 40: {
#line 480 "QdlParser.ypp"

	    yylval = std::make_shared<RangeExpression>(yystack[yylen - 1].expr(), yystack[yylen - 3].expr(), yystack[yylen - 5].expr());
          
#line 1102 "QdlParser.cpp"
break;
}
case 41: {
#line 483 "QdlParser.ypp"

	    yylval = std::make_shared<BiExpression>(BiExpression::Op::POW, yystack[yylen - 1].expr(), yystack[yylen - 3].expr());
          
#line 1110 "QdlParser.cpp"
break;
}
case 42: {
#line 487 "QdlParser.ypp"

	    yylval = yystack[yylen - 1];
	  
#line 1118 "QdlParser.cpp"
break;
}
case 43: {
#line 490 "QdlParser.ypp"

	    yylval = std::make_shared<BiExpression>(BiExpression::Op::CAT, yystack[yylen - 1].expr(), yystack[yylen - 3].expr());
          
#line 1126 "QdlParser.cpp"
break;
}
case 44: {
#line 494 "QdlParser.ypp"

	    yylval = yystack[yylen - 1];
	  
#line 1134 "QdlParser.cpp"
break;
}
case 45: {
#line 497 "QdlParser.ypp"

	    yylval = std::make_shared<BiExpression>(BiExpression::Op::MUL, yystack[yylen - 1].expr(), yystack[yylen - 3].expr());
          
#line 1142 "QdlParser.cpp"
break;
}
case 46: {
#line 500 "QdlParser.ypp"

	    yylval = std::make_shared<BiExpression>(BiExpression::Op::DIV, yystack[yylen - 1].expr(), yystack[yylen - 3].expr());
          
#line 1150 "QdlParser.cpp"
break;
}
case 47: {
#line 503 "QdlParser.ypp"

	    yylval = std::make_shared<BiExpression>(BiExpression::Op::MOD, yystack[yylen - 1].expr(), yystack[yylen - 3].expr());
          
#line 1158 "QdlParser.cpp"
break;
}
case 48: {
#line 507 "QdlParser.ypp"

	    yylval = yystack[yylen - 1];
	  
#line 1166 "QdlParser.cpp"
break;
}
case 49: {
#line 510 "QdlParser.ypp"

	    yylval = std::make_shared<BiExpression>(BiExpression::Op::AND, yystack[yylen - 1].expr(), yystack[yylen - 3].expr());
          
#line 1174 "QdlParser.cpp"
break;
}
case 50: {
#line 513 "QdlParser.ypp"

	    yylval = std::make_shared<BiExpression>(BiExpression::Op::OR, yystack[yylen - 1].expr(), yystack[yylen - 3].expr());
          
#line 1182 "QdlParser.cpp"
break;
}
case 51: {
#line 516 "QdlParser.ypp"

	    yylval = std::make_shared<BiExpression>(BiExpression::Op::XOR, yystack[yylen - 1].expr(), yystack[yylen - 3].expr());
          
#line 1190 "QdlParser.cpp"
break;
}
case 52: {
#line 520 "QdlParser.ypp"

	    yylval = yystack[yylen - 1];
	  
#line 1198 "QdlParser.cpp"
break;
}
case 53: {
#line 523 "QdlParser.ypp"

	    yylval = std::make_shared<BiExpression>(BiExpression::Op::ADD, yystack[yylen - 1].expr(), yystack[yylen - 3].expr());
          
#line 1206 "QdlParser.cpp"
break;
}
case 54: {
#line 526 "QdlParser.ypp"

	    yylval = std::make_shared<BiExpression>(BiExpression::Op::SUB, yystack[yylen - 1].expr(), yystack[yylen - 3].expr());
          
#line 1214 "QdlParser.cpp"
break;
}
case 55: {
#line 529 "QdlParser.ypp"

	    yylval = yystack[yylen - 1];
	  
#line 1222 "QdlParser.cpp"
break;
}
case 56: {
#line 532 "QdlParser.ypp"

	    yylval = std::make_shared<CondExpression>(yystack[yylen - 1].expr(), yystack[yylen - 3].expr(), yystack[yylen - 5].expr());
          
#line 1230 "QdlParser.cpp"
break;
}
        }
        
        yystack.pop(yylen);
        yystack.push(yygoto[*yystack][yylhs[yyrno]], yylval);
      }
    }
  }
}
