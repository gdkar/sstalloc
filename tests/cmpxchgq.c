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
typedef struct int32x2{
  int32_t v1;
  int32_t v2; 
}int32x2;
int main(int argc, char **argv){
  int32x2 o = (int32x2){.v2=(argc>1?atol(argv[1]):0),
                .v1=(argc>2?atol(argv[2]):0)},
           i = (int32x2){ .v2=(argc>3?atol(argv[3]):0),
                .v1=(argc>4?atol(argv[4]):0)},
           p = (int32x2){ .v2=((argc>5)?atol(argv[5]):0),
                .v1=((argc>6)?atol(argv[6]):0)};
  int ret;
  fprintf(stdout,"initial values (exp) %u:%u, (req) %u:%u: (ptr) %u:%u\n",o.v2,o.v1,i.v2,i.v1,p.v2,p.v1);
  for(int x = 0; x < 8; x++){
    ret = cmpxchgq((uint64_t*)&p, (uint64_t*)&o,*(uint64_t*)&i);
    fprintf(stdout,"operation returned %s\n",ret?"SUCCESS":"FAILURE");
    fprintf(stdout,"initial values (exp) %u:%u, (req) %u:%u: (ptr) %u:%u\n",o.v2,o.v1,i.v2,i.v1,p.v2,p.v1);
  }
  return 0;
}
