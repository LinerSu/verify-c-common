
#include <klee_switch.h>

size_t alloc_size(size_t sz) {
  switch(sz) {
    case 1: return 1;
    case 2: return 2;
    case 3: return 3;
    case 4: return 4;
    case 5: return 5;
    case 6: return 6;
    case 7: return 7;
    case 8: return 8;
    case 9: return 9;
    case 10: return 10;
    default: return sz <= 4096 ? 4096 : sz;
  }
}
