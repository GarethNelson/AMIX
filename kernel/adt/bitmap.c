#include "adt/bitmap.h"
#include "hal.h"
#include "string.h"
#include "stdio.h"
#include "assert.h"

#pragma GCC optimize ("O3")
/* Returns the index of the least significant bit that is set in byte.
   If byte == 0, the behaviour is undefined. */
static int lsb_set(uint8_t byte) {
unsigned int v;  // find the number of trailing zeros in 32-bit v 
int r;           // result goes here
static const int MultiplyDeBruijnBitPosition[32] = 
{
  0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8, 
  31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
};
r = MultiplyDeBruijnBitPosition[((uint32_t)((byte & -byte) * 0x077CB531U)) >> 27];
     return r; 
}

void bitmap_init(bitmap_t *xb, uint8_t *storage, int64_t max_extent) {
  xb->max_extent = max_extent;
  xb->data = storage;
  xb->first_set = -1;
  memset(xb->data, 0, max_extent / 8 + 1);
}

void bitmap_set(bitmap_t *xb, unsigned idx) {
  xb->data[idx/8] |= (1 << (idx%8));
  if(idx < xb->first_set) xb->first_set=idx;
  assert(bitmap_isset(xb, idx));
}

void bitmap_clear(bitmap_t *xb, unsigned idx) {
  xb->data[idx/8] &= ~(1 << (idx%8));
  if(idx== xb->first_set) xb->first_set=-1;
}

int bitmap_isset(bitmap_t *xb, unsigned idx) {
  return (xb->data[idx/8] & (1 << (idx%8))) ? 1 : 0;
}
int bitmap_isclear(bitmap_t *xb, unsigned idx) {
  return !bitmap_isset(xb, idx);
}

__attribute__((hot))
int64_t bitmap_first_set(bitmap_t *xb) {

  if(xb->first_set != -1) return xb->first_set;
  for (uint64_t i = 0; i < (xb->max_extent >> 3) + 1ULL; i ++) {
    if (xb->data[i] == 0) continue;

    int64_t idx = i * 8 + lsb_set(xb->data[i]);
    xb->first_set = (idx > xb->max_extent) ? -1 : idx;
    return xb->first_set;
  }
  return -1;
}
