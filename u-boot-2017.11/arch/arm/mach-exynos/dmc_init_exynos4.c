/*
 * Memory setup for board based on EXYNOS4210
 *
 * Copyright (C) 2013 Samsung Electronics
 * Rajeshwari Shinde <rajeshwari.s@samsung.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <config.h>
#include <asm/arch/dmc.h>
#include "common_setup.h"

#ifdef CONFIG_ITOP4412
# include "itop4412_setup.h"
#else
# include "exynos4_setup.h"
#endif

struct mem_timings mem = {
	.direct_cmd_msr = {
		DIRECT_CMD1, DIRECT_CMD2, DIRECT_CMD3, DIRECT_CMD4
	},
	.timingref = TIMINGREF_VAL,
	.timingrow = TIMINGROW_VAL,
	.timingdata = TIMINGDATA_VAL,
	.timingpower = TIMINGPOWER_VAL,
	.zqcontrol = ZQ_CONTROL_VAL,
	.control0 = CONTROL0_VAL,
	.control1 = CONTROL1_VAL,
	.control2 = CONTROL2_VAL,
	.concontrol = CONCONTROL_VAL,
	.prechconfig = PRECHCONFIG,
	.memcontrol = MEMCONTROL_VAL,
	.memconfig0 = MEMCONFIG0_VAL,
	.memconfig1 = MEMCONFIG1_VAL,
	.dll_resync = FORCE_DLL_RESYNC,
	.dll_on = DLL_CONTROL_ON,
};


static void phy_control_reset(int ctrl_no, struct exynos4_dmc *dmc)
{
	if (ctrl_no) {
		writel((mem.control1 | (1 << mem.dll_resync)),
		       &dmc->phycontrol1);
		writel((mem.control1 | (0 << mem.dll_resync)),
		       &dmc->phycontrol1);
	} else {
		writel((mem.control0 | (0 << mem.dll_on)),
		       &dmc->phycontrol0);
		writel((mem.control0 | (1 << mem.dll_on)),
		       &dmc->phycontrol0);
	}
}


static void dmc_config_mrs(struct exynos4_dmc *dmc, int chip)
{
	int i;
	unsigned long mask = 0;

	if (chip)
		mask = DIRECT_CMD_CHIP1_SHIFT;

	for (i = 0; i < MEM_TIMINGS_MSR_COUNT; i++) {
		writel(mem.direct_cmd_msr[i] | mask,
		       &dmc->directcmd);
	}
}

static void dmc_init(struct exynos4_dmc *dmc)
{

	/*
	 * ZQ Calibration
	 * Termination: Disable
	 * Auto Calibration Start: Enable
	 */
	writel(mem.zqcontrol, &dmc->phyzqcontrol);
//	sdelay(0x100000);

	writel(0x71101008, &dmc->phycontrol0);
	writel(0x7110100a, &dmc->phycontrol0);
	writel(0x00000084, &dmc->phycontrol1);
	writel(0x71101008, &dmc->phycontrol0);
	writel(0x0000008c, &dmc->phycontrol1);
	writel(0x00000084, &dmc->phycontrol1);
	writel(0x0000008c, &dmc->phycontrol1);
	writel(0x00000084, &dmc->phycontrol1);

	writel(0x0fff30ca, &dmc->concontrol);

	writel(mem.memcontrol, &dmc->memcontrol);

	writel(mem.memconfig0, &dmc->memconfig0);

	writel(0x8000001D, &dmc->ivcontrol);
	/* Config Precharge Policy */
	writel(mem.prechconfig, &dmc->prechconfig);
	/*
	 * TimingAref, TimingRow, TimingData, TimingPower Setting:
	 * Values as per Memory AC Parameters
	 */

	writel(0x0000005d, &dmc->timingref);
	writel(0x34498691, &dmc->timingrow);
	writel(0x36330306, &dmc->timingdata);
	writel(0x50380365, &dmc->timingpower);
	sdelay(0x100000);
	/* Chip0: NOP Command: Assert and Hold CKE to high level */
	writel(0x07000000, &dmc->directcmd);
	sdelay(0x100000);

	writel(0x00071c00, &dmc->directcmd);
	sdelay(0x100000);

	writel(0x00010bfc, &dmc->directcmd);
	sdelay(0x100000);

	writel(0x00000488, &dmc->directcmd);
//	sdelay(0x100000);


	writel(0x00000810, &dmc->directcmd);
//	sdelay(0x100000);

	writel(0x00000c08, &dmc->directcmd);
//	sdelay(0x100000);

	/* turn on DREX0, DREX1 */
	writel(0x0FFF303a, &dmc->concontrol);
}

void mem_ctrl_init(int reset)
{
	struct exynos4_dmc *dmc;

	/*
	 * Async bridge configuration at CPU_core:
	 * 1: half_sync
	 * 0: full_sync
	 */
	writel(1, ASYNC_CONFIG);
#ifdef CONFIG_ITOP4412
	writel(0x13113113, 0x10030000 + 0x10500);

	writel(0x00117713, 0x10040500);
	writel(0x00000000, 0x10020a00);
	writel(0x00010905, 0x10040a00);
#else
	writel(APB_SFR_INTERLEAVE_CONF_VAL, EXYNOS4_MIU_BASE +
		APB_SFR_INTERLEAVE_CONF_OFFSET);
	writel(INTERLEAVE_ADDR_MAP_START_ADDR, EXYNOS4_MIU_BASE +
		ABP_SFR_INTERLEAVE_ADDRMAP_START_OFFSET);
	writel(INTERLEAVE_ADDR_MAP_END_ADDR, EXYNOS4_MIU_BASE +
		ABP_SFR_INTERLEAVE_ADDRMAP_END_OFFSET);
	writel(INTERLEAVE_ADDR_MAP_EN, EXYNOS4_MIU_BASE +
		ABP_SFR_SLV_ADDRMAP_CONF_OFFSET);
#ifdef CONFIG_MIU_LINEAR
	writel(SLAVE0_SINGLE_ADDR_MAP_START_ADDR, EXYNOS4_MIU_BASE +
		ABP_SFR_SLV0_SINGLE_ADDRMAP_START_OFFSET);
	writel(SLAVE0_SINGLE_ADDR_MAP_END_ADDR, EXYNOS4_MIU_BASE +
		ABP_SFR_SLV0_SINGLE_ADDRMAP_END_OFFSET);
	writel(SLAVE1_SINGLE_ADDR_MAP_START_ADDR, EXYNOS4_MIU_BASE +
		ABP_SFR_SLV1_SINGLE_ADDRMAP_START_OFFSET);
	writel(SLAVE1_SINGLE_ADDR_MAP_END_ADDR, EXYNOS4_MIU_BASE +
		ABP_SFR_SLV1_SINGLE_ADDRMAP_END_OFFSET);
	writel(APB_SFR_SLV_ADDR_MAP_CONF_VAL, EXYNOS4_MIU_BASE +
		ABP_SFR_SLV_ADDRMAP_CONF_OFFSET);
#endif
#endif
	/* DREX0 */
	dmc = (struct exynos4_dmc *)samsung_get_base_dmc_ctrl();
	dmc_init(dmc);
	dmc = (struct exynos4_dmc *)(samsung_get_base_dmc_ctrl()
					+ DMC_OFFSET);
	dmc_init(dmc);
}
