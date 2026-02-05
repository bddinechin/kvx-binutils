/* KVX assembler/disassembler support.

   Copyright (C) 2009-2024 Free Software Foundation, Inc.
   Contributed by Kalray SA.

   This file is part of GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the license, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING3. If not,
   see <http://www.gnu.org/licenses/>.  */


#ifndef OPCODE_KVX_H
#define OPCODE_KVX_H

#define KVX_NUMCORES 3
#define KVX_MAXSYLLABLES 3
#define KVX_MAXOPERANDS 7
#define KVX_MAXBUNDLEISSUE 10
#define KVX_MAXBUNDLEWORDS 18


/*
 * The following macros are provided for compatibility with old
 * code.  They should not be used in new code.
 */

#define KV4_ACTIVATE_OFFSET	6
#define KV4_ACTIVATE_WIDTH	8
#define KV4_ACTIVATE_MASK	\
  (((1 << KV4_ACTIVATE_WIDTH) - 1) << KV4_ACTIVATE_OFFSET)
#define KV4_BCU_GUARD_OPCODE	0x0f800000
#define KV4_BCU_GUARD_MASK	0x7ffc0000
#define KV4_BCU_BLEND_OPCODE	0x0f840000
#define KV4_BCU_BLEND_MASK	0x7ffc0000

/***********************************************/
/*       DATA TYPES                            */
/***********************************************/

/*  Operand definition -- used in building     */
/*  format table                               */

enum kvx_rel {
  /* Absolute relocation. */
  KVX_REL_ABS,
  /* PC relative relocation. */
  KVX_REL_PC,
  /* GP relative relocation. */
  KVX_REL_GP,
  /* TP relative relocation. */
  KVX_REL_TP,
  /* GOT relative relocation. */
  KVX_REL_GOT,
  /* BASE load address relative relocation. */
  KVX_REL_BASE,
};

struct kvx_reloc {
  /* Size in bits. */
  int bitsize;
  /* Type of relative relocation. */
  enum kvx_rel relative;
  /* Number of BFD relocations. */
  int reloc_nb;
  /* List of BFD relocations. */
  unsigned int relocs[];
};

struct kvx_bitfield {
  /* Number of bits.  */
  int size;
  /* Offset in abstract value.  */
  int from_offset;
  /* Offset in encoded value.  */
  int to_offset;
};

struct kvx_operand {
  /* Operand type name.  */
  const char *tname;
  /* Type of operand.  */
  int type;
  /* Width of the operand. */
  int width;
  /* Encoded value shift. */
  int shift;
  /* Encoded value bias.  */
  int bias;
  /* Can be SIGNED|CANEXTEND|BITMASK|WRAPPED.  */
  int flags;
  /* Number of registers.  */
  int reg_nb;
  /* Valid registers for this operand (if no register get null pointer).  */
  int *regs;
  /* Number of relocations.  */
  int reloc_nb;
  /* List of relocations that can be applied to this operand.  */
  struct kvx_reloc **relocs;
  /* Number of given bitfields.  */
  int bitfields;
  /* Bitfields in most to least significant order.  */
  struct kvx_bitfield bfield[];
};

struct kvx_pseudo_relocs
{
  enum
  {
    S32_LO5_UP27,
    S37_LO10_UP27,
    S43_LO10_UP27_EX6,
    S64_LO10_UP27_EX27,
    S16,
    S32,
    S64,
  } reloc_type;

  int bitsize;

  /* Used when pseudo func should expand to different relocations
     based on the 32/64 bits mode.
     Enum values should match the kvx_arch_size var set by -m32
   */
  enum
  {
    PSEUDO_ALL = 0,
    PSEUDO_32_ONLY = 32,
    PSEUDO_64_ONLY = 64,
  } avail_modes;

  /* set to 1 when pseudo func does not take an argument */
  int has_no_arg;

  bfd_reloc_code_real_type reloc_lo5, reloc_lo10, reloc_up27, reloc_ex;
  bfd_reloc_code_real_type single;
  struct kvx_reloc *kreloc;
};

typedef struct symbol symbolS;

struct pseudo_func
{
  const char *name;

  symbolS *sym;
  struct kvx_pseudo_relocs pseudo_relocs;
};

/* Flags for kvx_operand  */
#define KVX_OPERAND_SIGNED    1
#define KVX_OPERAND_CANEXTEND 2
#define KVX_OPERAND_BITMASK   4
#define KVX_OPERAND_WRAPPED   8

#define KVX_OPCODE_FLAG_UNDEF 0
#define KVX_OPCODE_FLAG_IMMX 1
#define KVX_OPCODE_FLAG_COND 2
#define KVX_OPCODE_FLAG_CALL 4
#define KVX_OPCODE_FLAG_LOAD 8
#define KVX_OPCODE_FLAG_STORE 16
#define KVX_OPCODE_FLAG_MODE32 32
#define KVX_OPCODE_FLAG_MODE64 64
#define KVX_OPCODE_FLAG_RISCV 128

/* Opcode definition.  */

struct kvx_codeword {
  /* The opcode.  */
  unsigned opcode;
  /* Disassembly mask.  */
  unsigned mask;
  /* Target dependent flags.  */
  unsigned flags;
};

struct kvx_opc {
  /* asm name */
  const char *as_op;
  /* 32 bits code words. */
  struct kvx_codeword codewords[KVX_MAXSYLLABLES];
  /* Number of words in codewords[].  */
  int wordcount;
  /* Bundling class.  */
  short bundling;
  /* Reservation class.  */
  short reservation;
  /* 0 terminated.  */
  struct kvx_operand *format[KVX_MAXOPERANDS + 1];
  /* Formating string.  */
  const char *fmtstring;
};

struct kvx_core_info {
  struct kvx_opc *optab;
  const char *name;
  const int *resources;
  int elf_core;
  struct pseudo_func *pseudo_funcs;
  int nb_pseudo_funcs;
  int **reservation_tables;
  int reservation_table_cycles;
  int resource_count;
  char **resource_names;
};

struct kvx_register {
  int id;
  const char *name;
};

extern const int kv3_v1_reservation_table_cycles;
extern const int *kv3_v1_reservation_tables[];
extern const char *kv3_v1_resource_names[];

extern const int kv3_v1_resources[];
extern struct kvx_opc kv3_v1_optab[];
extern const struct kvx_core_info kv3_v1_core_info;
extern const int kv3_v2_reservation_table_cycles;
extern const int *kv3_v2_reservation_tables[];
extern const char *kv3_v2_resource_names[];

extern const int kv3_v2_resources[];
extern struct kvx_opc kv3_v2_optab[];
extern const struct kvx_core_info kv3_v2_core_info;
extern const int kv4_v1_reservation_table_cycles;
extern const int *kv4_v1_reservation_tables[];
extern const char *kv4_v1_resource_names[];

extern const int kv4_v1_resources[];
extern struct kvx_opc kv4_v1_optab[];
extern const struct kvx_core_info kv4_v1_core_info;
extern const struct kvx_core_info *kvx_core_info_table[];
extern const char ***kvx_modifiers_table[];
extern const struct kvx_register *kvx_registers_table[];
extern const int *kvx_regfiles_table[];
extern const int kvx_regfiles_size_table[];

#define KVX_REGFILE_FIRST_SFR 0
#define KVX_REGFILE_LAST_SFR 1
#define KVX_REGFILE_DEC_SFR 2
#define KVX_REGFILE_FIRST_GPR 3
#define KVX_REGFILE_LAST_GPR 4
#define KVX_REGFILE_DEC_GPR 5


#define KV3_V1_REGFILE_FIRST_SFR KVX_REGFILE_FIRST_SFR
#define KV3_V1_REGFILE_LAST_SFR KVX_REGFILE_LAST_SFR
#define KV3_V1_REGFILE_DEC_SFR KVX_REGFILE_DEC_SFR
#define KV3_V1_REGFILE_FIRST_GPR KVX_REGFILE_FIRST_GPR
#define KV3_V1_REGFILE_LAST_GPR KVX_REGFILE_LAST_GPR
#define KV3_V1_REGFILE_DEC_GPR KVX_REGFILE_DEC_GPR
#define KV3_V1_REGFILE_FIRST_PGR 6
#define KV3_V1_REGFILE_LAST_PGR 7
#define KV3_V1_REGFILE_DEC_PGR 8
#define KV3_V1_REGFILE_FIRST_QGR 9
#define KV3_V1_REGFILE_LAST_QGR 10
#define KV3_V1_REGFILE_DEC_QGR 11
#define KV3_V1_REGFILE_FIRST_X16R 12
#define KV3_V1_REGFILE_LAST_X16R 13
#define KV3_V1_REGFILE_DEC_X16R 14
#define KV3_V1_REGFILE_FIRST_X2R 15
#define KV3_V1_REGFILE_LAST_X2R 16
#define KV3_V1_REGFILE_DEC_X2R 17
#define KV3_V1_REGFILE_FIRST_X32R 18
#define KV3_V1_REGFILE_LAST_X32R 19
#define KV3_V1_REGFILE_DEC_X32R 20
#define KV3_V1_REGFILE_FIRST_X4R 21
#define KV3_V1_REGFILE_LAST_X4R 22
#define KV3_V1_REGFILE_DEC_X4R 23
#define KV3_V1_REGFILE_FIRST_X64R 24
#define KV3_V1_REGFILE_LAST_X64R 25
#define KV3_V1_REGFILE_DEC_X64R 26
#define KV3_V1_REGFILE_FIRST_X8R 27
#define KV3_V1_REGFILE_LAST_X8R 28
#define KV3_V1_REGFILE_DEC_X8R 29
#define KV3_V1_REGFILE_FIRST_XBR 30
#define KV3_V1_REGFILE_LAST_XBR 31
#define KV3_V1_REGFILE_DEC_XBR 32
#define KV3_V1_REGFILE_FIRST_XCR 33
#define KV3_V1_REGFILE_LAST_XCR 34
#define KV3_V1_REGFILE_DEC_XCR 35
#define KV3_V1_REGFILE_FIRST_XMR 36
#define KV3_V1_REGFILE_LAST_XMR 37
#define KV3_V1_REGFILE_DEC_XMR 38
#define KV3_V1_REGFILE_FIRST_XTR 39
#define KV3_V1_REGFILE_LAST_XTR 40
#define KV3_V1_REGFILE_DEC_XTR 41
#define KV3_V1_REGFILE_FIRST_XVR 42
#define KV3_V1_REGFILE_LAST_XVR 43
#define KV3_V1_REGFILE_DEC_XVR 44
#define KV3_V1_REGFILE_REGISTERS 45
#define KV3_V1_REGFILE_DEC_REGISTERS 46


extern int kv3_v1_regfiles[];
extern const char **kv3_v1_modifiers[];
extern struct kvx_register kv3_v1_registers[];

extern int kv3_v1_dec_registers[];

