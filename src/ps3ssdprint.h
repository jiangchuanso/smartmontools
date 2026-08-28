/*
 * ps3ssdprint.h
 *
 * Home page of code is: https://www.smartmontools.org
 *
 * Copyright (C) 2026 the smartmontools project
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef PS3SSDPRINT_H
#define PS3SSDPRINT_H

#include <smartmon/ps3ssdcmds.h>

/*
 *  Prints the parsed PS3 SSD health log (GP Log 0xE4/0xE5) data
 *  already present in the ps3_ssd_e4_log / ps3_ssd_e5_log structures
 *
 *  @param  e4:  Constant reference to parsed detailed counters (const smartmon::ps3_ssd_e4_log&)
 *  @param  e5:  Constant reference to parsed health levels (const smartmon::ps3_ssd_e5_log&)
 *  @param  have_e4: true if the GP Log 0xE4 data is valid (bool)
 *  @param  have_e5: true if the GP Log 0xE5 data is valid (bool)
 */
void ataPrintPs3SsdLog(const smartmon::ps3_ssd_e4_log& e4, const smartmon::ps3_ssd_e5_log& e5,
                       bool have_e4, bool have_e5);

#endif
