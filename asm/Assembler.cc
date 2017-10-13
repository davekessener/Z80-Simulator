#include <regex>
#include <fstream>

#include "Assembler.h"
#include "lib.h"

#define MXT_MAXMACROS 1000

namespace z80 {

Assembler::Matcher Assembler::mInstructions;

namespace
{
	class StringBuffer
	{
		public:
			std::string next( ) { std::string r = buf_.front(); buf_.pop_front(); return r; }
			bool hasNext( ) const { return !buf_.empty(); }
			void push(const std::string&);
		private:
			std::deque<std::string> buf_;
	};

	void StringBuffer::push(const std::string& s)
	{
		std::vector<std::string> tmp;
		auto i1 = s.begin(), i2 = s.end();

		auto isID = [](char c, bool ext) -> bool
			{ return (c >= 'a' && c <= 'z') || c == '_' || (ext && (c >= '0' && c <= '9')); };

		while(i1 != i2)
		{
			auto i = i1;

			while(i1 != i2 && isID(*i1, i != i1)) ++i1;

			if(i == i1) ++i1;
			tmp.emplace_back(i, i1);
		}

		for(auto r1 = tmp.rbegin(), r2 = tmp.rend() ; r1 != r2 ; ++r1)
		{
			buf_.push_front(*r1);
		}
	}

	class StringSplitter
	{
		public:
			StringSplitter(const std::string&);
			std::string next( ) { return buf_[idx_++]; }
			bool hasNext( ) const { return idx_ < buf_.size(); }
		private:
			std::vector<std::string> buf_;
			uint idx_;
	};

	StringSplitter::StringSplitter(const std::string& s)
	{
		auto i1 = s.begin(), i2 = s.end();
		idx_ = 0;

		while(i1 != i2)
		{
			while(i1 != i2 && (*i1 == ',' || *i1 == ' ')) ++i1;
			if(i1 == i2) break;
			auto i = i1;
			while(i1 != i2 && (*i1 != ',' && *i1 != ' ')) ++i1;
			if(i == i1) break;
			buf_.emplace_back(i, i1);
		}
	}

	std::string toLower(const std::string& s)
	{
		std::vector<char> t;
		bool isStr = false, isChar = false, esc = false;

		for(auto i1 = s.begin(), i2 = s.end() ; i1 != i2 ; ++i1)
		{
			if(esc)
			{
				esc = false;
			}
			else if(isStr)
			{
				if(*i1 == '\\') esc = true;
				else if(*i1 == '"') isStr = false;
			}
			else if(isChar)
			{
				if(*i1 == '\\') esc = true;
				else if(*i1 == '\'') isChar = false;
			}
			else
			{
				if(*i1 == '"') isStr = true;
				else if(*i1 == '\'') isChar = true;
			}

			t.push_back(*i1 == '\t' ? ' ' : (isStr || isChar ? *i1 : (*i1 >= 'A' && *i1 <= 'Z' ? *i1 - 'A' + 'a' : *i1)));
		}

		return std::string(t.begin(), t.end());
	}
}

// # ===========================================================================

Assembler::Assembler(const std::string& fn)
{
	initialize();

	Preprocessor reader(fn);

	mAddress = 0x0000;

	while(reader.hasNext())
	{
		line_t line = reader.next();

		if(!line.error.empty())
		{
			addError(line.error, line.meta);
		}

		if(!line.line.empty()) try
		{
			mInstructions[line.line](this, line.meta);
		}
		catch(const std::string& e)
		{
			addError(e, line.meta);
		}
	}

	for(const raw_t& raw : mRaw)
	{
		entry_t entry;

		entry.meta = raw.meta;
		entry.address = raw.address;
		entry.data.insert(entry.data.end(), raw.ins.cbegin(), raw.ins.cend());

		mAddress = raw.address + raw.ins.size();

		for(const auto& e : raw.expr)
		{
			mAddress += e.size;
		}

		if(raw.expr.size() > 0) for(const auto& e : raw.expr)
		try
		{
			uint v = eval(e.raw, e.relative);

			for(uint i = 0 ; i < e.size ; ++i)
			{
				entry.data.push_back(v & 0xFF); v >>= 8;
			}
		}
		catch(const std::string& e)
		{
			addError(e, entry.meta);
		}

		mProgram.data.push_back(entry);
	}
}

void Assembler::add(meta_t meta, const std::initializer_list<uint8_t>& data)
{
	add({}, meta, data);
}

void Assembler::add(const std::initializer_list<expr_t>& expr, meta_t meta, const std::initializer_list<uint8_t>& data)
{
	raw_t raw;

	raw.expr.insert(raw.expr.end(), expr.begin(), expr.end());
	raw.meta = meta;
	raw.ins.insert(raw.ins.end(), data.begin(), data.end());
	raw.address = mAddress;

	mRaw.push_back(raw);
	mAddress += raw.ins.size();
	for(const auto& e : expr) mAddress += e.size;
}

namespace
{
	std::string eliminate(const std::string& s)
	{
		std::vector<char> buf;

		for(const auto& c : s)
		{
			if(c != ' ' && c != '\t') buf.push_back(c);
		}

		return std::string(buf.cbegin(), buf.cend());
	}

	uint str_to_i(const std::string& s)
	{
		uint v = 0;

		for(const auto& c : s)
		{
			if(c < '0' || c > '9')
			{
				throw lib::stringf("Invalid character '%c' in number \"%s\"! (Expected decimal digit)", c, s.c_str());
			}

			v = v * 10 + (c - '0');
		}

		return v;
	}

	class Tokenizer
	{
		public:
			Tokenizer(const std::string&);
			std::string top( ) const { return buf_[idx_]; }
			void pop( ) { ++idx_; }
			bool empty( ) const { return idx_ >= buf_.size(); }
		private:
			void push(const std::string&);
			void pushNum(const std::string&, uint);
			static bool isIn(const std::string& s, char c) { return std::find(s.begin(), s.end(), c) != s.end(); }
			static bool isNum(char c, uint base)
				{ static std::string lit = "0123456789abcdef"; return isIn(lit.substr(0, base), c); }
			static bool isAlpha(char c) { return (c >= 'a' && c <= 'z') || c == '_' || (c >= '0' && c <= '9'); }

		private:
			std::vector<std::string> buf_;
			uint idx_;
	};

	class Evaluator
	{
		typedef std::function<uint(const std::string&)> deref_fn;

		public:
			Evaluator(deref_fn f) : lookup_(f) { }
			uint eval(const std::string&);
		private:
			int evalAddExpr(Tokenizer&);
			int evalMulExpr(Tokenizer&);
			int evalUnaExpr(Tokenizer&);
			int evalNumExpr(Tokenizer&);

		private:
			deref_fn lookup_;
	};

	Tokenizer::Tokenizer(const std::string& s)
	{
		idx_ = 0;

		for(auto i1 = s.begin(), i2 = s.end(), i = i1 ; i1 != i2 ; i1 = i)
		{
			if(*i == '$')
			{
				while(++i != i2 && isNum(*i, 16));
			}
			else if(*i == '0')
			{
				if(++i != i2 && (isIn("bdx", *i) || isNum(*i, 8)))
				{
					while(++i != i2 && isNum(*i, 16));
				}
			}
			else if(isNum(*i, 10))
			{
				while(++i != i2 && isNum(*i, 10));
			}
			else if(*i == '_' || (*i >= 'a' && *i <= 'z'))
			{
				while(++i != i2 && isAlpha(*i));
			}
			else switch(*i)
			{
				case '+':
				case '-':
				case '*':
				case '/':
				case '(':
				case ')':
				case '<':
				case '>':
				case '&':
				case '|':
				case '^':
				case '%':
				case '~':
					++i;
					break;
				default:
					throw lib::stringf("Invalid character in expression \"%s\": '%c'!", s.c_str(), *i);
			}

			push(std::string(i1, i));
		}
	}

	void Tokenizer::pushNum(const std::string& s, uint base)
	{
		if(s.empty())
		{
			throw std::string("Tokenizer tried to push empty number-string. [SYS ERR]");
		}

		uint v = 0;

		for(const auto& c : s)
		{
			if(!isNum(c, base))
			{
				throw lib::stringf("Number \"%s\" contains digit '%c' of invalid base! (Expected %u)", s.c_str(), c, base);
			}

			v = v * base + ((c >= '0' && c <= '9') ? c - '0' : c - 'a' + 10);
		}

		buf_.push_back(lib::stringf("%u", v));
	}

	void Tokenizer::push(const std::string& s)
	{
		if(s.empty())
		{
			throw std::string("Tokenizer tried to push empty string. [SYS ERR]");
		}

		if(s[0] == '$' && s.size() > 1)
		{
			pushNum(s.substr(1), 16);
		}
		else if(s[0] == '0' && s.size() > 1)
		{
			if(s[1] >= '1' && s[1] <= '7')
			{
				pushNum(s, 8);
			}
			else switch(s[1])
			{
				case 'b':
					pushNum(s.substr(2), 2);
					break;
				case 'd':
					pushNum(s.substr(2), 10);
					break;
				case 'x':
					pushNum(s.substr(2), 16);
					break;
				default:
					throw lib::stringf("In number \"%s\" invalid base specifier '%c'!", s.c_str(), s[1]);
			}
		}
		else
		{
			buf_.push_back(s);
		}
	}