enum Method_kv3_v1_enum {
  Immediate_kv3_v1_pcrel11 = 1,
  Immediate_kv3_v1_pcrel17 = 2,
  Immediate_kv3_v1_pcrel27 = 3,
  Immediate_kv3_v1_pcrel38 = 4,
  Immediate_kv3_v1_signed10 = 5,
  Immediate_kv3_v1_signed16 = 6,
  Immediate_kv3_v1_signed27 = 7,
  Immediate_kv3_v1_signed37 = 8,
  Immediate_kv3_v1_signed43 = 9,
  Immediate_kv3_v1_signed54 = 10,
  Immediate_kv3_v1_sysnumber = 11,
  Immediate_kv3_v1_unsigned6 = 12,
  Immediate_kv3_v1_wrapped32 = 13,
  Immediate_kv3_v1_wrapped64 = 14,
  Modifier_kv3_v1_column = 15,
  Modifier_kv3_v1_comparison = 16,
  Modifier_kv3_v1_doscale = 17,
  Modifier_kv3_v1_exunum = 18,
  Modifier_kv3_v1_floatcomp = 19,
  Modifier_kv3_v1_qindex = 20,
  Modifier_kv3_v1_rectify = 21,
  Modifier_kv3_v1_rounding = 22,
  Modifier_kv3_v1_roundint = 23,
  Modifier_kv3_v1_saturate = 24,
  Modifier_kv3_v1_scalarcond = 25,
  Modifier_kv3_v1_silent = 26,
  Modifier_kv3_v1_simdcond = 27,
  Modifier_kv3_v1_speculate = 28,
  Modifier_kv3_v1_splat32 = 29,
  Modifier_kv3_v1_variant = 30,
  RegClass_kv3_v1_aloneReg = 31,
  RegClass_kv3_v1_buffer16Reg = 32,
  RegClass_kv3_v1_buffer2Reg = 33,
  RegClass_kv3_v1_buffer32Reg = 34,
  RegClass_kv3_v1_buffer4Reg = 35,
  RegClass_kv3_v1_buffer64Reg = 36,
  RegClass_kv3_v1_buffer8Reg = 37,
  RegClass_kv3_v1_onlyfxReg = 38,
  RegClass_kv3_v1_onlygetReg = 39,
  RegClass_kv3_v1_onlyraReg = 40,
  RegClass_kv3_v1_onlysetReg = 41,
  RegClass_kv3_v1_onlyswapReg = 42,
  RegClass_kv3_v1_pairedReg = 43,
  RegClass_kv3_v1_quadReg = 44,
  RegClass_kv3_v1_singleReg = 45,
  RegClass_kv3_v1_systemReg = 46,
  RegClass_kv3_v1_xworddReg = 47,
  RegClass_kv3_v1_xworddReg0M4 = 48,
  RegClass_kv3_v1_xworddReg1M4 = 49,
  RegClass_kv3_v1_xworddReg2M4 = 50,
  RegClass_kv3_v1_xworddReg3M4 = 51,
  RegClass_kv3_v1_xwordoReg = 52,
  RegClass_kv3_v1_xwordoRegE = 53,
  RegClass_kv3_v1_xwordoRegO = 54,
  RegClass_kv3_v1_xwordqReg = 55,
  RegClass_kv3_v1_xwordqReg0M4 = 56,
  RegClass_kv3_v1_xwordqReg1M4 = 57,
  RegClass_kv3_v1_xwordqReg2M4 = 58,
  RegClass_kv3_v1_xwordqReg3M4 = 59,
  RegClass_kv3_v1_xwordqRegE = 60,
  RegClass_kv3_v1_xwordqRegO = 61,
  RegClass_kv3_v1_xwordvReg = 62,
  RegClass_kv3_v1_xwordxReg = 63,
  Instruction_kv3_v1_abdd = 64,
  Instruction_kv3_v1_abdhq = 65,
  Instruction_kv3_v1_abdw = 66,
  Instruction_kv3_v1_abdwp = 67,
  Instruction_kv3_v1_absd = 68,
  Instruction_kv3_v1_abshq = 69,
  Instruction_kv3_v1_absw = 70,
  Instruction_kv3_v1_abswp = 71,
  Instruction_kv3_v1_acswapd = 72,
  Instruction_kv3_v1_acswapw = 73,
  Instruction_kv3_v1_addcd = 74,
  Instruction_kv3_v1_addcd_i = 75,
  Instruction_kv3_v1_addd = 76,
  Instruction_kv3_v1_addhcp_c = 77,
  Instruction_kv3_v1_addhq = 78,
  Instruction_kv3_v1_addsd = 79,
  Instruction_kv3_v1_addshq = 80,
  Instruction_kv3_v1_addsw = 81,
  Instruction_kv3_v1_addswp = 82,
  Instruction_kv3_v1_adduwd = 83,
  Instruction_kv3_v1_addw = 84,
  Instruction_kv3_v1_addwc_c = 85,
  Instruction_kv3_v1_addwd = 86,
  Instruction_kv3_v1_addwp = 87,
  Instruction_kv3_v1_addx16d = 88,
  Instruction_kv3_v1_addx16hq = 89,
  Instruction_kv3_v1_addx16uwd = 90,
  Instruction_kv3_v1_addx16w = 91,
  Instruction_kv3_v1_addx16wd = 92,
  Instruction_kv3_v1_addx16wp = 93,
  Instruction_kv3_v1_addx2d = 94,
  Instruction_kv3_v1_addx2hq = 95,
  Instruction_kv3_v1_addx2uwd = 96,
  Instruction_kv3_v1_addx2w = 97,
  Instruction_kv3_v1_addx2wd = 98,
  Instruction_kv3_v1_addx2wp = 99,
  Instruction_kv3_v1_addx4d = 100,
  Instruction_kv3_v1_addx4hq = 101,
  Instruction_kv3_v1_addx4uwd = 102,
  Instruction_kv3_v1_addx4w = 103,
  Instruction_kv3_v1_addx4wd = 104,
  Instruction_kv3_v1_addx4wp = 105,
  Instruction_kv3_v1_addx8d = 106,
  Instruction_kv3_v1_addx8hq = 107,
  Instruction_kv3_v1_addx8uwd = 108,
  Instruction_kv3_v1_addx8w = 109,
  Instruction_kv3_v1_addx8wd = 110,
  Instruction_kv3_v1_addx8wp = 111,
  Instruction_kv3_v1_aladdd = 112,
  Instruction_kv3_v1_aladdw = 113,
  Instruction_kv3_v1_alclrd = 114,
  Instruction_kv3_v1_alclrw = 115,
  Instruction_kv3_v1_aligno = 116,
  Instruction_kv3_v1_alignv = 117,
  Instruction_kv3_v1_andd = 118,
  Instruction_kv3_v1_andnd = 119,
  Instruction_kv3_v1_andnw = 120,
  Instruction_kv3_v1_andw = 121,
  Instruction_kv3_v1_avghq = 122,
  Instruction_kv3_v1_avgrhq = 123,
  Instruction_kv3_v1_avgruhq = 124,
  Instruction_kv3_v1_avgruw = 125,
  Instruction_kv3_v1_avgruwp = 126,
  Instruction_kv3_v1_avgrw = 127,
  Instruction_kv3_v1_avgrwp = 128,
  Instruction_kv3_v1_avguhq = 129,
  Instruction_kv3_v1_avguw = 130,
  Instruction_kv3_v1_avguwp = 131,
  Instruction_kv3_v1_avgw = 132,
  Instruction_kv3_v1_avgwp = 133,
  Instruction_kv3_v1_await = 134,
  Instruction_kv3_v1_barrier = 135,
  Instruction_kv3_v1_call = 136,
  Instruction_kv3_v1_cb = 137,
  Instruction_kv3_v1_cbsd = 138,
  Instruction_kv3_v1_cbsw = 139,
  Instruction_kv3_v1_cbswp = 140,
  Instruction_kv3_v1_clrf = 141,
  Instruction_kv3_v1_clsd = 142,
  Instruction_kv3_v1_clsw = 143,
  Instruction_kv3_v1_clswp = 144,
  Instruction_kv3_v1_clzd = 145,
  Instruction_kv3_v1_clzw = 146,
  Instruction_kv3_v1_clzwp = 147,
  Instruction_kv3_v1_cmoved = 148,
  Instruction_kv3_v1_cmovehq = 149,
  Instruction_kv3_v1_cmovewp = 150,
  Instruction_kv3_v1_cmuldt = 151,
  Instruction_kv3_v1_cmulghxdt = 152,
  Instruction_kv3_v1_cmulglxdt = 153,
  Instruction_kv3_v1_cmulgmxdt = 154,
  Instruction_kv3_v1_cmulxdt = 155,
  Instruction_kv3_v1_compd = 156,
  Instruction_kv3_v1_compnhq = 157,
  Instruction_kv3_v1_compnwp = 158,
  Instruction_kv3_v1_compuwd = 159,
  Instruction_kv3_v1_compw = 160,
  Instruction_kv3_v1_compwd = 161,
  Instruction_kv3_v1_convdhv0 = 162,
  Instruction_kv3_v1_convdhv1 = 163,
  Instruction_kv3_v1_convwbv0 = 164,
  Instruction_kv3_v1_convwbv1 = 165,
  Instruction_kv3_v1_convwbv2 = 166,
  Instruction_kv3_v1_convwbv3 = 167,
  Instruction_kv3_v1_copyd = 168,
  Instruction_kv3_v1_copyo = 169,
  Instruction_kv3_v1_copyq = 170,
  Instruction_kv3_v1_copyw = 171,
  Instruction_kv3_v1_crcbellw = 172,
  Instruction_kv3_v1_crcbelmw = 173,
  Instruction_kv3_v1_crclellw = 174,
  Instruction_kv3_v1_crclelmw = 175,
  Instruction_kv3_v1_ctzd = 176,
  Instruction_kv3_v1_ctzw = 177,
  Instruction_kv3_v1_ctzwp = 178,
  Instruction_kv3_v1_d1inval = 179,
  Instruction_kv3_v1_dinvall = 180,
  Instruction_kv3_v1_dot2suwd = 181,
  Instruction_kv3_v1_dot2suwdp = 182,
  Instruction_kv3_v1_dot2uwd = 183,
  Instruction_kv3_v1_dot2uwdp = 184,
  Instruction_kv3_v1_dot2w = 185,
  Instruction_kv3_v1_dot2wd = 186,
  Instruction_kv3_v1_dot2wdp = 187,
  Instruction_kv3_v1_dot2wzp = 188,
  Instruction_kv3_v1_dtouchl = 189,
  Instruction_kv3_v1_dzerol = 190,
  Instruction_kv3_v1_eord = 191,
  Instruction_kv3_v1_eorw = 192,
  Instruction_kv3_v1_errop = 193,
  Instruction_kv3_v1_extfs = 194,
  Instruction_kv3_v1_extfz = 195,
  Instruction_kv3_v1_fabsd = 196,
  Instruction_kv3_v1_fabshq = 197,
  Instruction_kv3_v1_fabsw = 198,
  Instruction_kv3_v1_fabswp = 199,
  Instruction_kv3_v1_faddd = 200,
  Instruction_kv3_v1_fadddc = 201,
  Instruction_kv3_v1_fadddc_c = 202,
  Instruction_kv3_v1_fadddp = 203,
  Instruction_kv3_v1_faddhq = 204,
  Instruction_kv3_v1_faddw = 205,
  Instruction_kv3_v1_faddwc = 206,
  Instruction_kv3_v1_faddwc_c = 207,
  Instruction_kv3_v1_faddwcp = 208,
  Instruction_kv3_v1_faddwcp_c = 209,
  Instruction_kv3_v1_faddwp = 210,
  Instruction_kv3_v1_faddwq = 211,
  Instruction_kv3_v1_fcdivd = 212,
  Instruction_kv3_v1_fcdivw = 213,
  Instruction_kv3_v1_fcdivwp = 214,
  Instruction_kv3_v1_fcompd = 215,
  Instruction_kv3_v1_fcompnhq = 216,
  Instruction_kv3_v1_fcompnwp = 217,
  Instruction_kv3_v1_fcompw = 218,
  Instruction_kv3_v1_fdot2w = 219,
  Instruction_kv3_v1_fdot2wd = 220,
  Instruction_kv3_v1_fdot2wdp = 221,
  Instruction_kv3_v1_fdot2wzp = 222,
  Instruction_kv3_v1_fence = 223,
  Instruction_kv3_v1_ffmad = 224,
  Instruction_kv3_v1_ffmahq = 225,
  Instruction_kv3_v1_ffmahw = 226,
  Instruction_kv3_v1_ffmahwq = 227,
  Instruction_kv3_v1_ffmaw = 228,
  Instruction_kv3_v1_ffmawd = 229,
  Instruction_kv3_v1_ffmawdp = 230,
  Instruction_kv3_v1_ffmawp = 231,
  Instruction_kv3_v1_ffmsd = 232,
  Instruction_kv3_v1_ffmshq = 233,
  Instruction_kv3_v1_ffmshw = 234,
  Instruction_kv3_v1_ffmshwq = 235,
  Instruction_kv3_v1_ffmsw = 236,
  Instruction_kv3_v1_ffmswd = 237,
  Instruction_kv3_v1_ffmswdp = 238,
  Instruction_kv3_v1_ffmswp = 239,
  Instruction_kv3_v1_fixedd = 240,
  Instruction_kv3_v1_fixedud = 241,
  Instruction_kv3_v1_fixeduw = 242,
  Instruction_kv3_v1_fixeduwp = 243,
  Instruction_kv3_v1_fixedw = 244,
  Instruction_kv3_v1_fixedwp = 245,
  Instruction_kv3_v1_floatd = 246,
  Instruction_kv3_v1_floatud = 247,
  Instruction_kv3_v1_floatuw = 248,
  Instruction_kv3_v1_floatuwp = 249,
  Instruction_kv3_v1_floatw = 250,
  Instruction_kv3_v1_floatwp = 251,
  Instruction_kv3_v1_fmaxd = 252,
  Instruction_kv3_v1_fmaxhq = 253,
  Instruction_kv3_v1_fmaxw = 254,
  Instruction_kv3_v1_fmaxwp = 255,
  Instruction_kv3_v1_fmind = 256,
  Instruction_kv3_v1_fminhq = 257,
  Instruction_kv3_v1_fminw = 258,
  Instruction_kv3_v1_fminwp = 259,
  Instruction_kv3_v1_fmm212w = 260,
  Instruction_kv3_v1_fmma212w = 261,
  Instruction_kv3_v1_fmma242hw0 = 262,
  Instruction_kv3_v1_fmma242hw1 = 263,
  Instruction_kv3_v1_fmma242hw2 = 264,
  Instruction_kv3_v1_fmma242hw3 = 265,
  Instruction_kv3_v1_fmms212w = 266,
  Instruction_kv3_v1_fmuld = 267,
  Instruction_kv3_v1_fmulhq = 268,
  Instruction_kv3_v1_fmulhw = 269,
  Instruction_kv3_v1_fmulhwq = 270,
  Instruction_kv3_v1_fmulw = 271,
  Instruction_kv3_v1_fmulwc = 272,
  Instruction_kv3_v1_fmulwc_c = 273,
  Instruction_kv3_v1_fmulwd = 274,
  Instruction_kv3_v1_fmulwdc = 275,
  Instruction_kv3_v1_fmulwdc_c = 276,
  Instruction_kv3_v1_fmulwdp = 277,
  Instruction_kv3_v1_fmulwp = 278,
  Instruction_kv3_v1_fmulwq = 279,
  Instruction_kv3_v1_fnarrow44wh = 280,
  Instruction_kv3_v1_fnarrowdw = 281,
  Instruction_kv3_v1_fnarrowdwp = 282,
  Instruction_kv3_v1_fnarrowwh = 283,
  Instruction_kv3_v1_fnarrowwhq = 284,
  Instruction_kv3_v1_fnegd = 285,
  Instruction_kv3_v1_fneghq = 286,
  Instruction_kv3_v1_fnegw = 287,
  Instruction_kv3_v1_fnegwp = 288,
  Instruction_kv3_v1_frecw = 289,
  Instruction_kv3_v1_frsrw = 290,
  Instruction_kv3_v1_fsbfd = 291,
  Instruction_kv3_v1_fsbfdc = 292,
  Instruction_kv3_v1_fsbfdc_c = 293,
  Instruction_kv3_v1_fsbfdp = 294,
  Instruction_kv3_v1_fsbfhq = 295,
  Instruction_kv3_v1_fsbfw = 296,
  Instruction_kv3_v1_fsbfwc = 297,
  Instruction_kv3_v1_fsbfwc_c = 298,
  Instruction_kv3_v1_fsbfwcp = 299,
  Instruction_kv3_v1_fsbfwcp_c = 300,
  Instruction_kv3_v1_fsbfwp = 301,
  Instruction_kv3_v1_fsbfwq = 302,
  Instruction_kv3_v1_fscalewv = 303,
  Instruction_kv3_v1_fsdivd = 304,
  Instruction_kv3_v1_fsdivw = 305,
  Instruction_kv3_v1_fsdivwp = 306,
  Instruction_kv3_v1_fsrecd = 307,
  Instruction_kv3_v1_fsrecw = 308,
  Instruction_kv3_v1_fsrecwp = 309,
  Instruction_kv3_v1_fsrsrd = 310,
  Instruction_kv3_v1_fsrsrw = 311,
  Instruction_kv3_v1_fsrsrwp = 312,
  Instruction_kv3_v1_fwidenlhw = 313,
  Instruction_kv3_v1_fwidenlhwp = 314,
  Instruction_kv3_v1_fwidenlwd = 315,
  Instruction_kv3_v1_fwidenmhw = 316,
  Instruction_kv3_v1_fwidenmhwp = 317,
  Instruction_kv3_v1_fwidenmwd = 318,
  Instruction_kv3_v1_get = 319,
  Instruction_kv3_v1_goto = 320,
  Instruction_kv3_v1_i1inval = 321,
  Instruction_kv3_v1_i1invals = 322,
  Instruction_kv3_v1_icall = 323,
  Instruction_kv3_v1_iget = 324,
  Instruction_kv3_v1_igoto = 325,
  Instruction_kv3_v1_insf = 326,
  Instruction_kv3_v1_iord = 327,
  Instruction_kv3_v1_iornd = 328,
  Instruction_kv3_v1_iornw = 329,
  Instruction_kv3_v1_iorw = 330,
  Instruction_kv3_v1_landd = 331,
  Instruction_kv3_v1_landhq = 332,
  Instruction_kv3_v1_landw = 333,
  Instruction_kv3_v1_landwp = 334,
  Instruction_kv3_v1_lbs = 335,
  Instruction_kv3_v1_lbz = 336,
  Instruction_kv3_v1_ld = 337,
  Instruction_kv3_v1_lhs = 338,
  Instruction_kv3_v1_lhz = 339,
  Instruction_kv3_v1_liord = 340,
  Instruction_kv3_v1_liorhq = 341,
  Instruction_kv3_v1_liorw = 342,
  Instruction_kv3_v1_liorwp = 343,
  Instruction_kv3_v1_lnandd = 344,
  Instruction_kv3_v1_lnandhq = 345,
  Instruction_kv3_v1_lnandw = 346,
  Instruction_kv3_v1_lnandwp = 347,
  Instruction_kv3_v1_lniord = 348,
  Instruction_kv3_v1_lniorhq = 349,
  Instruction_kv3_v1_lniorw = 350,
  Instruction_kv3_v1_lniorwp = 351,
  Instruction_kv3_v1_lnord = 352,
  Instruction_kv3_v1_lnorhq = 353,
  Instruction_kv3_v1_lnorw = 354,
  Instruction_kv3_v1_lnorwp = 355,
  Instruction_kv3_v1_lo = 356,
  Instruction_kv3_v1_loopdo = 357,
  Instruction_kv3_v1_lord = 358,
  Instruction_kv3_v1_lorhq = 359,
  Instruction_kv3_v1_lorw = 360,
  Instruction_kv3_v1_lorwp = 361,
  Instruction_kv3_v1_lq = 362,
  Instruction_kv3_v1_lws = 363,
  Instruction_kv3_v1_lwz = 364,
  Instruction_kv3_v1_maddd = 365,
  Instruction_kv3_v1_madddt = 366,
  Instruction_kv3_v1_maddhq = 367,
  Instruction_kv3_v1_maddhwq = 368,
  Instruction_kv3_v1_maddsudt = 369,
  Instruction_kv3_v1_maddsuhwq = 370,
  Instruction_kv3_v1_maddsuwd = 371,
  Instruction_kv3_v1_maddsuwdp = 372,
  Instruction_kv3_v1_maddudt = 373,
  Instruction_kv3_v1_madduhwq = 374,
  Instruction_kv3_v1_madduwd = 375,
  Instruction_kv3_v1_madduwdp = 376,
  Instruction_kv3_v1_madduzdt = 377,
  Instruction_kv3_v1_maddw = 378,
  Instruction_kv3_v1_maddwd = 379,
  Instruction_kv3_v1_maddwdp = 380,
  Instruction_kv3_v1_maddwp = 381,
  Instruction_kv3_v1_make = 382,
  Instruction_kv3_v1_maxd = 383,
  Instruction_kv3_v1_maxhq = 384,
  Instruction_kv3_v1_maxud = 385,
  Instruction_kv3_v1_maxuhq = 386,
  Instruction_kv3_v1_maxuw = 387,
  Instruction_kv3_v1_maxuwp = 388,
  Instruction_kv3_v1_maxw = 389,
  Instruction_kv3_v1_maxwp = 390,
  Instruction_kv3_v1_mind = 391,
  Instruction_kv3_v1_minhq = 392,
  Instruction_kv3_v1_minud = 393,
  Instruction_kv3_v1_minuhq = 394,
  Instruction_kv3_v1_minuw = 395,
  Instruction_kv3_v1_minuwp = 396,
  Instruction_kv3_v1_minw = 397,
  Instruction_kv3_v1_minwp = 398,
  Instruction_kv3_v1_mm212w = 399,
  Instruction_kv3_v1_mma212w = 400,
  Instruction_kv3_v1_mma444hbd0 = 401,
  Instruction_kv3_v1_mma444hbd1 = 402,
  Instruction_kv3_v1_mma444hd = 403,
  Instruction_kv3_v1_mma444suhbd0 = 404,
  Instruction_kv3_v1_mma444suhbd1 = 405,
  Instruction_kv3_v1_mma444suhd = 406,
  Instruction_kv3_v1_mma444uhbd0 = 407,
  Instruction_kv3_v1_mma444uhbd1 = 408,
  Instruction_kv3_v1_mma444uhd = 409,
  Instruction_kv3_v1_mma444ushbd0 = 410,
  Instruction_kv3_v1_mma444ushbd1 = 411,
  Instruction_kv3_v1_mma444ushd = 412,
  Instruction_kv3_v1_mms212w = 413,
  Instruction_kv3_v1_movetq = 414,
  Instruction_kv3_v1_msbfd = 415,
  Instruction_kv3_v1_msbfdt = 416,
  Instruction_kv3_v1_msbfhq = 417,
  Instruction_kv3_v1_msbfhwq = 418,
  Instruction_kv3_v1_msbfsudt = 419,
  Instruction_kv3_v1_msbfsuhwq = 420,
  Instruction_kv3_v1_msbfsuwd = 421,
  Instruction_kv3_v1_msbfsuwdp = 422,
  Instruction_kv3_v1_msbfudt = 423,
  Instruction_kv3_v1_msbfuhwq = 424,
  Instruction_kv3_v1_msbfuwd = 425,
  Instruction_kv3_v1_msbfuwdp = 426,
  Instruction_kv3_v1_msbfuzdt = 427,
  Instruction_kv3_v1_msbfw = 428,
  Instruction_kv3_v1_msbfwd = 429,
  Instruction_kv3_v1_msbfwdp = 430,
  Instruction_kv3_v1_msbfwp = 431,
  Instruction_kv3_v1_muld = 432,
  Instruction_kv3_v1_muldt = 433,
  Instruction_kv3_v1_mulhq = 434,
  Instruction_kv3_v1_mulhwq = 435,
  Instruction_kv3_v1_mulsudt = 436,
  Instruction_kv3_v1_mulsuhwq = 437,
  Instruction_kv3_v1_mulsuwd = 438,
  Instruction_kv3_v1_mulsuwdp = 439,
  Instruction_kv3_v1_muludt = 440,
  Instruction_kv3_v1_muluhwq = 441,
  Instruction_kv3_v1_muluwd = 442,
  Instruction_kv3_v1_muluwdp = 443,
  Instruction_kv3_v1_mulw = 444,
  Instruction_kv3_v1_mulwc = 445,
  Instruction_kv3_v1_mulwc_c = 446,
  Instruction_kv3_v1_mulwd = 447,
  Instruction_kv3_v1_mulwdc = 448,
  Instruction_kv3_v1_mulwdc_c = 449,
  Instruction_kv3_v1_mulwdp = 450,
  Instruction_kv3_v1_mulwp = 451,
  Instruction_kv3_v1_mulwq = 452,
  Instruction_kv3_v1_nandd = 453,
  Instruction_kv3_v1_nandw = 454,
  Instruction_kv3_v1_negd = 455,
  Instruction_kv3_v1_neghq = 456,
  Instruction_kv3_v1_negw = 457,
  Instruction_kv3_v1_negwp = 458,
  Instruction_kv3_v1_neord = 459,
  Instruction_kv3_v1_neorw = 460,
  Instruction_kv3_v1_niord = 461,
  Instruction_kv3_v1_niorw = 462,
  Instruction_kv3_v1_nop = 463,
  Instruction_kv3_v1_nord = 464,
  Instruction_kv3_v1_norw = 465,
  Instruction_kv3_v1_notd = 466,
  Instruction_kv3_v1_notw = 467,
  Instruction_kv3_v1_nxord = 468,
  Instruction_kv3_v1_nxorw = 469,
  Instruction_kv3_v1_ord = 470,
  Instruction_kv3_v1_ornd = 471,
  Instruction_kv3_v1_ornw = 472,
  Instruction_kv3_v1_orw = 473,
  Instruction_kv3_v1_pcrel = 474,
  Instruction_kv3_v1_ret = 475,
  Instruction_kv3_v1_rfe = 476,
  Instruction_kv3_v1_rolw = 477,
  Instruction_kv3_v1_rolwps = 478,
  Instruction_kv3_v1_rorw = 479,
  Instruction_kv3_v1_rorwps = 480,
  Instruction_kv3_v1_rswap = 481,
  Instruction_kv3_v1_satd = 482,
  Instruction_kv3_v1_satdh = 483,
  Instruction_kv3_v1_satdw = 484,
  Instruction_kv3_v1_sb = 485,
  Instruction_kv3_v1_sbfcd = 486,
  Instruction_kv3_v1_sbfcd_i = 487,
  Instruction_kv3_v1_sbfd = 488,
  Instruction_kv3_v1_sbfhcp_c = 489,
  Instruction_kv3_v1_sbfhq = 490,
  Instruction_kv3_v1_sbfsd = 491,
  Instruction_kv3_v1_sbfshq = 492,
  Instruction_kv3_v1_sbfsw = 493,
  Instruction_kv3_v1_sbfswp = 494,
  Instruction_kv3_v1_sbfuwd = 495,
  Instruction_kv3_v1_sbfw = 496,
  Instruction_kv3_v1_sbfwc_c = 497,
  Instruction_kv3_v1_sbfwd = 498,
  Instruction_kv3_v1_sbfwp = 499,
  Instruction_kv3_v1_sbfx16d = 500,
  Instruction_kv3_v1_sbfx16hq = 501,
  Instruction_kv3_v1_sbfx16uwd = 502,
  Instruction_kv3_v1_sbfx16w = 503,
  Instruction_kv3_v1_sbfx16wd = 504,
  Instruction_kv3_v1_sbfx16wp = 505,
  Instruction_kv3_v1_sbfx2d = 506,
  Instruction_kv3_v1_sbfx2hq = 507,
  Instruction_kv3_v1_sbfx2uwd = 508,
  Instruction_kv3_v1_sbfx2w = 509,
  Instruction_kv3_v1_sbfx2wd = 510,
  Instruction_kv3_v1_sbfx2wp = 511,
  Instruction_kv3_v1_sbfx4d = 512,
  Instruction_kv3_v1_sbfx4hq = 513,
  Instruction_kv3_v1_sbfx4uwd = 514,
  Instruction_kv3_v1_sbfx4w = 515,
  Instruction_kv3_v1_sbfx4wd = 516,
  Instruction_kv3_v1_sbfx4wp = 517,
  Instruction_kv3_v1_sbfx8d = 518,
  Instruction_kv3_v1_sbfx8hq = 519,
  Instruction_kv3_v1_sbfx8uwd = 520,
  Instruction_kv3_v1_sbfx8w = 521,
  Instruction_kv3_v1_sbfx8wd = 522,
  Instruction_kv3_v1_sbfx8wp = 523,
  Instruction_kv3_v1_sbmm8 = 524,
  Instruction_kv3_v1_sbmm8d = 525,
  Instruction_kv3_v1_sbmmt8 = 526,
  Instruction_kv3_v1_sbmmt8d = 527,
  Instruction_kv3_v1_scall = 528,
  Instruction_kv3_v1_sd = 529,
  Instruction_kv3_v1_set = 530,
  Instruction_kv3_v1_sh = 531,
  Instruction_kv3_v1_sleep = 532,
  Instruction_kv3_v1_slld = 533,
  Instruction_kv3_v1_sllhqs = 534,
  Instruction_kv3_v1_sllw = 535,
  Instruction_kv3_v1_sllwps = 536,
  Instruction_kv3_v1_slsd = 537,
  Instruction_kv3_v1_slshqs = 538,
  Instruction_kv3_v1_slsw = 539,
  Instruction_kv3_v1_slswps = 540,
  Instruction_kv3_v1_so = 541,
  Instruction_kv3_v1_sq = 542,
  Instruction_kv3_v1_srad = 543,
  Instruction_kv3_v1_srahqs = 544,
  Instruction_kv3_v1_sraw = 545,
  Instruction_kv3_v1_srawps = 546,
  Instruction_kv3_v1_srld = 547,
  Instruction_kv3_v1_srlhqs = 548,
  Instruction_kv3_v1_srlw = 549,
  Instruction_kv3_v1_srlwps = 550,
  Instruction_kv3_v1_srsd = 551,
  Instruction_kv3_v1_srshqs = 552,
  Instruction_kv3_v1_srsw = 553,
  Instruction_kv3_v1_srswps = 554,
  Instruction_kv3_v1_stop = 555,
  Instruction_kv3_v1_stsud = 556,
  Instruction_kv3_v1_stsuw = 557,
  Instruction_kv3_v1_sw = 558,
  Instruction_kv3_v1_sxbd = 559,
  Instruction_kv3_v1_sxhd = 560,
  Instruction_kv3_v1_sxlbhq = 561,
  Instruction_kv3_v1_sxlhwp = 562,
  Instruction_kv3_v1_sxmbhq = 563,
  Instruction_kv3_v1_sxmhwp = 564,
  Instruction_kv3_v1_sxwd = 565,
  Instruction_kv3_v1_syncgroup = 566,
  Instruction_kv3_v1_tlbdinval = 567,
  Instruction_kv3_v1_tlbiinval = 568,
  Instruction_kv3_v1_tlbprobe = 569,
  Instruction_kv3_v1_tlbread = 570,
  Instruction_kv3_v1_tlbwrite = 571,
  Instruction_kv3_v1_waitit = 572,
  Instruction_kv3_v1_wfxl = 573,
  Instruction_kv3_v1_wfxm = 574,
  Instruction_kv3_v1_xcopyo = 575,
  Instruction_kv3_v1_xlo = 576,
  Instruction_kv3_v1_xmma484bw = 577,
  Instruction_kv3_v1_xmma484subw = 578,
  Instruction_kv3_v1_xmma484ubw = 579,
  Instruction_kv3_v1_xmma484usbw = 580,
  Instruction_kv3_v1_xmovefo = 581,
  Instruction_kv3_v1_xmovetq = 582,
  Instruction_kv3_v1_xmt44d = 583,
  Instruction_kv3_v1_xord = 584,
  Instruction_kv3_v1_xorw = 585,
  Instruction_kv3_v1_xso = 586,
  Instruction_kv3_v1_zxbd = 587,
  Instruction_kv3_v1_zxhd = 588,
  Instruction_kv3_v1_zxwd = 589,
  Separator_kv3_v1_comma = 590,
  Separator_kv3_v1_equal = 591,
  Separator_kv3_v1_qmark = 592,
  Separator_kv3_v1_rsbracket = 593,
  Separator_kv3_v1_lsbracket = 594
};

typedef enum {
  Modifier_kv3_v1_exunum_ALU0=0,
  Modifier_kv3_v1_exunum_ALU1=1,
  Modifier_kv3_v1_exunum_MAU=2,
  Modifier_kv3_v1_exunum_LSU=3,
} Modifier_kv3_v1_exunum_values;


extern const char *mod_kv3_v1_exunum[];
extern const char *mod_kv3_v1_scalarcond[];
extern const char *mod_kv3_v1_simdcond[];
extern const char *mod_kv3_v1_comparison[];
extern const char *mod_kv3_v1_floatcomp[];
extern const char *mod_kv3_v1_rounding[];
extern const char *mod_kv3_v1_silent[];
extern const char *mod_kv3_v1_roundint[];
extern const char *mod_kv3_v1_saturate[];
extern const char *mod_kv3_v1_rectify[];
extern const char *mod_kv3_v1_variant[];
extern const char *mod_kv3_v1_speculate[];
extern const char *mod_kv3_v1_column[];
extern const char *mod_kv3_v1_doscale[];
extern const char *mod_kv3_v1_qindex[];
extern const char *mod_kv3_v1_splat32[];

typedef enum {
  Bundling_kv3_v1_ALL,
  Bundling_kv3_v1_BCU,
  Bundling_kv3_v1_EXT,
  Bundling_kv3_v1_FULL,
  Bundling_kv3_v1_FULL_X,
  Bundling_kv3_v1_FULL_Y,
  Bundling_kv3_v1_LITE,
  Bundling_kv3_v1_LITE_X,
  Bundling_kv3_v1_LITE_Y,
  Bundling_kv3_v1_MAU,
  Bundling_kv3_v1_MAU_X,
  Bundling_kv3_v1_MAU_Y,
  Bundling_kv3_v1_LSU,
  Bundling_kv3_v1_LSU_X,
  Bundling_kv3_v1_LSU_Y,
  Bundling_kv3_v1_TINY,
  Bundling_kv3_v1_TINY_X,
  Bundling_kv3_v1_TINY_Y,
  Bundling_kv3_v1_NOP,
} Bundling_kv3_v1;

