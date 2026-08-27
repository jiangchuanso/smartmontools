/*
 * ps3ssdcmds.cpp
 *
 * Home page of code is: https://www.smartmontools.org
 *
 * Copyright (C) 2026 the smartmontools project
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "config.h"

#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>
#include <string.h>

#include <smartmon/ps3ssdcmds.h>

#include <smartmon/sg_unaligned.h>

namespace smartmon {

// GP log addresses of the SSD vendor health logs
static const unsigned char PS3_SSD_LOG_E4 = 0xE4; // Detailed counters
static const unsigned char PS3_SSD_LOG_E5 = 0xE5; // Health levels

/*
 *  Reads and parses the SSD vendor health log (GP Log 0xE4) detailed
 *  reliability counters from the device.
 *
 *  @param  device:  Pointer to instantiated device object (ata_device*)
 *  @param  log:     Reference to parsed data structure (ps3_ssd_e4_log&)
 *  @return true if read and parse successful, false otherwise (bool)
 */
bool ataReadPs3SsdLogE4(ata_device * device, ps3_ssd_e4_log & log) {
  log = {};
  unsigned char buf[512] = {};

  // The log is read with a standard ATA READ LOG EXT command (1 sector).
  // This works for SATA and SAT (USB bridge) devices without any vendor
  // library, the PS3 storage driver already provides the SATA passthrough.
  if (!ataReadLogExt(device, PS3_SSD_LOG_E4, 0, 0, buf, 1))
    return device->set_err(EIO, "Read PS3 SSD log (GP Log 0xE4) failed: %s",
                           device->get_errmsg());

  // Parse little-endian fields from the 512-byte log sector
  unsigned o = 0;
  log.revision_id = sg_get_unaligned_le16(buf + o); o += 2;
  memcpy(log.signature, buf + o, sizeof(log.signature)); o += sizeof(log.signature);
  log.type = buf[o++];
  log.health_level = buf[o++];
  log.plp_capacitance = buf[o++];
  log.plp_capacitor_volt = sg_get_unaligned_le16(buf + o); o += 2;
  log.ddr_ce_err_count = sg_get_unaligned_le32(buf + o); o += 4;
  log.ddr_uce_err_count = sg_get_unaligned_le32(buf + o); o += 4;
  log.sram_ce_err_count = sg_get_unaligned_le32(buf + o); o += 4;
  log.sram_uce_err_count = sg_get_unaligned_le32(buf + o); o += 4;
  log.sensor_err_count = sg_get_unaligned_le32(buf + o); o += 4;
  log.composite_temp = buf[o++];
  log.unc = buf[o++];
  log.unc_cnt = sg_get_unaligned_le16(buf + o); o += 2;
  log.uc = buf[o++];
  log.uc_cnt = sg_get_unaligned_le16(buf + o); o += 2;
  log.min_temp = buf[o++];
  log.max_temp = buf[o++];
  log.soc_temp = buf[o++];
  log.nand_temp = buf[o++];
  log.erase_err_cnt = sg_get_unaligned_le32(buf + o); o += 4;
  log.program_err_cnt = sg_get_unaligned_le32(buf + o); o += 4;
  log.ard_err_good_cnt = sg_get_unaligned_le32(buf + o); o += 4;
  log.mrd_err_good_cnt = sg_get_unaligned_le32(buf + o); o += 4;
  log.total_bad_blk_cnt = sg_get_unaligned_le32(buf + o); o += 4;
  log.nand_media_err_cnt = sg_get_unaligned_le32(buf + o); o += 4;
  log.ard_err_fail_cnt = sg_get_unaligned_le32(buf + o); o += 4;
  log.mrd_err_fail_cnt = sg_get_unaligned_le32(buf + o); o += 4;
  log.raid_fail_cnt = sg_get_unaligned_le32(buf + o); o += 4;
  log.raid_good_cnt = sg_get_unaligned_le32(buf + o); o += 4;
  log.realloc_sector_cnt = sg_get_unaligned_le32(buf + o); o += 4;
  log.cur_pending_sector_cnt = sg_get_unaligned_le32(buf + o); o += 4;
  log.sata_phy_err_cnt = sg_get_unaligned_le32(buf + o); o += 4;
  log.fis_crc_err_cnt = sg_get_unaligned_le64(buf + o); o += 8;
  log.link_speed_downgrade_cnt = sg_get_unaligned_le32(buf + o); o += 4;
  log.unsafe_shutdown_cnt = sg_get_unaligned_le32(buf + o); o += 4;
  log.available_spare_cnt = sg_get_unaligned_le32(buf + o); o += 4;
  log.high_lat_rcmd_ttl_cnt = sg_get_unaligned_le32(buf + o); o += 4;
  log.high_lat_wcmd_ttl_cnt = sg_get_unaligned_le32(buf + o); o += 4;
  log.fatal_rst_cnt = sg_get_unaligned_le32(buf + o); o += 4;
  log.read_only_mode_cnt = sg_get_unaligned_le32(buf + o); o += 4;
  log.hs_crc_err_cnt = sg_get_unaligned_le32(buf + o); o += 4;
  log.power_on_hours = sg_get_unaligned_le32(buf + o); o += 4;
  log.power_cycle_cnt = sg_get_unaligned_le32(buf + o); o += 4;
  log.lifetime_used = sg_get_unaligned_le32(buf + o);

  return true;
}

