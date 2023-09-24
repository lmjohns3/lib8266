#include "lib8266/numeric.h"


/* Sort the values of an array in-place (insertion sort). */
void ahoy_sort_u16(uint16_t *array, uint8_t size) {
  uint8_t i, j;
  uint16_t v;
  for (i = 1; i < size; ++i) {
    v = array[i];
    for (j = i; j > 0 && array[j-1] > v; --j) array[j] = array[j-1];
    array[j] = v;
  }
}

/* Sort the values of an array in-place (insertion sort). */
void ahoy_sort_u32(uint32_t *array, uint8_t size) {
  uint8_t i, j;
  uint32_t v;
  for (i = 1; i < size; ++i) {
    v = array[i];
    for (j = i; j > 0 && array[j-1] > v; --j) array[j] = array[j-1];
    array[j] = v;
  }
}