static int ATTRIBUTE_UNUSED
kv3_v1_base_bundling(int bundling) {
  static int base_bundlings[] = {
    Bundling_kv3_v1_ALL,	// Bundling_kv3_v1_ALL
    Bundling_kv3_v1_BCU,	// Bundling_kv3_v1_BCU
    Bundling_kv3_v1_EXT,	// Bundling_kv3_v1_EXT
    Bundling_kv3_v1_FULL,	// Bundling_kv3_v1_FULL
    Bundling_kv3_v1_FULL,	// Bundling_kv3_v1_FULL_X
    Bundling_kv3_v1_FULL,	// Bundling_kv3_v1_FULL_Y
    Bundling_kv3_v1_LITE,	// Bundling_kv3_v1_LITE
    Bundling_kv3_v1_LITE,	// Bundling_kv3_v1_LITE_X
    Bundling_kv3_v1_LITE,	// Bundling_kv3_v1_LITE_Y
    Bundling_kv3_v1_MAU,	// Bundling_kv3_v1_MAU
    Bundling_kv3_v1_MAU,	// Bundling_kv3_v1_MAU_X
    Bundling_kv3_v1_MAU,	// Bundling_kv3_v1_MAU_Y
    Bundling_kv3_v1_LSU,	// Bundling_kv3_v1_LSU
    Bundling_kv3_v1_LSU,	// Bundling_kv3_v1_LSU_X
    Bundling_kv3_v1_LSU,	// Bundling_kv3_v1_LSU_Y
    Bundling_kv3_v1_TINY,	// Bundling_kv3_v1_TINY
    Bundling_kv3_v1_TINY,	// Bundling_kv3_v1_TINY_X
    Bundling_kv3_v1_TINY,	// Bundling_kv3_v1_TINY_Y
    Bundling_kv3_v1_NOP,	// Bundling_kv3_v1_NOP
  };
  return base_bundlings[bundling];
};

typedef enum {
  Resource_kv3_v1_ISSUE,
  Resource_kv3_v1_TINY,
  Resource_kv3_v1_LITE,
  Resource_kv3_v1_FULL,
  Resource_kv3_v1_LSU,
  Resource_kv3_v1_MAU,
  Resource_kv3_v1_BCU,
  Resource_kv3_v1_EXT,
  Resource_kv3_v1_AUXR,
  Resource_kv3_v1_AUXW,
  Resource_kv3_v1_XFER,
  Resource_kv3_v1_MEMW,
  Resource_kv3_v1_SR12,
  Resource_kv3_v1_SR13,
  Resource_kv3_v1_SR14,
  Resource_kv3_v1_SR15,
} Resource_kv3_v1;
#define kv3_v1_RESOURCE_COUNT 16

typedef enum {
  Reservation_kv3_v1_ALL,
  Reservation_kv3_v1_ALU_TINY,
  Reservation_kv3_v1_ALU_TINY_X,
  Reservation_kv3_v1_ALU_TINY_Y,
  Reservation_kv3_v1_ALU_TINY_CRRP,
  Reservation_kv3_v1_ALU_TINY_CRWL_CRWH,
  Reservation_kv3_v1_ALU_TINY_CRWL_CRWH_X,
  Reservation_kv3_v1_ALU_TINY_CRWL_CRWH_Y,
  Reservation_kv3_v1_ALU_TINY_CRRP_CRWL_CRWH,
  Reservation_kv3_v1_ALU_TINY_CRWL,
  Reservation_kv3_v1_ALU_TINY_CRWH,
  Reservation_kv3_v1_ALU_NOP,
  Reservation_kv3_v1_ALU_LITE,
  Reservation_kv3_v1_ALU_LITE_X,
  Reservation_kv3_v1_ALU_LITE_Y,
  Reservation_kv3_v1_ALU_LITE_CRWL,
  Reservation_kv3_v1_ALU_LITE_CRWH,
  Reservation_kv3_v1_ALU_FULL,
  Reservation_kv3_v1_ALU_FULL_X,
  Reservation_kv3_v1_ALU_FULL_Y,
  Reservation_kv3_v1_BCU,
  Reservation_kv3_v1_BCU_XFER,
  Reservation_kv3_v1_BCU_CRRP_CRWL_CRWH,
  Reservation_kv3_v1_BCU_TINY_AUXW_CRRP,
  Reservation_kv3_v1_BCU_TINY_TINY_MAU_XNOP,
  Reservation_kv3_v1_EXT,
  Reservation_kv3_v1_LSU,
  Reservation_kv3_v1_LSU_X,
  Reservation_kv3_v1_LSU_Y,
  Reservation_kv3_v1_LSU_CRRP,
  Reservation_kv3_v1_LSU_CRRP_X,
  Reservation_kv3_v1_LSU_CRRP_Y,
  Reservation_kv3_v1_LSU_AUXR,
  Reservation_kv3_v1_LSU_AUXR_X,
  Reservation_kv3_v1_LSU_AUXR_Y,
  Reservation_kv3_v1_LSU_AUXW,
  Reservation_kv3_v1_LSU_AUXW_X,
  Reservation_kv3_v1_LSU_AUXW_Y,
  Reservation_kv3_v1_LSU_AUXR_AUXW,
  Reservation_kv3_v1_LSU_AUXR_AUXW_X,
  Reservation_kv3_v1_LSU_AUXR_AUXW_Y,
  Reservation_kv3_v1_MAU,
  Reservation_kv3_v1_MAU_X,
  Reservation_kv3_v1_MAU_Y,
  Reservation_kv3_v1_MAU_AUXR,
  Reservation_kv3_v1_MAU_AUXR_X,
  Reservation_kv3_v1_MAU_AUXR_Y,
} Reservation_kv3_v1;

extern struct kvx_reloc kv3_v1_rel16_reloc;
extern struct kvx_reloc kv3_v1_rel32_reloc;
extern struct kvx_reloc kv3_v1_rel64_reloc;
extern struct kvx_reloc kv3_v1_pcrel_signed16_reloc;
extern struct kvx_reloc kv3_v1_pcrel17_reloc;
extern struct kvx_reloc kv3_v1_pcrel27_reloc;
extern struct kvx_reloc kv3_v1_pcrel32_reloc;
extern struct kvx_reloc kv3_v1_pcrel_signed37_reloc;
extern struct kvx_reloc kv3_v1_pcrel_signed43_reloc;
extern struct kvx_reloc kv3_v1_pcrel_signed64_reloc;
extern struct kvx_reloc kv3_v1_pcrel64_reloc;
extern struct kvx_reloc kv3_v1_signed16_reloc;
extern struct kvx_reloc kv3_v1_signed32_reloc;
extern struct kvx_reloc kv3_v1_signed37_reloc;
extern struct kvx_reloc kv3_v1_gotoff_signed37_reloc;
extern struct kvx_reloc kv3_v1_gotoff_signed43_reloc;
extern struct kvx_reloc kv3_v1_gotoff_32_reloc;
extern struct kvx_reloc kv3_v1_gotoff_64_reloc;
extern struct kvx_reloc kv3_v1_got_32_reloc;
extern struct kvx_reloc kv3_v1_got_signed37_reloc;
extern struct kvx_reloc kv3_v1_got_signed43_reloc;
extern struct kvx_reloc kv3_v1_got_64_reloc;
extern struct kvx_reloc kv3_v1_glob_dat_reloc;
extern struct kvx_reloc kv3_v1_copy_reloc;
extern struct kvx_reloc kv3_v1_jump_slot_reloc;
extern struct kvx_reloc kv3_v1_relative_reloc;
extern struct kvx_reloc kv3_v1_signed43_reloc;
extern struct kvx_reloc kv3_v1_signed64_reloc;
extern struct kvx_reloc kv3_v1_gotaddr_signed37_reloc;
extern struct kvx_reloc kv3_v1_gotaddr_signed43_reloc;
extern struct kvx_reloc kv3_v1_gotaddr_signed64_reloc;
extern struct kvx_reloc kv3_v1_dtpmod64_reloc;
extern struct kvx_reloc kv3_v1_dtpoff64_reloc;
extern struct kvx_reloc kv3_v1_dtpoff_signed37_reloc;
extern struct kvx_reloc kv3_v1_dtpoff_signed43_reloc;
extern struct kvx_reloc kv3_v1_tlsgd_signed37_reloc;
extern struct kvx_reloc kv3_v1_tlsgd_signed43_reloc;
extern struct kvx_reloc kv3_v1_tlsld_signed37_reloc;
extern struct kvx_reloc kv3_v1_tlsld_signed43_reloc;
extern struct kvx_reloc kv3_v1_tpoff64_reloc;
extern struct kvx_reloc kv3_v1_tlsie_signed37_reloc;
extern struct kvx_reloc kv3_v1_tlsie_signed43_reloc;
extern struct kvx_reloc kv3_v1_tlsle_signed37_reloc;
extern struct kvx_reloc kv3_v1_tlsle_signed43_reloc;
extern struct kvx_reloc kv3_v1_rel8_reloc;
extern struct kvx_reloc kv3_v1_pcrel11_reloc;
extern struct kvx_reloc kv3_v1_pcrel38_reloc;

#define KV3_V2_REGFILE_FIRST_SFR KVX_REGFILE_FIRST_SFR
#define KV3_V2_REGFILE_LAST_SFR KVX_REGFILE_LAST_SFR
#define KV3_V2_REGFILE_DEC_SFR KVX_REGFILE_DEC_SFR
#define KV3_V2_REGFILE_FIRST_GPR KVX_REGFILE_FIRST_GPR
#define KV3_V2_REGFILE_LAST_GPR KVX_REGFILE_LAST_GPR
#define KV3_V2_REGFILE_DEC_GPR KVX_REGFILE_DEC_GPR
#define KV3_V2_REGFILE_FIRST_PGR 6
#define KV3_V2_REGFILE_LAST_PGR 7
#define KV3_V2_REGFILE_DEC_PGR 8
#define KV3_V2_REGFILE_FIRST_QGR 9
#define KV3_V2_REGFILE_LAST_QGR 10
#define KV3_V2_REGFILE_DEC_QGR 11
#define KV3_V2_REGFILE_FIRST_X16R 12
#define KV3_V2_REGFILE_LAST_X16R 13
#define KV3_V2_REGFILE_DEC_X16R 14
#define KV3_V2_REGFILE_FIRST_X2R 15
#define KV3_V2_REGFILE_LAST_X2R 16
#define KV3_V2_REGFILE_DEC_X2R 17
#define KV3_V2_REGFILE_FIRST_X32R 18
#define KV3_V2_REGFILE_LAST_X32R 19
#define KV3_V2_REGFILE_DEC_X32R 20
#define KV3_V2_REGFILE_FIRST_X4R 21
#define KV3_V2_REGFILE_LAST_X4R 22
#define KV3_V2_REGFILE_DEC_X4R 23
#define KV3_V2_REGFILE_FIRST_X64R 24
#define KV3_V2_REGFILE_LAST_X64R 25
#define KV3_V2_REGFILE_DEC_X64R 26
#define KV3_V2_REGFILE_FIRST_X8R 27
#define KV3_V2_REGFILE_LAST_X8R 28
#define KV3_V2_REGFILE_DEC_X8R 29
#define KV3_V2_REGFILE_FIRST_XBR 30
#define KV3_V2_REGFILE_LAST_XBR 31
#define KV3_V2_REGFILE_DEC_XBR 32
#define KV3_V2_REGFILE_FIRST_XCR 33
#define KV3_V2_REGFILE_LAST_XCR 34
#define KV3_V2_REGFILE_DEC_XCR 35
#define KV3_V2_REGFILE_FIRST_XMR 36
#define KV3_V2_REGFILE_LAST_XMR 37
#define KV3_V2_REGFILE_DEC_XMR 38
#define KV3_V2_REGFILE_FIRST_XTR 39
#define KV3_V2_REGFILE_LAST_XTR 40
#define KV3_V2_REGFILE_DEC_XTR 41
#define KV3_V2_REGFILE_FIRST_XVR 42
#define KV3_V2_REGFILE_LAST_XVR 43
#define KV3_V2_REGFILE_DEC_XVR 44
#define KV3_V2_REGFILE_REGISTERS 45
#define KV3_V2_REGFILE_DEC_REGISTERS 46


extern int kv3_v2_regfiles[];
extern const char **kv3_v2_modifiers[];
extern struct kvx_register kv3_v2_registers[];

extern int kv3_v2_dec_registers[];

