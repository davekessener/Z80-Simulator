#ifndef Z80_ASSEMBLER_H
#define Z80_ASSEMBLER_H

#include <string>
#include <map>
#include <vector>
#include <deque>
#include <stack>
#include <algorithm>
#include <functional>

typedef unsigned uint;

namespace z80
{
	class Assembler
	{
		struct meta_t
		{
			std::string file, source;
			uint line;
		};
		struct line_t
		{
			std::string line;
			meta_t meta;
			std::string error;
		};
		struct expr_t
		{
			uint size;
			std::string raw;
			bool relative;
		};
		struct raw_t
		{
			std::vector<expr_t> expr;
			meta_t meta;
			std::vector<uint8_t> ins;
			uint16_t address;
		};
		struct entry_t
		{
			std::vector<uint8_t> data;
			meta_t meta;
			uint16_t address;
		};
		struct error_t
		{
			std::string message;
			meta_t meta;
		};
		struct program_t
		{
			std::vector<entry_t> data;
			std::vector<error_t> errors;
		};

		class Tokenizer
		{
			public:
				Tokenizer(const std::string&);
				line_t next( ) { return lines.at(index++); }
				bool hasNext( ) const { return index < lines.size(); }
			private:
				std::vector<line_t> lines;
				uint index;
		};

		class Preprocessor
		{
			public:
				Preprocessor(const std::string& fn) { read(fn); mIndex = 0; }
				line_t next( ) { return mLines[mIndex++]; }
				bool hasNext( ) const { return mIndex < mLines.size(); }
			private:
				void read(const std::string&);
				std::string process(const std::string&);
				bool included(const std::string& fn) const
					{ return std::find(mIncludes.cbegin(), mIncludes.cend(), fn) != mIncludes.cend(); }

			private:
				std::map<std::string, std::string> mDefines;
				std::vector<std::string> mIncludes;
				std::vector<line_t> mLines;
				uint mIndex;
		};

		class Matcher
		{
			typedef std::function<void(Assembler *, meta_t, std::vector<std::string>)> callback_fn;
			typedef std::function<void(Assembler *, meta_t)> return_fn;
			
			typedef std::pair<std::string, callback_fn> value_type;

			public:
				void registerPattern(const std::string& s, callback_fn f)
				{
					if(contains(s, '*')) extP_.push_back(std::make_pair(s, f));
					else basicP_.push_back(std::make_pair(s, f));
				}
				void finalize( )
				{
					std::sort(basicP_.begin(), basicP_.end(), [](const value_type& v1, const value_type& v2)
						{ return v1.first.size() < v2.first.size(); });
					std::sort(extP_.begin(), extP_.end(), [](const value_type& v1, const value_type& v2)
						{ return v1.first.size() < v2.first.size(); });
					mPatterns.insert(mPatterns.end(), basicP_.begin(), basicP_.end());
					mPatterns.insert(mPatterns.end(), extP_.begin(), extP_.end());
				}
				return_fn operator[](const std::string&);
			private:
				static bool contains(const std::string& s, char c)
					{ return std::find(s.begin(), s.end(), c) != s.end(); }
			private:
				std::vector<value_type> mPatterns;
				std::vector<value_type> basicP_, extP_;
		};

		inline static expr_t make_expr(uint s, const std::string& e, bool r)
		{
			expr_t ex;

			ex.raw = e;
			ex.size = s;
			ex.relative = r;

			return ex;
		}

		public:
			Assembler(const std::string&);
			program_t get( ) const { return mProgram; }
		private:
			static void initialize( );
			void add(meta_t, const std::initializer_list<uint8_t>&);
			void add(const std::initializer_list<expr_t>&, meta_t, const std::initializer_list<uint8_t>&);
			void addError(const std::string&, meta_t);
			void addLabel(const std::string&, meta_t);
			void addRawData(const std::string&, uint, meta_t);
			void setOrigin(const std::string&, meta_t);
			uint eval(const std::string&, bool);

		private:
			program_t mProgram;
			std::vector<raw_t> mRaw;
			std::map<std::string, uint16_t> mSymbols;
			uint16_t mAddress;

			static Matcher mInstructions;
	};
}

#endif