	int Evaluator::evalAddExpr(Tokenizer& t)
	{
		if(t.empty())
		{
			throw std::string("Tokenizer prematurely emptied in ADD-expr.");
		}

		int r = evalMulExpr(t);

		while(!t.empty())
		{
			if(t.top() == "+")
			{
				t.pop();
				r += evalAddExpr(t);
			}
			else if(t.top() == "-")
			{
				t.pop();
				r -= evalAddExpr(t);
			}
			else
			{
				break;
			}
		}

		return r;
	}

	int Evaluator::evalMulExpr(Tokenizer& t)
	{
		if(t.empty())
		{
			throw std::string("Tokenizer prematurely emptied in MUL-expr.");
		}

		int r = evalUnaExpr(t);

		while(!t.empty())
		{
			if(t.top() == "*")
			{
				t.pop();
				r *= evalMulExpr(t);
			}
			else if(t.top() == "/")
			{
				t.pop();
				r /= evalMulExpr(t);
			}
			else if(t.top() == "%")
			{
				t.pop();
				r %= evalMulExpr(t);
			}
			else if(t.top() == "&")
			{
				t.pop();
				r &= evalMulExpr(t);
			}
			else if(t.top() == "|")
			{
				t.pop();
				r |= evalMulExpr(t);
			}
			else if(t.top() == "^")
			{
				t.pop();
				r ^= evalMulExpr(t);
			}
			else
			{
				break;
			}
		}

		return r;
	}

	int Evaluator::evalUnaExpr(Tokenizer& t)
	{
		if(t.empty())
		{
			throw std::string("Tokenizer prematurely emptied in UNA-expr.");
		}

		bool negate = false, bitwise = false;

		if(t.top() == "-")
		{
			t.pop();
			negate = true;
		}
		else if(t.top() == "~")
		{
			t.pop();
			bitwise = true;
		}

		int r = evalNumExpr(t);

		return bitwise ? ~r : (negate ? -r : r);
	}

	int Evaluator::evalNumExpr(Tokenizer& t)
	{
		if(t.empty())
		{
			throw std::string("Tokenizer prematurely emptied in NUM-expr.");
		}
		
		int r = 0;

		if(t.top() == "(")
		{
			t.pop();

			r = evalAddExpr(t);
			
			if(t.top() != ")")
			{
				throw lib::stringf("Malformed expression! Expected ')' instead of '%s'!", t.top().c_str());
			}
			
			t.pop();
		}
		else if(t.top()[0] >= '0' && t.top()[0] <= '9')
		{
			r = str_to_i(t.top());
			t.pop();
		}
		else
		{
			r = lookup_(t.top());
			t.pop();
		}

		return r;
	}