enum Method_kv3_v2_enum {
  Immediate_kv3_v2_brknumber = 1,
  Immediate_kv3_v2_pcrel11 = 2,
  Immediate_kv3_v2_pcrel17 = 3,
  Immediate_kv3_v2_pcrel27 = 4,
  Immediate_kv3_v2_pcrel38 = 5,
  Immediate_kv3_v2_signed10 = 6,
  Immediate_kv3_v2_signed16 = 7,
  Immediate_kv3_v2_signed27 = 8,
  Immediate_kv3_v2_signed37 = 9,
  Immediate_kv3_v2_signed43 = 10,
  Immediate_kv3_v2_signed54 = 11,
  Immediate_kv3_v2_sysnumber = 12,
  Immediate_kv3_v2_unsigned6 = 13,
  Immediate_kv3_v2_wrapped32 = 14,
  Immediate_kv3_v2_wrapped64 = 15,
  Immediate_kv3_v2_wrapped8 = 16,
  Modifier_kv3_v2_accesses = 17,
  Modifier_kv3_v2_boolcas = 18,
  Modifier_kv3_v2_cachelev = 19,
  Modifier_kv3_v2_channel = 20,
  Modifier_kv3_v2_coherency = 21,
  Modifier_kv3_v2_comparison = 22,
  Modifier_kv3_v2_conjugate = 23,
  Modifier_kv3_v2_doscale = 24,
  Modifier_kv3_v2_exunum = 25,
  Modifier_kv3_v2_floatcomp = 26,
  Modifier_kv3_v2_hindex = 27,
  Modifier_kv3_v2_lsomask = 28,
  Modifier_kv3_v2_lsumask = 29,
  Modifier_kv3_v2_qindex = 30,
  Modifier_kv3_v2_rounding = 31,
  Modifier_kv3_v2_scalarcond = 32,
  Modifier_kv3_v2_shuffleV = 33,
  Modifier_kv3_v2_shuffleX = 34,
  Modifier_kv3_v2_silent = 35,
  Modifier_kv3_v2_simdcond = 36,
  Modifier_kv3_v2_speculate = 37,
  Modifier_kv3_v2_splat32 = 38,
  Modifier_kv3_v2_transpose = 39,
  Modifier_kv3_v2_variant = 40,
  RegClass_kv3_v2_aloneReg = 41,
  RegClass_kv3_v2_buffer16Reg = 42,
  RegClass_kv3_v2_buffer2Reg = 43,
  RegClass_kv3_v2_buffer32Reg = 44,
  RegClass_kv3_v2_buffer4Reg = 45,
  RegClass_kv3_v2_buffer64Reg = 46,
  RegClass_kv3_v2_buffer8Reg = 47,
  RegClass_kv3_v2_onlyfxReg = 48,
  RegClass_kv3_v2_onlygetReg = 49,
  RegClass_kv3_v2_onlyraReg = 50,
  RegClass_kv3_v2_onlysetReg = 51,
  RegClass_kv3_v2_onlyswapReg = 52,
  RegClass_kv3_v2_pairedReg = 53,
  RegClass_kv3_v2_quadReg = 54,
  RegClass_kv3_v2_singleReg = 55,
  RegClass_kv3_v2_systemReg = 56,
  RegClass_kv3_v2_xworddReg = 57,
  RegClass_kv3_v2_xworddReg0M4 = 58,
  RegClass_kv3_v2_xworddReg1M4 = 59,
  RegClass_kv3_v2_xworddReg2M4 = 60,
  RegClass_kv3_v2_xworddReg3M4 = 61,
  RegClass_kv3_v2_xwordoReg = 62,
  RegClass_kv3_v2_xwordqReg = 63,
  RegClass_kv3_v2_xwordqRegE = 64,
  RegClass_kv3_v2_xwordqRegO = 65,
  RegClass_kv3_v2_xwordvReg = 66,
  RegClass_kv3_v2_xwordxReg = 67,
  Instruction_kv3_v2_abdbo = 68,
  Instruction_kv3_v2_abdd = 69,
  Instruction_kv3_v2_abdhq = 70,
  Instruction_kv3_v2_abdsbo = 71,
  Instruction_kv3_v2_abdsd = 72,
  Instruction_kv3_v2_abdshq = 73,
  Instruction_kv3_v2_abdsw = 74,
  Instruction_kv3_v2_abdswp = 75,
  Instruction_kv3_v2_abdubo = 76,
  Instruction_kv3_v2_abdud = 77,
  Instruction_kv3_v2_abduhq = 78,
  Instruction_kv3_v2_abduw = 79,
  Instruction_kv3_v2_abduwp = 80,
  Instruction_kv3_v2_abdw = 81,
  Instruction_kv3_v2_abdwp = 82,
  Instruction_kv3_v2_absbo = 83,
  Instruction_kv3_v2_absd = 84,
  Instruction_kv3_v2_abshq = 85,
  Instruction_kv3_v2_abssbo = 86,
  Instruction_kv3_v2_abssd = 87,
  Instruction_kv3_v2_absshq = 88,
  Instruction_kv3_v2_abssw = 89,
  Instruction_kv3_v2_absswp = 90,
  Instruction_kv3_v2_absw = 91,
  Instruction_kv3_v2_abswp = 92,
  Instruction_kv3_v2_acswapd = 93,
  Instruction_kv3_v2_acswapq = 94,
  Instruction_kv3_v2_acswapw = 95,
  Instruction_kv3_v2_addbo = 96,
  Instruction_kv3_v2_addcd = 97,
  Instruction_kv3_v2_addcd_i = 98,
  Instruction_kv3_v2_addd = 99,
  Instruction_kv3_v2_addhq = 100,
  Instruction_kv3_v2_addrbod = 101,
  Instruction_kv3_v2_addrhqd = 102,
  Instruction_kv3_v2_addrwpd = 103,
  Instruction_kv3_v2_addsbo = 104,
  Instruction_kv3_v2_addsd = 105,
  Instruction_kv3_v2_addshq = 106,
  Instruction_kv3_v2_addsw = 107,
  Instruction_kv3_v2_addswp = 108,
  Instruction_kv3_v2_addurbod = 109,
  Instruction_kv3_v2_addurhqd = 110,
  Instruction_kv3_v2_addurwpd = 111,
  Instruction_kv3_v2_addusbo = 112,
  Instruction_kv3_v2_addusd = 113,
  Instruction_kv3_v2_addushq = 114,
  Instruction_kv3_v2_addusw = 115,
  Instruction_kv3_v2_adduswp = 116,
  Instruction_kv3_v2_adduwd = 117,
  Instruction_kv3_v2_addw = 118,
  Instruction_kv3_v2_addwd = 119,
  Instruction_kv3_v2_addwp = 120,
  Instruction_kv3_v2_addx16bo = 121,
  Instruction_kv3_v2_addx16d = 122,
  Instruction_kv3_v2_addx16hq = 123,
  Instruction_kv3_v2_addx16uwd = 124,
  Instruction_kv3_v2_addx16w = 125,
  Instruction_kv3_v2_addx16wd = 126,
  Instruction_kv3_v2_addx16wp = 127,
  Instruction_kv3_v2_addx2bo = 128,
  Instruction_kv3_v2_addx2d = 129,
  Instruction_kv3_v2_addx2hq = 130,
  Instruction_kv3_v2_addx2uwd = 131,
  Instruction_kv3_v2_addx2w = 132,
  Instruction_kv3_v2_addx2wd = 133,
  Instruction_kv3_v2_addx2wp = 134,
  Instruction_kv3_v2_addx32d = 135,
  Instruction_kv3_v2_addx32uwd = 136,
  Instruction_kv3_v2_addx32w = 137,
  Instruction_kv3_v2_addx32wd = 138,
  Instruction_kv3_v2_addx4bo = 139,
  Instruction_kv3_v2_addx4d = 140,
  Instruction_kv3_v2_addx4hq = 141,
  Instruction_kv3_v2_addx4uwd = 142,
  Instruction_kv3_v2_addx4w = 143,
  Instruction_kv3_v2_addx4wd = 144,
  Instruction_kv3_v2_addx4wp = 145,
  Instruction_kv3_v2_addx64d = 146,
  Instruction_kv3_v2_addx64uwd = 147,
  Instruction_kv3_v2_addx64w = 148,
  Instruction_kv3_v2_addx64wd = 149,
  Instruction_kv3_v2_addx8bo = 150,
  Instruction_kv3_v2_addx8d = 151,
  Instruction_kv3_v2_addx8hq = 152,
  Instruction_kv3_v2_addx8uwd = 153,
  Instruction_kv3_v2_addx8w = 154,
  Instruction_kv3_v2_addx8wd = 155,
  Instruction_kv3_v2_addx8wp = 156,
  Instruction_kv3_v2_aladdd = 157,
  Instruction_kv3_v2_aladdw = 158,
  Instruction_kv3_v2_alclrd = 159,
  Instruction_kv3_v2_alclrw = 160,
  Instruction_kv3_v2_ald = 161,
  Instruction_kv3_v2_alw = 162,
  Instruction_kv3_v2_andd = 163,
  Instruction_kv3_v2_andnd = 164,
  Instruction_kv3_v2_andnw = 165,
  Instruction_kv3_v2_andrbod = 166,
  Instruction_kv3_v2_andrhqd = 167,
  Instruction_kv3_v2_andrwpd = 168,
  Instruction_kv3_v2_andw = 169,
  Instruction_kv3_v2_asd = 170,
  Instruction_kv3_v2_asw = 171,
  Instruction_kv3_v2_avgbo = 172,
  Instruction_kv3_v2_avghq = 173,
  Instruction_kv3_v2_avgrbo = 174,
  Instruction_kv3_v2_avgrhq = 175,
  Instruction_kv3_v2_avgrubo = 176,
  Instruction_kv3_v2_avgruhq = 177,
  Instruction_kv3_v2_avgruw = 178,
  Instruction_kv3_v2_avgruwp = 179,
  Instruction_kv3_v2_avgrw = 180,
  Instruction_kv3_v2_avgrwp = 181,
  Instruction_kv3_v2_avgubo = 182,
  Instruction_kv3_v2_avguhq = 183,
  Instruction_kv3_v2_avguw = 184,
  Instruction_kv3_v2_avguwp = 185,
  Instruction_kv3_v2_avgw = 186,
  Instruction_kv3_v2_avgwp = 187,
  Instruction_kv3_v2_await = 188,
  Instruction_kv3_v2_barrier = 189,
  Instruction_kv3_v2_break = 190,
  Instruction_kv3_v2_call = 191,
  Instruction_kv3_v2_cb = 192,
  Instruction_kv3_v2_cbsd = 193,
  Instruction_kv3_v2_cbsw = 194,
  Instruction_kv3_v2_cbswp = 195,
  Instruction_kv3_v2_clrf = 196,
  Instruction_kv3_v2_clsd = 197,
  Instruction_kv3_v2_clsw = 198,
  Instruction_kv3_v2_clswp = 199,
  Instruction_kv3_v2_clzd = 200,
  Instruction_kv3_v2_clzw = 201,
  Instruction_kv3_v2_clzwp = 202,
  Instruction_kv3_v2_cmovebo = 203,
  Instruction_kv3_v2_cmoved = 204,
  Instruction_kv3_v2_cmovehq = 205,
  Instruction_kv3_v2_cmovewp = 206,
  Instruction_kv3_v2_cmuldt = 207,
  Instruction_kv3_v2_cmulghxdt = 208,
  Instruction_kv3_v2_cmulglxdt = 209,
  Instruction_kv3_v2_cmulgmxdt = 210,
  Instruction_kv3_v2_cmulxdt = 211,
  Instruction_kv3_v2_compd = 212,
  Instruction_kv3_v2_compnbo = 213,
  Instruction_kv3_v2_compnd = 214,
  Instruction_kv3_v2_compnhq = 215,
  Instruction_kv3_v2_compnw = 216,
  Instruction_kv3_v2_compnwp = 217,
  Instruction_kv3_v2_compuwd = 218,
  Instruction_kv3_v2_compw = 219,
  Instruction_kv3_v2_compwd = 220,
  Instruction_kv3_v2_copyd = 221,
  Instruction_kv3_v2_copyo = 222,
  Instruction_kv3_v2_copyq = 223,
  Instruction_kv3_v2_copyw = 224,
  Instruction_kv3_v2_crcbellw = 225,
  Instruction_kv3_v2_crcbelmw = 226,
  Instruction_kv3_v2_crclellw = 227,
  Instruction_kv3_v2_crclelmw = 228,
  Instruction_kv3_v2_ctzd = 229,
  Instruction_kv3_v2_ctzw = 230,
  Instruction_kv3_v2_ctzwp = 231,
  Instruction_kv3_v2_d1inval = 232,
  Instruction_kv3_v2_dflushl = 233,
  Instruction_kv3_v2_dflushsw = 234,
  Instruction_kv3_v2_dinvall = 235,
  Instruction_kv3_v2_dinvalsw = 236,
  Instruction_kv3_v2_dot2suwd = 237,
  Instruction_kv3_v2_dot2suwdp = 238,
  Instruction_kv3_v2_dot2uwd = 239,
  Instruction_kv3_v2_dot2uwdp = 240,
  Instruction_kv3_v2_dot2w = 241,
  Instruction_kv3_v2_dot2wd = 242,
  Instruction_kv3_v2_dot2wdp = 243,
  Instruction_kv3_v2_dot2wzp = 244,
  Instruction_kv3_v2_dpurgel = 245,
  Instruction_kv3_v2_dpurgesw = 246,
  Instruction_kv3_v2_dtouchl = 247,
  Instruction_kv3_v2_eord = 248,
  Instruction_kv3_v2_eorrbod = 249,
  Instruction_kv3_v2_eorrhqd = 250,
  Instruction_kv3_v2_eorrwpd = 251,
  Instruction_kv3_v2_eorw = 252,
  Instruction_kv3_v2_errop = 253,
  Instruction_kv3_v2_extfs = 254,
  Instruction_kv3_v2_extfz = 255,
  Instruction_kv3_v2_fabsd = 256,
  Instruction_kv3_v2_fabshq = 257,
  Instruction_kv3_v2_fabsw = 258,
  Instruction_kv3_v2_fabswp = 259,
  Instruction_kv3_v2_faddd = 260,
  Instruction_kv3_v2_fadddc = 261,
  Instruction_kv3_v2_fadddc_c = 262,
  Instruction_kv3_v2_fadddp = 263,
  Instruction_kv3_v2_faddho = 264,
  Instruction_kv3_v2_faddhq = 265,
  Instruction_kv3_v2_faddw = 266,
  Instruction_kv3_v2_faddwc = 267,
  Instruction_kv3_v2_faddwc_c = 268,
  Instruction_kv3_v2_faddwcp = 269,
  Instruction_kv3_v2_faddwcp_c = 270,
  Instruction_kv3_v2_faddwp = 271,
  Instruction_kv3_v2_faddwq = 272,
  Instruction_kv3_v2_fcdivd = 273,
  Instruction_kv3_v2_fcdivw = 274,
  Instruction_kv3_v2_fcdivwp = 275,
  Instruction_kv3_v2_fcompd = 276,
  Instruction_kv3_v2_fcompnd = 277,
  Instruction_kv3_v2_fcompnhq = 278,
  Instruction_kv3_v2_fcompnw = 279,
  Instruction_kv3_v2_fcompnwp = 280,
  Instruction_kv3_v2_fcompw = 281,
  Instruction_kv3_v2_fdot2w = 282,
  Instruction_kv3_v2_fdot2wd = 283,
  Instruction_kv3_v2_fdot2wdp = 284,
  Instruction_kv3_v2_fdot2wzp = 285,
  Instruction_kv3_v2_fence = 286,
  Instruction_kv3_v2_ffdmasw = 287,
  Instruction_kv3_v2_ffdmaswp = 288,
  Instruction_kv3_v2_ffdmaswq = 289,
  Instruction_kv3_v2_ffdmaw = 290,
  Instruction_kv3_v2_ffdmawp = 291,
  Instruction_kv3_v2_ffdmawq = 292,
  Instruction_kv3_v2_ffdmdaw = 293,
  Instruction_kv3_v2_ffdmdawp = 294,
  Instruction_kv3_v2_ffdmdawq = 295,
  Instruction_kv3_v2_ffdmdsw = 296,
  Instruction_kv3_v2_ffdmdswp = 297,
  Instruction_kv3_v2_ffdmdswq = 298,
  Instruction_kv3_v2_ffdmsaw = 299,
  Instruction_kv3_v2_ffdmsawp = 300,
  Instruction_kv3_v2_ffdmsawq = 301,
  Instruction_kv3_v2_ffdmsw = 302,
  Instruction_kv3_v2_ffdmswp = 303,
  Instruction_kv3_v2_ffdmswq = 304,
  Instruction_kv3_v2_ffmad = 305,
  Instruction_kv3_v2_ffmaho = 306,
  Instruction_kv3_v2_ffmahq = 307,
  Instruction_kv3_v2_ffmahw = 308,
  Instruction_kv3_v2_ffmahwq = 309,
  Instruction_kv3_v2_ffmaw = 310,
  Instruction_kv3_v2_ffmawc = 311,
  Instruction_kv3_v2_ffmawcp = 312,
  Instruction_kv3_v2_ffmawd = 313,
  Instruction_kv3_v2_ffmawdp = 314,
  Instruction_kv3_v2_ffmawp = 315,
  Instruction_kv3_v2_ffmawq = 316,
  Instruction_kv3_v2_ffmsd = 317,
  Instruction_kv3_v2_ffmsho = 318,
  Instruction_kv3_v2_ffmshq = 319,
  Instruction_kv3_v2_ffmshw = 320,
  Instruction_kv3_v2_ffmshwq = 321,
  Instruction_kv3_v2_ffmsw = 322,
  Instruction_kv3_v2_ffmswc = 323,
  Instruction_kv3_v2_ffmswcp = 324,
  Instruction_kv3_v2_ffmswd = 325,
  Instruction_kv3_v2_ffmswdp = 326,
  Instruction_kv3_v2_ffmswp = 327,
  Instruction_kv3_v2_ffmswq = 328,
  Instruction_kv3_v2_fixedd = 329,
  Instruction_kv3_v2_fixedud = 330,
  Instruction_kv3_v2_fixeduw = 331,
  Instruction_kv3_v2_fixeduwp = 332,
  Instruction_kv3_v2_fixedw = 333,
  Instruction_kv3_v2_fixedwp = 334,
  Instruction_kv3_v2_floatd = 335,
  Instruction_kv3_v2_floatud = 336,
  Instruction_kv3_v2_floatuw = 337,
  Instruction_kv3_v2_floatuwp = 338,
  Instruction_kv3_v2_floatw = 339,
  Instruction_kv3_v2_floatwp = 340,
  Instruction_kv3_v2_fmaxd = 341,
  Instruction_kv3_v2_fmaxhq = 342,
  Instruction_kv3_v2_fmaxw = 343,
  Instruction_kv3_v2_fmaxwp = 344,
  Instruction_kv3_v2_fmind = 345,
  Instruction_kv3_v2_fminhq = 346,
  Instruction_kv3_v2_fminw = 347,
  Instruction_kv3_v2_fminwp = 348,
  Instruction_kv3_v2_fmm212w = 349,
  Instruction_kv3_v2_fmm222w = 350,
  Instruction_kv3_v2_fmma212w = 351,
  Instruction_kv3_v2_fmma222w = 352,
  Instruction_kv3_v2_fmms212w = 353,
  Instruction_kv3_v2_fmms222w = 354,
  Instruction_kv3_v2_fmuld = 355,
  Instruction_kv3_v2_fmulho = 356,
  Instruction_kv3_v2_fmulhq = 357,
  Instruction_kv3_v2_fmulhw = 358,
  Instruction_kv3_v2_fmulhwq = 359,
  Instruction_kv3_v2_fmulw = 360,
  Instruction_kv3_v2_fmulwc = 361,
  Instruction_kv3_v2_fmulwcp = 362,
  Instruction_kv3_v2_fmulwd = 363,
  Instruction_kv3_v2_fmulwdp = 364,
  Instruction_kv3_v2_fmulwp = 365,
  Instruction_kv3_v2_fmulwq = 366,
  Instruction_kv3_v2_fnarrowdw = 367,
  Instruction_kv3_v2_fnarrowdwp = 368,
  Instruction_kv3_v2_fnarrowwh = 369,
  Instruction_kv3_v2_fnarrowwhq = 370,
  Instruction_kv3_v2_fnegd = 371,
  Instruction_kv3_v2_fneghq = 372,
  Instruction_kv3_v2_fnegw = 373,
  Instruction_kv3_v2_fnegwp = 374,
  Instruction_kv3_v2_frecw = 375,
  Instruction_kv3_v2_frsrw = 376,
  Instruction_kv3_v2_fsbfd = 377,
  Instruction_kv3_v2_fsbfdc = 378,
  Instruction_kv3_v2_fsbfdc_c = 379,
  Instruction_kv3_v2_fsbfdp = 380,
  Instruction_kv3_v2_fsbfho = 381,
  Instruction_kv3_v2_fsbfhq = 382,
  Instruction_kv3_v2_fsbfw = 383,
  Instruction_kv3_v2_fsbfwc = 384,
  Instruction_kv3_v2_fsbfwc_c = 385,
  Instruction_kv3_v2_fsbfwcp = 386,
  Instruction_kv3_v2_fsbfwcp_c = 387,
  Instruction_kv3_v2_fsbfwp = 388,
  Instruction_kv3_v2_fsbfwq = 389,
  Instruction_kv3_v2_fsdivd = 390,
  Instruction_kv3_v2_fsdivw = 391,
  Instruction_kv3_v2_fsdivwp = 392,
  Instruction_kv3_v2_fsrecd = 393,
  Instruction_kv3_v2_fsrecw = 394,
  Instruction_kv3_v2_fsrecwp = 395,
  Instruction_kv3_v2_fsrsrd = 396,
  Instruction_kv3_v2_fsrsrw = 397,
  Instruction_kv3_v2_fsrsrwp = 398,
  Instruction_kv3_v2_fwidenlhw = 399,
  Instruction_kv3_v2_fwidenlhwp = 400,
  Instruction_kv3_v2_fwidenlwd = 401,
  Instruction_kv3_v2_fwidenmhw = 402,
  Instruction_kv3_v2_fwidenmhwp = 403,
  Instruction_kv3_v2_fwidenmwd = 404,
  Instruction_kv3_v2_get = 405,
  Instruction_kv3_v2_goto = 406,
  Instruction_kv3_v2_i1inval = 407,
  Instruction_kv3_v2_i1invals = 408,
  Instruction_kv3_v2_icall = 409,
  Instruction_kv3_v2_iget = 410,
  Instruction_kv3_v2_igoto = 411,
  Instruction_kv3_v2_insf = 412,
  Instruction_kv3_v2_iord = 413,
  Instruction_kv3_v2_iornd = 414,
  Instruction_kv3_v2_iornw = 415,
  Instruction_kv3_v2_iorrbod = 416,
  Instruction_kv3_v2_iorrhqd = 417,
  Instruction_kv3_v2_iorrwpd = 418,
  Instruction_kv3_v2_iorw = 419,
  Instruction_kv3_v2_landd = 420,
  Instruction_kv3_v2_landw = 421,
  Instruction_kv3_v2_lbs = 422,
  Instruction_kv3_v2_lbz = 423,
  Instruction_kv3_v2_ld = 424,
  Instruction_kv3_v2_lhs = 425,
  Instruction_kv3_v2_lhz = 426,
  Instruction_kv3_v2_liord = 427,
  Instruction_kv3_v2_liorw = 428,
  Instruction_kv3_v2_lnandd = 429,
  Instruction_kv3_v2_lnandw = 430,
  Instruction_kv3_v2_lniord = 431,
  Instruction_kv3_v2_lniorw = 432,
  Instruction_kv3_v2_lnord = 433,
  Instruction_kv3_v2_lnorw = 434,
  Instruction_kv3_v2_lo = 435,
  Instruction_kv3_v2_loopdo = 436,
  Instruction_kv3_v2_lord = 437,
  Instruction_kv3_v2_lorw = 438,
  Instruction_kv3_v2_lq = 439,
  Instruction_kv3_v2_lws = 440,
  Instruction_kv3_v2_lwz = 441,
  Instruction_kv3_v2_maddd = 442,
  Instruction_kv3_v2_madddt = 443,
  Instruction_kv3_v2_maddhq = 444,
  Instruction_kv3_v2_maddhwq = 445,
  Instruction_kv3_v2_maddmwq = 446,
  Instruction_kv3_v2_maddsudt = 447,
  Instruction_kv3_v2_maddsuhwq = 448,
  Instruction_kv3_v2_maddsumwq = 449,
  Instruction_kv3_v2_maddsuwd = 450,
  Instruction_kv3_v2_maddsuwdp = 451,
  Instruction_kv3_v2_maddudt = 452,
  Instruction_kv3_v2_madduhwq = 453,
  Instruction_kv3_v2_maddumwq = 454,
  Instruction_kv3_v2_madduwd = 455,
  Instruction_kv3_v2_madduwdp = 456,
  Instruction_kv3_v2_madduzdt = 457,
  Instruction_kv3_v2_maddw = 458,
  Instruction_kv3_v2_maddwd = 459,
  Instruction_kv3_v2_maddwdp = 460,
  Instruction_kv3_v2_maddwp = 461,
  Instruction_kv3_v2_maddwq = 462,
  Instruction_kv3_v2_make = 463,
  Instruction_kv3_v2_maxbo = 464,
  Instruction_kv3_v2_maxd = 465,
  Instruction_kv3_v2_maxhq = 466,
  Instruction_kv3_v2_maxrbod = 467,
  Instruction_kv3_v2_maxrhqd = 468,
  Instruction_kv3_v2_maxrwpd = 469,
  Instruction_kv3_v2_maxubo = 470,
  Instruction_kv3_v2_maxud = 471,
  Instruction_kv3_v2_maxuhq = 472,
  Instruction_kv3_v2_maxurbod = 473,
  Instruction_kv3_v2_maxurhqd = 474,
  Instruction_kv3_v2_maxurwpd = 475,
  Instruction_kv3_v2_maxuw = 476,
  Instruction_kv3_v2_maxuwp = 477,
  Instruction_kv3_v2_maxw = 478,
  Instruction_kv3_v2_maxwp = 479,
  Instruction_kv3_v2_minbo = 480,
  Instruction_kv3_v2_mind = 481,
  Instruction_kv3_v2_minhq = 482,
  Instruction_kv3_v2_minrbod = 483,
  Instruction_kv3_v2_minrhqd = 484,
  Instruction_kv3_v2_minrwpd = 485,
  Instruction_kv3_v2_minubo = 486,
  Instruction_kv3_v2_minud = 487,
  Instruction_kv3_v2_minuhq = 488,
  Instruction_kv3_v2_minurbod = 489,
  Instruction_kv3_v2_minurhqd = 490,
  Instruction_kv3_v2_minurwpd = 491,
  Instruction_kv3_v2_minuw = 492,
  Instruction_kv3_v2_minuwp = 493,
  Instruction_kv3_v2_minw = 494,
  Instruction_kv3_v2_minwp = 495,
  Instruction_kv3_v2_mm212w = 496,
  Instruction_kv3_v2_mma212w = 497,
  Instruction_kv3_v2_mms212w = 498,
  Instruction_kv3_v2_msbfd = 499,
  Instruction_kv3_v2_msbfdt = 500,
  Instruction_kv3_v2_msbfhq = 501,
  Instruction_kv3_v2_msbfhwq = 502,
  Instruction_kv3_v2_msbfmwq = 503,
  Instruction_kv3_v2_msbfsudt = 504,
  Instruction_kv3_v2_msbfsuhwq = 505,
  Instruction_kv3_v2_msbfsumwq = 506,
  Instruction_kv3_v2_msbfsuwd = 507,
  Instruction_kv3_v2_msbfsuwdp = 508,
  Instruction_kv3_v2_msbfudt = 509,
  Instruction_kv3_v2_msbfuhwq = 510,
  Instruction_kv3_v2_msbfumwq = 511,
  Instruction_kv3_v2_msbfuwd = 512,
  Instruction_kv3_v2_msbfuwdp = 513,
  Instruction_kv3_v2_msbfuzdt = 514,
  Instruction_kv3_v2_msbfw = 515,
  Instruction_kv3_v2_msbfwd = 516,
  Instruction_kv3_v2_msbfwdp = 517,
  Instruction_kv3_v2_msbfwp = 518,
  Instruction_kv3_v2_msbfwq = 519,
  Instruction_kv3_v2_muld = 520,
  Instruction_kv3_v2_muldt = 521,
  Instruction_kv3_v2_mulhq = 522,
  Instruction_kv3_v2_mulhwq = 523,
  Instruction_kv3_v2_mulmwq = 524,
  Instruction_kv3_v2_mulsudt = 525,
  Instruction_kv3_v2_mulsuhwq = 526,
  Instruction_kv3_v2_mulsumwq = 527,
  Instruction_kv3_v2_mulsuwd = 528,
  Instruction_kv3_v2_mulsuwdp = 529,
  Instruction_kv3_v2_muludt = 530,
  Instruction_kv3_v2_muluhwq = 531,
  Instruction_kv3_v2_mulumwq = 532,
  Instruction_kv3_v2_muluwd = 533,
  Instruction_kv3_v2_muluwdp = 534,
  Instruction_kv3_v2_mulw = 535,
  Instruction_kv3_v2_mulwd = 536,
  Instruction_kv3_v2_mulwdp = 537,
  Instruction_kv3_v2_mulwp = 538,
  Instruction_kv3_v2_mulwq = 539,
  Instruction_kv3_v2_nandd = 540,
  Instruction_kv3_v2_nandw = 541,
  Instruction_kv3_v2_negbo = 542,
  Instruction_kv3_v2_negd = 543,
  Instruction_kv3_v2_neghq = 544,
  Instruction_kv3_v2_negsbo = 545,
  Instruction_kv3_v2_negsd = 546,
  Instruction_kv3_v2_negshq = 547,
  Instruction_kv3_v2_negsw = 548,
  Instruction_kv3_v2_negswp = 549,
  Instruction_kv3_v2_negw = 550,
  Instruction_kv3_v2_negwp = 551,
  Instruction_kv3_v2_neord = 552,
  Instruction_kv3_v2_neorw = 553,
  Instruction_kv3_v2_niord = 554,
  Instruction_kv3_v2_niorw = 555,
  Instruction_kv3_v2_nop = 556,
  Instruction_kv3_v2_nord = 557,
  Instruction_kv3_v2_norw = 558,
  Instruction_kv3_v2_notd = 559,
  Instruction_kv3_v2_notw = 560,
  Instruction_kv3_v2_nxord = 561,
  Instruction_kv3_v2_nxorw = 562,
  Instruction_kv3_v2_ord = 563,
  Instruction_kv3_v2_ornd = 564,
  Instruction_kv3_v2_ornw = 565,
  Instruction_kv3_v2_orrbod = 566,
  Instruction_kv3_v2_orrhqd = 567,
  Instruction_kv3_v2_orrwpd = 568,
  Instruction_kv3_v2_orw = 569,
  Instruction_kv3_v2_pcrel = 570,
  Instruction_kv3_v2_ret = 571,
  Instruction_kv3_v2_rfe = 572,
  Instruction_kv3_v2_rolw = 573,
  Instruction_kv3_v2_rolwps = 574,
  Instruction_kv3_v2_rorw = 575,
  Instruction_kv3_v2_rorwps = 576,
  Instruction_kv3_v2_rswap = 577,
  Instruction_kv3_v2_sb = 578,
  Instruction_kv3_v2_sbfbo = 579,
  Instruction_kv3_v2_sbfcd = 580,
  Instruction_kv3_v2_sbfcd_i = 581,
  Instruction_kv3_v2_sbfd = 582,
  Instruction_kv3_v2_sbfhq = 583,
  Instruction_kv3_v2_sbfsbo = 584,
  Instruction_kv3_v2_sbfsd = 585,
  Instruction_kv3_v2_sbfshq = 586,
  Instruction_kv3_v2_sbfsw = 587,
  Instruction_kv3_v2_sbfswp = 588,
  Instruction_kv3_v2_sbfusbo = 589,
  Instruction_kv3_v2_sbfusd = 590,
  Instruction_kv3_v2_sbfushq = 591,
  Instruction_kv3_v2_sbfusw = 592,
  Instruction_kv3_v2_sbfuswp = 593,
  Instruction_kv3_v2_sbfuwd = 594,
  Instruction_kv3_v2_sbfw = 595,
  Instruction_kv3_v2_sbfwd = 596,
  Instruction_kv3_v2_sbfwp = 597,
  Instruction_kv3_v2_sbfx16bo = 598,
  Instruction_kv3_v2_sbfx16d = 599,
  Instruction_kv3_v2_sbfx16hq = 600,
  Instruction_kv3_v2_sbfx16uwd = 601,
  Instruction_kv3_v2_sbfx16w = 602,
  Instruction_kv3_v2_sbfx16wd = 603,
  Instruction_kv3_v2_sbfx16wp = 604,
  Instruction_kv3_v2_sbfx2bo = 605,
  Instruction_kv3_v2_sbfx2d = 606,
  Instruction_kv3_v2_sbfx2hq = 607,
  Instruction_kv3_v2_sbfx2uwd = 608,
  Instruction_kv3_v2_sbfx2w = 609,
  Instruction_kv3_v2_sbfx2wd = 610,
  Instruction_kv3_v2_sbfx2wp = 611,
  Instruction_kv3_v2_sbfx32d = 612,
  Instruction_kv3_v2_sbfx32uwd = 613,
  Instruction_kv3_v2_sbfx32w = 614,
  Instruction_kv3_v2_sbfx32wd = 615,
  Instruction_kv3_v2_sbfx4bo = 616,
  Instruction_kv3_v2_sbfx4d = 617,
  Instruction_kv3_v2_sbfx4hq = 618,
  Instruction_kv3_v2_sbfx4uwd = 619,
  Instruction_kv3_v2_sbfx4w = 620,
  Instruction_kv3_v2_sbfx4wd = 621,
  Instruction_kv3_v2_sbfx4wp = 622,
  Instruction_kv3_v2_sbfx64d = 623,
  Instruction_kv3_v2_sbfx64uwd = 624,
  Instruction_kv3_v2_sbfx64w = 625,
  Instruction_kv3_v2_sbfx64wd = 626,
  Instruction_kv3_v2_sbfx8bo = 627,
  Instruction_kv3_v2_sbfx8d = 628,
  Instruction_kv3_v2_sbfx8hq = 629,
  Instruction_kv3_v2_sbfx8uwd = 630,
  Instruction_kv3_v2_sbfx8w = 631,
  Instruction_kv3_v2_sbfx8wd = 632,
  Instruction_kv3_v2_sbfx8wp = 633,
  Instruction_kv3_v2_sbmm8 = 634,
  Instruction_kv3_v2_sbmm8d = 635,
  Instruction_kv3_v2_sbmmt8 = 636,
  Instruction_kv3_v2_sbmmt8d = 637,
  Instruction_kv3_v2_scall = 638,
  Instruction_kv3_v2_sd = 639,
  Instruction_kv3_v2_set = 640,
  Instruction_kv3_v2_sh = 641,
  Instruction_kv3_v2_sleep = 642,
  Instruction_kv3_v2_sllbos = 643,
  Instruction_kv3_v2_slld = 644,
  Instruction_kv3_v2_sllhqs = 645,
  Instruction_kv3_v2_sllw = 646,
  Instruction_kv3_v2_sllwps = 647,
  Instruction_kv3_v2_slsbos = 648,
  Instruction_kv3_v2_slsd = 649,
  Instruction_kv3_v2_slshqs = 650,
  Instruction_kv3_v2_slsw = 651,
  Instruction_kv3_v2_slswps = 652,
  Instruction_kv3_v2_slusbos = 653,
  Instruction_kv3_v2_slusd = 654,
  Instruction_kv3_v2_slushqs = 655,
  Instruction_kv3_v2_slusw = 656,
  Instruction_kv3_v2_sluswps = 657,
  Instruction_kv3_v2_so = 658,
  Instruction_kv3_v2_sq = 659,
  Instruction_kv3_v2_srabos = 660,
  Instruction_kv3_v2_srad = 661,
  Instruction_kv3_v2_srahqs = 662,
  Instruction_kv3_v2_sraw = 663,
  Instruction_kv3_v2_srawps = 664,
  Instruction_kv3_v2_srlbos = 665,
  Instruction_kv3_v2_srld = 666,
  Instruction_kv3_v2_srlhqs = 667,
  Instruction_kv3_v2_srlw = 668,
  Instruction_kv3_v2_srlwps = 669,
  Instruction_kv3_v2_srsbos = 670,
  Instruction_kv3_v2_srsd = 671,
  Instruction_kv3_v2_srshqs = 672,
  Instruction_kv3_v2_srsw = 673,
  Instruction_kv3_v2_srswps = 674,
  Instruction_kv3_v2_stop = 675,
  Instruction_kv3_v2_stsud = 676,
  Instruction_kv3_v2_stsuhq = 677,
  Instruction_kv3_v2_stsuw = 678,
  Instruction_kv3_v2_stsuwp = 679,
  Instruction_kv3_v2_sw = 680,
  Instruction_kv3_v2_sxbd = 681,
  Instruction_kv3_v2_sxhd = 682,
  Instruction_kv3_v2_sxlbhq = 683,
  Instruction_kv3_v2_sxlhwp = 684,
  Instruction_kv3_v2_sxmbhq = 685,
  Instruction_kv3_v2_sxmhwp = 686,
  Instruction_kv3_v2_sxwd = 687,
  Instruction_kv3_v2_syncgroup = 688,
  Instruction_kv3_v2_tlbdinval = 689,
  Instruction_kv3_v2_tlbiinval = 690,
  Instruction_kv3_v2_tlbprobe = 691,
  Instruction_kv3_v2_tlbread = 692,
  Instruction_kv3_v2_tlbwrite = 693,
  Instruction_kv3_v2_waitit = 694,
  Instruction_kv3_v2_wfxl = 695,
  Instruction_kv3_v2_wfxm = 696,
  Instruction_kv3_v2_xaccesso = 697,
  Instruction_kv3_v2_xaligno = 698,
  Instruction_kv3_v2_xandno = 699,
  Instruction_kv3_v2_xando = 700,
  Instruction_kv3_v2_xclampwo = 701,
  Instruction_kv3_v2_xcopyo = 702,
  Instruction_kv3_v2_xcopyv = 703,
  Instruction_kv3_v2_xcopyx = 704,
  Instruction_kv3_v2_xeoro = 705,
  Instruction_kv3_v2_xffma44hw = 706,
  Instruction_kv3_v2_xfmaxhx = 707,
  Instruction_kv3_v2_xfminhx = 708,
  Instruction_kv3_v2_xfmma484hw = 709,
  Instruction_kv3_v2_xfnarrow44wh = 710,
  Instruction_kv3_v2_xfscalewo = 711,
  Instruction_kv3_v2_xiorno = 712,
  Instruction_kv3_v2_xioro = 713,
  Instruction_kv3_v2_xlo = 714,
  Instruction_kv3_v2_xmadd44bw0 = 715,
  Instruction_kv3_v2_xmadd44bw1 = 716,
  Instruction_kv3_v2_xmaddifwo = 717,
  Instruction_kv3_v2_xmaddsu44bw0 = 718,
  Instruction_kv3_v2_xmaddsu44bw1 = 719,
  Instruction_kv3_v2_xmaddu44bw0 = 720,
  Instruction_kv3_v2_xmaddu44bw1 = 721,
  Instruction_kv3_v2_xmma4164bw = 722,
  Instruction_kv3_v2_xmma484bw = 723,
  Instruction_kv3_v2_xmmasu4164bw = 724,
  Instruction_kv3_v2_xmmasu484bw = 725,
  Instruction_kv3_v2_xmmau4164bw = 726,
  Instruction_kv3_v2_xmmau484bw = 727,
  Instruction_kv3_v2_xmmaus4164bw = 728,
  Instruction_kv3_v2_xmmaus484bw = 729,
  Instruction_kv3_v2_xmovefd = 730,
  Instruction_kv3_v2_xmovefo = 731,
  Instruction_kv3_v2_xmovefq = 732,
  Instruction_kv3_v2_xmovetd = 733,
  Instruction_kv3_v2_xmovetq = 734,
  Instruction_kv3_v2_xmsbfifwo = 735,
  Instruction_kv3_v2_xmt44d = 736,
  Instruction_kv3_v2_xnando = 737,
  Instruction_kv3_v2_xneoro = 738,
  Instruction_kv3_v2_xnioro = 739,
  Instruction_kv3_v2_xnoro = 740,
  Instruction_kv3_v2_xnxoro = 741,
  Instruction_kv3_v2_xord = 742,
  Instruction_kv3_v2_xorno = 743,
  Instruction_kv3_v2_xoro = 744,
  Instruction_kv3_v2_xorrbod = 745,
  Instruction_kv3_v2_xorrhqd = 746,
  Instruction_kv3_v2_xorrwpd = 747,
  Instruction_kv3_v2_xorw = 748,
  Instruction_kv3_v2_xplb = 749,
  Instruction_kv3_v2_xpld = 750,
  Instruction_kv3_v2_xplh = 751,
  Instruction_kv3_v2_xplo = 752,
  Instruction_kv3_v2_xplq = 753,
  Instruction_kv3_v2_xplw = 754,
  Instruction_kv3_v2_xrecvo = 755,
  Instruction_kv3_v2_xsbmm8dq = 756,
  Instruction_kv3_v2_xsbmmt8dq = 757,
  Instruction_kv3_v2_xsendo = 758,
  Instruction_kv3_v2_xsendrecvo = 759,
  Instruction_kv3_v2_xso = 760,
  Instruction_kv3_v2_xsplatdo = 761,
  Instruction_kv3_v2_xsplatov = 762,
  Instruction_kv3_v2_xsplatox = 763,
  Instruction_kv3_v2_xsx48bw = 764,
  Instruction_kv3_v2_xtrunc48wb = 765,
  Instruction_kv3_v2_xxoro = 766,
  Instruction_kv3_v2_xzx48bw = 767,
  Instruction_kv3_v2_zxbd = 768,
  Instruction_kv3_v2_zxhd = 769,
  Instruction_kv3_v2_zxlbhq = 770,
  Instruction_kv3_v2_zxlhwp = 771,
  Instruction_kv3_v2_zxmbhq = 772,
  Instruction_kv3_v2_zxmhwp = 773,
  Instruction_kv3_v2_zxwd = 774,
  Separator_kv3_v2_comma = 775,
  Separator_kv3_v2_equal = 776,
  Separator_kv3_v2_qmark = 777,
  Separator_kv3_v2_rsbracket = 778,
  Separator_kv3_v2_lsbracket = 779
};

