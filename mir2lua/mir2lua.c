/* This file is a part of MIR project.
   Copyright (C) 2018-2021 Vladimir Makarov <vmakarov.gcc@gmail.com>.
*/

#include "mir2lua.h"
#include <float.h>
#include <inttypes.h>

#define TAB "    "

static MIR_func_t curr_func;

static void out_op (MIR_context_t ctx, FILE *f, MIR_op_t op) {
  switch (op.mode) {
  case MIR_OP_REG: fprintf (f, "%s", MIR_reg_name (ctx, op.u.reg, curr_func)); break;
  case MIR_OP_INT: fprintf (f, "%i", (int) op.u.i); break;
  case MIR_OP_UINT: fprintf (f, "%li", (long) op.u.u); break;
  case MIR_OP_FLOAT: fprintf (f, "%f", (double) op.u.f); break;
  case MIR_OP_DOUBLE: fprintf (f, "%f", (double) op.u.d); break;
  case MIR_OP_LDOUBLE: fprintf (f, "%f", (double) op.u.d); break;
  case MIR_OP_REF: fprintf (f, "%s", MIR_item_name (ctx, op.u.ref)); break;
  case MIR_OP_LABEL: fprintf (f, "l%" PRId64, op.u.label->ops[0].u.i); break;
  }
}

static void out_op2 (MIR_context_t ctx, FILE *f, MIR_op_t *ops, const char *str) {
  out_op (ctx, f, ops[0]);
  fprintf (f, " = ");
  if (str != NULL) fprintf (f, "%s ", str);
  out_op (ctx, f, ops[1]);
  fprintf (f, "\n");
}

static void out_op3 (MIR_context_t ctx, FILE *f, MIR_op_t *ops, const char *str) {
  out_op (ctx, f, ops[0]);
  fprintf (f, " = ");
  out_op (ctx, f, ops[1]);
  fprintf (f, " %s ", str);
  out_op (ctx, f, ops[2]);
  fprintf (f, "\n");
}

static void out_jmp (MIR_context_t ctx, FILE *f, MIR_op_t label_op) {
  mir_assert (label_op.mode == MIR_OP_LABEL);
  fprintf (f, "goto ");
  out_op (ctx, f, label_op);
}

static void out_cmp (MIR_context_t ctx, FILE *f, MIR_op_t *ops, const char *str) {
  fprintf (f, "if ");
  out_op (ctx, f, ops[1]);
  fprintf (f, " %s ", str);
  out_op (ctx, f, ops[2]);
  fprintf (f, " then ");
  out_jmp (ctx, f, ops[0]);
  fprintf (f, " end\n");
}