	uint Evaluator::eval(const std::string& expression)
	{
		Tokenizer t(eliminate(expression));

		uint v = static_cast<uint>(evalAddExpr(t));

		if(!t.empty())
		{
			throw lib::stringf("Trailing token(s) \"%s\" in expression \"%s\"!", t.top().c_str(), expression.c_str());
		}

		return v;
	}
}

uint Assembler::eval(const std::string& s, bool relative)
{
	if(s.empty())
	{
		throw std::string("Trying to evaluate empty expression. [SYS ERR]");
	}

	auto i = mSymbols.find(s);

	if(i != mSymbols.end())
	{
		return relative ? i->second - mAddress : i->second;
	}
	else if(s == "$")
	{
		return mAddress;
	}
	else
	{
		Evaluator e([this](const std::string& str) -> uint
		{
			if(str == "$") return mAddress;
			else 
			{
				auto i = mSymbols.find(str);

				if(i != mSymbols.end())
				{
					return i->second;
				}
				else
				{
					throw lib::stringf("Unknown symbol \"%s\"!", str.c_str());
				}
			}
		});

		return e.eval(s);
	}
}

void Assembler::addLabel(const std::string& lbl, meta_t meta)
{
	if(mSymbols.count(lbl))
	{
		throw lib::stringf("Multiple instances of label '%s': $%04X -> $%04X", lbl.c_str(), mSymbols[lbl], mAddress);
	}

	mSymbols[lbl] = mAddress;

	raw_t raw;
	raw.meta = meta;
	raw.address = mAddress;
	mRaw.push_back(raw);
}

void Assembler::addRawData(const std::string& s, uint w, meta_t meta)
{
	StringSplitter ss(s);

	raw_t raw;
	raw.meta = meta;
	raw.address = mAddress;

	while(ss.hasNext())
	{
		expr_t e;

		e.raw = ss.next();
		e.size = w;
		e.relative = false;

		raw.expr.push_back(e);
	}

	mRaw.push_back(raw);
	mAddress += raw.expr.size() * w;
}

void Assembler::reserveSpace(const std::string& s, meta_t meta)
{
	uint v = eval(s, false);

	raw_t raw;
	raw.meta = meta;
	raw.address = mAddress;

	for(uint i = 0 ; i < v ; ++i)
	{
		expr_t e;

		e.raw = "0";
		e.size = 1;
		e.relative = false;

		raw.expr.push_back(e);
	}

	mRaw.push_back(raw);
	mAddress += v;
}

void Assembler::setOrigin(const std::string& s, meta_t meta)
{
	mAddress = eval(s, false);

	raw_t raw;
	raw.meta = meta;
	raw.address = mAddress;
	mRaw.push_back(raw);
}

void Assembler::addError(const std::string& msg, meta_t meta)
{
	error_t error;

	error.message = msg;
	error.meta = meta;

	mProgram.errors.push_back(error);
}

// # ===========================================================================

Assembler::Tokenizer::Tokenizer(const std::string& fn)
{
	std::ifstream in(fn);
	uint c = 0;

	if(!in.good())
	{
		throw lib::stringf("Could not open file \"%s\"!", fn.c_str());
	}

	for(std::string s ; std::getline(in, s) ; ++c)
	{
#define STATE_DEF 0
#define STATE_CHAR 2
#define STATE_STRING 3
#define STATE_ESC 4
#define STATE_CHAR_END 5
		uint state = STATE_DEF, old;
		bool needComma = false;

		if(s.empty()) continue;

		std::vector<char> buf;

		auto pushChar = [&buf](char c)
			{ for(const char& ch : std::to_string(static_cast<int>(c))) buf.push_back(ch); };

		line_t line;

		line.meta.file = fn;
		line.meta.source = s;
		line.meta.line = c;

		if(s.at(0) == '#') line.line = s;
		else try
		{ 
			for(auto i1 = s.cbegin(), i2 = s.cend() ; i1 != i2 ; ++i1)
			switch(state)
			{
				case STATE_DEF:
					switch(*i1)
					{
						case ';':
							i1 = i2;
							--i1;
							break;
						case '\t':
						case ' ':
							buf.push_back(' ');
							while(i1 != i2 && (*i1 == ' ' || *i1 == '\t')) ++i1;
							--i1;
							break;
						case '\'':
							state = STATE_CHAR;
							break;
						case '"':
							state = STATE_STRING;
							needComma = false;
							break;
						default:
							if(*i1 >= 'A' && *i1 <= 'Z')
								buf.push_back(*i1 - 'A' + 'a');
							else
								buf.push_back(*i1);
							break;
					}
					break;
				case STATE_CHAR:
					if(*i1 == '\'') throw std::string("Empty char constant!");
					else if(*i1 == '\\')
					{
						state = STATE_ESC;
						old = STATE_CHAR_END;
					}
					else
					{
						pushChar(*i1);
						state = STATE_CHAR_END;
					}
					break;
				case STATE_CHAR_END:
					if(*i1 != '\'')
					{
						throw lib::stringf("Char not properly terminated! (Expected ' instead of %c)", *i1);
					}
					state = STATE_DEF;
					break;
				case STATE_STRING:
					if(*i1 == '"') state = STATE_DEF;
					else if(*i1 == '\\')
					{
						state = STATE_ESC;
						old = STATE_STRING;
					}
					else
					{
						if(needComma) buf.push_back(',');
						pushChar(*i1);
						needComma = true;
					}
					break;
				case STATE_ESC:
					switch(*i1)
					{
						case 'n':
							pushChar('\n');
							break;
						case '\\':
							pushChar('\\');
							break;
						case 't':
							pushChar('\t');
							break;
						default:
							throw lib::stringf("Malformed escape expression! (Got invalid character '%d')", *i1);
					}
					state = old;
					break;
			}

			auto toStr = [](uint state) -> const char *
			{
				switch(state)
				{
					case STATE_DEF:
						return "DEFAULT";
					case STATE_STRING:
						return "STRING";
					case STATE_CHAR:
						return "CHAR";
					case STATE_CHAR_END:
						return "CHAR_END";
					case STATE_ESC:
						return "ESCAPE";
					default:
						return "INVALID";
				}
			};

			if(state != STATE_DEF)
				throw lib::stringf("Non-terminated expression of type %s!", toStr(state));

#undef STATE_DEF
#undef STATE_CHAR
#undef STATE_CHAR_END
#undef STATE_STRING
#undef STATE_ESC

			auto i1 = buf.cbegin(), i2 = buf.cend();

			if(i1 == i2) continue;
			while(i1 != i2 && *i1 == ' ') ++i1;
			if(i1 == i2) continue;
			while(i1 != i2 && *--i2 == ' ');
			++i2;
			if(i1 == i2) continue;

			line.line = std::string(i1, i2);
		}
		catch(const std::string& e)
		{
			line.error = e;
		}

		lines.push_back(line);
	}

	in.close();

	index = 0;
}

// # ===========================================================================

void Assembler::Preprocessor::read(const std::string &fn)
{
	if(included(fn)) return;

	mIncludes.push_back(fn);

	Tokenizer t(fn);

	while(t.hasNext())
	{
		line_t line = t.next();

		try
		{
			line.line = process(line.line);
		}
		catch(const std::string& e)
		{
			line.error = e;
			line.line = "";
		}

		mLines.push_back(line);
	}
}

std::string Assembler::Preprocessor::process(const std::string &s)
{
	if(s.empty()) return "";

	std::string line = toLower(s);

	Matcher m;

	std::regex pDefine("# *define +([a-zA-Z_][a-zA-Z0-9_]*) +(.+)");
	std::regex pInclude("# *include +\"(.*)\" *");

	std::smatch mDefine, mInclude;

	std::regex_search(line, mDefine, pDefine);
	std::regex_search(line, mInclude, pInclude);

	if(line.at(0) == '#')
	{
		if(mDefine.size() > 2)
		{
			if(mDefines.count(mDefine.str(1)) > 0)
			{
				throw lib::stringf("Trying to overwrite alias '%s' which holds '%s' with '%s'!",
					mDefine.str(1).c_str(), mDefines[mDefine.str(1)].c_str(), mDefine.str(2).c_str());
			}

			mDefines[mDefine.str(1)] = mDefine.str(2);
		}
		else if(mInclude.size() > 1)
		{
			read(mInclude.str(1));
		}
		else
		{
			throw lib::stringf("Invalid preprocessor directive '%s'!", line.c_str());
		}

		return "";
	}
	else
	{
		StringBuffer buf;
		std::string r;
		uint max = MXT_MAXMACROS;

		buf.push(line);

		while(buf.hasNext())
		{
			std::string t = buf.next();
			auto i = mDefines.find(t);

			if(i != mDefines.end())
			{
				--max;
				
				if(max == 0)
				{
					throw lib::stringf("Reached max alias substitution depth.");
				}

				buf.push(i->second);
			}
			else
			{
				r += t;
			}
		}

		return r;
	}
}

// # ===========================================================================

Assembler::Matcher::return_fn Assembler::Matcher::operator[](const std::string& s)
{
	for(auto p : mPatterns)
	{
		std::string pattern = p.first;
		callback_fn f = p.second;
		std::vector<std::string> match;
		uint j = 0;

		if(pattern.size() > s.size()) continue;

		for(uint i = 0 ; i < pattern.size() ; ++i)
		{
			if(j == s.size()) goto not_matching;

			if(pattern[i] == '*')
			{
				uint t = j;
				while(j < s.size() && s[j] != ',' && s[j] != ']' && s[j] != ':') ++j;
				if(t == j) goto not_matching;
				match.emplace_back(s, t, j - t);
			}
			else if(pattern[i] == '$')
			{
				if(i != pattern.size() - 1)
				{
					throw lib::stringf("Invalid pattern '%s'! [SYS_ERR]", pattern.c_str());
				}
				if(j == s.size())
				{
					goto not_matching;
				}
				match.emplace_back(s, j, s.size() - j);
				j = s.size();
			}
			else
			{
				if(j < s.size() && s[j] == ' ' && pattern[i] == ',') ++j;
				if(j == s.size()) goto not_matching;
				if(i > 0 && pattern[i - 1] == ',' && s[j] == ' ') ++j;
				if(j == s.size() || pattern[i] != s[j]) goto not_matching;
				++j;
			}
		}

		if(j != s.size()) goto not_matching;

		return [match, f](Assembler *that, meta_t meta) { return f(that, meta, match); };

not_matching:
		;
	}

	throw lib::stringf("Unknown instruction '%s'!", s.c_str());
}

// # ===========================================================================

void Assembler::initialize(void)
{
	static bool done = false;

	if(done) return;

#define P(ins) mInstructions.registerPattern(ins, [](Assembler *that, meta_t meta, std::vector<std::string> ptrn) {
#define P0(ins) mInstructions.registerPattern(ins, [](Assembler *that, meta_t meta, std::vector<std::string> ptrn) { that->add(meta,
#define P1(ins) mInstructions.registerPattern(ins, [](Assembler *that, meta_t meta, std::vector<std::string> ptrn) { that->add({make_expr(1u, ptrn.at(0), false)}, meta,
#define P1r(ins) mInstructions.registerPattern(ins, [](Assembler *that, meta_t meta, std::vector<std::string> ptrn) { that->add({make_expr(1u, ptrn.at(0), true)}, meta,
#define P2(ins) mInstructions.registerPattern(ins, [](Assembler *that, meta_t meta, std::vector<std::string> ptrn) { that->add({make_expr(2u, ptrn.at(0), false)}, meta,
#define P2e(ins) mInstructions.registerPattern(ins, [](Assembler *that, meta_t meta, std::vector<std::string> ptrn) { that->add({make_expr(1u, ptrn.at(0), false), make_expr(1u, ptrn.at(1), false)}, meta,
#define PNE ); })
#define P1x(ins,ext) mInstructions.registerPattern(ins, [](Assembler *that, meta_t meta, std::vector<std::string> ptrn) { that->add({make_expr(1u, ptrn.at(0), false), make_expr(1u, lib::stringf("%u", ext), false)}, meta, 
#define PE })

	P0("nop") 		{0x00} PNE;
	P2("ld bc,*")	{0x01} PNE;
	P0("ld (bc),a")	{0x02} PNE;
	P0("inc bc")	{0x03} PNE;
	P0("inc b")		{0x04} PNE;
	P0("dec b")		{0x05} PNE;
	P1("ld b,*")	{0x06} PNE;
	P0("rlca")		{0x07} PNE;
	P0("ex af,af'")	{0x08} PNE;
	P0("add hl,bc")	{0x09} PNE;
	P0("ld a,(bc)")	{0x0A} PNE;
	P0("dec bc")	{0x0B} PNE;
	P0("inc c")		{0x0C} PNE;
	P0("dec c")		{0x0D} PNE;
	P1("ld c,*")	{0x0E} PNE;
	P0("rrca")		{0x0F} PNE;
	P1r("djnz *")	{0x10} PNE;
	P2("ld de,*")	{0x11} PNE;
	P0("ld (de),a")	{0x12} PNE;
	P0("inc de")	{0x13} PNE;
	P0("inc d")		{0x14} PNE;
	P0("dec d")		{0x15} PNE;
	P1("ld d,*")	{0x16} PNE;
	P0("rla")		{0x17} PNE;
	P1r("jr *")		{0x18} PNE;
	P0("add hl,de")	{0x19} PNE;
	P0("ld a,(de)")	{0x1A} PNE;
	P0("dec de") 	{0x1B} PNE;
	P0("inc e")		{0x1C} PNE;
	P0("dec e")		{0x1D} PNE;
	P1("ld e,*")	{0x1E} PNE;
	P0("rra")		{0x1F} PNE;
	P1r("jr nz,*")	{0x20} PNE;
	P2("ld hl,*")	{0x21} PNE;
	P2("ld [*],hl")	{0x22} PNE;
	P0("inc hl")	{0x23} PNE;
	P0("inc h")		{0x24} PNE;
	P0("dec h")		{0x25} PNE;
	P1("ld h,*")	{0x26} PNE;
	P0("daa")		{0x27} PNE;
	P1r("jr z,*")	{0x28} PNE;
	P0("add hl,hl")	{0x29} PNE;
	P2("ld hl,[*]")	{0x2A} PNE;
	P0("dec hl")	{0x2B} PNE;
	P0("inc l")		{0x2C} PNE;
	P0("dec l")		{0x2D} PNE;
	P1("ld l,*")	{0x2E} PNE;
	P0("cpl")		{0x2F} PNE;
	P1r("jr nc,*")	{0x30} PNE;
	P2("ld sp,*")	{0x31} PNE;
	P2("ld [*],a")	{0x32} PNE;
	P0("inc sp")	{0x33} PNE;
	P0("inc (hl)")	{0x34} PNE;
	P0("dec (hl)")	{0x35} PNE;
	P1("ld (hl),*")	{0x36} PNE;
	P0("scf")		{0x37} PNE;
	P1r("jr c,*")	{0x38} PNE;
	P0("add hl,sp")	{0x39} PNE;
	P2("ld a,[*]")	{0x3A} PNE;
	P0("dec sp")		{0x3B} PNE;
	P0("inc a")			{0x3C} PNE;
	P0("dec a")			{0x3D} PNE;
	P1("ld a,*")		{0x3E} PNE;
	P0("ccf")			{0x3F} PNE;
	P0("ld b,b")		{0x40} PNE;
	P0("ld b,c")		{0x41} PNE;
	P0("ld b,d")		{0x42} PNE;
	P0("ld b,e")		{0x43} PNE;
	P0("ld b,h")		{0x44} PNE;
	P0("ld b,l")		{0x45} PNE;
	P0("ld b,(hl)")	{0x46} PNE;
	P0("ld b,a")		{0x47} PNE;
	P0("ld c,b")		{0x48} PNE;
	P0("ld c,c")		{0x49} PNE;
	P0("ld c,d")		{0x4A} PNE;
	P0("ld c,e")		{0x4B} PNE;
	P0("ld c,h")		{0x4C} PNE;
	P0("ld c,l")		{0x4D} PNE;
	P0("ld c,(hl)")	{0x4E} PNE;
	P0("ld c,a")		{0x4F} PNE;
	P0("ld d,b")		{0x50} PNE;
	P0("ld d,c")		{0x51} PNE;
	P0("ld d,d")		{0x52} PNE;
	P0("ld d,e")		{0x53} PNE;
	P0("ld d,h")		{0x54} PNE;
	P0("ld d,l")		{0x55} PNE;
	P0("ld d,(hl)")	{0x56} PNE;
	P0("ld d,a")		{0x57} PNE;
	P0("ld e,b")		{0x58} PNE;
	P0("ld e,c")		{0x59} PNE;
	P0("ld e,d")		{0x5A} PNE;
	P0("ld e,e")		{0x5B} PNE;
	P0("ld e,h")		{0x5C} PNE;
	P0("ld e,l")		{0x5D} PNE;
	P0("ld e,(hl)")	{0x5E} PNE;
	P0("ld e,a")		{0x5F} PNE;
	P0("ld h,b")		{0x60} PNE;
	P0("ld h,c")		{0x61} PNE;
	P0("ld h,d")		{0x62} PNE;
	P0("ld h,e")		{0x63} PNE;
	P0("ld h,h")		{0x64} PNE;
	P0("ld h,l")		{0x65} PNE;
	P0("ld h,(hl)")	{0x66} PNE;
	P0("ld h,a")		{0x67} PNE;
	P0("ld l,b")		{0x68} PNE;
	P0("ld l,c")		{0x69} PNE;
	P0("ld l,d")		{0x6A} PNE;
	P0("ld l,e")		{0x6B} PNE;
	P0("ld l,h")		{0x6C} PNE;
	P0("ld l,l")		{0x6D} PNE;
	P0("ld l,(hl)")	{0x6E} PNE;
	P0("ld l,a")		{0x6F} PNE;
	P0("ld (hl),b")	{0x70} PNE;
	P0("ld (hl),c")	{0x71} PNE;
	P0("ld (hl),d")	{0x72} PNE;
	P0("ld (hl),e")	{0x73} PNE;
	P0("ld (hl),h")	{0x74} PNE;
	P0("ld (hl),l")	{0x75} PNE;
	P0("halt")	{0x76} PNE;
	P0("ld (hl),a")	{0x77} PNE;
	P0("ld a,b")		{0x78} PNE;
	P0("ld a,c")		{0x79} PNE;
	P0("ld a,d")		{0x7A} PNE;
	P0("ld a,e")		{0x7B} PNE;
	P0("ld a,h")		{0x7C} PNE;
	P0("ld a,l")		{0x7D} PNE;
	P0("ld a,(hl)")	{0x7E} PNE;
	P0("ld a,a")		{0x7F} PNE;
	P0("add a,b")		{0x80} PNE;
	P0("add a,c")		{0x81} PNE;
	P0("add a,d")		{0x82} PNE;
	P0("add a,e")		{0x83} PNE;
	P0("add a,h")		{0x84} PNE;
	P0("add a,l")		{0x85} PNE;
	P0("add a,(hl)")	{0x86} PNE;
	P0("add a,a")		{0x87} PNE;
	P0("adc a,b")		{0x88} PNE;
	P0("adc a,c")		{0x89} PNE;
	P0("adc a,d")		{0x8A} PNE;
	P0("adc a,e")		{0x8B} PNE;
	P0("adc a,h")		{0x8C} PNE;
	P0("adc a,l")		{0x8D} PNE;
	P0("adc a,(hl)")	{0x8E} PNE;
	P0("adc a,a")		{0x8F} PNE;
	P0("sub b")			{0x90} PNE;
	P0("sub c")			{0x91} PNE;
	P0("sub d")			{0x92} PNE;
	P0("sub e")			{0x93} PNE;
	P0("sub h")			{0x94} PNE;
	P0("sub l")			{0x95} PNE;
	P0("sub (hl)")		{0x96} PNE;
	P0("sub a")			{0x97} PNE;
	P0("sub a,b")		{0x90} PNE;
	P0("sub a,c")		{0x91} PNE;
	P0("sub a,d")		{0x92} PNE;
	P0("sub a,e")		{0x93} PNE;
	P0("sub a,h")		{0x94} PNE;
	P0("sub a,l")		{0x95} PNE;
	P0("sub a,(hl)")	{0x96} PNE;
	P0("sub a,a")		{0x97} PNE;
	P0("sbc a,b")		{0x98} PNE;
	P0("sbc a,c")		{0x99} PNE;
	P0("sbc a,d")		{0x9A} PNE;
	P0("sbc a,e")		{0x9B} PNE;
	P0("sbc a,h")		{0x9C} PNE;
	P0("sbc a,l")		{0x9D} PNE;
	P0("sbc a,(hl)")	{0x9E} PNE;
	P0("sbc a,a")		{0x9F} PNE;
	P0("and b")			{0xA0} PNE;
	P0("and c")			{0xA1} PNE;
	P0("and d")			{0xA2} PNE;
	P0("and e")			{0xA3} PNE;
	P0("and h")			{0xA4} PNE;
	P0("and l")			{0xA5} PNE;
	P0("and (hl)")		{0xA6} PNE;
	P0("and a")			{0xA7} PNE;
	P0("xor b")			{0xA8} PNE;
	P0("xor c")			{0xA9} PNE;
	P0("xor d")			{0xAA} PNE;
	P0("xor e")			{0xAB} PNE;
	P0("xor h")			{0xAC} PNE;
	P0("xor l")			{0xAD} PNE;
	P0("xor (hl)")		{0xAE} PNE;
	P0("xor a")			{0xAF} PNE;
	P0("or b")			{0xB0} PNE;
	P0("or c")			{0xB1} PNE;
	P0("or d")			{0xB2} PNE;
	P0("or e")			{0xB3} PNE;
	P0("or h")			{0xB4} PNE;
	P0("or l")			{0xB5} PNE;
	P0("or (hl)")		{0xB6} PNE;
	P0("or a")			{0xB7} PNE;
	P0("cp b")			{0xB8} PNE;
	P0("cp c")			{0xB9} PNE;
	P0("cp d")			{0xBA} PNE;
	P0("cp e")			{0xBB} PNE;
	P0("cp h")			{0xBC} PNE;
	P0("cp l")			{0xBD} PNE;
	P0("cp (hl)")		{0xBE} PNE;
	P0("cp a")			{0xBF} PNE;
	P0("and a,b")		{0xA0} PNE;
	P0("and a,c")		{0xA1} PNE;
	P0("and a,d")		{0xA2} PNE;
	P0("and a,e")		{0xA3} PNE;
	P0("and a,h")		{0xA4} PNE;
	P0("and a,l")		{0xA5} PNE;
	P0("and a,(hl)")	{0xA6} PNE;
	P0("and a,a")		{0xA7} PNE;
	P0("xor a,b")		{0xA8} PNE;
	P0("xor a,c")		{0xA9} PNE;
	P0("xor a,d")		{0xAA} PNE;
	P0("xor a,e")		{0xAB} PNE;
	P0("xor a,h")		{0xAC} PNE;
	P0("xor a,l")		{0xAD} PNE;
	P0("xor a,(hl)")	{0xAE} PNE;
	P0("xor a,a")		{0xAF} PNE;
	P0("or a,b")		{0xB0} PNE;
	P0("or a,c")		{0xB1} PNE;
	P0("or a,d")		{0xB2} PNE;
	P0("or a,e")		{0xB3} PNE;
	P0("or a,h")		{0xB4} PNE;
	P0("or a,l")		{0xB5} PNE;
	P0("or a,(hl)")		{0xB6} PNE;
	P0("or a,a")		{0xB7} PNE;
	P0("cp a,b")		{0xB8} PNE;
	P0("cp a,c")		{0xB9} PNE;
	P0("cp a,d")		{0xBA} PNE;
	P0("cp a,e")		{0xBB} PNE;
	P0("cp a,h")		{0xBC} PNE;
	P0("cp a,l")		{0xBD} PNE;
	P0("cp a,(hl)")		{0xBE} PNE;
	P0("cp a,a")		{0xBF} PNE;
	P0("ret nz")		{0xC0} PNE;
	P0("pop bc")		{0xC1} PNE;
	P2("jp nz,*")		{0xC2} PNE;
	P2("jp *")			{0xC3} PNE;
	P2("call nz,*")		{0xC4} PNE;
	P0("push bc")		{0xC5} PNE;
	P1("add a,*")		{0xC6} PNE;
	P0("rst 00h")		{0xC7} PNE;
	P0("ret z")			{0xC8} PNE;
	P0("ret")			{0xC9} PNE;
	P2("jp z,*")		{0xCA} PNE;
	P2("call z,*")		{0xCC} PNE;
	P2("call *")		{0xCD} PNE;
	P1("adc a,*")		{0xCE} PNE;
	P0("rst 08h")		{0xCF} PNE;
	P0("ret nc")		{0xD0} PNE;
	P0("pop de")		{0xD1} PNE;
	P2("jp nc,*")		{0xD2} PNE;
	P1("out [*],a")		{0xD3} PNE;
	P2("call nc,*")		{0xD4} PNE;
	P0("push de")		{0xD5} PNE;
	P1("sub *")			{0xD6} PNE;
	P1("sub a,*")		{0xD6} PNE;
	P0("rst 10h")		{0xD7} PNE;
	P0("ret c")			{0xD8} PNE;
	P0("exx")			{0xD9} PNE;
	P2("jp c,*")		{0xDA} PNE;
	P1("in a,[*]")		{0xDB} PNE;
	P2("call c,*")		{0xDC} PNE;
	P1("sbc a,*")		{0xDE} PNE;
	P0("rst 18h")		{0xDF} PNE;
	P0("ret po")		{0xE0} PNE;
	P0("pop hl")		{0xE1} PNE;
	P2("jp po,*")		{0xE2} PNE;
	P0("ex (sp),hl")	{0xE3} PNE;
	P2("call po,*")		{0xE4} PNE;
	P0("push hl")		{0xE5} PNE;
	P1("and *")			{0xE6} PNE;
	P1("and a,*")		{0xE6} PNE;
	P0("rst 20h")		{0xE7} PNE;
	P0("ret pe")		{0xE8} PNE;
	P0("jp (hl)")		{0xE9} PNE;
	P2("jp pe,*")		{0xEA} PNE;
	P0("ex de,hl")		{0xEB} PNE;
	P2("call pe,*")		{0xEC} PNE;
	P1("xor *")			{0xEE} PNE;
	P1("xor a,*")		{0xEE} PNE;
	P0("rst 28h")		{0xEF} PNE;
	P0("ret p")			{0xF0} PNE;
	P0("pop af")		{0xF1} PNE;
	P2("jp p,*")		{0xF2} PNE;
	P0("di")			{0xF3} PNE;
	P2("call p,*")		{0xF4} PNE;
	P0("push af")		{0xF5} PNE;
	P1("or *")			{0xF6} PNE;
	P1("or a,*")		{0xF6} PNE;
	P0("rst 30h")		{0xF7} PNE;
	P0("ret m")			{0xF8} PNE;
	P0("ld sp,hl")		{0xF9} PNE;
	P2("jp m,*")		{0xFA} PNE;
	P0("ei")			{0xFB} PNE;
	P2("call m,*")		{0xFC} PNE;
	P1("cp *")			{0xFE} PNE;
	P1("cp a,*")		{0xFE} PNE;
	P0("rst 38h")		{0xFF} PNE;
	
	P2("ld [*],bc")		{0xED, 0x43} PNE;
	P2("ld [*],de")		{0xED, 0x53} PNE;
	P2("ld [*],hl")		{0xED, 0x63} PNE;
	P2("ld [*],sp")		{0xED, 0x73} PNE;
	P2("ld bc,[*]")		{0xED, 0x4B} PNE;
	P2("ld de,[*]")		{0xED, 0x5B} PNE;
	P2("ld hl,[*]")		{0xED, 0x6B} PNE;
	P2("ld sp,[*]")		{0xED, 0x7B} PNE;
	P0("neg")			{0xED, 0x44} PNE;
	P0("im 0")			{0xED, 0x46} PNE;
	P0("im 1")			{0xED, 0x56} PNE;
	P0("out (c),b")		{0xED, 0x41} PNE;
	P0("out (c),c")		{0xED, 0x49} PNE;
	P0("out (c),d")		{0xED, 0x51} PNE;
	P0("out (c),e")		{0xED, 0x59} PNE;
	P0("out (c),h")		{0xED, 0x61} PNE;
	P0("out (c),l")		{0xED, 0x69} PNE;
	P0("out (c),0")		{0xED, 0x71} PNE;
	P0("out (c),a")		{0xED, 0x79} PNE;
	P0("in b,(c)")		{0xED, 0x40} PNE;
	P0("in c,(c)")		{0xED, 0x48} PNE;
	P0("in d,(c)")		{0xED, 0x50} PNE;
	P0("in e,(c)")		{0xED, 0x58} PNE;
	P0("in h,(c)")		{0xED, 0x60} PNE;
	P0("in l,(c)")		{0xED, 0x68} PNE;
	P0("in (c)")		{0xED, 0x70} PNE;
	P0("in a,(c)")		{0xED, 0x78} PNE;
	P0("sbc hl,bc")		{0xED, 0x42} PNE;
	P0("sbc hl,de")		{0xED, 0x52} PNE;
	P0("sbc hl,hl")		{0xED, 0x62} PNE;
	P0("sbc hl,sp")		{0xED, 0x72} PNE;
	P0("adc hl,bc")		{0xED, 0x4A} PNE;
	P0("adc hl,de")		{0xED, 0x5A} PNE;
	P0("adc hl,hl")		{0xED, 0x6A} PNE;
	P0("adc hl,sp")		{0xED, 0x7A} PNE;

	P0("rlc b")			{0xCB, 0x00} PNE;
	P0("rlc c")			{0xCB, 0x01} PNE;
	P0("rlc d")			{0xCB, 0x02} PNE;
	P0("rlc e")			{0xCB, 0x03} PNE;
	P0("rlc h")			{0xCB, 0x04} PNE;
	P0("rlc l")			{0xCB, 0x05} PNE;
	P0("rlc (hl)")		{0xCB, 0x06} PNE;
	P0("rlc a")			{0xCB, 0x07} PNE;
	P0("rrc b")			{0xCB, 0x08} PNE;
	P0("rrc c")			{0xCB, 0x09} PNE;
	P0("rrc d")			{0xCB, 0x0A} PNE;
	P0("rrc e")			{0xCB, 0x0B} PNE;
	P0("rrc h")			{0xCB, 0x0C} PNE;
	P0("rrc l")			{0xCB, 0x0D} PNE;
	P0("rrc (hl)")		{0xCB, 0x0E} PNE;
	P0("rrc a")			{0xCB, 0x0F} PNE;
	P0("rl b")			{0xCB, 0x10} PNE;
	P0("rl c")			{0xCB, 0x11} PNE;
	P0("rl d")			{0xCB, 0x12} PNE;
	P0("rl e")			{0xCB, 0x13} PNE;
	P0("rl h")			{0xCB, 0x14} PNE;
	P0("rl l")			{0xCB, 0x15} PNE;
	P0("rl (hl)")		{0xCB, 0x16} PNE;
	P0("rl a")			{0xCB, 0x17} PNE;
	P0("rr b")			{0xCB, 0x18} PNE;
	P0("rr c")			{0xCB, 0x19} PNE;
	P0("rr d")			{0xCB, 0x1A} PNE;
	P0("rr e")			{0xCB, 0x1B} PNE;
	P0("rr h")			{0xCB, 0x1C} PNE;
	P0("rr l")			{0xCB, 0x1D} PNE;
	P0("rr (hl)")		{0xCB, 0x1E} PNE;
	P0("rr a")			{0xCB, 0x1F} PNE;
	P0("sla b")			{0xCB, 0x20} PNE;
	P0("sla c")			{0xCB, 0x21} PNE;
	P0("sla d")			{0xCB, 0x22} PNE;
	P0("sla e")			{0xCB, 0x23} PNE;
	P0("sla h")			{0xCB, 0x24} PNE;
	P0("sla l")			{0xCB, 0x25} PNE;
	P0("sla (hl)")		{0xCB, 0x26} PNE;
	P0("sla a")			{0xCB, 0x27} PNE;
	P0("sra b")			{0xCB, 0x28} PNE;
	P0("sra c")			{0xCB, 0x29} PNE;
	P0("sra d")			{0xCB, 0x2A} PNE;
	P0("sra e")			{0xCB, 0x2B} PNE;
	P0("sra h")			{0xCB, 0x2C} PNE;
	P0("sra l")			{0xCB, 0x2D} PNE;
	P0("sra (hl)")		{0xCB, 0x2E} PNE;
	P0("sra a")			{0xCB, 0x2F} PNE;
	P0("sll b")			{0xCB, 0x30} PNE;
	P0("sll c")			{0xCB, 0x31} PNE;
	P0("sll d")			{0xCB, 0x32} PNE;
	P0("sll e")			{0xCB, 0x33} PNE;
	P0("sll h")			{0xCB, 0x34} PNE;
	P0("sll l")			{0xCB, 0x35} PNE;
	P0("sll (hl)")		{0xCB, 0x36} PNE;
	P0("sll a")			{0xCB, 0x37} PNE;
	P0("srl b")			{0xCB, 0x38} PNE;
	P0("srl c")			{0xCB, 0x39} PNE;
	P0("srl d")			{0xCB, 0x3A} PNE;
	P0("srl e")			{0xCB, 0x3B} PNE;
	P0("srl h")			{0xCB, 0x3C} PNE;
	P0("srl l")			{0xCB, 0x3D} PNE;
	P0("srl (hl)")		{0xCB, 0x3E} PNE;
	P0("srl a")			{0xCB, 0x3F} PNE;
	P0("bit 0,b")		{0xCB, 0x40} PNE;
	P0("bit 0,c")		{0xCB, 0x41} PNE;
	P0("bit 0,d")		{0xCB, 0x42} PNE;
	P0("bit 0,e")		{0xCB, 0x43} PNE;
	P0("bit 0,h")		{0xCB, 0x44} PNE;
	P0("bit 0,l")		{0xCB, 0x45} PNE;
	P0("bit 0,(hl)")	{0xCB, 0x46} PNE;
	P0("bit 0,a")		{0xCB, 0x47} PNE;
	P0("bit 1,b")		{0xCB, 0x48} PNE;
	P0("bit 1,c")		{0xCB, 0x49} PNE;
	P0("bit 1,d")		{0xCB, 0x4A} PNE;
	P0("bit 1,e")		{0xCB, 0x4B} PNE;
	P0("bit 1,h")		{0xCB, 0x4C} PNE;
	P0("bit 1,l")		{0xCB, 0x4D} PNE;
	P0("bit 1,(hl)")	{0xCB, 0x4E} PNE;
	P0("bit 1,a")		{0xCB, 0x4F} PNE;
	P0("bit 2,b")		{0xCB, 0x50} PNE;
	P0("bit 2,c")		{0xCB, 0x51} PNE;
	P0("bit 2,d")		{0xCB, 0x52} PNE;
	P0("bit 2,e")		{0xCB, 0x53} PNE;
	P0("bit 2,h")		{0xCB, 0x54} PNE;
	P0("bit 2,l")		{0xCB, 0x55} PNE;
	P0("bit 2,(hl)")	{0xCB, 0x56} PNE;
	P0("bit 2,a")		{0xCB, 0x57} PNE;
	P0("bit 3,b")		{0xCB, 0x58} PNE;
	P0("bit 3,c")		{0xCB, 0x59} PNE;
	P0("bit 3,d")		{0xCB, 0x5A} PNE;
	P0("bit 3,e")		{0xCB, 0x5B} PNE;
	P0("bit 3,h")		{0xCB, 0x5C} PNE;
	P0("bit 3,l")		{0xCB, 0x5D} PNE;
	P0("bit 3,(hl)")	{0xCB, 0x5E} PNE;
	P0("bit 3,a")		{0xCB, 0x5F} PNE;
	P0("bit 4,b")		{0xCB, 0x60} PNE;
	P0("bit 4,c")		{0xCB, 0x61} PNE;
	P0("bit 4,d")		{0xCB, 0x62} PNE;
	P0("bit 4,e")		{0xCB, 0x63} PNE;
	P0("bit 4,h")		{0xCB, 0x64} PNE;
	P0("bit 4,l")		{0xCB, 0x65} PNE;
	P0("bit 4,(hl)")	{0xCB, 0x66} PNE;
	P0("bit 4,a")		{0xCB, 0x67} PNE;
	P0("bit 5,b")		{0xCB, 0x68} PNE;
	P0("bit 5,c")		{0xCB, 0x69} PNE;
	P0("bit 5,d")		{0xCB, 0x6A} PNE;
	P0("bit 5,e")		{0xCB, 0x6B} PNE;
	P0("bit 5,h")		{0xCB, 0x6C} PNE;
	P0("bit 5,l")		{0xCB, 0x6D} PNE;
	P0("bit 5,(hl)")	{0xCB, 0x6E} PNE;
	P0("bit 5,a")		{0xCB, 0x6F} PNE;
	P0("bit 6,b")		{0xCB, 0x70} PNE;
	P0("bit 6,c")		{0xCB, 0x71} PNE;
	P0("bit 6,d")		{0xCB, 0x72} PNE;
	P0("bit 6,e")		{0xCB, 0x73} PNE;
	P0("bit 6,h")		{0xCB, 0x74} PNE;
	P0("bit 6,l")		{0xCB, 0x75} PNE;
	P0("bit 6,(hl)")	{0xCB, 0x76} PNE;
	P0("bit 6,a")		{0xCB, 0x77} PNE;
	P0("bit 7,b")		{0xCB, 0x78} PNE;
	P0("bit 7,c")		{0xCB, 0x79} PNE;
	P0("bit 7,d")		{0xCB, 0x7A} PNE;
	P0("bit 7,e")		{0xCB, 0x7B} PNE;
	P0("bit 7,h")		{0xCB, 0x7C} PNE;
	P0("bit 7,l")		{0xCB, 0x7D} PNE;
	P0("bit 7,(hl)")	{0xCB, 0x7E} PNE;
	P0("bit 7,a")		{0xCB, 0x7F} PNE;
	P0("res 0,b")		{0xCB, 0x80} PNE;
	P0("res 0,c")		{0xCB, 0x81} PNE;
	P0("res 0,d")		{0xCB, 0x82} PNE;
	P0("res 0,e")		{0xCB, 0x83} PNE;
	P0("res 0,h")		{0xCB, 0x84} PNE;
	P0("res 0,l")		{0xCB, 0x85} PNE;
	P0("res 0,(hl)")	{0xCB, 0x86} PNE;
	P0("res 0,a")		{0xCB, 0x87} PNE;
	P0("res 1,b")		{0xCB, 0x88} PNE;
	P0("res 1,c")		{0xCB, 0x89} PNE;
	P0("res 1,d")		{0xCB, 0x8A} PNE;
	P0("res 1,e")		{0xCB, 0x8B} PNE;
	P0("res 1,h")		{0xCB, 0x8C} PNE;
	P0("res 1,l")		{0xCB, 0x8D} PNE;
	P0("res 1,(hl)")	{0xCB, 0x8E} PNE;
	P0("res 1,a")		{0xCB, 0x8F} PNE;
	P0("res 2,b")		{0xCB, 0x90} PNE;
	P0("res 2,c")		{0xCB, 0x91} PNE;
	P0("res 2,d")		{0xCB, 0x92} PNE;
	P0("res 2,e")		{0xCB, 0x93} PNE;
	P0("res 2,h")		{0xCB, 0x94} PNE;
	P0("res 2,l")		{0xCB, 0x95} PNE;
	P0("res 2,(hl)")	{0xCB, 0x96} PNE;
	P0("res 2,a")		{0xCB, 0x97} PNE;
	P0("res 3,b")		{0xCB, 0x98} PNE;
	P0("res 3,c")		{0xCB, 0x99} PNE;
	P0("res 3,d")		{0xCB, 0x9A} PNE;
	P0("res 3,e")		{0xCB, 0x9B} PNE;
	P0("res 3,h")		{0xCB, 0x9C} PNE;
	P0("res 3,l")		{0xCB, 0x9D} PNE;
	P0("res 3,(hl)")	{0xCB, 0x9E} PNE;
	P0("res 3,a")		{0xCB, 0x9F} PNE;
	P0("res 4,b")		{0xCB, 0xA0} PNE;
	P0("res 4,c")		{0xCB, 0xA1} PNE;
	P0("res 4,d")		{0xCB, 0xA2} PNE;
	P0("res 4,e")		{0xCB, 0xA3} PNE;
	P0("res 4,h")		{0xCB, 0xA4} PNE;
	P0("res 4,l")		{0xCB, 0xA5} PNE;
	P0("res 4,(hl)")	{0xCB, 0xA6} PNE;
	P0("res 4,a")		{0xCB, 0xA7} PNE;
	P0("res 5,b")		{0xCB, 0xA8} PNE;
	P0("res 5,c")		{0xCB, 0xA9} PNE;
	P0("res 5,d")		{0xCB, 0xAA} PNE;
	P0("res 5,e")		{0xCB, 0xAB} PNE;
	P0("res 5,h")		{0xCB, 0xAC} PNE;
	P0("res 5,l")		{0xCB, 0xAD} PNE;
	P0("res 5,(hl)")	{0xCB, 0xAE} PNE;
	P0("res 5,a")		{0xCB, 0xAF} PNE;
	P0("res 6,b")		{0xCB, 0xB0} PNE;
	P0("res 6,c")		{0xCB, 0xB1} PNE;
	P0("res 6,d")		{0xCB, 0xB2} PNE;
	P0("res 6,e")		{0xCB, 0xB3} PNE;
	P0("res 6,h")		{0xCB, 0xB4} PNE;
	P0("res 6,l")		{0xCB, 0xB5} PNE;
	P0("res 6,(hl)")	{0xCB, 0xB6} PNE;
	P0("res 6,a")		{0xCB, 0xB7} PNE;
	P0("res 7,b")		{0xCB, 0xB8} PNE;
	P0("res 7,c")		{0xCB, 0xB9} PNE;
	P0("res 7,d")		{0xCB, 0xBA} PNE;
	P0("res 7,e")		{0xCB, 0xBB} PNE;
	P0("res 7,h")		{0xCB, 0xBC} PNE;
	P0("res 7,l")		{0xCB, 0xBD} PNE;
	P0("res 7,(hl)")	{0xCB, 0xBE} PNE;
	P0("res 7,a")		{0xCB, 0xBF} PNE;
	P0("set 0,b")		{0xCB, 0xC0} PNE;
	P0("set 0,c")		{0xCB, 0xC1} PNE;
	P0("set 0,d")		{0xCB, 0xC2} PNE;
	P0("set 0,e")		{0xCB, 0xC3} PNE;
	P0("set 0,h")		{0xCB, 0xC4} PNE;
	P0("set 0,l")		{0xCB, 0xC5} PNE;
	P0("set 0,(hl)")	{0xCB, 0xC6} PNE;
	P0("set 0,a")		{0xCB, 0xC7} PNE;
	P0("set 1,b")		{0xCB, 0xC8} PNE;
	P0("set 1,c")		{0xCB, 0xC9} PNE;
	P0("set 1,d")		{0xCB, 0xCA} PNE;
	P0("set 1,e")		{0xCB, 0xCB} PNE;
	P0("set 1,h")		{0xCB, 0xCC} PNE;
	P0("set 1,l")		{0xCB, 0xCD} PNE;
	P0("set 1,(hl)")	{0xCB, 0xCE} PNE;
	P0("set 1,a")		{0xCB, 0xCF} PNE;
	P0("set 2,b")		{0xCB, 0xD0} PNE;
	P0("set 2,c")		{0xCB, 0xD1} PNE;
	P0("set 2,d")		{0xCB, 0xD2} PNE;
	P0("set 2,e")		{0xCB, 0xD3} PNE;
	P0("set 2,h")		{0xCB, 0xD4} PNE;
	P0("set 2,l")		{0xCB, 0xD5} PNE;
	P0("set 2,(hl)")	{0xCB, 0xD6} PNE;
	P0("set 2,a")		{0xCB, 0xD7} PNE;
	P0("set 3,b")		{0xCB, 0xD8} PNE;
	P0("set 3,c")		{0xCB, 0xD9} PNE;
	P0("set 3,d")		{0xCB, 0xDA} PNE;
	P0("set 3,e")		{0xCB, 0xDB} PNE;
	P0("set 3,h")		{0xCB, 0xDC} PNE;
	P0("set 3,l")		{0xCB, 0xDD} PNE;
	P0("set 3,(hl)")	{0xCB, 0xDE} PNE;
	P0("set 3,a")		{0xCB, 0xDF} PNE;
	P0("set 4,b")		{0xCB, 0xE0} PNE;
	P0("set 4,c")		{0xCB, 0xE1} PNE;
	P0("set 4,d")		{0xCB, 0xE2} PNE;
	P0("set 4,e")		{0xCB, 0xE3} PNE;
	P0("set 4,h")		{0xCB, 0xE4} PNE;
	P0("set 4,l")		{0xCB, 0xE5} PNE;
	P0("set 4,(hl)")	{0xCB, 0xE6} PNE;
	P0("set 4,a")		{0xCB, 0xE7} PNE;
	P0("set 5,b")		{0xCB, 0xE8} PNE;
	P0("set 5,c")		{0xCB, 0xE9} PNE;
	P0("set 5,d")		{0xCB, 0xEA} PNE;
	P0("set 5,e")		{0xCB, 0xEB} PNE;
	P0("set 5,h")		{0xCB, 0xEC} PNE;
	P0("set 5,l")		{0xCB, 0xED} PNE;
	P0("set 5,(hl)")	{0xCB, 0xEE} PNE;
	P0("set 5,a")		{0xCB, 0xEF} PNE;
	P0("set 6,b")		{0xCB, 0xF0} PNE;
	P0("set 6,c")		{0xCB, 0xF1} PNE;
	P0("set 6,d")		{0xCB, 0xF2} PNE;
	P0("set 6,e")		{0xCB, 0xF3} PNE;
	P0("set 6,h")		{0xCB, 0xF4} PNE;
	P0("set 6,l")		{0xCB, 0xF5} PNE;
	P0("set 6,(hl)")	{0xCB, 0xF6} PNE;
	P0("set 6,a")		{0xCB, 0xF7} PNE;
	P0("set 7,b")		{0xCB, 0xF8} PNE;
	P0("set 7,c")		{0xCB, 0xF9} PNE;
	P0("set 7,d")		{0xCB, 0xFA} PNE;
	P0("set 7,e")		{0xCB, 0xFB} PNE;
	P0("set 7,h")		{0xCB, 0xFC} PNE;
	P0("set 7,l")		{0xCB, 0xFD} PNE;
	P0("set 7,(hl)")	{0xCB, 0xFE} PNE;
	P0("set 7,a")		{0xCB, 0xFF} PNE;

	P0("add ix,bc")			{0xDD, 0x09} PNE;
	P0("add ix,de")			{0xDD, 0x19} PNE;
	P0("add ix,ix")			{0xDD, 0x29} PNE;
	P0("add ix,sp")			{0xDD, 0x39} PNE;
	P2("ld ix,*")			{0xDD, 0x21} PNE;
	P2("ld [*],ix")			{0xDD, 0x22} PNE;
	P0("inc ix")			{0xDD, 0x23} PNE;
	P2("ld ix,[*]")			{0xDD, 0x2A} PNE;
	P0("dec ix")			{0xDD, 0x2B} PNE;
	P1("inc [*] (ix)")		{0xDD, 0x34} PNE;
	P1("dec [*] (ix)")		{0xDD, 0x35} PNE;
	P2e("ld [*] (ix),*")	{0xDD, 0x36} PNE;
	P1("ld [*] (ix),b")		{0xDD, 0x70} PNE;
	P1("ld [*] (ix),c")		{0xDD, 0x71} PNE;
	P1("ld [*] (ix),d")		{0xDD, 0x72} PNE;
	P1("ld [*] (ix),e")		{0xDD, 0x73} PNE;
	P1("ld [*] (ix),h")		{0xDD, 0x74} PNE;
	P1("ld [*] (ix),l")		{0xDD, 0x75} PNE;
	P1("ld [*] (ix),a")		{0xDD, 0x77} PNE;
	P1("ld [*] b,(ix)")		{0xDD, 0x46} PNE;
	P1("ld [*] c,(ix)")		{0xDD, 0x4E} PNE;
	P1("ld [*] d,(ix)")		{0xDD, 0x56} PNE;
	P1("ld [*] e,(ix)")		{0xDD, 0x5E} PNE;
	P1("ld [*] h,(ix)")		{0xDD, 0x66} PNE;
	P1("ld [*] l,(ix)")		{0xDD, 0x6E} PNE;
	P1("ld [*] a,(ix)")		{0xDD, 0x7E} PNE;
	P1("add [*] a,(ix)")	{0xDD, 0x86} PNE;
	P1("adc [*] a,(ix)")	{0xDD, 0x8E} PNE;
	P1("sub [*] (ix)")		{0xDD, 0x96} PNE;
	P1("sub [*] a,(ix)")	{0xDD, 0x96} PNE;
	P1("sbc [*] a,(ix)")	{0xDD, 0x9E} PNE;
	P1("and [*] (ix)")		{0xDD, 0xA6} PNE;
	P1("and [*] a,(ix)")	{0xDD, 0xA6} PNE;
	P1("xor [*] (ix)")		{0xDD, 0xAE} PNE;
	P1("xor [*] a,(ix)")	{0xDD, 0xAE} PNE;
	P1("or [*] (ix)")		{0xDD, 0xB6} PNE;
	P1("or [*] a,(ix)")		{0xDD, 0xB6} PNE;
	P1("cp [*] (ix)")		{0xDD, 0xBE} PNE;
	P1("cp [*] a,(ix)")		{0xDD, 0xBE} PNE;
	P0("pop ix")			{0xDD, 0xE1} PNE;
	P0("ex (sp),ix")		{0xDD, 0xE3} PNE;
	P0("push ix")			{0xDD, 0xE5} PNE;
	P0("jp (ix)")			{0xDD, 0xE9} PNE;
	P0("ld sp,ix")			{0xDD, 0xF9} PNE;

	P1x("rlc [*] (ix)", 0x06)	{0xDD, 0xCB} PNE;
	P1x("rrc [*] (ix)", 0x0E)	{0xDD, 0xCB} PNE;
	P1x("rl [*] (ix)",  0x16)	{0xDD, 0xCB} PNE;
	P1x("rr [*] (ix)",  0x1E)	{0xDD, 0xCB} PNE;
	P1x("sla [*] (ix)", 0x26)	{0xDD, 0xCB} PNE;
	P1x("sra [*] (ix)", 0x2E)	{0xDD, 0xCB} PNE;
	P1x("sll [*] (ix)", 0x36)	{0xDD, 0xCB} PNE;
	P1x("srl [*] (ix)", 0x3E)	{0xDD, 0xCB} PNE;
	P1x("bit [*] 0,(ix)", 0x46)	{0xDD, 0xCB} PNE;
	P1x("bit [*] 1,(ix)", 0x4E)	{0xDD, 0xCB} PNE;
	P1x("bit [*] 2,(ix)", 0x56)	{0xDD, 0xCB} PNE;
	P1x("bit [*] 3,(ix)", 0x5E)	{0xDD, 0xCB} PNE;
	P1x("bit [*] 4,(ix)", 0x66)	{0xDD, 0xCB} PNE;
	P1x("bit [*] 5,(ix)", 0x6E)	{0xDD, 0xCB} PNE;
	P1x("bit [*] 6,(ix)", 0x76)	{0xDD, 0xCB} PNE;
	P1x("bit [*] 7,(ix)", 0x7E)	{0xDD, 0xCB} PNE;
	P1x("res [*] 0,(ix)", 0x86)	{0xDD, 0xCB} PNE;
	P1x("res [*] 1,(ix)", 0x8E)	{0xDD, 0xCB} PNE;
	P1x("res [*] 2,(ix)", 0x96)	{0xDD, 0xCB} PNE;
	P1x("res [*] 3,(ix)", 0x9E)	{0xDD, 0xCB} PNE;
	P1x("res [*] 4,(ix)", 0xA6)	{0xDD, 0xCB} PNE;
	P1x("res [*] 5,(ix)", 0xAE)	{0xDD, 0xCB} PNE;
	P1x("res [*] 6,(ix)", 0xB6)	{0xDD, 0xCB} PNE;
	P1x("res [*] 7,(ix)", 0xBE)	{0xDD, 0xCB} PNE;
	P1x("set [*] 0,(ix)", 0xC6)	{0xDD, 0xCB} PNE;
	P1x("set [*] 1,(ix)", 0xCE)	{0xDD, 0xCB} PNE;
	P1x("set [*] 2,(ix)", 0xD6)	{0xDD, 0xCB} PNE;
	P1x("set [*] 3,(ix)", 0xDE)	{0xDD, 0xCB} PNE;
	P1x("set [*] 4,(ix)", 0xE6)	{0xDD, 0xCB} PNE;
	P1x("set [*] 5,(ix)", 0xEE)	{0xDD, 0xCB} PNE;
	P1x("set [*] 6,(ix)", 0xF6)	{0xDD, 0xCB} PNE;
	P1x("set [*] 7,(ix)", 0xFE)	{0xDD, 0xCB} PNE;

	P0("add iy,bc")			{0xFD, 0x09} PNE;
	P0("add iy,de")			{0xFD, 0x19} PNE;
	P0("add iy,iy")			{0xFD, 0x29} PNE;
	P0("add iy,sp")			{0xFD, 0x39} PNE;
	P2("ld iy,*")			{0xFD, 0x21} PNE;
	P2("ld [*],iy")			{0xFD, 0x22} PNE;
	P0("inc iy")			{0xFD, 0x23} PNE;
	P2("ld iy,[*]")			{0xFD, 0x2A} PNE;
	P0("dec iy")			{0xFD, 0x2B} PNE;
	P1("inc [*] (iy)")		{0xFD, 0x34} PNE;
	P1("dec [*] (iy)")		{0xFD, 0x35} PNE;
	P2e("ld [*] (iy),*")	{0xFD, 0x36} PNE;
	P1("ld [*] (iy),b")		{0xFD, 0x70} PNE;
	P1("ld [*] (iy),c")		{0xFD, 0x71} PNE;
	P1("ld [*] (iy),d")		{0xFD, 0x72} PNE;
	P1("ld [*] (iy),e")		{0xFD, 0x73} PNE;
	P1("ld [*] (iy),h")		{0xFD, 0x74} PNE;
	P1("ld [*] (iy),l")		{0xFD, 0x75} PNE;
	P1("ld [*] (iy),a")		{0xFD, 0x77} PNE;
	P1("ld [*] b,(iy)")		{0xFD, 0x46} PNE;
	P1("ld [*] c,(iy)")		{0xFD, 0x4E} PNE;
	P1("ld [*] d,(iy)")		{0xFD, 0x56} PNE;
	P1("ld [*] e,(iy)")		{0xFD, 0x5E} PNE;
	P1("ld [*] h,(iy)")		{0xFD, 0x66} PNE;
	P1("ld [*] l,(iy)")		{0xFD, 0x6E} PNE;
	P1("ld [*] a,(iy)")		{0xFD, 0x7E} PNE;
	P1("add [*] a,(iy)")	{0xFD, 0x86} PNE;
	P1("adc [*] a,(iy)")	{0xFD, 0x8E} PNE;
	P1("sub [*] (iy)")		{0xFD, 0x96} PNE;
	P1("sub [*] a,(iy)")	{0xFD, 0x96} PNE;
	P1("sbc [*] a,(iy)")	{0xFD, 0x9E} PNE;
	P1("and [*] (iy)")		{0xFD, 0xA6} PNE;
	P1("and [*] a,(iy)")	{0xFD, 0xA6} PNE;
	P1("xor [*] (iy)")		{0xFD, 0xAE} PNE;
	P1("xor [*] a,(iy)")	{0xFD, 0xAE} PNE;
	P1("or [*] (iy)")		{0xFD, 0xB6} PNE;
	P1("or [*] a,(iy)")		{0xFD, 0xB6} PNE;
	P1("cp [*] (iy)")		{0xFD, 0xBE} PNE;
	P1("cp [*] a,(iy)")		{0xFD, 0xBE} PNE;
	P0("pop iy")			{0xFD, 0xE1} PNE;
	P0("ex (sp),iy")		{0xFD, 0xE3} PNE;
	P0("push iy")			{0xFD, 0xE5} PNE;
	P0("jp (iy)")			{0xFD, 0xE9} PNE;
	P0("ld sp,iy")			{0xFD, 0xF9} PNE;

	P1x("rlc [*] (iy)", 0x06)	{0xFD, 0xCB} PNE;
	P1x("rrc [*] (iy)", 0x0E)	{0xFD, 0xCB} PNE;
	P1x("rl [*] (iy)",  0x16)	{0xFD, 0xCB} PNE;
	P1x("rr [*] (iy)",  0x1E)	{0xFD, 0xCB} PNE;
	P1x("sla [*] (iy)", 0x26)	{0xFD, 0xCB} PNE;
	P1x("sra [*] (iy)", 0x2E)	{0xFD, 0xCB} PNE;
	P1x("sll [*] (iy)", 0x36)	{0xFD, 0xCB} PNE;
	P1x("srl [*] (iy)", 0x3E)	{0xFD, 0xCB} PNE;
	P1x("bit [*] 0,(iy)", 0x46)	{0xFD, 0xCB} PNE;
	P1x("bit [*] 1,(iy)", 0x4E)	{0xFD, 0xCB} PNE;
	P1x("bit [*] 2,(iy)", 0x56)	{0xFD, 0xCB} PNE;
	P1x("bit [*] 3,(iy)", 0x5E)	{0xFD, 0xCB} PNE;
	P1x("bit [*] 4,(iy)", 0x66)	{0xFD, 0xCB} PNE;
	P1x("bit [*] 5,(iy)", 0x6E)	{0xFD, 0xCB} PNE;
	P1x("bit [*] 6,(iy)", 0x76)	{0xFD, 0xCB} PNE;
	P1x("bit [*] 7,(iy)", 0x7E)	{0xFD, 0xCB} PNE;
	P1x("res [*] 0,(iy)", 0x86)	{0xFD, 0xCB} PNE;
	P1x("res [*] 1,(iy)", 0x8E)	{0xFD, 0xCB} PNE;
	P1x("res [*] 2,(iy)", 0x96)	{0xFD, 0xCB} PNE;
	P1x("res [*] 3,(iy)", 0x9E)	{0xFD, 0xCB} PNE;
	P1x("res [*] 4,(iy)", 0xA6)	{0xFD, 0xCB} PNE;
	P1x("res [*] 5,(iy)", 0xAE)	{0xFD, 0xCB} PNE;
	P1x("res [*] 6,(iy)", 0xB6)	{0xFD, 0xCB} PNE;
	P1x("res [*] 7,(iy)", 0xBE)	{0xFD, 0xCB} PNE;
	P1x("set [*] 0,(iy)", 0xC6)	{0xFD, 0xCB} PNE;
	P1x("set [*] 1,(iy)", 0xCE)	{0xFD, 0xCB} PNE;
	P1x("set [*] 2,(iy)", 0xD6)	{0xFD, 0xCB} PNE;
	P1x("set [*] 3,(iy)", 0xDE)	{0xFD, 0xCB} PNE;
	P1x("set [*] 4,(iy)", 0xE6)	{0xFD, 0xCB} PNE;
	P1x("set [*] 5,(iy)", 0xEE)	{0xFD, 0xCB} PNE;
	P1x("set [*] 6,(iy)", 0xF6)	{0xFD, 0xCB} PNE;
	P1x("set [*] 7,(iy)", 0xFE)	{0xFD, 0xCB} PNE;

	P("*:") that->addLabel(ptrn.at(0), meta); PE;
	P(".org *") that->setOrigin(ptrn.at(0), meta); PE;
	P(".db $") that->addRawData(ptrn.at(0), 1, meta); PE;
	P(".dw $") that->addRawData(ptrn.at(0), 2, meta); PE;
	P(".ds *") that->reserveSpace(ptrn.at(0), meta); PE;

#undef PE
#undef PNE
#undef P1
#undef P0
#undef P

	mInstructions.finalize();

	done = true;
}

}