typedef enum {
  Modifier_kv3_v2_exunum_ALU0=0,
  Modifier_kv3_v2_exunum_ALU1=1,
  Modifier_kv3_v2_exunum_MAU=2,
  Modifier_kv3_v2_exunum_LSU=3,
} Modifier_kv3_v2_exunum_values;


extern const char *mod_kv3_v2_exunum[];
extern const char *mod_kv3_v2_scalarcond[];
extern const char *mod_kv3_v2_lsomask[];
extern const char *mod_kv3_v2_lsumask[];
extern const char *mod_kv3_v2_simdcond[];
extern const char *mod_kv3_v2_comparison[];
extern const char *mod_kv3_v2_floatcomp[];
extern const char *mod_kv3_v2_rounding[];
extern const char *mod_kv3_v2_silent[];
extern const char *mod_kv3_v2_variant[];
extern const char *mod_kv3_v2_speculate[];
extern const char *mod_kv3_v2_doscale[];
extern const char *mod_kv3_v2_qindex[];
extern const char *mod_kv3_v2_hindex[];
extern const char *mod_kv3_v2_cachelev[];
extern const char *mod_kv3_v2_coherency[];
extern const char *mod_kv3_v2_boolcas[];
extern const char *mod_kv3_v2_accesses[];
extern const char *mod_kv3_v2_channel[];
extern const char *mod_kv3_v2_conjugate[];
extern const char *mod_kv3_v2_transpose[];
extern const char *mod_kv3_v2_shuffleV[];
extern const char *mod_kv3_v2_shuffleX[];
extern const char *mod_kv3_v2_splat32[];

typedef enum {
  Bundling_kv3_v2_ALL,
  Bundling_kv3_v2_BCU,
  Bundling_kv3_v2_EXT,
  Bundling_kv3_v2_FULL,
  Bundling_kv3_v2_FULL_X,
  Bundling_kv3_v2_FULL_Y,
  Bundling_kv3_v2_LITE,
  Bundling_kv3_v2_LITE_X,
  Bundling_kv3_v2_LITE_Y,
  Bundling_kv3_v2_MAU,
  Bundling_kv3_v2_MAU_X,
  Bundling_kv3_v2_MAU_Y,
  Bundling_kv3_v2_LSU,
  Bundling_kv3_v2_LSU_X,
  Bundling_kv3_v2_LSU_Y,
  Bundling_kv3_v2_TINY,
  Bundling_kv3_v2_TINY_X,
  Bundling_kv3_v2_TINY_Y,
  Bundling_kv3_v2_NOP,
} Bundling_kv3_v2;

static int ATTRIBUTE_UNUSED
kv3_v2_base_bundling(int bundling) {
  static int base_bundlings[] = {
    Bundling_kv3_v2_ALL,	// Bundling_kv3_v2_ALL
    Bundling_kv3_v2_BCU,	// Bundling_kv3_v2_BCU
    Bundling_kv3_v2_EXT,	// Bundling_kv3_v2_EXT
    Bundling_kv3_v2_FULL,	// Bundling_kv3_v2_FULL
    Bundling_kv3_v2_FULL,	// Bundling_kv3_v2_FULL_X
    Bundling_kv3_v2_FULL,	// Bundling_kv3_v2_FULL_Y
    Bundling_kv3_v2_LITE,	// Bundling_kv3_v2_LITE
    Bundling_kv3_v2_LITE,	// Bundling_kv3_v2_LITE_X
    Bundling_kv3_v2_LITE,	// Bundling_kv3_v2_LITE_Y
    Bundling_kv3_v2_MAU,	// Bundling_kv3_v2_MAU
    Bundling_kv3_v2_MAU,	// Bundling_kv3_v2_MAU_X
    Bundling_kv3_v2_MAU,	// Bundling_kv3_v2_MAU_Y
    Bundling_kv3_v2_LSU,	// Bundling_kv3_v2_LSU
    Bundling_kv3_v2_LSU,	// Bundling_kv3_v2_LSU_X
    Bundling_kv3_v2_LSU,	// Bundling_kv3_v2_LSU_Y
    Bundling_kv3_v2_TINY,	// Bundling_kv3_v2_TINY
    Bundling_kv3_v2_TINY,	// Bundling_kv3_v2_TINY_X
    Bundling_kv3_v2_TINY,	// Bundling_kv3_v2_TINY_Y
    Bundling_kv3_v2_NOP,	// Bundling_kv3_v2_NOP
  };
  return base_bundlings[bundling];
};

typedef enum {
  Resource_kv3_v2_ISSUE,
  Resource_kv3_v2_TINY,
  Resource_kv3_v2_LITE,
  Resource_kv3_v2_FULL,
  Resource_kv3_v2_LSU,
  Resource_kv3_v2_MAU,
  Resource_kv3_v2_BCU,
  Resource_kv3_v2_EXT,
  Resource_kv3_v2_AUXR,
  Resource_kv3_v2_AUXW,
  Resource_kv3_v2_XFER,
  Resource_kv3_v2_MEMW,
  Resource_kv3_v2_SR12,
  Resource_kv3_v2_SR13,
  Resource_kv3_v2_SR14,
  Resource_kv3_v2_SR15,
} Resource_kv3_v2;
#define kv3_v2_RESOURCE_COUNT 16

typedef enum {
  Reservation_kv3_v2_ALL,
  Reservation_kv3_v2_ALU_TINY,
  Reservation_kv3_v2_ALU_TINY_X,
  Reservation_kv3_v2_ALU_TINY_Y,
  Reservation_kv3_v2_ALU_TINY_CRRP,
  Reservation_kv3_v2_ALU_TINY_CRWL_CRWH,
  Reservation_kv3_v2_ALU_TINY_CRWL_CRWH_X,
  Reservation_kv3_v2_ALU_TINY_CRWL_CRWH_Y,
  Reservation_kv3_v2_ALU_TINY_CRRP_CRWL_CRWH,
  Reservation_kv3_v2_ALU_TINY_CRWL,
  Reservation_kv3_v2_ALU_TINY_CRWH,
  Reservation_kv3_v2_ALU_NOP,
  Reservation_kv3_v2_ALU_LITE,
  Reservation_kv3_v2_ALU_LITE_X,
  Reservation_kv3_v2_ALU_LITE_Y,
  Reservation_kv3_v2_ALU_LITE_CRWL,
  Reservation_kv3_v2_ALU_LITE_CRWH,
  Reservation_kv3_v2_ALU_FULL,
  Reservation_kv3_v2_ALU_FULL_X,
  Reservation_kv3_v2_ALU_FULL_Y,
  Reservation_kv3_v2_BCU,
  Reservation_kv3_v2_BCU_XFER,
  Reservation_kv3_v2_BCU_CRRP_CRWL_CRWH,
  Reservation_kv3_v2_BCU_TINY_AUXW_CRRP,
  Reservation_kv3_v2_BCU_TINY_TINY_MAU_XNOP,
  Reservation_kv3_v2_EXT,
  Reservation_kv3_v2_LSU,
  Reservation_kv3_v2_LSU_X,
  Reservation_kv3_v2_LSU_Y,
  Reservation_kv3_v2_LSU_CRRP,
  Reservation_kv3_v2_LSU_CRRP_X,
  Reservation_kv3_v2_LSU_CRRP_Y,
  Reservation_kv3_v2_LSU_AUXR,
  Reservation_kv3_v2_LSU_AUXR_X,
  Reservation_kv3_v2_LSU_AUXR_Y,
  Reservation_kv3_v2_LSU_AUXW,
  Reservation_kv3_v2_LSU_AUXW_X,
  Reservation_kv3_v2_LSU_AUXW_Y,
  Reservation_kv3_v2_LSU_AUXR_AUXW,
  Reservation_kv3_v2_LSU_AUXR_AUXW_X,
  Reservation_kv3_v2_LSU_AUXR_AUXW_Y,
  Reservation_kv3_v2_MAU,
  Reservation_kv3_v2_MAU_X,
  Reservation_kv3_v2_MAU_Y,
  Reservation_kv3_v2_MAU_AUXR,
  Reservation_kv3_v2_MAU_AUXR_X,
  Reservation_kv3_v2_MAU_AUXR_Y,
} Reservation_kv3_v2;

extern struct kvx_reloc kv3_v2_rel16_reloc;
extern struct kvx_reloc kv3_v2_rel32_reloc;
extern struct kvx_reloc kv3_v2_rel64_reloc;
extern struct kvx_reloc kv3_v2_pcrel_signed16_reloc;
extern struct kvx_reloc kv3_v2_pcrel17_reloc;
extern struct kvx_reloc kv3_v2_pcrel27_reloc;
extern struct kvx_reloc kv3_v2_pcrel32_reloc;
extern struct kvx_reloc kv3_v2_pcrel_signed37_reloc;
extern struct kvx_reloc kv3_v2_pcrel_signed43_reloc;
extern struct kvx_reloc kv3_v2_pcrel_signed64_reloc;
extern struct kvx_reloc kv3_v2_pcrel64_reloc;
extern struct kvx_reloc kv3_v2_signed16_reloc;
extern struct kvx_reloc kv3_v2_signed32_reloc;
extern struct kvx_reloc kv3_v2_signed37_reloc;
extern struct kvx_reloc kv3_v2_gotoff_signed37_reloc;
extern struct kvx_reloc kv3_v2_gotoff_signed43_reloc;
extern struct kvx_reloc kv3_v2_gotoff_32_reloc;
extern struct kvx_reloc kv3_v2_gotoff_64_reloc;
extern struct kvx_reloc kv3_v2_got_32_reloc;
extern struct kvx_reloc kv3_v2_got_signed37_reloc;
extern struct kvx_reloc kv3_v2_got_signed43_reloc;
extern struct kvx_reloc kv3_v2_got_64_reloc;
extern struct kvx_reloc kv3_v2_glob_dat_reloc;
extern struct kvx_reloc kv3_v2_copy_reloc;
extern struct kvx_reloc kv3_v2_jump_slot_reloc;
extern struct kvx_reloc kv3_v2_relative_reloc;
extern struct kvx_reloc kv3_v2_signed43_reloc;
extern struct kvx_reloc kv3_v2_signed64_reloc;
extern struct kvx_reloc kv3_v2_gotaddr_signed37_reloc;
extern struct kvx_reloc kv3_v2_gotaddr_signed43_reloc;
extern struct kvx_reloc kv3_v2_gotaddr_signed64_reloc;
extern struct kvx_reloc kv3_v2_dtpmod64_reloc;
extern struct kvx_reloc kv3_v2_dtpoff64_reloc;
extern struct kvx_reloc kv3_v2_dtpoff_signed37_reloc;
extern struct kvx_reloc kv3_v2_dtpoff_signed43_reloc;
extern struct kvx_reloc kv3_v2_tlsgd_signed37_reloc;
extern struct kvx_reloc kv3_v2_tlsgd_signed43_reloc;
extern struct kvx_reloc kv3_v2_tlsld_signed37_reloc;
extern struct kvx_reloc kv3_v2_tlsld_signed43_reloc;
extern struct kvx_reloc kv3_v2_tpoff64_reloc;
extern struct kvx_reloc kv3_v2_tlsie_signed37_reloc;
extern struct kvx_reloc kv3_v2_tlsie_signed43_reloc;
extern struct kvx_reloc kv3_v2_tlsle_signed37_reloc;
extern struct kvx_reloc kv3_v2_tlsle_signed43_reloc;
extern struct kvx_reloc kv3_v2_rel8_reloc;
extern struct kvx_reloc kv3_v2_pcrel11_reloc;
extern struct kvx_reloc kv3_v2_pcrel38_reloc;

