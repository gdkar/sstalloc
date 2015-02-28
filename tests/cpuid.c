#define __GNU_SOURCE
#include <features.h>
#include <stdatomic.h>
#include <x86intrin.h>
#include <stdlib.h>

#include <stddef.h>
#include <stdint.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
//#include "st_arch_x86-64.h"
static inline void st_cpuid_full(int32_t a, char *buf, size_t len){
  int i;
  int32_t _a,_b,_c,_d;
  __asm__ volatile (
      "mov %4, %%eax\n\t"
      "cpuid\n\t"
      :"=a"(_a)
      ,"=b"(_b)
      ,"=c"(_c)
      ,"=d"(_d)
      : "a"(a)
      :"cc");
  if(len>=4)((int32_t*)buf)[3] =_a;
  if(len>=8)((int32_t*)buf)[2] =_c;
  if(len>=12)((int32_t*)buf)[1]=_d;
  if(len>=16)((int32_t*)buf)[0]=_b;
  for(i = 16; i < len;i++){
    buf[i]=0;
  }
}
int main(int argc, char **argv){
  char cpuid[16];int32_t max_leaf,max_eleaf;
  size_t i;
  size_t ntsc= 4096;
  int64_t tsc;
  int32_t *tscs; 

  st_cpuid_full(0,cpuid,sizeof(cpuid));
  fprintf(stdout,"CPUID(0) returned ");
  fwrite(cpuid,1,12,stdout);
  fprintf(stdout,"%08x\n",((int32_t*)cpuid)[3]);
  max_leaf = ((int32_t*)cpuid)[3];
  st_cpuid_full(0x80000000,cpuid,sizeof(cpuid));
  max_eleaf = ((int32_t*)cpuid)[3];
  fprintf(stdout,"CPUID(0x80000000) returned eax=%08x ebx=%08x ecx=%08x edx=%08x\n",((int32_t*)cpuid)[3],((int32_t*)cpuid)[0],
      ((int32_t*)cpuid)[2],((int32_t*)cpuid)[1]);
  st_cpuid_full(1,cpuid,sizeof(cpuid));
  fprintf(stdout,"CPUID(1) returned eax=%08x ebx=%08x ecx=%08x edx=%08x\n",((int32_t*)cpuid)[3],((int32_t*)cpuid)[0],
      ((int32_t*)cpuid)[2],((int32_t*)cpuid)[1]);
  if(posix_memalign((void**)&tscs,4096,ntsc*sizeof(*tscs))<0){
    fprintf(stderr,"posix_memalign returned negative\n");
    exit(-1);
  }
  for(i = 0; i < ntsc; i++){
    tscs[i]=0;
  }
  for(i = 0; i < ntsc; i+=4){
    register int32_t tsct0,tsct1,tsct2,tsct3;
    __asm__ volatile ("cpuid \n\trdtsc\n\t":"=A"((tsct0))::"rbx","rcx","rdx","cc");
    __asm__ volatile ("cpuid \n\trdtsc\n\t":"=A"((tsct1))::"rbx","rcx","rdx","cc");
    __asm__ volatile ("rdtsc\n\t":"=A"((tsct2))::"rbx","rdx","cc");
    __asm__ volatile ("rdtsc\n\t":"=A"((tsct3))::"rbx","rdx","cc");
    tscs[i+0]=tsct0;
    tscs[i+1]=tsct1;
    tscs[i+2]=tsct2;
    tscs[i+3]=tsct3;
  }
  for(i = 1; i < ntsc; i++){
    if(i%8==1)
      fprintf(stdout,"\n%zu: ",i);
    fprintf(stdout,"%d, ",tscs[i]-tscs[i-1]);
  }
  return 0;
}
