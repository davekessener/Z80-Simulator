#ifndef Z80_H
#define Z80_H

#include <map>
#include <iostream>
#include <stdint.h>

#include "Peripheral.h"
#include "Program.h"

namespace z80
{
	class Z80
	{
		public:
			typedef uint16_t addr_t;
			typedef uint8_t byte_t;
			typedef uint8_t port_t;
			typedef uint16_t size_t;

			static const uint8_t FLAG_S  = 0x80;
			static const uint8_t FLAG_Z  = 0x40;
			static const uint8_t FLAG_H  = 0x10;
			static const uint8_t FLAG_PV = 0x04;
			static const uint8_t FLAG_N  = 0x02;
			static const uint8_t FLAG_C  = 0x01;
			static const uint8_t FLAG_ALL= 0xFF;

		public:
			void printStatus(std::ostream&);
			void printRAM(std::ostream&, addr_t, size_t);
			uint16_t getPC( ) const { return PC; }
			void reset( );
			void loadRAM(addr_t, const Program&);
			void registerPeripheral(port_t, Peripheral&);
			void execute( );
			bool isHalted( ) const { return halted_; }
			void restart( ) { halted_ = false; }
			void interrupt( ) { if(int_) interrupted_ = true; }
			uint8_t& RAM(uint16_t a) { return ram_[a]; }
		private:
			void pushB(uint8_t);
			void pushW(uint16_t);
			uint8_t popB( );
			uint16_t popW( );
			void jr();
			void out(uint8_t, uint8_t);
			uint8_t in(uint8_t);
			uint8_t loadB( );
			uint16_t loadW( );
			uint8_t loadB(uint16_t);
			uint16_t loadW(uint16_t);
			void storeB(uint16_t, uint8_t);
			void storeW(uint16_t, uint16_t);
			void call(uint16_t);
			void ret( );
			uint16_t getOff(uint16_t, uint8_t);
			void set_inc_flags(uint8_t);
			void set_dec_flags(uint8_t);
			void set_flags(uint, uint /*P/V*/, uint /*S*/, uint /*Z*/, uint /*H*/, uint /*N*/, uint /*C*/);
			bool parityEven(uint8_t);
			void addHL(uint16_t, bool = false);
			void addA(uint8_t, bool = false);
			void subA(uint8_t, bool = false);
			void logicA(uint8_t, bool);
			void rlca( );
			void rla( );
			void daa( );
			void rrca( );
			void rra( );
			uint8_t& A() { return ((uint8_t *) &AF)[1]; }
			uint8_t& F() { return ((uint8_t *) &AF)[0]; }
			uint8_t& B() { return ((uint8_t *) &BC)[1]; }
			uint8_t& C() { return ((uint8_t *) &BC)[0]; }
			uint8_t& D() { return ((uint8_t *) &DE)[1]; }
			uint8_t& E() { return ((uint8_t *) &DE)[0]; }
			uint8_t& H() { return ((uint8_t *) &HL)[1]; }
			uint8_t& L() { return ((uint8_t *) &HL)[0]; }

		private:
			byte_t ram_[0x10000];
			uint16_t AF, AFp, BC, BCp, DE, DEp, HL, HLp;
			uint16_t IR, IX, IY, SP, PC;
			std::map<port_t, Peripheral *> periphs_;
			bool int_, halted_, interrupted_;
	};
}

#endif