#define KV4_V1_REGFILE_FIRST_SFR KVX_REGFILE_FIRST_SFR
#define KV4_V1_REGFILE_LAST_SFR KVX_REGFILE_LAST_SFR
#define KV4_V1_REGFILE_DEC_SFR KVX_REGFILE_DEC_SFR
#define KV4_V1_REGFILE_FIRST_GPR KVX_REGFILE_FIRST_GPR
#define KV4_V1_REGFILE_LAST_GPR KVX_REGFILE_LAST_GPR
#define KV4_V1_REGFILE_DEC_GPR KVX_REGFILE_DEC_GPR
#define KV4_V1_REGFILE_FIRST_CSR 6
#define KV4_V1_REGFILE_LAST_CSR 7
#define KV4_V1_REGFILE_DEC_CSR 8
#define KV4_V1_REGFILE_FIRST_PGR 9
#define KV4_V1_REGFILE_LAST_PGR 10
#define KV4_V1_REGFILE_DEC_PGR 11
#define KV4_V1_REGFILE_FIRST_QGR 12
#define KV4_V1_REGFILE_LAST_QGR 13
#define KV4_V1_REGFILE_DEC_QGR 14
#define KV4_V1_REGFILE_FIRST_RV_BIR 15
#define KV4_V1_REGFILE_LAST_RV_BIR 16
#define KV4_V1_REGFILE_DEC_RV_BIR 17
#define KV4_V1_REGFILE_FIRST_RV_BIRP 18
#define KV4_V1_REGFILE_LAST_RV_BIRP 19
#define KV4_V1_REGFILE_DEC_RV_BIRP 20
#define KV4_V1_REGFILE_FIRST_RV_FPR 21
#define KV4_V1_REGFILE_LAST_RV_FPR 22
#define KV4_V1_REGFILE_DEC_RV_FPR 23
#define KV4_V1_REGFILE_FIRST_X16R 24
#define KV4_V1_REGFILE_LAST_X16R 25
#define KV4_V1_REGFILE_DEC_X16R 26
#define KV4_V1_REGFILE_FIRST_X2R 27
#define KV4_V1_REGFILE_LAST_X2R 28
#define KV4_V1_REGFILE_DEC_X2R 29
#define KV4_V1_REGFILE_FIRST_X32R 30
#define KV4_V1_REGFILE_LAST_X32R 31
#define KV4_V1_REGFILE_DEC_X32R 32
#define KV4_V1_REGFILE_FIRST_X4R 33
#define KV4_V1_REGFILE_LAST_X4R 34
#define KV4_V1_REGFILE_DEC_X4R 35
#define KV4_V1_REGFILE_FIRST_X64R 36
#define KV4_V1_REGFILE_LAST_X64R 37
#define KV4_V1_REGFILE_DEC_X64R 38
#define KV4_V1_REGFILE_FIRST_X8R 39
#define KV4_V1_REGFILE_LAST_X8R 40
#define KV4_V1_REGFILE_DEC_X8R 41
#define KV4_V1_REGFILE_FIRST_XBR 42
#define KV4_V1_REGFILE_LAST_XBR 43
#define KV4_V1_REGFILE_DEC_XBR 44
#define KV4_V1_REGFILE_FIRST_XCR 45
#define KV4_V1_REGFILE_LAST_XCR 46
#define KV4_V1_REGFILE_DEC_XCR 47
#define KV4_V1_REGFILE_FIRST_XMR 48
#define KV4_V1_REGFILE_LAST_XMR 49
#define KV4_V1_REGFILE_DEC_XMR 50
#define KV4_V1_REGFILE_FIRST_XTR 51
#define KV4_V1_REGFILE_LAST_XTR 52
#define KV4_V1_REGFILE_DEC_XTR 53
#define KV4_V1_REGFILE_FIRST_XVR 54
#define KV4_V1_REGFILE_LAST_XVR 55
#define KV4_V1_REGFILE_DEC_XVR 56
#define KV4_V1_REGFILE_REGISTERS 57
#define KV4_V1_REGFILE_DEC_REGISTERS 58


extern int kv4_v1_regfiles[];
extern const char **kv4_v1_modifiers[];
extern struct kvx_register kv4_v1_registers[];

extern int kv4_v1_dec_registers[];

