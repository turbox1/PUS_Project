#ifndef VSCP_FRAME 
#define VSCP_FRAME

#include <stdio.h>

//--- VSCP STRUCTURE ---

struct vscp_frame {
  unsigned char v_class;
  unsigned char v_type;
  unsigned char v_data[8];
};

#endif //VSCP_FRAME
