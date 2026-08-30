/*
 * ps3ssdcmds.h
 *
 * Home page of code is: https://www.smartmontools.org
 *
 * Copyright (C) 2026 the smartmontools project
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef SMARTMON_PS3SSDCMDS_H
#define SMARTMON_PS3SSDCMDS_H

#include <smartmon/smartmon_defs.h>

#include <stdint.h>

#include <smartmon/atacmds.h>
#include <smartmon/dev_interface.h>

namespace smartmon {

//////////////////////////////////////////////////////////////////////////////////
// SSD vendor health log structures (GP Log 0xE4 detailed counters / GP Log 0xE5
// health levels) as used by SSDs in the PS3 storage controller scenario.
// The logs are read via standard ATA READ LOG EXT (SATA/SAT), no vendor
// library or kernel driver changes are required.
//
// All multi-byte fields in the logs are little-endian.  Parsing is done with
// sg_get_unaligned_le* helpers so the host byte order does not matter.
//
// WARNING: This layout is NOT covered by any public documentation of the
// storage controller (the vendor documentation only describes the generic
// SMART information provided by 'ps3cli /cx /ex /sx show smart').  It is
// assumed to be defined by the SSD vendor.  Readers therefore check the
// vendor signature and refuse to report values from a log page whose layout
// does not match (see ataReadPs3SsdLogE4()).

// GP Log 0xE4: detailed reliability counters
struct ps3_ssd_e4_log {
  uint16_t revision_id;             // Log revision
  uint8_t signature[5];             // Vendor signature
  uint8_t type;                     // Log type
  uint8_t health_level;             // Overall health level (0 = ok .. 4 = fatal)
  uint8_t plp_capacitance;          // PLP capacitance (uF)
  uint16_t plp_capacitor_volt;      // PLP capacitor voltage (mV)
  uint32_t ddr_ce_err_count;        // DDR correctable errors
  uint32_t ddr_uce_err_count;       // DDR uncorrectable errors
  uint32_t sram_ce_err_count;       // SRAM correctable errors
  uint32_t sram_uce_err_count;      // SRAM uncorrectable errors
  uint32_t sensor_err_count;        // Sensor errors
  uint8_t composite_temp;           // Composite temperature (C)
  uint8_t unc;                      // Uncorrectable error type
  uint16_t unc_cnt;                 // Uncorrectable error count
  uint8_t uc;                       // Correctable error type
  uint16_t uc_cnt;                  // Correctable error count
  uint8_t min_temp;                 // Minimum temperature (C)
  uint8_t max_temp;                 // Maximum temperature (C)
  uint8_t soc_temp;                 // SoC temperature (C)
  uint8_t nand_temp;                // NAND temperature (C)
  uint32_t erase_err_cnt;           // NAND erase errors
  uint32_t program_err_cnt;         // NAND program errors
  uint32_t ard_err_good_cnt;        // ARD correctable read errors
  uint32_t mrd_err_good_cnt;        // MRD correctable read errors
  uint32_t total_bad_blk_cnt;       // Total bad blocks
  uint32_t nand_media_err_cnt;      // NAND media errors
  uint32_t ard_err_fail_cnt;        // ARD uncorrectable read errors
  uint32_t mrd_err_fail_cnt;        // MRD uncorrectable read errors
  uint32_t raid_fail_cnt;           // RAID recovery failures
  uint32_t raid_good_cnt;           // RAID recovery successes
  uint32_t realloc_sector_cnt;      // Reallocated sectors
  uint32_t cur_pending_sector_cnt;  // Current pending sectors
  uint32_t sata_phy_err_cnt;        // SATA PHY errors
  uint64_t fis_crc_err_cnt;         // FIS CRC errors
  uint32_t link_speed_downgrade_cnt;// Link speed downgrades
  uint32_t unsafe_shutdown_cnt;     // Unsafe shutdowns
  uint32_t available_spare_cnt;     // Available spare blocks
  uint32_t high_lat_rcmd_ttl_cnt;   // High latency read commands
  uint32_t high_lat_wcmd_ttl_cnt;   // High latency write commands
  uint32_t fatal_rst_cnt;           // Fatal resets
  uint32_t read_only_mode_cnt;      // Read-only mode events
  uint32_t hs_crc_err_cnt;          // Host side CRC errors
  uint32_t power_on_hours;          // Power-on hours
  uint32_t power_cycle_cnt;         // Power cycle count
  uint32_t lifetime_used;           // Lifetime used (%)
};

// GP Log 0xE5: health level per counter (0 = ok .. 4 = fatal)
struct ps3_ssd_e5_log {
  uint8_t health_level;                    // Overall health level
  uint8_t plp_cap_err_level;               // PLP capacitance
  uint8_t plp_cap_volt_err_level;          // PLP capacitor voltage
  uint8_t ddr_ce_err_level;                // DDR correctable errors
  uint8_t ddr_uce_err_level;               // DDR uncorrectable errors
  uint8_t sram_ce_err_level;               // SRAM correctable errors
  uint8_t sram_uce_err_level;              // SRAM uncorrectable errors
  uint8_t sensor_err_level;                // Sensor errors
  uint8_t comp_temp_err_level;             // Composite temperature
  uint8_t critical_comp_err_level;         // Critical component temperature
  uint8_t warning_comp_err_level;          // Warning component temperature
  uint8_t min_temp_err_level;              // Minimum temperature
  uint8_t max_temp_err_level;              // Maximum temperature
  uint8_t soc_temp_err_level;              // SoC temperature
  uint8_t nand_temp_err_level;             // NAND temperature
  uint8_t erase_err_level;                 // NAND erase errors
  uint8_t program_err_level;               // NAND program errors
  uint8_t ard_err_good_level;              // ARD correctable read errors
  uint8_t mrd_err_good_level;              // MRD correctable read errors
  uint8_t total_bad_blk_level;             // Total bad blocks
  uint8_t nand_media_err_level;            // NAND media errors
  uint8_t ard_err_fail_level;              // ARD uncorrectable read errors
  uint8_t mrd_err_fail_level;              // MRD uncorrectable read errors
  uint8_t raid_fail_level;                 // RAID recovery failures
  uint8_t raid_good_level;                 // RAID recovery successes
  uint8_t realloc_sector_level;            // Reallocated sectors
  uint8_t cur_pending_sector_level;        // Current pending sectors
  uint8_t sata_phy_err_level;              // SATA PHY errors
  uint8_t fis_crc_err_level;               // FIS CRC errors
  uint8_t link_speed_downgrade_level;      // Link speed downgrades
  uint8_t unsafe_shutdown_level;           // Unsafe shutdowns
  uint8_t available_spare_level;           // Available spare blocks
  uint8_t high_lat_rcmd_ttl_level;         // High latency read commands
  uint8_t high_lat_wcmd_ttl_level;         // High latency write commands
  uint8_t fatal_rst_level;                 // Fatal resets
  uint8_t read_only_mode_level;            // Read-only mode events
  uint8_t hs_crc_err_level;                // Host side CRC errors
  uint8_t nor_critical_log_full_err_level; // NOR critical log full errors
  uint8_t num_err_log_entry_err_level;     // Error log entry errors
  uint8_t power_on_hours_err_level;        // Power-on hours
  uint8_t power_cycle_err_level;           // Power cycle count
  uint8_t lifetime_used_err_level;         // Lifetime used
};

/*
 *  Reads and parses the SSD vendor health log (GP Log 0xE4) detailed
 *  reliability counters from the device.
 *
 *  @param  device:  Pointer to instantiated device object (ata_device*)
 *  @param  log:     Reference to parsed data structure (ps3_ssd_e4_log&)
 *  @return true if read and parse successful, false otherwise (bool)
 */
bool ataReadPs3SsdLogE4(ata_device * device, ps3_ssd_e4_log & log);

/*
 *  Reads and parses the SSD vendor health log (GP Log 0xE5) health levels
 *  from the device.
 *
 *  @param  device:  Pointer to instantiated device object (ata_device*)
 *  @param  log:     Reference to parsed data structure (ps3_ssd_e5_log&)
 *  @return true if read and parse successful, false otherwise (bool)
 */
bool ataReadPs3SsdLogE5(ata_device * device, ps3_ssd_e5_log & log);

} // namespace smartmon

#endif // SMARTMON_PS3SSDCMDS_H