enum Method_kv4_v1_enum {
  Immediate_kv4_v1_brknumber = 1,
  Immediate_kv4_v1_pcrel11 = 2,
  Immediate_kv4_v1_pcrel12 = 3,
  Immediate_kv4_v1_pcrel17 = 4,
  Immediate_kv4_v1_pcrel20 = 5,
  Immediate_kv4_v1_pcrel27 = 6,
  Immediate_kv4_v1_pcrel38 = 7,
  Immediate_kv4_v1_signed10 = 8,
  Immediate_kv4_v1_signed12 = 9,
  Immediate_kv4_v1_signed16 = 10,
  Immediate_kv4_v1_signed20 = 11,
  Immediate_kv4_v1_signed27 = 12,
  Immediate_kv4_v1_signed37 = 13,
  Immediate_kv4_v1_signed43 = 14,
  Immediate_kv4_v1_signed54 = 15,
  Immediate_kv4_v1_signed6 = 16,
  Immediate_kv4_v1_sysnumber = 17,
  Immediate_kv4_v1_unsigned5 = 18,
  Immediate_kv4_v1_unsigned6 = 19,
  Immediate_kv4_v1_wrapped32 = 20,
  Immediate_kv4_v1_wrapped64 = 21,
  Immediate_kv4_v1_wrapped8 = 22,
  Modifier_kv4_v1_accesses = 23,
  Modifier_kv4_v1_acqrel = 24,
  Modifier_kv4_v1_bcucond = 25,
  Modifier_kv4_v1_boolcas = 26,
  Modifier_kv4_v1_cachelev = 27,
  Modifier_kv4_v1_ccbcomp = 28,
  Modifier_kv4_v1_channel = 29,
  Modifier_kv4_v1_coherency = 30,
  Modifier_kv4_v1_conjugate = 31,
  Modifier_kv4_v1_doscale = 32,
  Modifier_kv4_v1_exunum = 33,
  Modifier_kv4_v1_floatcomp = 34,
  Modifier_kv4_v1_floatmode = 35,
  Modifier_kv4_v1_froundmode = 36,
  Modifier_kv4_v1_hindex = 37,
  Modifier_kv4_v1_intcomp = 38,
  Modifier_kv4_v1_lanecond = 39,
  Modifier_kv4_v1_lanesize = 40,
  Modifier_kv4_v1_lanetodo = 41,
  Modifier_kv4_v1_ordering = 42,
  Modifier_kv4_v1_qindex = 43,
  Modifier_kv4_v1_realimag = 44,
  Modifier_kv4_v1_shuffleV = 45,
  Modifier_kv4_v1_shuffleX = 46,
  Modifier_kv4_v1_signextw = 47,
  Modifier_kv4_v1_speculate = 48,
  Modifier_kv4_v1_splat32 = 49,
  Modifier_kv4_v1_variant = 50,
  RegClass_kv4_v1_aloneReg = 51,
  RegClass_kv4_v1_buffer16Reg = 52,
  RegClass_kv4_v1_buffer2Reg = 53,
  RegClass_kv4_v1_buffer32Reg = 54,
  RegClass_kv4_v1_buffer4Reg = 55,
  RegClass_kv4_v1_buffer64Reg = 56,
  RegClass_kv4_v1_buffer8Reg = 57,
  RegClass_kv4_v1_csReg = 58,
  RegClass_kv4_v1_floatReg = 59,
  RegClass_kv4_v1_mainReg = 60,
  RegClass_kv4_v1_mainRegPair = 61,
  RegClass_kv4_v1_onlyfxReg = 62,
  RegClass_kv4_v1_onlygetReg = 63,
  RegClass_kv4_v1_onlyraReg = 64,
  RegClass_kv4_v1_onlysetReg = 65,
  RegClass_kv4_v1_onlyswapReg = 66,
  RegClass_kv4_v1_pairedReg = 67,
  RegClass_kv4_v1_quadReg = 68,
  RegClass_kv4_v1_singleReg = 69,
  RegClass_kv4_v1_systemReg = 70,
  RegClass_kv4_v1_xworddReg = 71,
  RegClass_kv4_v1_xworddReg0M4 = 72,
  RegClass_kv4_v1_xworddReg1M4 = 73,
  RegClass_kv4_v1_xworddReg2M4 = 74,
  RegClass_kv4_v1_xworddReg3M4 = 75,
  RegClass_kv4_v1_xwordoReg = 76,
  RegClass_kv4_v1_xwordqReg = 77,
  RegClass_kv4_v1_xwordqRegE = 78,
  RegClass_kv4_v1_xwordqRegO = 79,
  RegClass_kv4_v1_xwordvReg = 80,
  RegClass_kv4_v1_xwordxReg = 81,
  Instruction_kv4_v1_abdbo = 82,
  Instruction_kv4_v1_abdd = 83,
  Instruction_kv4_v1_abdhq = 84,
  Instruction_kv4_v1_abdsbo = 85,
  Instruction_kv4_v1_abdsd = 86,
  Instruction_kv4_v1_abdshq = 87,
  Instruction_kv4_v1_abdsw = 88,
  Instruction_kv4_v1_abdswp = 89,
  Instruction_kv4_v1_abdubo = 90,
  Instruction_kv4_v1_abdud = 91,
  Instruction_kv4_v1_abduhq = 92,
  Instruction_kv4_v1_abduw = 93,
  Instruction_kv4_v1_abduwp = 94,
  Instruction_kv4_v1_abdw = 95,
  Instruction_kv4_v1_abdwp = 96,
  Instruction_kv4_v1_absbo = 97,
  Instruction_kv4_v1_absd = 98,
  Instruction_kv4_v1_abshq = 99,
  Instruction_kv4_v1_abssbo = 100,
  Instruction_kv4_v1_abssd = 101,
  Instruction_kv4_v1_absshq = 102,
  Instruction_kv4_v1_abssw = 103,
  Instruction_kv4_v1_absswp = 104,
  Instruction_kv4_v1_absw = 105,
  Instruction_kv4_v1_abswp = 106,
  Instruction_kv4_v1_acswapb = 107,
  Instruction_kv4_v1_acswapd = 108,
  Instruction_kv4_v1_acswaph = 109,
  Instruction_kv4_v1_acswapq = 110,
  Instruction_kv4_v1_acswapw = 111,
  Instruction_kv4_v1_add = 112,
  Instruction_kv4_v1_add_uw = 113,
  Instruction_kv4_v1_addbo = 114,
  Instruction_kv4_v1_addcd = 115,
  Instruction_kv4_v1_addcd_i = 116,
  Instruction_kv4_v1_addd = 117,
  Instruction_kv4_v1_addhq = 118,
  Instruction_kv4_v1_addi = 119,
  Instruction_kv4_v1_addiw = 120,
  Instruction_kv4_v1_addrbod = 121,
  Instruction_kv4_v1_addrhqd = 122,
  Instruction_kv4_v1_addrwpd = 123,
  Instruction_kv4_v1_addsbo = 124,
  Instruction_kv4_v1_addsd = 125,
  Instruction_kv4_v1_addshq = 126,
  Instruction_kv4_v1_addsw = 127,
  Instruction_kv4_v1_addswp = 128,
  Instruction_kv4_v1_addurbod = 129,
  Instruction_kv4_v1_addurhqd = 130,
  Instruction_kv4_v1_addurwpd = 131,
  Instruction_kv4_v1_addusbo = 132,
  Instruction_kv4_v1_addusd = 133,
  Instruction_kv4_v1_addushq = 134,
  Instruction_kv4_v1_addusw = 135,
  Instruction_kv4_v1_adduswp = 136,
  Instruction_kv4_v1_addw = 137,
  Instruction_kv4_v1_addwp = 138,
  Instruction_kv4_v1_addx16bo = 139,
  Instruction_kv4_v1_addx16d = 140,
  Instruction_kv4_v1_addx16hq = 141,
  Instruction_kv4_v1_addx16w = 142,
  Instruction_kv4_v1_addx16wp = 143,
  Instruction_kv4_v1_addx2bo = 144,
  Instruction_kv4_v1_addx2d = 145,
  Instruction_kv4_v1_addx2hq = 146,
  Instruction_kv4_v1_addx2w = 147,
  Instruction_kv4_v1_addx2wp = 148,
  Instruction_kv4_v1_addx32d = 149,
  Instruction_kv4_v1_addx32w = 150,
  Instruction_kv4_v1_addx4bo = 151,
  Instruction_kv4_v1_addx4d = 152,
  Instruction_kv4_v1_addx4hq = 153,
  Instruction_kv4_v1_addx4w = 154,
  Instruction_kv4_v1_addx4wp = 155,
  Instruction_kv4_v1_addx64d = 156,
  Instruction_kv4_v1_addx64w = 157,
  Instruction_kv4_v1_addx8bo = 158,
  Instruction_kv4_v1_addx8d = 159,
  Instruction_kv4_v1_addx8hq = 160,
  Instruction_kv4_v1_addx8w = 161,
  Instruction_kv4_v1_addx8wp = 162,
  Instruction_kv4_v1_aladdb = 163,
  Instruction_kv4_v1_aladdd = 164,
  Instruction_kv4_v1_aladdh = 165,
  Instruction_kv4_v1_aladdw = 166,
  Instruction_kv4_v1_alandb = 167,
  Instruction_kv4_v1_alandd = 168,
  Instruction_kv4_v1_alandh = 169,
  Instruction_kv4_v1_alandw = 170,
  Instruction_kv4_v1_alb = 171,
  Instruction_kv4_v1_alclrb = 172,
  Instruction_kv4_v1_alclrd = 173,
  Instruction_kv4_v1_alclrh = 174,
  Instruction_kv4_v1_alclrw = 175,
  Instruction_kv4_v1_ald = 176,
  Instruction_kv4_v1_aldusb = 177,
  Instruction_kv4_v1_aldusd = 178,
  Instruction_kv4_v1_aldush = 179,
  Instruction_kv4_v1_aldusw = 180,
  Instruction_kv4_v1_aleorb = 181,
  Instruction_kv4_v1_aleord = 182,
  Instruction_kv4_v1_aleorh = 183,
  Instruction_kv4_v1_aleorw = 184,
  Instruction_kv4_v1_alh = 185,
  Instruction_kv4_v1_aliorb = 186,
  Instruction_kv4_v1_aliord = 187,
  Instruction_kv4_v1_aliorh = 188,
  Instruction_kv4_v1_aliorw = 189,
  Instruction_kv4_v1_almaxb = 190,
  Instruction_kv4_v1_almaxd = 191,
  Instruction_kv4_v1_almaxh = 192,
  Instruction_kv4_v1_almaxw = 193,
  Instruction_kv4_v1_alminb = 194,
  Instruction_kv4_v1_almind = 195,
  Instruction_kv4_v1_alminh = 196,
  Instruction_kv4_v1_alminw = 197,
  Instruction_kv4_v1_alw = 198,
  Instruction_kv4_v1_amoadd_b = 199,
  Instruction_kv4_v1_amoadd_d = 200,
  Instruction_kv4_v1_amoadd_h = 201,
  Instruction_kv4_v1_amoadd_w = 202,
  Instruction_kv4_v1_amoand_b = 203,
  Instruction_kv4_v1_amoand_d = 204,
  Instruction_kv4_v1_amoand_h = 205,
  Instruction_kv4_v1_amoand_w = 206,
  Instruction_kv4_v1_amocas_b = 207,
  Instruction_kv4_v1_amocas_d = 208,
  Instruction_kv4_v1_amocas_h = 209,
  Instruction_kv4_v1_amocas_q = 210,
  Instruction_kv4_v1_amocas_w = 211,
  Instruction_kv4_v1_amomax_b = 212,
  Instruction_kv4_v1_amomax_d = 213,
  Instruction_kv4_v1_amomax_h = 214,
  Instruction_kv4_v1_amomax_w = 215,
  Instruction_kv4_v1_amomaxu_b = 216,
  Instruction_kv4_v1_amomaxu_d = 217,
  Instruction_kv4_v1_amomaxu_h = 218,
  Instruction_kv4_v1_amomaxu_w = 219,
  Instruction_kv4_v1_amomin_b = 220,
  Instruction_kv4_v1_amomin_d = 221,
  Instruction_kv4_v1_amomin_h = 222,
  Instruction_kv4_v1_amomin_w = 223,
  Instruction_kv4_v1_amominu_b = 224,
  Instruction_kv4_v1_amominu_d = 225,
  Instruction_kv4_v1_amominu_h = 226,
  Instruction_kv4_v1_amominu_w = 227,
  Instruction_kv4_v1_amoor_b = 228,
  Instruction_kv4_v1_amoor_d = 229,
  Instruction_kv4_v1_amoor_h = 230,
  Instruction_kv4_v1_amoor_w = 231,
  Instruction_kv4_v1_amoswap_b = 232,
  Instruction_kv4_v1_amoswap_d = 233,
  Instruction_kv4_v1_amoswap_h = 234,
  Instruction_kv4_v1_amoswap_w = 235,
  Instruction_kv4_v1_amoxor_b = 236,
  Instruction_kv4_v1_amoxor_d = 237,
  Instruction_kv4_v1_amoxor_h = 238,
  Instruction_kv4_v1_amoxor_w = 239,
  Instruction_kv4_v1_and = 240,
  Instruction_kv4_v1_andd = 241,
  Instruction_kv4_v1_andi = 242,
  Instruction_kv4_v1_andn = 243,
  Instruction_kv4_v1_andnd = 244,
  Instruction_kv4_v1_andnw = 245,
  Instruction_kv4_v1_andrbod = 246,
  Instruction_kv4_v1_andrhqd = 247,
  Instruction_kv4_v1_andrwpd = 248,
  Instruction_kv4_v1_andw = 249,
  Instruction_kv4_v1_asaddb = 250,
  Instruction_kv4_v1_asaddd = 251,
  Instruction_kv4_v1_asaddh = 252,
  Instruction_kv4_v1_asaddw = 253,
  Instruction_kv4_v1_asandb = 254,
  Instruction_kv4_v1_asandd = 255,
  Instruction_kv4_v1_asandh = 256,
  Instruction_kv4_v1_asandw = 257,
  Instruction_kv4_v1_asb = 258,
  Instruction_kv4_v1_asd = 259,
  Instruction_kv4_v1_asdusb = 260,
  Instruction_kv4_v1_asdusd = 261,
  Instruction_kv4_v1_asdush = 262,
  Instruction_kv4_v1_asdusw = 263,
  Instruction_kv4_v1_aseorb = 264,
  Instruction_kv4_v1_aseord = 265,
  Instruction_kv4_v1_aseorh = 266,
  Instruction_kv4_v1_aseorw = 267,
  Instruction_kv4_v1_ash = 268,
  Instruction_kv4_v1_asiorb = 269,
  Instruction_kv4_v1_asiord = 270,
  Instruction_kv4_v1_asiorh = 271,
  Instruction_kv4_v1_asiorw = 272,
  Instruction_kv4_v1_asmaxb = 273,
  Instruction_kv4_v1_asmaxd = 274,
  Instruction_kv4_v1_asmaxh = 275,
  Instruction_kv4_v1_asmaxw = 276,
  Instruction_kv4_v1_asminb = 277,
  Instruction_kv4_v1_asmind = 278,
  Instruction_kv4_v1_asminh = 279,
  Instruction_kv4_v1_asminw = 280,
  Instruction_kv4_v1_asw = 281,
  Instruction_kv4_v1_aswapb = 282,
  Instruction_kv4_v1_aswapd = 283,
  Instruction_kv4_v1_aswaph = 284,
  Instruction_kv4_v1_aswapw = 285,
  Instruction_kv4_v1_auipc = 286,
  Instruction_kv4_v1_avgbo = 287,
  Instruction_kv4_v1_avghq = 288,
  Instruction_kv4_v1_avgrbo = 289,
  Instruction_kv4_v1_avgrhq = 290,
  Instruction_kv4_v1_avgrubo = 291,
  Instruction_kv4_v1_avgruhq = 292,
  Instruction_kv4_v1_avgruw = 293,
  Instruction_kv4_v1_avgruwp = 294,
  Instruction_kv4_v1_avgrw = 295,
  Instruction_kv4_v1_avgrwp = 296,
  Instruction_kv4_v1_avgubo = 297,
  Instruction_kv4_v1_avguhq = 298,
  Instruction_kv4_v1_avguw = 299,
  Instruction_kv4_v1_avguwp = 300,
  Instruction_kv4_v1_avgw = 301,
  Instruction_kv4_v1_avgwp = 302,
  Instruction_kv4_v1_await = 303,
  Instruction_kv4_v1_barrier = 304,
  Instruction_kv4_v1_bclr = 305,
  Instruction_kv4_v1_bclri = 306,
  Instruction_kv4_v1_beq = 307,
  Instruction_kv4_v1_beqz = 308,
  Instruction_kv4_v1_bext = 309,
  Instruction_kv4_v1_bexti = 310,
  Instruction_kv4_v1_bge = 311,
  Instruction_kv4_v1_bgeu = 312,
  Instruction_kv4_v1_bgez = 313,
  Instruction_kv4_v1_bgtz = 314,
  Instruction_kv4_v1_binv = 315,
  Instruction_kv4_v1_binvi = 316,
  Instruction_kv4_v1_blend = 317,
  Instruction_kv4_v1_blez = 318,
  Instruction_kv4_v1_blt = 319,
  Instruction_kv4_v1_bltu = 320,
  Instruction_kv4_v1_bltz = 321,
  Instruction_kv4_v1_bne = 322,
  Instruction_kv4_v1_bnez = 323,
  Instruction_kv4_v1_break = 324,
  Instruction_kv4_v1_bset = 325,
  Instruction_kv4_v1_bseti = 326,
  Instruction_kv4_v1_call = 327,
  Instruction_kv4_v1_cb = 328,
  Instruction_kv4_v1_cbo_clean = 329,
  Instruction_kv4_v1_cbo_flush = 330,
  Instruction_kv4_v1_cbo_inval = 331,
  Instruction_kv4_v1_cbo_zero = 332,
  Instruction_kv4_v1_cbsd = 333,
  Instruction_kv4_v1_cbsw = 334,
  Instruction_kv4_v1_cbswp = 335,
  Instruction_kv4_v1_ccb = 336,
  Instruction_kv4_v1_clmul = 337,
  Instruction_kv4_v1_clmulh = 338,
  Instruction_kv4_v1_clmulr = 339,
  Instruction_kv4_v1_clsd = 340,
  Instruction_kv4_v1_clsw = 341,
  Instruction_kv4_v1_clswp = 342,
  Instruction_kv4_v1_clz = 343,
  Instruction_kv4_v1_clzd = 344,
  Instruction_kv4_v1_clzw = 345,
  Instruction_kv4_v1_clzwp = 346,
  Instruction_kv4_v1_cmovebo = 347,
  Instruction_kv4_v1_cmoved = 348,
  Instruction_kv4_v1_cmovehq = 349,
  Instruction_kv4_v1_cmovewp = 350,
  Instruction_kv4_v1_cmuldt = 351,
  Instruction_kv4_v1_compd = 352,
  Instruction_kv4_v1_compnbo = 353,
  Instruction_kv4_v1_compnd = 354,
  Instruction_kv4_v1_compnhq = 355,
  Instruction_kv4_v1_compnw = 356,
  Instruction_kv4_v1_compnwp = 357,
  Instruction_kv4_v1_compw = 358,
  Instruction_kv4_v1_copyd = 359,
  Instruction_kv4_v1_copyo = 360,
  Instruction_kv4_v1_copyq = 361,
  Instruction_kv4_v1_copyw = 362,
  Instruction_kv4_v1_cpop = 363,
  Instruction_kv4_v1_cpopw = 364,
  Instruction_kv4_v1_crcbellw = 365,
  Instruction_kv4_v1_crcbelmw = 366,
  Instruction_kv4_v1_crclellw = 367,
  Instruction_kv4_v1_crclelmw = 368,
  Instruction_kv4_v1_csrr = 369,
  Instruction_kv4_v1_csrrc = 370,
  Instruction_kv4_v1_csrrci = 371,
  Instruction_kv4_v1_csrrs = 372,
  Instruction_kv4_v1_csrrsi = 373,
  Instruction_kv4_v1_csrrw = 374,
  Instruction_kv4_v1_csrrwi = 375,
  Instruction_kv4_v1_csrw = 376,
  Instruction_kv4_v1_ctz = 377,
  Instruction_kv4_v1_ctzd = 378,
  Instruction_kv4_v1_ctzw = 379,
  Instruction_kv4_v1_ctzwp = 380,
  Instruction_kv4_v1_czero_eqz = 381,
  Instruction_kv4_v1_czero_nez = 382,
  Instruction_kv4_v1_d1inval = 383,
  Instruction_kv4_v1_dflushl = 384,
  Instruction_kv4_v1_dflushsw = 385,
  Instruction_kv4_v1_dinvall = 386,
  Instruction_kv4_v1_dinvalsw = 387,
  Instruction_kv4_v1_div = 388,
  Instruction_kv4_v1_divu = 389,
  Instruction_kv4_v1_divuw = 390,
  Instruction_kv4_v1_divw = 391,
  Instruction_kv4_v1_dpurgel = 392,
  Instruction_kv4_v1_dpurgesw = 393,
  Instruction_kv4_v1_dtouchl = 394,
  Instruction_kv4_v1_ebreak = 395,
  Instruction_kv4_v1_ecall = 396,
  Instruction_kv4_v1_eord = 397,
  Instruction_kv4_v1_eorrbod = 398,
  Instruction_kv4_v1_eorrhqd = 399,
  Instruction_kv4_v1_eorrwpd = 400,
  Instruction_kv4_v1_eorw = 401,
  Instruction_kv4_v1_errop = 402,
  Instruction_kv4_v1_extfs = 403,
  Instruction_kv4_v1_extfz = 404,
  Instruction_kv4_v1_fabsd = 405,
  Instruction_kv4_v1_fabshq = 406,
  Instruction_kv4_v1_fabsw = 407,
  Instruction_kv4_v1_fabswp = 408,
  Instruction_kv4_v1_fadd_d = 409,
  Instruction_kv4_v1_fadd_s = 410,
  Instruction_kv4_v1_faddd = 411,
  Instruction_kv4_v1_faddhq = 412,
  Instruction_kv4_v1_faddw = 413,
  Instruction_kv4_v1_faddwc = 414,
  Instruction_kv4_v1_faddwp = 415,
  Instruction_kv4_v1_fclass_d = 416,
  Instruction_kv4_v1_fclass_s = 417,
  Instruction_kv4_v1_fcompd = 418,
  Instruction_kv4_v1_fcompnd = 419,
  Instruction_kv4_v1_fcompnhq = 420,
  Instruction_kv4_v1_fcompnw = 421,
  Instruction_kv4_v1_fcompnwp = 422,
  Instruction_kv4_v1_fcompw = 423,
  Instruction_kv4_v1_fcvt_d_l = 424,
  Instruction_kv4_v1_fcvt_d_lu = 425,
  Instruction_kv4_v1_fcvt_d_s = 426,
  Instruction_kv4_v1_fcvt_d_w = 427,
  Instruction_kv4_v1_fcvt_d_wu = 428,
  Instruction_kv4_v1_fcvt_l_d = 429,
  Instruction_kv4_v1_fcvt_l_s = 430,
  Instruction_kv4_v1_fcvt_lu_d = 431,
  Instruction_kv4_v1_fcvt_lu_s = 432,
  Instruction_kv4_v1_fcvt_s_d = 433,
  Instruction_kv4_v1_fcvt_s_l = 434,
  Instruction_kv4_v1_fcvt_s_lu = 435,
  Instruction_kv4_v1_fcvt_s_w = 436,
  Instruction_kv4_v1_fcvt_s_wu = 437,
  Instruction_kv4_v1_fcvt_w_d = 438,
  Instruction_kv4_v1_fcvt_w_s = 439,
  Instruction_kv4_v1_fcvt_wu_d = 440,
  Instruction_kv4_v1_fcvt_wu_s = 441,
  Instruction_kv4_v1_fdiv_d = 442,
  Instruction_kv4_v1_fdiv_s = 443,
  Instruction_kv4_v1_fence = 444,
  Instruction_kv4_v1_fence_i = 445,
  Instruction_kv4_v1_fence_mem = 446,
  Instruction_kv4_v1_feq_d = 447,
  Instruction_kv4_v1_feq_s = 448,
  Instruction_kv4_v1_ffmad = 449,
  Instruction_kv4_v1_ffmahq = 450,
  Instruction_kv4_v1_ffmaw = 451,
  Instruction_kv4_v1_ffmawc = 452,
  Instruction_kv4_v1_ffmawp = 453,
  Instruction_kv4_v1_ffmsd = 454,
  Instruction_kv4_v1_ffmshq = 455,
  Instruction_kv4_v1_ffmsw = 456,
  Instruction_kv4_v1_ffmswc = 457,
  Instruction_kv4_v1_ffmswp = 458,
  Instruction_kv4_v1_fixedd = 459,
  Instruction_kv4_v1_fixedud = 460,
  Instruction_kv4_v1_fixeduw = 461,
  Instruction_kv4_v1_fixeduwp = 462,
  Instruction_kv4_v1_fixedw = 463,
  Instruction_kv4_v1_fixedwp = 464,
  Instruction_kv4_v1_fld = 465,
  Instruction_kv4_v1_fle_d = 466,
  Instruction_kv4_v1_fle_s = 467,
  Instruction_kv4_v1_floatd = 468,
  Instruction_kv4_v1_floatud = 469,
  Instruction_kv4_v1_floatuw = 470,
  Instruction_kv4_v1_floatuwp = 471,
  Instruction_kv4_v1_floatw = 472,
  Instruction_kv4_v1_floatwp = 473,
  Instruction_kv4_v1_flt_d = 474,
  Instruction_kv4_v1_flt_s = 475,
  Instruction_kv4_v1_flw = 476,
  Instruction_kv4_v1_fmadd_d = 477,
  Instruction_kv4_v1_fmadd_s = 478,
  Instruction_kv4_v1_fmax_d = 479,
  Instruction_kv4_v1_fmax_s = 480,
  Instruction_kv4_v1_fmaxd = 481,
  Instruction_kv4_v1_fmaxhq = 482,
  Instruction_kv4_v1_fmaxw = 483,
  Instruction_kv4_v1_fmaxwp = 484,
  Instruction_kv4_v1_fmin_d = 485,
  Instruction_kv4_v1_fmin_s = 486,
  Instruction_kv4_v1_fmind = 487,
  Instruction_kv4_v1_fminhq = 488,
  Instruction_kv4_v1_fminw = 489,
  Instruction_kv4_v1_fminwp = 490,
  Instruction_kv4_v1_fmsub_d = 491,
  Instruction_kv4_v1_fmsub_s = 492,
  Instruction_kv4_v1_fmul_d = 493,
  Instruction_kv4_v1_fmul_s = 494,
  Instruction_kv4_v1_fmuld = 495,
  Instruction_kv4_v1_fmulhq = 496,
  Instruction_kv4_v1_fmulw = 497,
  Instruction_kv4_v1_fmulwc = 498,
  Instruction_kv4_v1_fmulwp = 499,
  Instruction_kv4_v1_fmv_d_x = 500,
  Instruction_kv4_v1_fmv_w_x = 501,
  Instruction_kv4_v1_fmv_x_d = 502,
  Instruction_kv4_v1_fmv_x_w = 503,
  Instruction_kv4_v1_fnarrowdw = 504,
  Instruction_kv4_v1_fnarrowdwp = 505,
  Instruction_kv4_v1_fnarrowwh = 506,
  Instruction_kv4_v1_fnarrowwhq = 507,
  Instruction_kv4_v1_fnegd = 508,
  Instruction_kv4_v1_fneghq = 509,
  Instruction_kv4_v1_fnegw = 510,
  Instruction_kv4_v1_fnegwp = 511,
  Instruction_kv4_v1_fnmadd_d = 512,
  Instruction_kv4_v1_fnmadd_s = 513,
  Instruction_kv4_v1_fnmsub_d = 514,
  Instruction_kv4_v1_fnmsub_s = 515,
  Instruction_kv4_v1_frecw = 516,
  Instruction_kv4_v1_frsrw = 517,
  Instruction_kv4_v1_fsbfd = 518,
  Instruction_kv4_v1_fsbfhq = 519,
  Instruction_kv4_v1_fsbfw = 520,
  Instruction_kv4_v1_fsbfwc = 521,
  Instruction_kv4_v1_fsbfwp = 522,
  Instruction_kv4_v1_fsd = 523,
  Instruction_kv4_v1_fsgnj_d = 524,
  Instruction_kv4_v1_fsgnj_s = 525,
  Instruction_kv4_v1_fsgnjn_d = 526,
  Instruction_kv4_v1_fsgnjn_s = 527,
  Instruction_kv4_v1_fsgnjx_d = 528,
  Instruction_kv4_v1_fsgnjx_s = 529,
  Instruction_kv4_v1_fsqrt_d = 530,
  Instruction_kv4_v1_fsqrt_s = 531,
  Instruction_kv4_v1_fsrecd = 532,
  Instruction_kv4_v1_fsrecw = 533,
  Instruction_kv4_v1_fsrecwp = 534,
  Instruction_kv4_v1_fsrsrd = 535,
  Instruction_kv4_v1_fsrsrw = 536,
  Instruction_kv4_v1_fsrsrwp = 537,
  Instruction_kv4_v1_fsub_d = 538,
  Instruction_kv4_v1_fsub_s = 539,
  Instruction_kv4_v1_fsw = 540,
  Instruction_kv4_v1_fwidenlhw = 541,
  Instruction_kv4_v1_fwidenlhwp = 542,
  Instruction_kv4_v1_fwidenlwd = 543,
  Instruction_kv4_v1_fwidenmhw = 544,
  Instruction_kv4_v1_fwidenmhwp = 545,
  Instruction_kv4_v1_fwidenmwd = 546,
  Instruction_kv4_v1_get = 547,
  Instruction_kv4_v1_goto = 548,
  Instruction_kv4_v1_guard = 549,
  Instruction_kv4_v1_i1inval = 550,
  Instruction_kv4_v1_i1invals = 551,
  Instruction_kv4_v1_icall = 552,
  Instruction_kv4_v1_iget = 553,
  Instruction_kv4_v1_igoto = 554,
  Instruction_kv4_v1_insf = 555,
  Instruction_kv4_v1_iord = 556,
  Instruction_kv4_v1_iornd = 557,
  Instruction_kv4_v1_iornw = 558,
  Instruction_kv4_v1_iorrbod = 559,
  Instruction_kv4_v1_iorrhqd = 560,
  Instruction_kv4_v1_iorrwpd = 561,
  Instruction_kv4_v1_iorw = 562,
  Instruction_kv4_v1_j = 563,
  Instruction_kv4_v1_jal = 564,
  Instruction_kv4_v1_jalr = 565,
  Instruction_kv4_v1_jr = 566,
  Instruction_kv4_v1_kv_lq = 567,
  Instruction_kv4_v1_kv_sq = 568,
  Instruction_kv4_v1_landd = 569,
  Instruction_kv4_v1_landw = 570,
  Instruction_kv4_v1_lb = 571,
  Instruction_kv4_v1_lbs = 572,
  Instruction_kv4_v1_lbu = 573,
  Instruction_kv4_v1_lbz = 574,
  Instruction_kv4_v1_ld = 575,
  Instruction_kv4_v1_lh = 576,
  Instruction_kv4_v1_lhs = 577,
  Instruction_kv4_v1_lhu = 578,
  Instruction_kv4_v1_lhz = 579,
  Instruction_kv4_v1_li = 580,
  Instruction_kv4_v1_liord = 581,
  Instruction_kv4_v1_liorw = 582,
  Instruction_kv4_v1_lnandd = 583,
  Instruction_kv4_v1_lnandw = 584,
  Instruction_kv4_v1_lniord = 585,
  Instruction_kv4_v1_lniorw = 586,
  Instruction_kv4_v1_lo = 587,
  Instruction_kv4_v1_loopdo = 588,
  Instruction_kv4_v1_lq = 589,
  Instruction_kv4_v1_lr_d = 590,
  Instruction_kv4_v1_lr_w = 591,
  Instruction_kv4_v1_lui = 592,
  Instruction_kv4_v1_lw = 593,
  Instruction_kv4_v1_lws = 594,
  Instruction_kv4_v1_lwu = 595,
  Instruction_kv4_v1_lwz = 596,
  Instruction_kv4_v1_maddcxdt = 597,
  Instruction_kv4_v1_maddd = 598,
  Instruction_kv4_v1_madddt = 599,
  Instruction_kv4_v1_maddghxdt = 600,
  Instruction_kv4_v1_maddglxdt = 601,
  Instruction_kv4_v1_maddgmxdt = 602,
  Instruction_kv4_v1_maddhq = 603,
  Instruction_kv4_v1_maddhwq = 604,
  Instruction_kv4_v1_maddsudt = 605,
  Instruction_kv4_v1_maddsuhwq = 606,
  Instruction_kv4_v1_maddsuwd = 607,
  Instruction_kv4_v1_maddsuwdp = 608,
  Instruction_kv4_v1_maddudt = 609,
  Instruction_kv4_v1_madduhwq = 610,
  Instruction_kv4_v1_madduwd = 611,
  Instruction_kv4_v1_madduwdp = 612,
  Instruction_kv4_v1_madduzdt = 613,
  Instruction_kv4_v1_maddw = 614,
  Instruction_kv4_v1_maddwd = 615,
  Instruction_kv4_v1_maddwdp = 616,
  Instruction_kv4_v1_maddwp = 617,
  Instruction_kv4_v1_maddxdt = 618,
  Instruction_kv4_v1_maddxhwq = 619,
  Instruction_kv4_v1_maddxsudt = 620,
  Instruction_kv4_v1_maddxsuhwq = 621,
  Instruction_kv4_v1_maddxsuwd = 622,
  Instruction_kv4_v1_maddxsuwdp = 623,
  Instruction_kv4_v1_maddxudt = 624,
  Instruction_kv4_v1_maddxuhwq = 625,
  Instruction_kv4_v1_maddxuwd = 626,
  Instruction_kv4_v1_maddxuwdp = 627,
  Instruction_kv4_v1_maddxwd = 628,
  Instruction_kv4_v1_maddxwdp = 629,
  Instruction_kv4_v1_make = 630,
  Instruction_kv4_v1_max = 631,
  Instruction_kv4_v1_maxbo = 632,
  Instruction_kv4_v1_maxd = 633,
  Instruction_kv4_v1_maxhq = 634,
  Instruction_kv4_v1_maxrbod = 635,
  Instruction_kv4_v1_maxrhqd = 636,
  Instruction_kv4_v1_maxrwpd = 637,
  Instruction_kv4_v1_maxu = 638,
  Instruction_kv4_v1_maxubo = 639,
  Instruction_kv4_v1_maxud = 640,
  Instruction_kv4_v1_maxuhq = 641,
  Instruction_kv4_v1_maxurbod = 642,
  Instruction_kv4_v1_maxurhqd = 643,
  Instruction_kv4_v1_maxurwpd = 644,
  Instruction_kv4_v1_maxuw = 645,
  Instruction_kv4_v1_maxuwp = 646,
  Instruction_kv4_v1_maxw = 647,
  Instruction_kv4_v1_maxwp = 648,
  Instruction_kv4_v1_min = 649,
  Instruction_kv4_v1_minbo = 650,
  Instruction_kv4_v1_mind = 651,
  Instruction_kv4_v1_minhq = 652,
  Instruction_kv4_v1_minrbod = 653,
  Instruction_kv4_v1_minrhqd = 654,
  Instruction_kv4_v1_minrwpd = 655,
  Instruction_kv4_v1_minu = 656,
  Instruction_kv4_v1_minubo = 657,
  Instruction_kv4_v1_minud = 658,
  Instruction_kv4_v1_minuhq = 659,
  Instruction_kv4_v1_minurbod = 660,
  Instruction_kv4_v1_minurhqd = 661,
  Instruction_kv4_v1_minurwpd = 662,
  Instruction_kv4_v1_minuw = 663,
  Instruction_kv4_v1_minuwp = 664,
  Instruction_kv4_v1_minw = 665,
  Instruction_kv4_v1_minwp = 666,
  Instruction_kv4_v1_mm212w = 667,
  Instruction_kv4_v1_mma212w = 668,
  Instruction_kv4_v1_mms212w = 669,
  Instruction_kv4_v1_msbfd = 670,
  Instruction_kv4_v1_msbfdt = 671,
  Instruction_kv4_v1_msbfhq = 672,
  Instruction_kv4_v1_msbfhwq = 673,
  Instruction_kv4_v1_msbfsudt = 674,
  Instruction_kv4_v1_msbfsuhwq = 675,
  Instruction_kv4_v1_msbfsuwd = 676,
  Instruction_kv4_v1_msbfsuwdp = 677,
  Instruction_kv4_v1_msbfudt = 678,
  Instruction_kv4_v1_msbfuhwq = 679,
  Instruction_kv4_v1_msbfuwd = 680,
  Instruction_kv4_v1_msbfuwdp = 681,
  Instruction_kv4_v1_msbfuzdt = 682,
  Instruction_kv4_v1_msbfw = 683,
  Instruction_kv4_v1_msbfwd = 684,
  Instruction_kv4_v1_msbfwdp = 685,
  Instruction_kv4_v1_msbfwp = 686,
  Instruction_kv4_v1_msbfxdt = 687,
  Instruction_kv4_v1_msbfxhwq = 688,
  Instruction_kv4_v1_msbfxsudt = 689,
  Instruction_kv4_v1_msbfxsuhwq = 690,
  Instruction_kv4_v1_msbfxsuwd = 691,
  Instruction_kv4_v1_msbfxsuwdp = 692,
  Instruction_kv4_v1_msbfxudt = 693,
  Instruction_kv4_v1_msbfxuhwq = 694,
  Instruction_kv4_v1_msbfxuwd = 695,
  Instruction_kv4_v1_msbfxuwdp = 696,
  Instruction_kv4_v1_msbfxwd = 697,
  Instruction_kv4_v1_msbfxwdp = 698,
  Instruction_kv4_v1_mul = 699,
  Instruction_kv4_v1_mulcxdt = 700,
  Instruction_kv4_v1_muld = 701,
  Instruction_kv4_v1_muldt = 702,
  Instruction_kv4_v1_mulh = 703,
  Instruction_kv4_v1_mulhq = 704,
  Instruction_kv4_v1_mulhsu = 705,
  Instruction_kv4_v1_mulhu = 706,
  Instruction_kv4_v1_mulhwq = 707,
  Instruction_kv4_v1_mulsudt = 708,
  Instruction_kv4_v1_mulsuhwq = 709,
  Instruction_kv4_v1_mulsuwd = 710,
  Instruction_kv4_v1_mulsuwdp = 711,
  Instruction_kv4_v1_muludt = 712,
  Instruction_kv4_v1_muluhwq = 713,
  Instruction_kv4_v1_muluwd = 714,
  Instruction_kv4_v1_muluwdp = 715,
  Instruction_kv4_v1_mulw = 716,
  Instruction_kv4_v1_mulwd = 717,
  Instruction_kv4_v1_mulwdp = 718,
  Instruction_kv4_v1_mulwp = 719,
  Instruction_kv4_v1_mulwq = 720,
  Instruction_kv4_v1_mulxdt = 721,
  Instruction_kv4_v1_mulxhwq = 722,
  Instruction_kv4_v1_mulxsudt = 723,
  Instruction_kv4_v1_mulxsuhwq = 724,
  Instruction_kv4_v1_mulxsuwd = 725,
  Instruction_kv4_v1_mulxsuwdp = 726,
  Instruction_kv4_v1_mulxudt = 727,
  Instruction_kv4_v1_mulxuhwq = 728,
  Instruction_kv4_v1_mulxuwd = 729,
  Instruction_kv4_v1_mulxuwdp = 730,
  Instruction_kv4_v1_mulxwd = 731,
  Instruction_kv4_v1_mulxwdp = 732,
  Instruction_kv4_v1_mv = 733,
  Instruction_kv4_v1_nandd = 734,
  Instruction_kv4_v1_nandw = 735,
  Instruction_kv4_v1_neg = 736,
  Instruction_kv4_v1_negbo = 737,
  Instruction_kv4_v1_negd = 738,
  Instruction_kv4_v1_neghq = 739,
  Instruction_kv4_v1_negsbo = 740,
  Instruction_kv4_v1_negsd = 741,
  Instruction_kv4_v1_negshq = 742,
  Instruction_kv4_v1_negsw = 743,
  Instruction_kv4_v1_negswp = 744,
  Instruction_kv4_v1_negw = 745,
  Instruction_kv4_v1_negwp = 746,
  Instruction_kv4_v1_neord = 747,
  Instruction_kv4_v1_neorw = 748,
  Instruction_kv4_v1_niord = 749,
  Instruction_kv4_v1_niorw = 750,
  Instruction_kv4_v1_nop = 751,
  Instruction_kv4_v1_not = 752,
  Instruction_kv4_v1_notd = 753,
  Instruction_kv4_v1_notw = 754,
  Instruction_kv4_v1_or = 755,
  Instruction_kv4_v1_orc_b = 756,
  Instruction_kv4_v1_ori = 757,
  Instruction_kv4_v1_orn = 758,
  Instruction_kv4_v1_pcrel = 759,
  Instruction_kv4_v1_prefetch_i = 760,
  Instruction_kv4_v1_prefetch_r = 761,
  Instruction_kv4_v1_prefetch_w = 762,
  Instruction_kv4_v1_rem = 763,
  Instruction_kv4_v1_remu = 764,
  Instruction_kv4_v1_remuw = 765,
  Instruction_kv4_v1_remw = 766,
  Instruction_kv4_v1_ret = 767,
  Instruction_kv4_v1_rev8 = 768,
  Instruction_kv4_v1_rfe = 769,
  Instruction_kv4_v1_rol = 770,
  Instruction_kv4_v1_rolw = 771,
  Instruction_kv4_v1_rolwps = 772,
  Instruction_kv4_v1_ror = 773,
  Instruction_kv4_v1_rori = 774,
  Instruction_kv4_v1_roriw = 775,
  Instruction_kv4_v1_rorw = 776,
  Instruction_kv4_v1_rorwps = 777,
  Instruction_kv4_v1_rswap = 778,
  Instruction_kv4_v1_sb = 779,
  Instruction_kv4_v1_sbfbo = 780,
  Instruction_kv4_v1_sbfcd = 781,
  Instruction_kv4_v1_sbfcd_i = 782,
  Instruction_kv4_v1_sbfd = 783,
  Instruction_kv4_v1_sbfhq = 784,
  Instruction_kv4_v1_sbfsbo = 785,
  Instruction_kv4_v1_sbfsd = 786,
  Instruction_kv4_v1_sbfshq = 787,
  Instruction_kv4_v1_sbfsw = 788,
  Instruction_kv4_v1_sbfswp = 789,
  Instruction_kv4_v1_sbfusbo = 790,
  Instruction_kv4_v1_sbfusd = 791,
  Instruction_kv4_v1_sbfushq = 792,
  Instruction_kv4_v1_sbfusw = 793,
  Instruction_kv4_v1_sbfuswp = 794,
  Instruction_kv4_v1_sbfw = 795,
  Instruction_kv4_v1_sbfwp = 796,
  Instruction_kv4_v1_sbmm8 = 797,
  Instruction_kv4_v1_sbmm8d = 798,
  Instruction_kv4_v1_sbmmt8 = 799,
  Instruction_kv4_v1_sbmmt8d = 800,
  Instruction_kv4_v1_sc_d = 801,
  Instruction_kv4_v1_sc_w = 802,
  Instruction_kv4_v1_scall = 803,
  Instruction_kv4_v1_sd = 804,
  Instruction_kv4_v1_seqz = 805,
  Instruction_kv4_v1_set = 806,
  Instruction_kv4_v1_sext_b = 807,
  Instruction_kv4_v1_sext_h = 808,
  Instruction_kv4_v1_sext_w = 809,
  Instruction_kv4_v1_sgtz = 810,
  Instruction_kv4_v1_sh = 811,
  Instruction_kv4_v1_sh1add = 812,
  Instruction_kv4_v1_sh1add_uw = 813,
  Instruction_kv4_v1_sh2add = 814,
  Instruction_kv4_v1_sh2add_uw = 815,
  Instruction_kv4_v1_sh3add = 816,
  Instruction_kv4_v1_sh3add_uw = 817,
  Instruction_kv4_v1_sleep = 818,
  Instruction_kv4_v1_sll = 819,
  Instruction_kv4_v1_sllbos = 820,
  Instruction_kv4_v1_slld = 821,
  Instruction_kv4_v1_sllhqs = 822,
  Instruction_kv4_v1_slli = 823,
  Instruction_kv4_v1_slli_uw = 824,
  Instruction_kv4_v1_slliw = 825,
  Instruction_kv4_v1_sllw = 826,
  Instruction_kv4_v1_sllwps = 827,
  Instruction_kv4_v1_slsbos = 828,
  Instruction_kv4_v1_slsd = 829,
  Instruction_kv4_v1_slshqs = 830,
  Instruction_kv4_v1_slsw = 831,
  Instruction_kv4_v1_slswps = 832,
  Instruction_kv4_v1_slt = 833,
  Instruction_kv4_v1_slti = 834,
  Instruction_kv4_v1_sltiu = 835,
  Instruction_kv4_v1_sltu = 836,
  Instruction_kv4_v1_sltz = 837,
  Instruction_kv4_v1_slusbos = 838,
  Instruction_kv4_v1_slusd = 839,
  Instruction_kv4_v1_slushqs = 840,
  Instruction_kv4_v1_slusw = 841,
  Instruction_kv4_v1_sluswps = 842,
  Instruction_kv4_v1_snez = 843,
  Instruction_kv4_v1_so = 844,
  Instruction_kv4_v1_sq = 845,
  Instruction_kv4_v1_sra = 846,
  Instruction_kv4_v1_srabos = 847,
  Instruction_kv4_v1_srad = 848,
  Instruction_kv4_v1_srahqs = 849,
  Instruction_kv4_v1_srai = 850,
  Instruction_kv4_v1_sraiw = 851,
  Instruction_kv4_v1_sraw = 852,
  Instruction_kv4_v1_srawps = 853,
  Instruction_kv4_v1_srl = 854,
  Instruction_kv4_v1_srlbos = 855,
  Instruction_kv4_v1_srld = 856,
  Instruction_kv4_v1_srlhqs = 857,
  Instruction_kv4_v1_srli = 858,
  Instruction_kv4_v1_srliw = 859,
  Instruction_kv4_v1_srlw = 860,
  Instruction_kv4_v1_srlwps = 861,
  Instruction_kv4_v1_srsbos = 862,
  Instruction_kv4_v1_srsd = 863,
  Instruction_kv4_v1_srshqs = 864,
  Instruction_kv4_v1_srsw = 865,
  Instruction_kv4_v1_srswps = 866,
  Instruction_kv4_v1_stop = 867,
  Instruction_kv4_v1_stsud = 868,
  Instruction_kv4_v1_stsuhq = 869,
  Instruction_kv4_v1_stsuw = 870,
  Instruction_kv4_v1_stsuwp = 871,
  Instruction_kv4_v1_sub = 872,
  Instruction_kv4_v1_subw = 873,
  Instruction_kv4_v1_sw = 874,
  Instruction_kv4_v1_sxbd = 875,
  Instruction_kv4_v1_sxhd = 876,
  Instruction_kv4_v1_sxlbhq = 877,
  Instruction_kv4_v1_sxlhwp = 878,
  Instruction_kv4_v1_sxmbhq = 879,
  Instruction_kv4_v1_sxmhwp = 880,
  Instruction_kv4_v1_sxwd = 881,
  Instruction_kv4_v1_syncgroup = 882,
  Instruction_kv4_v1_tlbdinval = 883,
  Instruction_kv4_v1_tlbiinval = 884,
  Instruction_kv4_v1_tlbprobe = 885,
  Instruction_kv4_v1_tlbread = 886,
  Instruction_kv4_v1_tlbwrite = 887,
  Instruction_kv4_v1_waitit = 888,
  Instruction_kv4_v1_wfxl = 889,
  Instruction_kv4_v1_wfxm = 890,
  Instruction_kv4_v1_xaccesso = 891,
  Instruction_kv4_v1_xaligno = 892,
  Instruction_kv4_v1_xandno = 893,
  Instruction_kv4_v1_xando = 894,
  Instruction_kv4_v1_xclampwo = 895,
  Instruction_kv4_v1_xcopyo = 896,
  Instruction_kv4_v1_xcopyv = 897,
  Instruction_kv4_v1_xcopyx = 898,
  Instruction_kv4_v1_xeoro = 899,
  Instruction_kv4_v1_xffma44hw = 900,
  Instruction_kv4_v1_xfmaxhx = 901,
  Instruction_kv4_v1_xfminhx = 902,
  Instruction_kv4_v1_xfmma424d_0 = 903,
  Instruction_kv4_v1_xfmma424d_1 = 904,
  Instruction_kv4_v1_xfmma444w_0 = 905,
  Instruction_kv4_v1_xfmma444w_1 = 906,
  Instruction_kv4_v1_xfmma484hw = 907,
  Instruction_kv4_v1_xfnarrow44wh = 908,
  Instruction_kv4_v1_xfscalewo = 909,
  Instruction_kv4_v1_xiorno = 910,
  Instruction_kv4_v1_xioro = 911,
  Instruction_kv4_v1_xlo = 912,
  Instruction_kv4_v1_xmadd44bw0 = 913,
  Instruction_kv4_v1_xmadd44bw1 = 914,
  Instruction_kv4_v1_xmaddifwo = 915,
  Instruction_kv4_v1_xmaddsu44bw0 = 916,
  Instruction_kv4_v1_xmaddsu44bw1 = 917,
  Instruction_kv4_v1_xmaddu44bw0 = 918,
  Instruction_kv4_v1_xmaddu44bw1 = 919,
  Instruction_kv4_v1_xmma4164bw = 920,
  Instruction_kv4_v1_xmma484bw = 921,
  Instruction_kv4_v1_xmmasu4164bw = 922,
  Instruction_kv4_v1_xmmasu484bw = 923,
  Instruction_kv4_v1_xmmau4164bw = 924,
  Instruction_kv4_v1_xmmau484bw = 925,
  Instruction_kv4_v1_xmmaus4164bw = 926,
  Instruction_kv4_v1_xmmaus484bw = 927,
  Instruction_kv4_v1_xmovefd = 928,
  Instruction_kv4_v1_xmovefo = 929,
  Instruction_kv4_v1_xmovefq = 930,
  Instruction_kv4_v1_xmovetd = 931,
  Instruction_kv4_v1_xmoveto = 932,
  Instruction_kv4_v1_xmovetq = 933,
  Instruction_kv4_v1_xmsbfifwo = 934,
  Instruction_kv4_v1_xmt44d = 935,
  Instruction_kv4_v1_xnando = 936,
  Instruction_kv4_v1_xneoro = 937,
  Instruction_kv4_v1_xnioro = 938,
  Instruction_kv4_v1_xnor = 939,
  Instruction_kv4_v1_xor = 940,
  Instruction_kv4_v1_xori = 941,
  Instruction_kv4_v1_xplb = 942,
  Instruction_kv4_v1_xpld = 943,
  Instruction_kv4_v1_xplh = 944,
  Instruction_kv4_v1_xplo = 945,
  Instruction_kv4_v1_xplq = 946,
  Instruction_kv4_v1_xplw = 947,
  Instruction_kv4_v1_xrecvo = 948,
  Instruction_kv4_v1_xsbmm8dq = 949,
  Instruction_kv4_v1_xsbmmt8dq = 950,
  Instruction_kv4_v1_xsendo = 951,
  Instruction_kv4_v1_xsendrecvo = 952,
  Instruction_kv4_v1_xso = 953,
  Instruction_kv4_v1_xsplatdo = 954,
  Instruction_kv4_v1_xsplatov = 955,
  Instruction_kv4_v1_xsplatox = 956,
  Instruction_kv4_v1_xsx48bw = 957,
  Instruction_kv4_v1_xtrunc48wb = 958,
  Instruction_kv4_v1_xzx48bw = 959,
  Instruction_kv4_v1_zext_h = 960,
  Instruction_kv4_v1_zxbd = 961,
  Instruction_kv4_v1_zxhd = 962,
  Instruction_kv4_v1_zxlbhq = 963,
  Instruction_kv4_v1_zxlhwp = 964,
  Instruction_kv4_v1_zxmbhq = 965,
  Instruction_kv4_v1_zxmhwp = 966,
  Instruction_kv4_v1_zxwd = 967,
  Separator_kv4_v1_comma = 968,
  Separator_kv4_v1_equal = 969,
  Separator_kv4_v1_qmark = 970,
  Separator_kv4_v1_rsbracket = 971,
  Separator_kv4_v1_lsbracket = 972
};

