#pragma once
#include <Windows.h>

#include "rlua.h"
#include "encrypt.h"
#include "util.h"

extern "C" {
	// lua core
#include "Lua\lua.h"
#include "Lua\lauxlib.h"
#include "Lua\lualib.h"
	// lua internals
#include "Lua\lobject.h"
#include "Lua\lstate.h"
#include "Lua\lfunc.h"
#include "Lua\lopcodes.h"
}

#define DISCONNECT_ROBLOX true

static lua_State* L;
static bool parented_script = false;
static bool debug_mode = false;
static DWORD script;

/* instruction format */
#define R_POS_OP				26
#define R_POS_A					18
#define R_POS_B					0
#define R_POS_C					9
#define R_POS_Bx				R_POS_B

#define R_SET_OPCODE(i,o) ((i) = (((i)&MASK0(SIZE_OP,R_POS_OP)) | ((cast(Instruction, o)<<R_POS_OP)&MASK1(SIZE_OP,R_POS_OP))))
#define R_SETARG_A(i,u)   ((i) = (((i)&MASK0(SIZE_A,R_POS_A)) | ((cast(Instruction, u)<<R_POS_A)&MASK1(SIZE_A,R_POS_A))))
#define R_SETARG_B(i,b)   ((i) = (((i)&MASK0(SIZE_B,R_POS_B)) | ((cast(Instruction, b)<<R_POS_B)&MASK1(SIZE_B,R_POS_B))))
#define R_SETARG_C(i,b)   ((i) = (((i)&MASK0(SIZE_C,R_POS_C)) | ((cast(Instruction, b)<<R_POS_C)&MASK1(SIZE_C,R_POS_C))))
#define R_SETARG_Bx(i,b)  ((i) = (((i)&MASK0(SIZE_Bx,R_POS_Bx)) | ((cast(Instruction, b)<<R_POS_Bx)&MASK1(SIZE_Bx,R_POS_Bx))))
#define R_SETARG_sBx(i,b) R_SETARG_Bx((i),cast(Instruction, (b)+MAXARG_sBx))

#define RSET_ABCPPPEPEPEPEP(i,o)	((i) = (((i) & MASK0(6, 26)) | (((Instruction)o << 26) & MASK1(6, 26))))
#define RGET_OPCODE(i)		(i >> 26 & MASK1(6, 0))

#define RSETARG_A(i,o)		((i) = (((i) & MASK0(8, 18)) | (((Instruction)o << 18) & MASK1(8, 18))))
#define RGETARG_A(i)		(i >> 18 & MASK1(8, 0))

#define RSETARG_B(i,o)		((i) = (((i) & MASK0(9, 0)) | (((Instruction)o << 0) & MASK1(9, 0))))
#define RGETARG_B(i)		(i >>  0 & MASK1(9, 0))

#define RSETARG_C(i,o)		((i) = (((i) & MASK0(9, 9)) | (((Instruction)o << 9) & MASK1(9, 9))))
#define RGETARG_C(i)		(i >>  9 & MASK1(9, 0))

#define RSETARG_Bx(i,b)		((i) = (((i) & MASK0(18, 0)) | (((Instruction)b << 0) & MASK1(18, 0))))
#define RGETARG_Bx(i)		(i >>  0 & MASK1(18, 0))

#define RSETARG_sBx(i,b)	RSETARG_Bx((i),cast(unsigned int, (b)+MAXARG_sBx))
#define RGETARG_sBx(i)		(RGETARG_Bx(i)-MAXARG_sBx)

#define STORE_OP(L, R) (((L) << SIZE_OP) + (R))

