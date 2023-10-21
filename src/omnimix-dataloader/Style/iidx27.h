#ifndef _IIDX27_H
#define _IIDX27_H
#include "defines.h"
//_BUG: most likely broken?
/*_TODO:
*  In x64dbg break on node_data_va to check which register holds required data (previously r11 register)
*  if unsure how valid data looks, run iidx26 with hook and check what data r11 holds there and find same thing for node_data_va of iidx27
*/



static const od_payload_t REV_20200929 = {
        .revision ="20200929",
        .clear_rate_va = 0x295960,
        .fc_rate_va = 0x296E80,
        .node_data_va = 0x69BF30, //_REM: this or 0x697280 listed for some reason as alternative
        .node_data_rtn_va = 0x6988BB
};

#endif