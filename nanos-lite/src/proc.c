#include <proc.h>

static PCB pcb_boot = {};
static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
PCB *current = NULL;

void switch_boot_pcb() {
  current = &pcb_boot;
}

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    Log("Hello World from Nanos-lite with arg '%p' for the %dth time!", (uintptr_t)arg, j);
    j ++;
    yield();
  }
}

void init_proc() {
  // switch_boot_pcb();

  Log("Initializing processes...");

  // load program here
  // naive_uload(NULL, "/bin/menu");

  // 191220052DEC = b65c954HEX
  context_kload(&pcb[0], hello_fun, (void*)0xb65c954); 
  char* args[] = {"/bin/nterm", NULL, NULL, NULL};
  context_uload(&pcb[1], "/bin/nterm", args, NULL);
  args[0] = "/bin/nslider";
  context_uload(&pcb[2], "/bin/nslider", args, NULL);
  args[0] = "/bin/pal"; args[1] = "--skip";
  context_uload(&pcb[3], "/bin/pal", args, NULL);
  switch_boot_pcb();
}

int pcb_id = -1, set_pcb_id = 1;
PCB* get_free_pcb() {
  pcb_id++;
  #ifdef DEBUG
    Log("Goto PCB: %d (0x%08x) cp: 0x%08x", pcb_id, &pcb[pcb_id], pcb[pcb_id].cp);
  #endif
  if (pcb_id == MAX_NR_PROC) panic("No free PCB available");
  return &pcb[pcb_id];
}

PCB* get_current_pcb() {
  return &pcb[pcb_id];
}

void set_pcb(int x) {
  set_pcb_id = x;
}

PCB* get_last_pcb() {
  pcb_id--;
  if (pcb_id < 0) pcb_id = 0;
  #ifdef DEBUG
    Log("Goto PCB: %d (0x%08x) cp: 0x%08x", pcb_id, &pcb[pcb_id], pcb[pcb_id].cp);
  #endif
  return &pcb[pcb_id];
}

Context* schedule(Context *prev) {
  current->cp = prev;
  #ifdef DEBUG
    Log("[Schedule] Saved current context pointer to 0x%08x", prev);
  #endif

  if (pcb_id == 0) pcb_id = set_pcb_id;
  else pcb_id = 0;
  current = &pcb[pcb_id];

  #ifdef DEBUG
    Log("[Schedule] Switched to 0x%08x, context 0x%08x, entry 0x%08x", current, current->cp, current->cp->eip);
  #endif
  return current->cp;
}
