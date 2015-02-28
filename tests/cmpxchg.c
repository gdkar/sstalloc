#define __GNU_SOURCE
#include <features.h>
#include <stdatomic.h>
#include <x86intrin.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "src/atomic.h"

int main(int argc, char **argv){
  int128_t o = (int128_t){.v2=(argc>1?atoll(argv[1]):0),
                .v1=(argc>2?atoll(argv[2]):0)},
           i = (int128_t){ .v2=(argc>3?atoll(argv[3]):0),
                .v1=(argc>4?atoll(argv[4]):0)},
           p = (int128_t){ .v2=((argc>5)?atoll(argv[5]):0),
                .v1=((argc>6)?atoll(argv[6]):0)};
  int ret;
  fprintf(stdout,"initial values (exp) %zu:%zu, (req) %zu:%zu: (ptr) %zu:%zu\n",o.v2,o.v1,i.v2,i.v1,p.v2,p.v1);
  for(int x = 0; x < 8; x++){
    ret = cmpxchg16b(&p, &o,i);
    fprintf(stdout,"operation returned %s\n",ret?"SUCCESS":"FAILURE");
    fprintf(stdout,"initial values (exp) %zu:%zu, (req) %zu:%zu: (ptr) %zu:%zu\n",o.v2,o.v1,i.v2,i.v1,p.v2,p.v1);
  }
  return 0;
}