static void out_insn (MIR_context_t ctx, FILE *f, MIR_insn_t insn) {
  MIR_op_t *ops = insn->ops;

  if (insn->code != MIR_LABEL) fprintf (f, TAB);

  switch (insn->code) {
  case MIR_MOV:
  case MIR_FMOV: 
  case MIR_DMOV:
  case MIR_EXT8: 
  case MIR_EXT16:
  case MIR_EXT32:
  case MIR_UEXT8:
  case MIR_UEXT16:
  case MIR_UEXT32:
  case MIR_F2I:
  case MIR_D2I:
  case MIR_LD2I:
  case MIR_I2D:
  case MIR_F2D:
  case MIR_LD2D:
  case MIR_I2F: 
  case MIR_D2F: 
  case MIR_LD2F:
  case MIR_I2LD:
  case MIR_D2LD:
  case MIR_F2LD:
  case MIR_UI2D:
  case MIR_UI2F:
  case MIR_UI2LD: {
    out_op2 (ctx, f, ops, "");
    break;
  }
  case MIR_NEG: 
  case MIR_NEGS:
  case MIR_FNEG:
  case MIR_DNEG:
  case MIR_LDNEG: {
    out_op2 (ctx, f, ops, "0 - ");
    break;
  }
  case MIR_FADD: 
  case MIR_DADD: 
  case MIR_LDADD:
  case MIR_ADDS:
  case MIR_ADD: {
    out_op3 (ctx, f, ops, "+");
    break;
  }
  case MIR_FSUB: 
  case MIR_DSUB: 
  case MIR_LDSUB:
  case MIR_SUBS:
  case MIR_SUB: {
    out_op3 (ctx, f, ops, "-");
    break;
  }
  case MIR_FMUL: 
  case MIR_DMUL: 
  case MIR_LDMUL:
  case MIR_MULS:
  case MIR_MUL:{
    out_op3 (ctx, f, ops, "*");
    break;
  }
  case MIR_UMODS:
  case MIR_UDIV: 
  case MIR_DIVS:
  case MIR_DIV:{
    out_op3 (ctx, f, ops, "/");
    break;
  }
  case MIR_FDIV: 
  case MIR_DDIV: 
  case MIR_LDDIV:
  case MIR_UDIVS:
  case MIR_UMOD: 
  case MIR_MODS:
  case MIR_MOD:{
    out_op3 (ctx, f, ops, "%%");
    break;
  }
  case MIR_AND:
  case MIR_ANDS:{
    out_op3 (ctx, f, ops, "&");
    break;
  }
  case MIR_OR: 
  case MIR_ORS:{
    out_op3 (ctx, f, ops, "|");
    break;
  }
  case MIR_XOR:
  case MIR_XORS:{
    out_op3 (ctx, f, ops, "~");
    break;
  }
  case MIR_LSH:
  case MIR_LSHS:{
    out_op3 (ctx, f, ops, "<<");
    break;
  }
  case MIR_RSH:
  case MIR_URSH:{
    out_op3 (ctx, f, ops, ">>");
    break;
  }
  case MIR_RSHS:
  case MIR_URSHS:{
    out_op3 (ctx, f, ops, ">>");
    break;
  }
  case MIR_EQ: 
  case MIR_EQS:
  case MIR_FEQ: 
  case MIR_DEQ: 
  case MIR_LDEQ:{
    out_op3 (ctx, f, ops, "==");
    break;
  }
  case MIR_NE: 
  case MIR_NES:
  case MIR_FNE: 
  case MIR_DNE: 
  case MIR_LDNE:{
    out_op3 (ctx, f, ops, "~=");
    break;
  }
  case MIR_LT: 
  case MIR_LTS:
  case MIR_ULT:
  case MIR_ULTS:
  case MIR_FLT: 
  case MIR_DLT: 
  case MIR_LDLT:{
    out_op3 (ctx, f, ops, "<");
    break;
  }
  case MIR_LE: 
  case MIR_LES:
  case MIR_ULE:
  case MIR_ULES:
  case MIR_FLE: 
  case MIR_DLE: 
  case MIR_LDLE:{
    out_op3 (ctx, f, ops, "<=");
    break;
  }
  case MIR_GT: 
  case MIR_GTS:
  case MIR_UGT:
  case MIR_UGTS:
  case MIR_FGT: 
  case MIR_DGT: 
  case MIR_LDGT:{
    out_op3 (ctx, f, ops, ">");
    break;
  }
  case MIR_GE: 
  case MIR_GES:
  case MIR_UGE:
  case MIR_UGES:
  case MIR_FGE: 
  case MIR_DGE: 
  case MIR_LDGE:{
    out_op3 (ctx, f, ops, ">=");
    break;
  }
  case MIR_JMP: {
    out_jmp(ctx, f, ops[0]);
    break;
  }
  case MIR_BT: { 
    fprintf (f, "if ");
    out_op (ctx, f, ops[1]);
    fprintf (f, " then ");
    out_jmp (ctx, f, ops[0]);
    fprintf (f, "end\n");
    break;
  }
  case MIR_BF: { 
    fprintf (f, "if not ");
    out_op (ctx, f, ops[1]);
    fprintf (f, " then ");
    out_jmp (ctx, f, ops[0]);
    fprintf (f, "end\n");
    break;
  }
  case MIR_BEQ: 
  case MIR_BEQS:
  case MIR_FBEQ:
  case MIR_DBEQ:
  case MIR_LDBEQ:{ 
    out_cmp (ctx, f, ops, "==");
    break;
  }

  case MIR_BNE: 
  case MIR_BNES:
  case MIR_FBNE:
  case MIR_DBNE:
  case MIR_LDBNE:{ 
    out_cmp (ctx, f, ops, "~=");
    break;
  }

  case MIR_BLT: 
  case MIR_BLTS:
  case MIR_UBLT:
  case MIR_UBLTS:
  case MIR_FBLT:
  case MIR_DBLT:
  case MIR_LDBLT:{ 
    out_cmp (ctx, f, ops, "<");
    break;
  }

  case MIR_BLE: 
  case MIR_BLES:
  case MIR_UBLE:
  case MIR_UBLES:
  case MIR_FBLE:
  case MIR_DBLE:
  case MIR_LDBLE:{ 
    out_cmp (ctx, f, ops, "<=");
    break;
  }

  case MIR_BGT: 
  case MIR_BGTS:
  case MIR_UBGT:
  case MIR_UBGTS:
  case MIR_FBGT:
  case MIR_DBGT:
  case MIR_LDBGT:{ 
    out_cmp (ctx, f, ops, ">");
    break;
  }

  case MIR_BGE: 
  case MIR_BGES:
  case MIR_UBGE:
  case MIR_UBGES:
  case MIR_FBGE:
  case MIR_DBGE:
  case MIR_LDBGE: { 
    out_cmp (ctx, f, ops, ">=");
    break;
  }

  case MIR_ALLOCA: printf ("unk: %i", (int) insn->code);
  case MIR_CALL:
  case MIR_INLINE: {
    MIR_proto_t proto;
    size_t start = 2;

    mir_assert (insn->nops >= 2 && ops[0].mode == MIR_OP_REF
                && ops[0].u.ref->item_type == MIR_proto_item);
    proto = ops[0].u.ref->u.proto;
    if (proto->nres > 1) {
      (*MIR_get_error_func (ctx)) (MIR_call_op_error,
                                   " can not translate multiple results functions into C");
    } else if (proto->nres == 1) {
      out_op (ctx, f, ops[2]);
      fprintf (f, " = ");
      start = 3;
    }
    out_op (ctx, f, ops[1]);
    fprintf (f, "(");
    for (size_t i = start; i < insn->nops; i++) {
      if (i != start) fprintf (f, ", ");
      out_op (ctx, f, ops[i]);
    }
    fprintf (f, ");\n");
    break;
  }
  case MIR_RET: {
    fprintf (f, "return ");
    if (insn->nops == 0) {
      fprintf (f, "none");
    } else {
      out_op (ctx, f, ops[0]);
    }
    fprintf (f, "\n");
    break;
  }
  case MIR_LABEL: printf ("unk: %i", (int) insn->code);
  default: mir_assert (FALSE);
  }
}

