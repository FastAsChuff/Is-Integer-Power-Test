#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <math.h>
#include </home/simon/int128fns.c>

// gcc ispower.c -o ispower.bin -lm -O3  -Wall -ffast-math -march=native -std=c11 -fstrict-aliasing -fasm
// using gcc (Ubuntu 9.4.0-1ubuntu1~20.04.1) 9.4.0

uint64_t  get_cycles () {
  uint32_t lo,hi;
  asm  volatile("rdtsc":"=a"(lo),"=d"(hi));
  return  (( uint64_t)hi<<32 | lo);
}

int warmup() {
  uint32_t i;
  for(i=0; i<100000000; i++) {
    if (!get_cycles()) {
      printf("Warm up message\n");
    }
  };
  return 0;
}


_Bool isperfectsquare(const uint64_t x, uint64_t *isqrtx) {  
  _Bool resbool;
  __int128 isqrtx128;
  resbool = isqrt128((const __int128)x, &isqrtx128);
  if (!resbool) return false;
  if (isqrtx != NULL) *isqrtx = (uint64_t)isqrtx128;
  return isqrtx128*isqrtx128 == (__int128)x;
}


_Bool isperfectcube(uint64_t x, uint64_t *icbrtx) {
  const uint8_t guess[8] = {1, 1, 1, 1, 1, 2, 2, 2};
  uint32_t i;
  uint64_t y, ai;
  if (x == 0) {
    if (icbrtx != NULL) *icbrtx = 0;
    return true;
  }
  if (x == 1) {
    if (icbrtx != NULL) *icbrtx = 1;
    return true;
  }
  if (x < 8) return false;
  if (x == 18446724184312856125UL) {
    if (icbrtx != NULL) *icbrtx = 2642245;
    return true;
  }
  y = x;
  i = 0;
  while ((y >> i) >= 8) i+=3;
  ai = ((uint64_t)guess[y >> i]) << (i/3);
  //printf("Initial guess = %lu\n", ai);
  ai = (2*ai + x/(ai*ai))/3;
  ai = (2*ai + x/(ai*ai))/3;
  ai = (2*ai + x/(ai*ai))/3;
  ai = (2*ai + x/(ai*ai))/3;
  while (!((ai*ai*ai <= x) && ((ai+1)*(ai+1)*(ai+1) > x))) {   
    ai = (2*ai + x/(ai*ai))/3;
  }
  if (icbrtx != NULL) icbrtx[0] = ai;
  return ai*ai*ai == x;
}

_Bool isperfectpow5(uint64_t x, uint64_t *i5thrtx) {
  uint32_t i;
  uint64_t y;
  __int128 ai, ai2;
  if (x == 0) {
    if (i5thrtx != NULL) *i5thrtx = 0;
    return true;
  }
  if (x == 1) {
    if (i5thrtx != NULL) *i5thrtx = 1;
    return true;
  }
  if (x < 32) return false;
  y = x;
  i = 0;
  while ((y >> i) >= 32) i+=5;
  if ((y >> i) <= 13) {
    ai = ((uint64_t)1) << (i/5);
  } else {
    ai = ((uint64_t)1) << (1 + (i/5));
  }
  ai2 = ai*ai;
  //printf("Initial guess = %lu\n", (uint64_t)ai);
  ai = (4*ai + x/(ai2*ai2))/5;
  ai2 = ai*ai;
  while (!((ai2*ai2*ai <= x) && ((ai+1)*(ai+1)*(ai+1)*(ai+1)*(ai+1) > x))) {   
    ai = (4*ai + x/(ai2*ai2))/5;
    ai2 = ai*ai;
  }
  if (i5thrtx != NULL) i5thrtx[0] = ai;
  return ai2*ai2*ai == x;
}

