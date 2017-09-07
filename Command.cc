#include "Command.h"

namespace z80
{
	Tokenizer::Tokenizer(const std::string& s)
	{
		for(auto i1 = s.cbegin(), i2 = s.cend() ; i1 != i2 ; ++i1)
		{
			process(*i1);
		}

		process('\0');
	}

	void Tokenizer::process(char c)
	{
		static std::string token;
		static TokenType type = TokenType::UNKNOWN;
		static uint value = 0, base = 0;
		static uint checkNum = 0;
		static bool inEscape = false;

		if(c == '\0')
		{
			if(type == TokenType::STRING)
			{
				throw std::string("Nonterminated string!");
			}
			else if(type == TokenType::UNKNOWN)
			{
			}
			else
			{
				push(type, token, value);
			}

			type = TokenType::UNKNOWN;
		}
		else switch(type)
		{
			case TokenType::UNKNOWN:
				token = "";
				value = base = checkNum = 0;
				inEscape = false;
				if((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))
				{
					type = TokenType::LITERAL;
					process(c);
				}
				else if(c == '$')
				{
					type = TokenType::NUMBER;
					base = 16;
					token.push_back('$');
				}
				else if(c == '0')
				{
					type = TokenType::NUMBER;
					base = 8;
					checkNum = 1;
					token.push_back('0');
				}
				else if(c >= '1' && c <= '9')
				{
					type = TokenType::NUMBER;
					base = 10;
					token.push_back(c);
				}
				else if(c == '"')
				{
					type = TokenType::STRING;
				}
				break;
			case TokenType::LITERAL:
				if(c >= 'A' && c <= 'Z')
				{
					process(c - 'A' + 'a');
				}
				else if((c >= 'a' && c <= 'z') || c == '_' || c == '-')
				{
					token.push_back(c);
				}
				else
				{
					push(TokenType::LITERAL, token, 0);

					type = TokenType::UNKNOWN;

					process(c);
				}
				break;
			case TokenType::STRING:
				if(inEscape) switch(c)
				{
					case 'n':
						token.push_back('\n');
						break;
					case '0':
						token.push_back('\0');
						break;
					case 't':
						token.push_back('\t');
						break;
					default:
						token.push_back(c);
						break;
				}
				else if(c == '"')
				{
					push(TokenType::STRING, token, 0);

					type = TokenType::UNKNOWN;
				}
				else if(c == '\\')
				{
					inEscape = true;
				}
				break;
			case TokenType::NUMBER:
				if(checkNum == 1) switch(c)
				{
					case 'b':
						base = 2;
						checkNum = 2;
						token.push_back(c);
						break;
					case 'o':
						base = 8;
						checkNum = 2;
						token.push_back(c);
						break;
					case 'd':
						base = 10;
						checkNum = 2;
						token.push_back(c);
						break;
					case 'x':
						base = 16;
						checkNum = 2;
						token.push_back(c);
						break;
					case '0':
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
						checkNum = 0;
						process(c);
						break;
					default:
						throw std::string("Malformed numerical constant.");
				}
				else if(checkNum == 2 || isInt(c, base))
				{
					value = value * base + toInt(c, base);
					checkNum = 0;
					token.push_back(c);
				}
				else
				{
					push(TokenType::NUMBER, token, value);

					type = TokenType::UNKNOWN;

					process(c);
				}
		}
	}

	void Tokenizer::push(TokenType type, const std::string& token, uint value)
	{
		Token t;
		
		t.type = type;
		t.token = token;
		t.value = value;

		buf_.push_back(t);
	}

	uint Tokenizer::toInt(char c, uint b)
	{
		uint v = 0;

		if(c >= '0' && c <= '9')
		{
			v = c - '0';
		}
		else if(c >= 'a' && c <= 'f')
		{
			v = 10 + c - 'a';
		}
		else if(c >= 'A' && c <= 'F')
		{
			v = 10 + c - 'A';
		}
		else
		{
			throw std::string("Invalid character in num.");
		}

		if(v >= b)
		{
			throw std::string("Invalid character for base.");
		}

		return v;
	}

	bool Tokenizer::isInt(char c, uint b)
	{
		uint v = 0;

		if(c >= '0' && c <= '9')
		{
			v = c - '0';
		}
		else if(c >= 'a' && c <= 'f')
		{
			v = 10 + c - 'a';
		}
		else if(c >= 'A' && c <= 'F')
		{
			v = 10 + c - 'A';
		}
		else
		{
			return false;
		}

		if(v >= b)
		{
			return false;
		}

		return true;
	}
}

