#ifndef ORB_INSTR_HH
#define ORB_INSTR_HH

#include <stdint.h>

/* REGISTERS */

// General Purpose Registers
#define REG0 0x0
#define REG1 0x1
#define REG2 0x2
#define REG3 0x3

// Get first and second register from instruction
#define GET_R1(instr)         ((instr >> 6) & 0x3)
#define GET_R2(instr)         ((instr >> 4) & 0x3)

// Set first and second register in instruction
#define SET_R1(r1)            (((r1) & 0x3) << 6)
#define SET_R2(r2)            (((r2) & 0x3) << 4)

// Calculates wrap around for index/PC base on code length (256)
#define WRAP_IDX(idx, off)    (((idx) + (off)) & 0xFF)
#define NEXT_IDX(idx)         WRAP_IDX(idx, 0x1)

// Status Register Flags
#define ZF_BIT 0x1
#define NF_BIT 0x2

// Zero-Flag: get/set/clear
#define GET_ST_ZF(stat)       (((stat) >> 4) & ZF_BIT)
#define SET_ST_ZF(stat)       (stat |= (ZF_BIT << 4))
#define CLR_ST_ZF(stat)       (stat &= ~(ZF_BIT << 4))

// Negative-Flag: get/set/clear
#define GET_ST_NF(stat)       (((stat) >> 4) & NF_BIT)
#define SET_ST_NF(stat)       (stat |= (NF_BIT << 4))
#define CLR_ST_NF(stat)       (stat &= ~(NF_BIT << 4))

// Clear status
#define CLEAR_ST(stat)        (stat &= 0x30)

// Set status register flags based on last result
#define SET_ST_ARIT(stat, res) \
  CLEAR_ST(stat); \
  if ((res) == 0) { SET_ST_ZF(stat); } \
  else if ((res) < 0) { SET_ST_NF(stat); }



/* INSTRUCTIONS */

// Instruction r1r2cccc
//
// R1: Bits [7,6]
// R2: Bits [5,4]
// C : Bits [3,0]
//
// r1: Sets R1 field in instruction, 2 bits
// r2: Sets R2 field in instruction, 2 bits
// c : Sets instruction code field, 4 bits
//
#define INSTR(r1, r2, c)      (uint8_t)((SET_R1(r1) | SET_R2(r2) | (c)) & 0xFF)

//
//  Direction Coding:
//        [3]
//     [2] O [0]
//        [1]
//

//
//  Type coding: { 0x0 = 'o', 0x1 = 'O', 0x2 = '+', 0x3 = '#' }
//

// Act unconditional [dd0x 0000]
//
//  R1: Direction or register, 2 bits
//  R2: Flag [x]: Direction from register or R1, 1 bit
//
#define ACT_CODE              0x0
#define ACT(dir, x)           INSTR(dir, (x) & 0x1, ACT_CODE)
#define ACT_GET_X(instr)      (instr & 0x10)

// Act conditional: [ccdd 0001]
//
//  R1: Condition compared to zero/negative status flags, 2 bits
//  R2: Direction, 2 bits
//
#define ACTCND_CODE           0x1
#define ACTCND(cnd, dir)      INSTR(cnd, dir, 0x1)

// Sense:
//
//  R1: Defines type to sense for, 2 bits
//  R2: Defines direction to sense, 2 bits
//
#define SENSE_CODE            0x2
#define SENSE(tp, dir)        INSTR(tp, dir, SENSE_CODE)

// Jump: [cclx 0011] [yyyy yyyy]
//
//  CC: If not zero, condition to compare with status register bits, 2 bits.
//  L : If to jump and set the link register to following instruction, 1 bit.
//  X : If bit is set negates the condition CC, 1 bit.
//
//  When none of the above 4 bits are set this instruction behaves like
//  a basic jump.
//
//  The following immediate value is interpreted as the offset (wrap around)
//  from the current PC/index to the position to jump to.
//
#define JMP_CODE              0x3
#define JMP_INSTR(cnd, x, l)  INSTR(cnd, ((l << 1) | x), JMP_CODE)
#define JMP_IF(cnd)           JMP_INSTR(cnd, 0x0, 0x0)
#define JMP_IFN(cnd)          JMP_INSTR(cnd, 0x1, 0x0)
#define JMP                   JMP_INSTR(0x0, 0x0, 0x0)
#define JMP_IFLR(cnd)         JMP_INSTR(cnd, 0x0, 0x1)
#define JMP_IFNLR(cnd)        JMP_INSTR(cnd, 0x1, 0x1)
#define JMP_LR                JMP_INSTR(0x0, 0x0, 0x1)
#define JMP_ADDR(idx, addr)   ((0x100 - (idx) + (addr)) & 0xFF)

// Move: r1 = (r1 != r2) ? r2 : immediate
//
//  R1: Defines register [r1], 2 bits
//  R2: Defines register [r2], 2 bits
//
#define MOV_CODE              0x4
#define MOV(r1, r2)           INSTR(r1, r2, MOV_CODE)
#define MOVI(r1)              INSTR(r1, r1, MOV_CODE)

// Addition: r1 += r2, [r1r2 0101]
//
//  R1: Defines register [r1], 2 bits
//  R2: Defines register [r2], 2 bits
//
#define ADD_CODE              0x5
#define ADD(r1, r2)           INSTR(r1, r2, ADD_CODE)

// Subtraction: r1 -= r2, [r1r2 0110]
//
//  R1: Defines register [r1], 2 bits
//  R2: Defines register [r2], 2 bits
//
#define SUB_CODE              0x6
#define SUB(r1, r2)           INSTR(r1, r2, SUB_CODE)

// Increment/Decrement: r1++/r1-- [r10x 0111]
//
//  R1: Defines register [r1], 2 bits
//  R2: Flag [x]: If set increment, else decrement, 1 bit
//
#define IDC_CODE              0x7
#define IDC(r1, x)            INSTR(r1, (x) & 0x1, IDC_CODE)
#define INC(r1)               IDC(r1, 0x1)
#define DEC(r1)               IDC(r1, 0x0)
#define IDC_GET_X(instr)      (instr & 0x10)

// Load/Store: [r1r2 1001]
//
//  R1: Defines action to perform
//      - 0x0: Loads status into into [r2]
//      - 0x1: Loads score / 1000 into [r2]
//      - 0x2: Loads [r2] into [idx]
//      - 0x3: Loads [r2] into [lr]
//  R2: Defines register [r2], 2 bits
//
#define LDS_CODE              0x9
#define LDS(r1, r2)           INSTR(r1, r2, LDS_CODE)

// Return: Sets [idx] to [lr], [cc00 1010]
//
//  R1: Condition compared to zero/negative status flags, 2 bits
//  R2: (unused)
//
#define RET_CODE              0xA
#define RET(cnd)              INSTR(cnd, 0x0, RET_CODE)

// No-Operation
#define NOP_CODE              0xF
#define NOP                   0xFF

// Immediate value
#define IMD(val)              ((val) & 0xFF)

#endif // #ifndef ORB_INSTR_HH