_Bool ispower64(uint64_t num, uint32_t *a, uint32_t *b) {
  // MAX num = 0x7fffffffffffffffUL
  if (num == 0) {
    if (a != NULL) *a = 0;
    if (b != NULL) *b = 1;
    return true;
  }
  if (num == 1) {
    if (a != NULL) *a = 1;
    if (b != NULL) *b = 1;
    return true;
  } 
  uint16_t s[] = {2, 3, 5, 7, 11, 13, 17, 19};
  uint32_t i;
  for (i=0; i<sizeof(s)/sizeof(s[0]); i++) {
    if ((num % s[i]) == 0) {
      if ((num % ((uint32_t)s[i]*s[i])) != 0) return false;
    }
  }
  uint64_t base, power, numsqrt, numcbrt, num5thrt;
  uint32_t tempa, tempb;
  __int128 basetopower7, basetopower;
  if (isperfectsquare(num, &numsqrt)) {
    if ((a == NULL) && (b == NULL)) return true;
    if (a != NULL) *a = (uint32_t)numsqrt;
    if (b != NULL) *b = 2;
    if (ispower64(numsqrt, &tempa, &tempb)) {      
      if (a != NULL) *a = tempa;
      if (b != NULL) *b = 2*tempb;
    }
    return true;
  }
  if (isperfectcube(num, &numcbrt)) {
    if ((a == NULL) && (b == NULL)) return true;
    if (a != NULL) *a = (uint32_t)numcbrt;
    if (b != NULL) *b = 3;
    if (ispower64(numcbrt, &tempa, &tempb)) {      
      if (a != NULL) *a = tempa;
      if (b != NULL) *b = 3*tempb;
    }
    return true;
  }
  if (isperfectpow5(num, &num5thrt)) {
    if ((a == NULL) && (b == NULL)) return true;
    if (a != NULL) *a = (uint32_t)num5thrt;
    if (b != NULL) *b = 5;
    if (ispower64(num5thrt, &tempa, &tempb)) {      
      if (a != NULL) *a = tempa;
      if (b != NULL) *b = 5*tempb;
    }
    return true;
  }
  basetopower7 = 128;
  for (base=2; basetopower7<=num; ) {
    if ((num % base) == 0) {
      basetopower = basetopower7;
      for (power=7; basetopower<num; power++) {
        basetopower *= base;
      }
      if (basetopower == num) {
        if (a != NULL) *a = base;
        if (b != NULL) *b = power;
        return true;
      }
    }
    base++;
    basetopower7 = (__int128)base*base*base;
    basetopower7 = basetopower7*basetopower7*base;
  }
  return false;
}

int main(int argc, char **argv)
{
  uint32_t base, power, pow;
  _Bool res;
  if (warmup() != 0) exit(1);
  uint64_t b, num = 13, cyclesstart, cyclesend;
  myint128_t input;
  input.i128 = num;
  if (argc > 1) {
    strtomyint128((unsigned char*)argv[1], &input);
    num = (uint64_t)input.i128;
  }
  cyclesstart = get_cycles();
  res = ispower64(num, &base, &power);  
  cyclesend = get_cycles();
  if (res) {
    printf("%lu = %u ^ %u\n", num, base, power);
  } else {
    printf("%lu is not a power\n", num);
  }
  printf("Cycles = %li\n", cyclesend - cyclesstart);
  exit(0);
  __int128 basetopower;
  uint64_t callcount = 0;
  for (b=2; b<=0xffffffff; b++) {
    pow = 2;
    basetopower = b*b;
    if ((b % 100000000) == 0) printf("Currently at %lu\n", b);
    while (basetopower<=0xffffffffffffffffUL) {
      res = ispower64(basetopower, &base, &power);  
      if (pow >= 7) callcount++;
      if (!res) printf("Failed %lu ^ %i\n", b, pow);
      basetopower *= b;
      pow++;
      if (pow > 64) exit(1);
    }
  }
  printf("Calls = %lu\n", callcount);
  exit(0);
}
