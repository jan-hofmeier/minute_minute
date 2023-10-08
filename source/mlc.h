/*
 *  minute - a port of the "mini" IOS replacement for the Wii U.
 *
 *  Copyright (C) 2016          SALT
 *  Copyright (C) 2016          Daz Jones <daz@dazzozo.com>
 *
 *  Copyright (C) 2008, 2009    Sven Peter <svenpeter@gmail.com>
 *
 *  This code is licensed to you under the terms of the GNU GPL, version 2;
 *  see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

#ifndef __MLC_H__
#define __MLC_H__

#include "bsdtypes.h"
#include "sdmmc.h"

struct mmc_ext_csd {
	u8			rev;
	u8			erase_group_def;
	u8			sec_feature_support;
	u8			rel_sectors;
	u8			rel_param;
	u8			part_config;
	u8			cache_ctrl;
	u8			rst_n_function;
	u8			max_packed_writes;
	u8			max_packed_reads;
	u8			packed_event_en;
	unsigned int		part_time;		/* Units: ms */
	unsigned int		sa_timeout;		/* Units: 100ns */
	unsigned int		sleep_notification_time; /* Units: 10us */
	unsigned int		generic_cmd6_time;	/* Units: 10ms */
	unsigned int            power_off_longtime;     /* Units: ms */
	u8			power_off_notification;	/* state */
	unsigned int		hs_max_dtr;
	unsigned int		hs200_max_dtr;
#define MMC_HIGH_26_MAX_DTR	26000000
#define MMC_HIGH_52_MAX_DTR	52000000
#define MMC_HIGH_DDR_MAX_DTR	52000000
#define MMC_HS200_MAX_DTR	200000000
	unsigned int		sectors;
	unsigned int		hc_erase_size;		/* In sectors */
	unsigned int		hc_erase_timeout;	/* In milliseconds */
	unsigned int		sec_trim_mult;	/* Secure trim multiplier  */
	unsigned int		sec_erase_mult;	/* Secure erase multiplier */
	unsigned int		trim_timeout;		/* In milliseconds */
	bool			partition_setting_completed;	/* enable bit */
	unsigned long long	enhanced_area_offset;	/* Units: Byte */
	unsigned int		enhanced_area_size;	/* Units: KB */
	unsigned int		cache_size;		/* Units: KB */
	bool			hpi_en;			/* HPI enablebit */
	bool			hpi;			/* HPI support bit */
	unsigned int		hpi_cmd;		/* cmd used as HPI */
	bool			bkops;		/* background support bit */
	bool			man_bkops_en;	/* manual bkops enable bit */
	bool			auto_bkops;	/* manual bkops support bit */
	bool			auto_bkops_en;	/* auto BKOPS enable bit */
	unsigned int            data_sector_size;       /* 512 bytes or 4KB */
	unsigned int            data_tag_unit_size;     /* DATA TAG UNIT size */
	unsigned int		boot_ro_lock;		/* ro lock support */
	bool			boot_ro_lockable;
	bool			ffu_capable;	/* Firmware upgrade support */
#define MMC_FIRMWARE_LEN 8
	u8			fwrev[MMC_FIRMWARE_LEN];  /* FW version */
	u8			raw_exception_status;	/* 54 */
	u8			raw_partition_support;	/* 160 */
	u8			raw_rpmb_size_mult;	/* 168 */
	u8			raw_erased_mem_count;	/* 181 */
	u8			strobe_support;		/* 184 */
	u8			raw_ext_csd_structure;	/* 194 */
	u8			raw_card_type;		/* 196 */
	u8			raw_driver_strength;	/* 197 */
	u8			out_of_int_time;	/* 198 */
	u8			raw_pwr_cl_52_195;	/* 200 */
	u8			raw_pwr_cl_26_195;	/* 201 */
	u8			raw_pwr_cl_52_360;	/* 202 */
	u8			raw_pwr_cl_26_360;	/* 203 */
	u8			raw_s_a_timeout;	/* 217 */
	u8			raw_hc_erase_gap_size;	/* 221 */
	u8			raw_erase_timeout_mult;	/* 223 */
	u8			raw_hc_erase_grp_size;	/* 224 */
	u8			raw_sec_trim_mult;	/* 229 */
	u8			raw_sec_erase_mult;	/* 230 */
	u8			raw_sec_feature_support;/* 231 */
	u8			raw_trim_mult;		/* 232 */
	u8			raw_pwr_cl_200_195;	/* 236 */
	u8			raw_pwr_cl_200_360;	/* 237 */
	u8			raw_pwr_cl_ddr_52_195;	/* 238 */
	u8			raw_pwr_cl_ddr_52_360;	/* 239 */
	u8			raw_pwr_cl_ddr_200_360;	/* 253 */
	u8			raw_bkops_status;	/* 246 */
	u8			raw_sectors[4];		/* 212 - 4 bytes */
	unsigned int firmware_version; /* 254 - 257 */
	u8			dev_left_time;		/* 267*/
	u8			dev_left_time_a;	/*268*/
	u8			dev_left_time_b;	/*269*/
	u8			pre_eol_info;		/* 267 */
	u8			device_life_time_est_typ_a;	/* 268 */
	u8			device_life_time_est_typ_b;	/* 269 */

#ifdef CONFIG_MTK_EMMC_CQ_SUPPORT
	#define MMC_CMDQ_MODE_EN	(1)
	u8			cmdq_support;
	u8			cmdq_mode_en;
	u8			cmdq_depth;
#endif
	u8			supported_modes; /* 493 */
	unsigned int            feature_support;
#define MMC_DISCARD_FEATURE	BIT(0)                  /* CMD38 feature */
};

void mlc_init(void);
void mlc_exit(void);
void mlc_irq(void);

void mlc_attach(sdmmc_chipset_handle_t handle);
void mlc_needs_discover(void);
int mlc_wait_data(void);

int mlc_select(void);
int mlc_check_card(void);
int mlc_ack_card(void);
u32 mlc_get_sectors(void);

int mlc_read(u32 blk_start, u32 blk_count, void *data);
int mlc_write(u32 blk_start, u32 blk_count, void *data);

int mlc_start_read(u32 blk_start, u32 blk_count, void *data, struct sdmmc_command* cmdbuf);
int mlc_end_read(struct sdmmc_command* cmdbuf);

int mlc_start_write(u32 blk_start, u32 blk_count, void *data, struct sdmmc_command* cmdbuf);
int mlc_end_write(struct sdmmc_command* cmdbuf);

int mlc_erase(void);

#endif