/*
 *  Reads and parses the SSD vendor health log (GP Log 0xE5) health levels
 *  from the device.
 *
 *  @param  device:  Pointer to instantiated device object (ata_device*)
 *  @param  log:     Reference to parsed data structure (ps3_ssd_e5_log&)
 *  @return true if read and parse successful, false otherwise (bool)
 */
bool ataReadPs3SsdLogE5(ata_device * device, ps3_ssd_e5_log & log) {
  log = {};
  unsigned char buf[512] = {};

  if (!ataReadLogExt(device, PS3_SSD_LOG_E5, 0, 0, buf, 1))
    return device->set_err(EIO, "Read PS3 SSD log (GP Log 0xE5) failed: %s",
                           device->get_errmsg());

  // All fields are single bytes (health level per counter)
  unsigned o = 0;
  log.health_level = buf[o++];
  log.plp_cap_err_level = buf[o++];
  log.plp_cap_volt_err_level = buf[o++];
  log.ddr_ce_err_level = buf[o++];
  log.ddr_uce_err_level = buf[o++];
  log.sram_ce_err_level = buf[o++];
  log.sram_uce_err_level = buf[o++];
  log.sensor_err_level = buf[o++];
  log.comp_temp_err_level = buf[o++];
  log.critical_comp_err_level = buf[o++];
  log.warning_comp_err_level = buf[o++];
  log.min_temp_err_level = buf[o++];
  log.max_temp_err_level = buf[o++];
  log.soc_temp_err_level = buf[o++];
  log.nand_temp_err_level = buf[o++];
  log.erase_err_level = buf[o++];
  log.program_err_level = buf[o++];
  log.ard_err_good_level = buf[o++];
  log.mrd_err_good_level = buf[o++];
  log.total_bad_blk_level = buf[o++];
  log.nand_media_err_level = buf[o++];
  log.ard_err_fail_level = buf[o++];
  log.mrd_err_fail_level = buf[o++];
  log.raid_fail_level = buf[o++];
  log.raid_good_level = buf[o++];
  log.realloc_sector_level = buf[o++];
  log.cur_pending_sector_level = buf[o++];
  log.sata_phy_err_level = buf[o++];
  log.fis_crc_err_level = buf[o++];
  log.link_speed_downgrade_level = buf[o++];
  log.unsafe_shutdown_level = buf[o++];
  log.available_spare_level = buf[o++];
  log.high_lat_rcmd_ttl_level = buf[o++];
  log.high_lat_wcmd_ttl_level = buf[o++];
  log.fatal_rst_level = buf[o++];
  log.read_only_mode_level = buf[o++];
  log.hs_crc_err_level = buf[o++];
  log.nor_critical_log_full_err_level = buf[o++];
  log.num_err_log_entry_err_level = buf[o++];
  log.power_on_hours_err_level = buf[o++];
  log.power_cycle_err_level = buf[o++];
  log.lifetime_used_err_level = buf[o++];

  return true;
}

} // namespace smartmon
