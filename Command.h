#ifndef Z80_COMMAND_H
#define Z80_COMMAND_H

#include <string>
#include <vector>

typedef unsigned uint;

namespace z80
{
	enum class TokenType
	{
		UNKNOWN,
		LITERAL,
		STRING,
		NUMBER
	};

	class Tokenizer
	{
		public:
		struct Token
		{
			TokenType type;
			std::string token;
			uint value;
		};

		typedef std::vector<Token> vec_t;
		typedef vec_t::const_iterator const_iterator;

		public:
			Tokenizer(const std::string&);
			const_iterator begin( ) const { return buf_.cbegin(); }
			const_iterator end( ) const { return buf_.cend(); }
			size_t size( ) const { return buf_.size(); }
			const Token& operator[](size_t i) const { return buf_.at(i); }

		private:
			void process(char);
			void push(TokenType, const std::string&, uint);
			static uint toInt(char, uint);
			static bool isInt(char, uint);

		private:
			vec_t buf_;
	};

	std::string toString(const TokenType&);
}

#endif

