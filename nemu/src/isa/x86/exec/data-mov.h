static inline def_EHelper(mov) {
  operand_write(s, id_dest, dsrc1);
  print_asm_template2(mov);
}

static inline def_EHelper(push) {
  rtl_push(s, ddest);

  print_asm_template1(push);
}

static inline def_EHelper(pop) {
  rtl_pop(s, ddest);
  operand_write(s, id_dest, ddest);

  print_asm_template1(pop);
}

static inline def_EHelper(pusha) {
  rtl_mv(s, s0, &reg_l(R_ESP));
  for (int i = R_EAX; i <= R_EDI; i++)
    if (i == R_ESP) rtl_push(s, s0);
    else rtl_push(s, &reg_l(i));

  print_asm("pusha");
}

static inline def_EHelper(popa) {
  for (int i = R_EDI; i >= R_EAX; i--)
    if (i == R_ESP) rtl_pop(s, s0);
    else rtl_pop(s, &reg_l(i));

  print_asm("popa");
}

static inline def_EHelper(leave) {
  rtl_mv(s, &cpu.esp, &cpu.ebp);
  rtl_pop(s, &cpu.ebp);

  print_asm("leave");
}

static inline def_EHelper(cltd) {
  if (s->isa.is_operand_size_16) {
    if (cpu.eax & 0x8000) cpu.edx |= 0xffff;
    else cpu.edx &= 0xffff0000;
  } else {
    if (cpu.eax & 0x80000000) cpu.edx = 0xffffffff;
    else cpu.edx = 0;
  }

  print_asm(s->isa.is_operand_size_16 ? "cwtd" : "cltd");
}

static inline def_EHelper(cwtl) {
  if (s->isa.is_operand_size_16) {
    rtl_lr(s, s0, R_AL, 1);
    rtl_sext(s, s0, s0, 1);
    rtl_sr(s, R_AX, s0, 2);
  }
  else {
    rtl_lr(s, s0, R_AX, 2);
    rtl_sext(s, s0, s0, 2);
    rtl_sr(s, R_EAX, s0, 4);
  }

  print_asm(s->isa.is_operand_size_16 ? "cbtw" : "cwtl");
}

static inline def_EHelper(movsx) {
  id_dest->width = s->isa.is_operand_size_16 ? 2 : 4;
  rtl_sext(s, ddest, dsrc1, id_src1->width);
  operand_write(s, id_dest, ddest);

  print_asm_template2(movsx);
}

static inline def_EHelper(movzx) {
  id_dest->width = s->isa.is_operand_size_16 ? 2 : 4;
  operand_write(s, id_dest, dsrc1);

  print_asm_template2(movzx);
}

static inline def_EHelper(movsb) {
  rtl_lm(s, s0, &cpu.esi, 0, 1);
  rtl_sm(s, &cpu.edi, 0, s0, 1);
  rtl_addi(s, &cpu.esi, &cpu.esi, 1);
  rtl_addi(s, &cpu.edi, &cpu.edi, 1);

  print_asm("movsb");
}

static inline def_EHelper(movsd) {
  rtl_lm(s, s0, &cpu.esi, 0, 4);
  rtl_sm(s, &cpu.edi, 0, s0, 4);
  rtl_addi(s, &cpu.esi, &cpu.esi, 4);
  rtl_addi(s, &cpu.edi, &cpu.edi, 4);

  print_asm("movsd");
}

static inline def_EHelper(lea) {
  rtl_addi(s, ddest, s->isa.mbase, s->isa.moff);
  operand_write(s, id_dest, ddest);
  
  print_asm_template2(lea);
}

