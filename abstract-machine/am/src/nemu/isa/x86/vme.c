#include <am.h>
#include <nemu.h>
#include <klib.h>

static AddrSpace kas = {};
static void* (*pgalloc_usr)(int) = NULL;
static void (*pgfree_usr)(void*) = NULL;
static int vme_enable = 0;

static Area segments[] = {      // Kernel memory mappings
  NEMU_PADDR_SPACE
};

#define USER_SPACE RANGE(0x40000000, 0xc0000000)

bool vme_init(void* (*pgalloc_f)(int), void (*pgfree_f)(void*)) {
  pgalloc_usr = pgalloc_f;
  pgfree_usr = pgfree_f;

  kas.ptr = pgalloc_f(PGSIZE);

  int i;
  for (i = 0; i < LENGTH(segments); i ++) {
    void *va = segments[i].start;
    for (; va < segments[i].end; va += PGSIZE) {
      map(&kas, va, va, 0);
    }
  }

  set_cr3(kas.ptr);
  set_cr0(get_cr0() | CR0_PG);
  vme_enable = 1;

  return true;
}

void protect(AddrSpace *as) {
  PTE *updir = (PTE*)(pgalloc_usr(PGSIZE));
  as->ptr = updir;
  as->area = USER_SPACE;
  as->pgsize = PGSIZE;
  // map kernel space
  memcpy(updir, kas.ptr, PGSIZE);
}

void unprotect(AddrSpace *as) {
}

void __am_get_cur_as(Context *c) {
  c->cr3 = (vme_enable ? (void *)get_cr3() : NULL);
}

void __am_switch(Context *c) {
  if (vme_enable && c->cr3 != NULL) {
    set_cr3(c->cr3);
  }
}

void map(AddrSpace *as, void *va, void *pa, int prot) {
  // printf("Mapping vaddr 0x%08x to 0x%08x, writing into ptr 0x%08x\n", va, pa, as->ptr);
  uint32_t dir = (((uint32_t)(va) >> 22) & 0x3ff);
  uint32_t page = (((uint32_t)(va) >> 12) & 0x3ff);

  if ((((PTE*)as->ptr)[dir] & PTE_P) == 0) {
    ((PTE*)as->ptr)[dir] = (PTE)pgalloc_usr(PGSIZE) | PTE_P;
  }

  PTE* pg_table = (PTE*)(((PTE*)as->ptr)[dir] & 0xfffff000);
  if ((pg_table[page] & PTE_P) == 0) {
    pg_table[page] = ((uint32_t)pa & 0xfffff000) | PTE_P;
  } else {
    printf("Mapping from vaddr 0x%08x to 0x%08x already exists\n", va, pa);
  }
}

Context* ucontext(AddrSpace *as, Area kstack, void *entry) {
  Context *c = (Context*)kstack.end - 1;
  c->eip = (uintptr_t)entry;
  c->eflags.IF = 1;
  c->cr3 = as->ptr;
  c->cs = USEL(3);
  c->ss3 = USEL(4);
  return c;
}
