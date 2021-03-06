#include <cpu/exec.h>
#include "local-include/rtl.h"

#define IRQ_TIMER 0x20

vaddr_t get_ksp() {
  vaddr_t gdtr_gate = cpu.gdtr.base + 5 * 8; // TODO: turn 5 to TR register
  vaddr_t gdtr_addr = (vaddr_read(gdtr_gate + 2, 4) & 0xffffff) | (vaddr_read(gdtr_gate + 7, 1) << 24); // Address of TSS32
  return vaddr_read(gdtr_addr + 4, 4); // tss.esp0
}

void set_ksp(uint32_t val) {
  vaddr_t gdtr_gate = cpu.gdtr.base + 5 * 8; // TODO: turn 5 to TR register
  vaddr_t gdtr_addr = (vaddr_read(gdtr_gate + 2, 4) & 0xffffff) | (vaddr_read(gdtr_gate + 7, 1) << 24); // Address of TSS32
  vaddr_write(gdtr_addr + 4, val, 4); // tss.esp0
}

void raise_intr(DecodeExecState *s, uint32_t NO, vaddr_t ret_addr) {
  vaddr_t idtr_gate = cpu.idtr.base + NO * 8;

  uint32_t gate_l = vaddr_read(idtr_gate, 2);
  uint32_t gate_h = vaddr_read(idtr_gate + 4, 4);
  if ((gate_h & 0x8000) == 0) return;

  // Push ss, esp into TSS.esp0 if in user privilege
  if ((cpu.cs & 0x3) == 0x3) {
    // Push ss and esp
    vaddr_t ksp = get_ksp();
    // printf("Read ksp 0x%08x from tss, saved usp 0x%08x, cr3 0x%08x\n", ksp, cpu.esp, cpu.cr3);
    rtl_mv(s, s0, &cpu.esp);
    if (ksp != 0)
      rtl_li(s, &cpu.esp, ksp);
    rtl_li(s, s1, cpu.ss);
    rtl_push(s, s1);
    rtl_push(s, s0);
    set_ksp(0);
  }

  // Push eflags, cs, eip
  vaddr_t idtr_addr = ((gate_h >> 16) << 16) | gate_l;

  rtl_push(s, &cpu.eflags.val);
  cpu.eflags.IF = 0;

  rtl_li(s, s0, cpu.cs);
  rtl_push(s, s0);

  rtl_li(s, s0, ret_addr);
  rtl_push(s, s0);

  rtl_j(s, idtr_addr);
}

void restore_intr(DecodeExecState *s) {
  rtl_pop(s, s0);
  rtl_jr(s, s0);

  rtl_pop(s, s0);
  cpu.cs = *s0;

  rtl_pop(s, &cpu.eflags.val);

  // Returning to user privilege
  if ((cpu.cs & 0x3) == 0x3) {
    rtl_pop(s, s0); // esp
    rtl_pop(s, s1); // ss
    cpu.ss = *s1;

    set_ksp(cpu.esp);
    // printf("Saved ksp 0x%08x to tss, switched to usp 0x%08x\n", cpu.esp, *s0);

    rtl_mv(s, &cpu.esp, s0);
  }
}

void query_intr(DecodeExecState *s) {
  if (cpu.INTR & cpu.eflags.IF) {
    cpu.INTR = false;
    raise_intr(s, IRQ_TIMER, cpu.pc);
    update_pc(s);
  }
}
