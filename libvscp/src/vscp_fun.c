#include "vscp_fun.h"
#include "vscp_frame.h"


void print_vscp_frame(const struct vscp_frame *vf) {
  int i;
  printf("Class: %d\n", vf->v_class);
  printf("Type: %d\n", vf->v_type);
  for(i=0; i<8; i++)
    printf("Data[%d]: %x\n", i, vf->v_data[i]);
}
