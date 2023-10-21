#ifndef _IIDX26_H
#define _IIDX26_H
#include "defines.h"


static const od_payload_t REV_20190902 = {
        .revision = "20190902",
        .clear_rate_va = 0xE08E0,
        .fc_rate_va = 0xE2010,
        .node_data_va = 0x3E4D10,
        .node_data_rtn_va = 0x3E681B
};

static const od_payload_t REV_20191007 = {
        .revision ="REV_20191007",
        .clear_rate_va = 0xE08E0,
        .fc_rate_va = 0xE2010,
        .node_data_va = 0x3E4D10,
        .node_data_rtn_va = 0x3E689B
};

#endif