#define ReadROpcode(n) (opcodes[(n)] & MASK1(SIZE_OP, 0))
#define ReadLOpcode(n) (opcodes[(n)] >> SIZE_OP)
inline int opcodes[NUM_OPCODES]
{
	STORE_OP(OP_LOADBOOL,  0x06 /* OP________________________________________________________________________________________________________MOVE      */),
	STORE_OP(OP_GETTABLE,  0x04 /* OP________________________________________________________________________________________________________LOADK     */),
	STORE_OP(OP_GETUPVAL,  0x00 /* OP________________________________________________________________________________________________________LOADBOOL  */),
	STORE_OP(OP_SETGLOBAL, 0x07 /* OP________________________________________________________________________________________________________LOADNIL   */),
	STORE_OP(OP_LOADK,     0x02 /* OP________________________________________________________________________________________________________GETUPVAL  */),
	STORE_OP(OP_SETUPVAL,  0x08 /* OP________________________________________________________________________________________________________GETGLOBAL */),
	STORE_OP(OP_MOVE,      0x01 /* OP________________________________________________________________________________________________________GETTABLE  */),
	STORE_OP(OP_LOADNIL,   0x03 /* OP________________________________________________________________________________________________________SETGLOBAL */),
	STORE_OP(OP_GETGLOBAL, 0x05 /* OP________________________________________________________________________________________________________SETUPVAL  */),
	STORE_OP(OP_SELF,      0x0F /* OP________________________________________________________________________________________________________SETTABLE  */),
	STORE_OP(OP_DIV,       0x0D /* OP________________________________________________________________________________________________________NEWTABLE  */),
	STORE_OP(OP_SUB,       0x09 /* OP________________________________________________________________________________________________________SELF      */),
	STORE_OP(OP_MOD,       0x10 /* OP________________________________________________________________________________________________________ADD       */),
	STORE_OP(OP_NEWTABLE,  0x0B /* OP________________________________________________________________________________________________________SUB       */),
	STORE_OP(OP_POW,       0x11 /* OP________________________________________________________________________________________________________MUL       */),
	STORE_OP(OP_SETTABLE,  0x0A /* OP________________________________________________________________________________________________________DIV       */),
	STORE_OP(OP_ADD,       0x0C /* OP________________________________________________________________________________________________________MOD       */),
	STORE_OP(OP_MUL,       0x0E /* OP________________________________________________________________________________________________________POW       */),
	STORE_OP(OP_LEN,       0x18 /* OP________________________________________________________________________________________________________UNM       */),
	STORE_OP(OP_LT,        0x16 /* OP________________________________________________________________________________________________________NOT       */),
	STORE_OP(OP_JMP,       0x12 /* OP________________________________________________________________________________________________________LEN       */),
	STORE_OP(OP_LE,        0x19 /* OP________________________________________________________________________________________________________CONCAT    */),
	STORE_OP(OP_NOT,       0x14 /* OP________________________________________________________________________________________________________JMP       */),
	STORE_OP(OP_TEST,      0x1A /* OP________________________________________________________________________________________________________EQ        */),
	STORE_OP(OP_UNM,       0x13 /* OP________________________________________________________________________________________________________LT        */),
	STORE_OP(OP_CONCAT,    0x15 /* OP________________________________________________________________________________________________________LE        */),
	STORE_OP(OP_EQ,        0x17 /* OP________________________________________________________________________________________________________TEST      */),
	STORE_OP(OP_TAILCALL,  0x21 /* OP________________________________________________________________________________________________________TESTSET   */),
	STORE_OP(OP_TFORLOOP,  0x1F /* OP________________________________________________________________________________________________________CALL      */),
	STORE_OP(OP_FORLOOP,   0x1B /* OP________________________________________________________________________________________________________TAILCALL  */),
	STORE_OP(OP_SETLIST,   0x22 /* OP________________________________________________________________________________________________________RETURN    */),
	STORE_OP(OP_CALL,      0x1D /* OP________________________________________________________________________________________________________FORLOOP   */),
	STORE_OP(OP_CLOSE,     0x23 /* OP________________________________________________________________________________________________________FORPREP   */),
	STORE_OP(OP_TESTSET,   0x1C /* OP________________________________________________________________________________________________________TFORLOOP  */),
	STORE_OP(OP_RETURN,    0x1E /* OP________________________________________________________________________________________________________SETLIST   */),
	STORE_OP(OP_FORPREP,   0x20 /* OP________________________________________________________________________________________________________CLOSE     */),
	STORE_OP(OP_VARARG,    0x25 /* OP________________________________________________________________________________________________________CLOSURE   */),
	STORE_OP(OP_CLOSURE,   0x24 /* OP________________________________________________________________________________________________________VARARG    */),
};

const char get_minecraft_java_class_name_hash[NUM_OPCODES]{ // modified instruction set enum
	6, // OP_MOVE
	4, // OP_LOADK
	0, // OP_LOADBOOL
	7, // OP_LOADNIL
	2, // OP_GETUPVAL
	8, // OP_GETGLOBAL
	1, // OP_GETTABLE
	3, // OP_SETGLOBAL
	5, // OP_SETUPVAL
	15, // OP_SETTABLE
	13, // OP_NEWTABLE
	9, // OP_SELF
	16, // OP_ADD
	11, // OP_SUB
	17, // OP_MUL
	10, // OP_DIV
	12, // OP_MOD
	14, // OP_POW
	24, // OP_UNM
	22, // OP_NOT
	18, // OP_LEN
	25, // OP_CONCAT
	20, // OP_JMP
	26, // OP_EQ
	19, // OP_LT
	21, // OP_LE
	23, // OP_TEST
	33, // OP_TESTSET
	31, // OP_CALL
	27, // OP_TAILCALL
	34, // OP_RETURN
	29, // OP_FORLOOP
	35, // OP_FORPREP
	28, // OP_TFORLOOP
	30, // OP_SETLIST
	32, // OP_CLOSE
	37, // OP_CLOSURE
	36, // OP_VARARG
};

namespace lua_to_roblox {
	VOID convert_proto(int rl, Proto* p, r_proto* rp, int* rpnups);
	VOID set_l_closure(int rl, LClosure* lcl);
	VOID call_l_closure(int rl, LClosure* lcl);
	VOID execute_script(int rl, lua_State* l, std::string source);
}

int initializeWSJ(lua_State* L);
VOID execute_script(int rL, lua_State* L, const std::string &source);
