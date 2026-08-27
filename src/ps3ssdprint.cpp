/*
 * ps3ssdprint.cpp
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
#include <stdio.h>

#include "ps3ssdprint.h"
#include "smartctl.h"

using namespace smartmon;

// Health level descriptors used by GP Log 0xE5 (0 = ok .. 4 = fatal)
static const char * ps3_ssd_level_desc(uint8_t level)
{
  switch (level) {
    case 0: return "ok";
    case 1: return "warning";
    case 2: return "error";
    case 3: return "critical";
    case 4: return "fatal";
    default: return "unknown";
  }
}

// Print signature bytes (printable chars only)
static void ps3_ssd_print_signature(const uint8_t signature[5], char (& out)[6])
{
  unsigned j = 0;
  for (unsigned i = 0; i < 5; i++) {
    if (32 <= signature[i] && signature[i] < 127)
      out[j++] = (char)signature[i];
  }
  out[j] = '\0';
}

/*
 *  Prints the parsed PS3 SSD health log (GP Log 0xE4/0xE5) data
 *  already present in the ps3_ssd_e4_log / ps3_ssd_e5_log structures
 *
 *  @param  e4:  Constant reference to parsed detailed counters (const ps3_ssd_e4_log&)
 *  @param  e5:  Constant reference to parsed health levels (const ps3_ssd_e5_log&)
 */
