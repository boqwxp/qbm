#include "ipasir.h"
#include "stdlib.h"

char const* ipasir_signature() { return "<dummy>"; }
void* ipasir_init()                   { abort(); }
void  ipasir_release(void *impl)      { abort(); }
void  ipasir_add(void *impl, int lit) { abort(); }
int   ipasir_solve(void *impl)        { abort(); }
int   ipasir_val(void *impl, int lit) { abort(); }