typedef enum {
  Modifier_kv4_v1_exunum_ALU0=0,
  Modifier_kv4_v1_exunum_ALU1=1,
  Modifier_kv4_v1_exunum_LSU0=2,
  Modifier_kv4_v1_exunum_LSU1=3,
} Modifier_kv4_v1_exunum_values;


extern const char *mod_kv4_v1_exunum[];
extern const char *mod_kv4_v1_ccbcomp[];
extern const char *mod_kv4_v1_bcucond[];
extern const char *mod_kv4_v1_intcomp[];
extern const char *mod_kv4_v1_lanecond[];
extern const char *mod_kv4_v1_lanetodo[];
extern const char *mod_kv4_v1_lanesize[];
extern const char *mod_kv4_v1_floatcomp[];
extern const char *mod_kv4_v1_floatmode[];
extern const char *mod_kv4_v1_signextw[];
extern const char *mod_kv4_v1_variant[];
extern const char *mod_kv4_v1_speculate[];
extern const char *mod_kv4_v1_doscale[];
extern const char *mod_kv4_v1_qindex[];
extern const char *mod_kv4_v1_hindex[];
extern const char *mod_kv4_v1_cachelev[];
extern const char *mod_kv4_v1_coherency[];
extern const char *mod_kv4_v1_boolcas[];
extern const char *mod_kv4_v1_accesses[];
extern const char *mod_kv4_v1_channel[];
extern const char *mod_kv4_v1_conjugate[];
extern const char *mod_kv4_v1_realimag[];
extern const char *mod_kv4_v1_shuffleV[];
extern const char *mod_kv4_v1_shuffleX[];
extern const char *mod_kv4_v1_splat32[];
extern const char *mod_kv4_v1_ordering[];
extern const char *mod_kv4_v1_acqrel[];
extern const char *mod_kv4_v1_froundmode[];

typedef enum {
  Bundling_kv4_v1_ALL,
  Bundling_kv4_v1_BCUD,
  Bundling_kv4_v1_BCU0,
  Bundling_kv4_v1_BCU,
  Bundling_kv4_v1_FULL,
  Bundling_kv4_v1_FULL_X,
  Bundling_kv4_v1_FULL_Y,
  Bundling_kv4_v1_LITE,
  Bundling_kv4_v1_LITE_X,
  Bundling_kv4_v1_LITE_Y,
  Bundling_kv4_v1_LSU0,
  Bundling_kv4_v1_LSU0_X,
  Bundling_kv4_v1_LSU0_Y,
  Bundling_kv4_v1_LSU,
  Bundling_kv4_v1_LSU_X,
  Bundling_kv4_v1_LSU_Y,
  Bundling_kv4_v1_TINY,
  Bundling_kv4_v1_TINY_X,
  Bundling_kv4_v1_TINY_Y,
  Bundling_kv4_v1_EXT,
  Bundling_kv4_v1_NOP,
} Bundling_kv4_v1;

static int ATTRIBUTE_UNUSED
kv4_v1_base_bundling(int bundling) {
  static int base_bundlings[] = {
    Bundling_kv4_v1_ALL,	// Bundling_kv4_v1_ALL
    Bundling_kv4_v1_BCUD,	// Bundling_kv4_v1_BCUD
    Bundling_kv4_v1_BCU0,	// Bundling_kv4_v1_BCU0
    Bundling_kv4_v1_BCU,	// Bundling_kv4_v1_BCU
    Bundling_kv4_v1_FULL,	// Bundling_kv4_v1_FULL
    Bundling_kv4_v1_FULL,	// Bundling_kv4_v1_FULL_X
    Bundling_kv4_v1_FULL,	// Bundling_kv4_v1_FULL_Y
    Bundling_kv4_v1_LITE,	// Bundling_kv4_v1_LITE
    Bundling_kv4_v1_LITE,	// Bundling_kv4_v1_LITE_X
    Bundling_kv4_v1_LITE,	// Bundling_kv4_v1_LITE_Y
    Bundling_kv4_v1_LSU0,	// Bundling_kv4_v1_LSU0
    Bundling_kv4_v1_LSU0,	// Bundling_kv4_v1_LSU0_X
    Bundling_kv4_v1_LSU0,	// Bundling_kv4_v1_LSU0_Y
    Bundling_kv4_v1_LSU,	// Bundling_kv4_v1_LSU
    Bundling_kv4_v1_LSU,	// Bundling_kv4_v1_LSU_X
    Bundling_kv4_v1_LSU,	// Bundling_kv4_v1_LSU_Y
    Bundling_kv4_v1_TINY,	// Bundling_kv4_v1_TINY
    Bundling_kv4_v1_TINY,	// Bundling_kv4_v1_TINY_X
    Bundling_kv4_v1_TINY,	// Bundling_kv4_v1_TINY_Y
    Bundling_kv4_v1_EXT,	// Bundling_kv4_v1_EXT
    Bundling_kv4_v1_NOP,	// Bundling_kv4_v1_NOP
  };
  return base_bundlings[bundling];
};

typedef enum {
  Resource_kv4_v1_ISSUE,
  Resource_kv4_v1_TINY,
  Resource_kv4_v1_LITE,
  Resource_kv4_v1_FULL,
  Resource_kv4_v1_LSU,
  Resource_kv4_v1_MAU,
  Resource_kv4_v1_BCU,
  Resource_kv4_v1_EXT,
  Resource_kv4_v1_AUXR,
  Resource_kv4_v1_AUXW,
  Resource_kv4_v1_XFER,
  Resource_kv4_v1_MEMW,
  Resource_kv4_v1_SR12,
  Resource_kv4_v1_SR13,
  Resource_kv4_v1_SR14,
  Resource_kv4_v1_SR15,
} Resource_kv4_v1;
#define kv4_v1_RESOURCE_COUNT 16

typedef enum {
  Reservation_kv4_v1_ALL,
  Reservation_kv4_v1_ALU_TINY,
  Reservation_kv4_v1_ALU_TINY_X,
  Reservation_kv4_v1_ALU_TINY_Y,
  Reservation_kv4_v1_ALU_TINY_AUXR,
  Reservation_kv4_v1_ALU_LITE,
  Reservation_kv4_v1_ALU_LITE_X,
  Reservation_kv4_v1_ALU_LITE_Y,
  Reservation_kv4_v1_ALU_LITE_MISC,
  Reservation_kv4_v1_ALU_FULL,
  Reservation_kv4_v1_ALU_FULL_X,
  Reservation_kv4_v1_ALU_FULL_Y,
  Reservation_kv4_v1_BCU,
  Reservation_kv4_v1_BCU_BRRP,
  Reservation_kv4_v1_BCU_BRRP2,
  Reservation_kv4_v1_BCUD,
  Reservation_kv4_v1_BCU2,
  Reservation_kv4_v1_BCU_XFER,
  Reservation_kv4_v1_BCU_XFER_BRRP,
  Reservation_kv4_v1_BCU2_TINY_LSU,
  Reservation_kv4_v1_LSU,
  Reservation_kv4_v1_LSU_X,
  Reservation_kv4_v1_LSU_Y,
  Reservation_kv4_v1_LSU_MEMW_ACCR,
  Reservation_kv4_v1_LSU_MEMW_ACCR_X,
  Reservation_kv4_v1_LSU_MEMW_ACCR_Y,
  Reservation_kv4_v1_LSU2_MEMW,
  Reservation_kv4_v1_LSU2_MEMW_X,
  Reservation_kv4_v1_LSU2_MEMW_Y,
  Reservation_kv4_v1_LSU_AUXR,
  Reservation_kv4_v1_LSU_AUXR_X,
  Reservation_kv4_v1_LSU_AUXR_Y,
  Reservation_kv4_v1_LSU_MEMW_AUXR,
  Reservation_kv4_v1_LSU_MEMW_AUXR_X,
  Reservation_kv4_v1_LSU_MEMW_AUXR_Y,
  Reservation_kv4_v1_LSU2_MEMW_AUXR,
  Reservation_kv4_v1_LSU2_MEMW_AUXR_X,
  Reservation_kv4_v1_LSU2_MEMW_AUXR_Y,
  Reservation_kv4_v1_LSU_MEMW_AUXW,
  Reservation_kv4_v1_LSU_MEMW_AUXW_X,
  Reservation_kv4_v1_LSU_MEMW_AUXW_Y,
  Reservation_kv4_v1_LSU2_MEMW_AUXW,
  Reservation_kv4_v1_LSU2_MEMW_AUXW_X,
  Reservation_kv4_v1_LSU2_MEMW_AUXW_Y,
  Reservation_kv4_v1_LSU_AUXW,
  Reservation_kv4_v1_LSU_AUXW_X,
  Reservation_kv4_v1_LSU_AUXW_Y,
  Reservation_kv4_v1_LSU_AUXR_AUXW,
  Reservation_kv4_v1_LSU_AUXR_AUXW_X,
  Reservation_kv4_v1_LSU_AUXR_AUXW_Y,
  Reservation_kv4_v1_LSU2_MEMW_AUXR_AUXW,
  Reservation_kv4_v1_LSU2_MEMW_AUXR_AUXW_X,
  Reservation_kv4_v1_LSU2_MEMW_AUXR_AUXW_Y,
  Reservation_kv4_v1_MAU,
  Reservation_kv4_v1_EXT,
  Reservation_kv4_v1_EXT_COMP,
  Reservation_kv4_v1_EXT_MISC,
  Reservation_kv4_v1_EXT_MISC_AUXW,
} Reservation_kv4_v1;

extern struct kvx_reloc kv4_v1_rel16_reloc;
extern struct kvx_reloc kv4_v1_rel32_reloc;
extern struct kvx_reloc kv4_v1_rel64_reloc;
extern struct kvx_reloc kv4_v1_pcrel_signed16_reloc;
extern struct kvx_reloc kv4_v1_pcrel17_reloc;
extern struct kvx_reloc kv4_v1_pcrel27_reloc;
extern struct kvx_reloc kv4_v1_pcrel32_reloc;
extern struct kvx_reloc kv4_v1_pcrel_signed37_reloc;
extern struct kvx_reloc kv4_v1_pcrel_signed43_reloc;
extern struct kvx_reloc kv4_v1_pcrel_signed64_reloc;
extern struct kvx_reloc kv4_v1_pcrel64_reloc;
extern struct kvx_reloc kv4_v1_signed16_reloc;
extern struct kvx_reloc kv4_v1_signed32_reloc;
extern struct kvx_reloc kv4_v1_signed37_reloc;
extern struct kvx_reloc kv4_v1_gotoff_signed37_reloc;
extern struct kvx_reloc kv4_v1_gotoff_signed43_reloc;
extern struct kvx_reloc kv4_v1_gotoff_32_reloc;
extern struct kvx_reloc kv4_v1_gotoff_64_reloc;
extern struct kvx_reloc kv4_v1_got_32_reloc;
extern struct kvx_reloc kv4_v1_got_signed37_reloc;
extern struct kvx_reloc kv4_v1_got_signed43_reloc;
extern struct kvx_reloc kv4_v1_got_64_reloc;
extern struct kvx_reloc kv4_v1_glob_dat_reloc;
extern struct kvx_reloc kv4_v1_copy_reloc;
extern struct kvx_reloc kv4_v1_jump_slot_reloc;
extern struct kvx_reloc kv4_v1_relative_reloc;
extern struct kvx_reloc kv4_v1_signed43_reloc;
extern struct kvx_reloc kv4_v1_signed64_reloc;
extern struct kvx_reloc kv4_v1_gotaddr_signed37_reloc;
extern struct kvx_reloc kv4_v1_gotaddr_signed43_reloc;
extern struct kvx_reloc kv4_v1_gotaddr_signed64_reloc;
extern struct kvx_reloc kv4_v1_dtpmod64_reloc;
extern struct kvx_reloc kv4_v1_dtpoff64_reloc;
extern struct kvx_reloc kv4_v1_dtpoff_signed37_reloc;
extern struct kvx_reloc kv4_v1_dtpoff_signed43_reloc;
extern struct kvx_reloc kv4_v1_tlsgd_signed37_reloc;
extern struct kvx_reloc kv4_v1_tlsgd_signed43_reloc;
extern struct kvx_reloc kv4_v1_tlsld_signed37_reloc;
extern struct kvx_reloc kv4_v1_tlsld_signed43_reloc;
extern struct kvx_reloc kv4_v1_tpoff64_reloc;
extern struct kvx_reloc kv4_v1_tlsie_signed37_reloc;
extern struct kvx_reloc kv4_v1_tlsie_signed43_reloc;
extern struct kvx_reloc kv4_v1_tlsle_signed37_reloc;
extern struct kvx_reloc kv4_v1_tlsle_signed43_reloc;
extern struct kvx_reloc kv4_v1_rel8_reloc;
extern struct kvx_reloc kv4_v1_pcrel11_reloc;
extern struct kvx_reloc kv4_v1_pcrel38_reloc;


#endif /* OPCODE_KVX_H */
