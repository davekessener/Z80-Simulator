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

			static const uint BIT_BIT7 = 0;
			static const uint BIT_CARRY = 1;
			static const uint BIT_A = 2;
			static const uint BIT_L = 3;

		public:
			void printStatus(std::ostream&);
			void printRAM(std::ostream&, addr_t, size_t);
			void reset( );
			void loadRAM(addr_t, const Program&);
			void registerPeripheral(port_t, Peripheral&);
			void execute( );
			bool isHalted( ) const { return halted_; }
			void restart( ) { halted_ = false; }
			void interrupt( ) { if(int_) interrupted_ = true; }
			uint8_t& RAM(uint16_t a) { return ram_[a]; }
			uint16_t getPC( ) const { return PC; }
			uint16_t getSP( ) const { return SP; }
			uint16_t getAF( ) const { return AF; }
			uint16_t getAFp( ) const { return AFp; }
			uint16_t getBC( ) const { return BC; }
			uint16_t getBCp( ) const { return BCp; }
			uint16_t getDE( ) const { return DE; }
			uint16_t getDEp( ) const { return DEp; }
			uint16_t getHL( ) const { return HL; }
			uint16_t getHLp( ) const { return HLp; }
			uint16_t getIX( ) const { return IX; }
			uint16_t getIY( ) const { return IY; }
			void setPC(uint16_t v) { PC = v; }
			void setSP(uint16_t v) { SP = v; }
			void setAF(uint16_t v) { AF = v; }
			void setBC(uint16_t v) { BC = v; }
			void setDE(uint16_t v) { DE = v; }
			void setHL(uint16_t v) { HL = v; }
			void setIX(uint16_t v) { IX = v; }
			void setIY(uint16_t v) { IY = v; }
			bool getFlagS( ) const { return AF & FLAG_S; }
			bool getFlagZ( ) const { return AF & FLAG_Z; }
			bool getFlagH( ) const { return AF & FLAG_H; }
			bool getFlagPV( ) const { return AF & FLAG_PV; }
			bool getFlagN( ) const { return AF & FLAG_N; }
			bool getFlagC( ) const { return AF & FLAG_C; }
			std::string disassemble(uint16_t) const;
			void clear( );
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
			void subHL(uint16_t, bool = false);
			void addA(uint8_t, bool = false);
			void subA(uint8_t, bool = false);
			void logicA(uint8_t, bool);
			void rlca( );
			void rla( );
			void daa( );
			void rrca( );
			void rra( );
			void rotate_left(uint8_t&, uint);
			void rotate_right(uint8_t&, uint);
			void test_bit(uint8_t v, uint i) { set_flags(FLAG_N | FLAG_H | FLAG_Z, 0, 0, v & (1 << i), 1, 0, 0); }
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

