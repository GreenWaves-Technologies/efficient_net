#ifndef __efficientnet_lite4_H__
#define __efficientnet_lite4_H__

#define __PREFIX(x) efficientnet_lite4 ## x

// Include basic GAP builtins defined in the Autotiler
#include "Gap.h"

#ifdef __EMUL__
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/param.h>
#include <string.h>
#endif

extern AT_HYPERFLASH_FS_EXT_ADDR_TYPE efficientnet_lite4_L3_Flash;
#include "../../power_meas_utils/measurments_utils.h"
#endif