void ataPrintPs3SsdLog(const ps3_ssd_e4_log& e4, const ps3_ssd_e5_log& e5)
{
  jout("PS3 SSD Health Log (GP Log 0xe4/0xe5)\n");

  // Print plain-text detailed counters (GP Log 0xE4)
  jout("\tDetailed Counters (GP Log 0xe4)\n");
  char sig[6];
  ps3_ssd_print_signature(e4.signature, sig);
  jout("\t\tLog Revision: %" PRIu16 "\n", e4.revision_id);
  jout("\t\tSignature: %s\n", sig);
  jout("\t\tLog Type: %u\n", e4.type);
  jout("\t\tOverall Health Level: %u (%s)\n", e4.health_level, ps3_ssd_level_desc(e4.health_level));
  jout("\t\tPLP Capacitance: %u uF\n", e4.plp_capacitance);
  jout("\t\tPLP Capacitor Voltage: %u mV\n", e4.plp_capacitor_volt);
  jout("\t\tDDR Correctable Errors: %" PRIu32 "\n", e4.ddr_ce_err_count);
  jout("\t\tDDR Uncorrectable Errors: %" PRIu32 "\n", e4.ddr_uce_err_count);
  jout("\t\tSRAM Correctable Errors: %" PRIu32 "\n", e4.sram_ce_err_count);
  jout("\t\tSRAM Uncorrectable Errors: %" PRIu32 "\n", e4.sram_uce_err_count);
  jout("\t\tSensor Errors: %" PRIu32 "\n", e4.sensor_err_count);
  jout("\t\tComposite Temperature: %u C\n", e4.composite_temp);
  jout("\t\tUncorrectable Error Type: %u\n", e4.unc);
  jout("\t\tUncorrectable Error Count: %" PRIu16 "\n", e4.unc_cnt);
  jout("\t\tCorrectable Error Type: %u\n", e4.uc);
  jout("\t\tCorrectable Error Count: %" PRIu16 "\n", e4.uc_cnt);
  jout("\t\tMinimum Temperature: %u C\n", e4.min_temp);
  jout("\t\tMaximum Temperature: %u C\n", e4.max_temp);
  jout("\t\tSoC Temperature: %u C\n", e4.soc_temp);
  jout("\t\tNAND Temperature: %u C\n", e4.nand_temp);
  jout("\t\tNAND Erase Errors: %" PRIu32 "\n", e4.erase_err_cnt);
  jout("\t\tNAND Program Errors: %" PRIu32 "\n", e4.program_err_cnt);
  jout("\t\tARD Correctable Read Errors: %" PRIu32 "\n", e4.ard_err_good_cnt);
  jout("\t\tMRD Correctable Read Errors: %" PRIu32 "\n", e4.mrd_err_good_cnt);
  jout("\t\tTotal Bad Blocks: %" PRIu32 "\n", e4.total_bad_blk_cnt);
  jout("\t\tNAND Media Errors: %" PRIu32 "\n", e4.nand_media_err_cnt);
  jout("\t\tARD Uncorrectable Read Errors: %" PRIu32 "\n", e4.ard_err_fail_cnt);
  jout("\t\tMRD Uncorrectable Read Errors: %" PRIu32 "\n", e4.mrd_err_fail_cnt);
  jout("\t\tRAID Recovery Failures: %" PRIu32 "\n", e4.raid_fail_cnt);
  jout("\t\tRAID Recovery Successes: %" PRIu32 "\n", e4.raid_good_cnt);
  jout("\t\tReallocated Sectors: %" PRIu32 "\n", e4.realloc_sector_cnt);
  jout("\t\tCurrent Pending Sectors: %" PRIu32 "\n", e4.cur_pending_sector_cnt);
  jout("\t\tSATA PHY Errors: %" PRIu32 "\n", e4.sata_phy_err_cnt);
  jout("\t\tFIS CRC Errors: %" PRIu64 "\n", e4.fis_crc_err_cnt);
  jout("\t\tLink Speed Downgrades: %" PRIu32 "\n", e4.link_speed_downgrade_cnt);
  jout("\t\tUnsafe Shutdowns: %" PRIu32 "\n", e4.unsafe_shutdown_cnt);
  jout("\t\tAvailable Spare Blocks: %" PRIu32 "\n", e4.available_spare_cnt);
  jout("\t\tHigh Latency Read Commands: %" PRIu32 "\n", e4.high_lat_rcmd_ttl_cnt);
  jout("\t\tHigh Latency Write Commands: %" PRIu32 "\n", e4.high_lat_wcmd_ttl_cnt);
  jout("\t\tFatal Resets: %" PRIu32 "\n", e4.fatal_rst_cnt);
  jout("\t\tRead-Only Mode Events: %" PRIu32 "\n", e4.read_only_mode_cnt);
  jout("\t\tHost Side CRC Errors: %" PRIu32 "\n", e4.hs_crc_err_cnt);
  jout("\t\tPower-On Hours: %" PRIu32 "\n", e4.power_on_hours);
  jout("\t\tPower Cycle Count: %" PRIu32 "\n", e4.power_cycle_cnt);
  jout("\t\tLifetime Used: %" PRIu32 " %%\n", e4.lifetime_used);

  // Print plain-text health levels (GP Log 0xE5)
  jout("\tHealth Levels (GP Log 0xe5)\n");
  jout("\t\tOverall Health Level: %u (%s)\n", e5.health_level, ps3_ssd_level_desc(e5.health_level));
  jout("\t\tPLP Capacitance: %u (%s)\n", e5.plp_cap_err_level, ps3_ssd_level_desc(e5.plp_cap_err_level));
  jout("\t\tPLP Capacitor Voltage: %u (%s)\n", e5.plp_cap_volt_err_level, ps3_ssd_level_desc(e5.plp_cap_volt_err_level));
  jout("\t\tDDR Correctable Errors: %u (%s)\n", e5.ddr_ce_err_level, ps3_ssd_level_desc(e5.ddr_ce_err_level));
  jout("\t\tDDR Uncorrectable Errors: %u (%s)\n", e5.ddr_uce_err_level, ps3_ssd_level_desc(e5.ddr_uce_err_level));
  jout("\t\tSRAM Correctable Errors: %u (%s)\n", e5.sram_ce_err_level, ps3_ssd_level_desc(e5.sram_ce_err_level));
  jout("\t\tSRAM Uncorrectable Errors: %u (%s)\n", e5.sram_uce_err_level, ps3_ssd_level_desc(e5.sram_uce_err_level));
  jout("\t\tSensor Errors: %u (%s)\n", e5.sensor_err_level, ps3_ssd_level_desc(e5.sensor_err_level));
  jout("\t\tComposite Temperature: %u (%s)\n", e5.comp_temp_err_level, ps3_ssd_level_desc(e5.comp_temp_err_level));
  jout("\t\tCritical Component Temperature: %u (%s)\n", e5.critical_comp_err_level, ps3_ssd_level_desc(e5.critical_comp_err_level));
  jout("\t\tWarning Component Temperature: %u (%s)\n", e5.warning_comp_err_level, ps3_ssd_level_desc(e5.warning_comp_err_level));
  jout("\t\tMinimum Temperature: %u (%s)\n", e5.min_temp_err_level, ps3_ssd_level_desc(e5.min_temp_err_level));
  jout("\t\tMaximum Temperature: %u (%s)\n", e5.max_temp_err_level, ps3_ssd_level_desc(e5.max_temp_err_level));
  jout("\t\tSoC Temperature: %u (%s)\n", e5.soc_temp_err_level, ps3_ssd_level_desc(e5.soc_temp_err_level));
  jout("\t\tNAND Temperature: %u (%s)\n", e5.nand_temp_err_level, ps3_ssd_level_desc(e5.nand_temp_err_level));
  jout("\t\tNAND Erase Errors: %u (%s)\n", e5.erase_err_level, ps3_ssd_level_desc(e5.erase_err_level));
  jout("\t\tNAND Program Errors: %u (%s)\n", e5.program_err_level, ps3_ssd_level_desc(e5.program_err_level));
  jout("\t\tARD Correctable Read Errors: %u (%s)\n", e5.ard_err_good_level, ps3_ssd_level_desc(e5.ard_err_good_level));
  jout("\t\tMRD Correctable Read Errors: %u (%s)\n", e5.mrd_err_good_level, ps3_ssd_level_desc(e5.mrd_err_good_level));
  jout("\t\tTotal Bad Blocks: %u (%s)\n", e5.total_bad_blk_level, ps3_ssd_level_desc(e5.total_bad_blk_level));
  jout("\t\tNAND Media Errors: %u (%s)\n", e5.nand_media_err_level, ps3_ssd_level_desc(e5.nand_media_err_level));
  jout("\t\tARD Uncorrectable Read Errors: %u (%s)\n", e5.ard_err_fail_level, ps3_ssd_level_desc(e5.ard_err_fail_level));
  jout("\t\tMRD Uncorrectable Read Errors: %u (%s)\n", e5.mrd_err_fail_level, ps3_ssd_level_desc(e5.mrd_err_fail_level));
  jout("\t\tRAID Recovery Failures: %u (%s)\n", e5.raid_fail_level, ps3_ssd_level_desc(e5.raid_fail_level));
  jout("\t\tRAID Recovery Successes: %u (%s)\n", e5.raid_good_level, ps3_ssd_level_desc(e5.raid_good_level));
  jout("\t\tReallocated Sectors: %u (%s)\n", e5.realloc_sector_level, ps3_ssd_level_desc(e5.realloc_sector_level));
  jout("\t\tCurrent Pending Sectors: %u (%s)\n", e5.cur_pending_sector_level, ps3_ssd_level_desc(e5.cur_pending_sector_level));
  jout("\t\tSATA PHY Errors: %u (%s)\n", e5.sata_phy_err_level, ps3_ssd_level_desc(e5.sata_phy_err_level));
  jout("\t\tFIS CRC Errors: %u (%s)\n", e5.fis_crc_err_level, ps3_ssd_level_desc(e5.fis_crc_err_level));
  jout("\t\tLink Speed Downgrades: %u (%s)\n", e5.link_speed_downgrade_level, ps3_ssd_level_desc(e5.link_speed_downgrade_level));
  jout("\t\tUnsafe Shutdowns: %u (%s)\n", e5.unsafe_shutdown_level, ps3_ssd_level_desc(e5.unsafe_shutdown_level));
  jout("\t\tAvailable Spare Blocks: %u (%s)\n", e5.available_spare_level, ps3_ssd_level_desc(e5.available_spare_level));
  jout("\t\tHigh Latency Read Commands: %u (%s)\n", e5.high_lat_rcmd_ttl_level, ps3_ssd_level_desc(e5.high_lat_rcmd_ttl_level));
  jout("\t\tHigh Latency Write Commands: %u (%s)\n", e5.high_lat_wcmd_ttl_level, ps3_ssd_level_desc(e5.high_lat_wcmd_ttl_level));
  jout("\t\tFatal Resets: %u (%s)\n", e5.fatal_rst_level, ps3_ssd_level_desc(e5.fatal_rst_level));
  jout("\t\tRead-Only Mode Events: %u (%s)\n", e5.read_only_mode_level, ps3_ssd_level_desc(e5.read_only_mode_level));
  jout("\t\tHost Side CRC Errors: %u (%s)\n", e5.hs_crc_err_level, ps3_ssd_level_desc(e5.hs_crc_err_level));
  jout("\t\tNOR Critical Log Full Errors: %u (%s)\n", e5.nor_critical_log_full_err_level, ps3_ssd_level_desc(e5.nor_critical_log_full_err_level));
  jout("\t\tError Log Entry Errors: %u (%s)\n", e5.num_err_log_entry_err_level, ps3_ssd_level_desc(e5.num_err_log_entry_err_level));
  jout("\t\tPower-On Hours: %u (%s)\n", e5.power_on_hours_err_level, ps3_ssd_level_desc(e5.power_on_hours_err_level));
  jout("\t\tPower Cycle Count: %u (%s)\n", e5.power_cycle_err_level, ps3_ssd_level_desc(e5.power_cycle_err_level));
  jout("\t\tLifetime Used: %u (%s)\n", e5.lifetime_used_err_level, ps3_ssd_level_desc(e5.lifetime_used_err_level));

  // Print JSON if --json or -j is specified
  json::ref jref = jglb["ps3_ssd_log"];

  // Detailed counters (GP Log 0xE4)
  json::ref jref4 = jref["e4_detailed_counters"];
  jref4["log_revision"] = e4.revision_id;
  jref4["signature"] = sig;
  jref4["log_type"] = e4.type;
  jref4["health_level"] = e4.health_level;
  jref4["plp_capacitance"] = e4.plp_capacitance;
  jref4["plp_capacitor_volt"] = e4.plp_capacitor_volt;
  jref4["ddr_ce_err_count"] = e4.ddr_ce_err_count;
  jref4["ddr_uce_err_count"] = e4.ddr_uce_err_count;
  jref4["sram_ce_err_count"] = e4.sram_ce_err_count;
  jref4["sram_uce_err_count"] = e4.sram_uce_err_count;
  jref4["sensor_err_count"] = e4.sensor_err_count;
  jref4["composite_temp"] = e4.composite_temp;
  jref4["unc"] = e4.unc;
  jref4["unc_cnt"] = e4.unc_cnt;
  jref4["uc"] = e4.uc;
  jref4["uc_cnt"] = e4.uc_cnt;
  jref4["min_temp"] = e4.min_temp;
  jref4["max_temp"] = e4.max_temp;
  jref4["soc_temp"] = e4.soc_temp;
  jref4["nand_temp"] = e4.nand_temp;
  jref4["erase_err_cnt"] = e4.erase_err_cnt;
  jref4["program_err_cnt"] = e4.program_err_cnt;
  jref4["ard_err_good_cnt"] = e4.ard_err_good_cnt;
  jref4["mrd_err_good_cnt"] = e4.mrd_err_good_cnt;
  jref4["total_bad_blk_cnt"] = e4.total_bad_blk_cnt;
  jref4["nand_media_err_cnt"] = e4.nand_media_err_cnt;
  jref4["ard_err_fail_cnt"] = e4.ard_err_fail_cnt;
  jref4["mrd_err_fail_cnt"] = e4.mrd_err_fail_cnt;
  jref4["raid_fail_cnt"] = e4.raid_fail_cnt;
  jref4["raid_good_cnt"] = e4.raid_good_cnt;
  jref4["realloc_sector_cnt"] = e4.realloc_sector_cnt;
  jref4["cur_pending_sector_cnt"] = e4.cur_pending_sector_cnt;
  jref4["sata_phy_err_cnt"] = e4.sata_phy_err_cnt;
  jref4["fis_crc_err_cnt"] = e4.fis_crc_err_cnt;
  jref4["link_speed_downgrade_cnt"] = e4.link_speed_downgrade_cnt;
  jref4["unsafe_shutdown_cnt"] = e4.unsafe_shutdown_cnt;
  jref4["available_spare_cnt"] = e4.available_spare_cnt;
  jref4["high_lat_rcmd_ttl_cnt"] = e4.high_lat_rcmd_ttl_cnt;
  jref4["high_lat_wcmd_ttl_cnt"] = e4.high_lat_wcmd_ttl_cnt;
  jref4["fatal_rst_cnt"] = e4.fatal_rst_cnt;
  jref4["read_only_mode_cnt"] = e4.read_only_mode_cnt;
  jref4["hs_crc_err_cnt"] = e4.hs_crc_err_cnt;
  jref4["power_on_hours"] = e4.power_on_hours;
  jref4["power_cycle_cnt"] = e4.power_cycle_cnt;
  jref4["lifetime_used"] = e4.lifetime_used;

  // Health levels (GP Log 0xE5)
  json::ref jref5 = jref["e5_health_levels"];
  jref5["health_level"] = e5.health_level;
  jref5["plp_cap_err_level"] = e5.plp_cap_err_level;
  jref5["plp_cap_volt_err_level"] = e5.plp_cap_volt_err_level;
  jref5["ddr_ce_err_level"] = e5.ddr_ce_err_level;
  jref5["ddr_uce_err_level"] = e5.ddr_uce_err_level;
  jref5["sram_ce_err_level"] = e5.sram_ce_err_level;
  jref5["sram_uce_err_level"] = e5.sram_uce_err_level;
  jref5["sensor_err_level"] = e5.sensor_err_level;
  jref5["comp_temp_err_level"] = e5.comp_temp_err_level;
  jref5["critical_comp_err_level"] = e5.critical_comp_err_level;
  jref5["warning_comp_err_level"] = e5.warning_comp_err_level;
  jref5["min_temp_err_level"] = e5.min_temp_err_level;
  jref5["max_temp_err_level"] = e5.max_temp_err_level;
  jref5["soc_temp_err_level"] = e5.soc_temp_err_level;
  jref5["nand_temp_err_level"] = e5.nand_temp_err_level;
  jref5["erase_err_level"] = e5.erase_err_level;
  jref5["program_err_level"] = e5.program_err_level;
  jref5["ard_err_good_level"] = e5.ard_err_good_level;
  jref5["mrd_err_good_level"] = e5.mrd_err_good_level;
  jref5["total_bad_blk_level"] = e5.total_bad_blk_level;
  jref5["nand_media_err_level"] = e5.nand_media_err_level;
  jref5["ard_err_fail_level"] = e5.ard_err_fail_level;
  jref5["mrd_err_fail_level"] = e5.mrd_err_fail_level;
  jref5["raid_fail_level"] = e5.raid_fail_level;
  jref5["raid_good_level"] = e5.raid_good_level;
  jref5["realloc_sector_level"] = e5.realloc_sector_level;
  jref5["cur_pending_sector_level"] = e5.cur_pending_sector_level;
  jref5["sata_phy_err_level"] = e5.sata_phy_err_level;
  jref5["fis_crc_err_level"] = e5.fis_crc_err_level;
  jref5["link_speed_downgrade_level"] = e5.link_speed_downgrade_level;
  jref5["unsafe_shutdown_level"] = e5.unsafe_shutdown_level;
  jref5["available_spare_level"] = e5.available_spare_level;
  jref5["high_lat_rcmd_ttl_level"] = e5.high_lat_rcmd_ttl_level;
  jref5["high_lat_wcmd_ttl_level"] = e5.high_lat_wcmd_ttl_level;
  jref5["fatal_rst_level"] = e5.fatal_rst_level;
  jref5["read_only_mode_level"] = e5.read_only_mode_level;
  jref5["hs_crc_err_level"] = e5.hs_crc_err_level;
  jref5["nor_critical_log_full_err_level"] = e5.nor_critical_log_full_err_level;
  jref5["num_err_log_entry_err_level"] = e5.num_err_log_entry_err_level;
  jref5["power_on_hours_err_level"] = e5.power_on_hours_err_level;
  jref5["power_cycle_err_level"] = e5.power_cycle_err_level;
  jref5["lifetime_used_err_level"] = e5.lifetime_used_err_level;
}