void out_item (MIR_context_t ctx, FILE *f, MIR_item_t item) {
  MIR_var_t var;
  size_t i, nlocals;

  if (item->item_type == MIR_export_item) return;
  if (item->item_type == MIR_import_item) return;
  if (item->item_type == MIR_forward_item) return;
  if (item->item_type == MIR_proto_item) return;
  curr_func = item->u.func;
  fprintf (f, "function ");
  fprintf (f, "%s(", curr_func->name);
  for (i = 0; i < curr_func->nargs; i++) {
    if (i != 0) fprintf (f, ", ");
    var = VARR_GET (MIR_var_t, curr_func->vars, i);
    fprintf (f, "%s", var.name);
  }
  fprintf (f, ")\n");
  nlocals = VARR_LENGTH (MIR_var_t, curr_func->vars) - curr_func->nargs;
  for (i = 0; i < nlocals; i++) {
    var = VARR_GET (MIR_var_t, curr_func->vars, i + curr_func->nargs);
    fprintf (f, TAB "local %s\n", var.name);
  }
  for (MIR_insn_t insn = DLIST_HEAD (MIR_insn_t, curr_func->insns); insn != NULL;
       insn = DLIST_NEXT (MIR_insn_t, insn)) {
    out_insn (ctx, f, insn);
  }
  fprintf (f, "end");
  fprintf (f, "\n");
}

void MIR_module2lua (MIR_context_t ctx, FILE *f, MIR_module_t m) {
  for (MIR_item_t item = DLIST_HEAD (MIR_item_t, m->items); item != NULL;
       item = DLIST_NEXT (MIR_item_t, item))
    out_item (ctx, f, item);
}

DEF_VARR (char);

int main (int argc, const char *argv[]) {
  int c;
  FILE *f;
  VARR (char) * input;
  MIR_module_t m;
  MIR_context_t ctx = MIR_init ();

  if (argc == 1)
    f = stdin;
  else if (argc == 2) {
    if ((f = fopen (argv[1], "r")) == NULL) {
      fprintf (stderr, "%s: cannot open file %s\n", argv[0], argv[1]);
      exit (1);
    }
  } else {
    fprintf (stderr, "usage: %s < file or %s mir-file\n", argv[0], argv[0]);
    exit (1);
  }
  VARR_CREATE (char, input, 0);
  while ((c = getc (f)) != EOF) VARR_PUSH (char, input, c);
  VARR_PUSH (char, input, 0);
  if (ferror (f)) {
    fprintf (stderr, "%s: error in reading input file\n", argv[0]);
    exit (1);
  }
  fclose (f);
  MIR_scan_string (ctx, VARR_ADDR (char, input));
  m = DLIST_TAIL (MIR_module_t, *MIR_get_module_list (ctx));
  MIR_module2lua (ctx, stdout, m);
  MIR_finish (ctx);
  VARR_DESTROY (char, input);
  return 0;
}