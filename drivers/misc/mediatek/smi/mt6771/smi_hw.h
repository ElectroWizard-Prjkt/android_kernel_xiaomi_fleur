/*
 * Copyright (C) 2018 MediaTek Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __SMI_HW_H__
#define __SMI_HW_H__

#include <clk-mt6771-pg.h>

#define SMI_OSTD_MAX		(0x1f)

#define SMI_COMM_MASTER_NUM	(8)
#define SMI_LARB_NUM		(7)
#define SMI_LARB0_PORT_NUM	(10)	/* SYS_DIS */
#define SMI_LARB1_PORT_NUM	(7)	/* SYS_VDE */
#define SMI_LARB2_PORT_NUM	(3)	/* SYS_ISP */
#define SMI_LARB3_PORT_NUM	(5)	/* SYS_CAM */
#define SMI_LARB4_PORT_NUM	(11)	/* SYS_VEN */
#define SMI_LARB5_PORT_NUM	(25)	/* SYS_ISP */
#define SMI_LARB6_PORT_NUM	(31)	/* SYS_CAM */

static const u32 smi_subsys_to_larbs[NR_SYSS] = {
	[SYS_DIS] = ((1 << 0) | (1 << (SMI_LARB_NUM))),
	[SYS_VDE] = (1 << 1),
	[SYS_ISP] = ((1 << 2) | (1 << 5)),
	[SYS_CAM] = ((1 << 3) | (1 << 6)),
	[SYS_VEN] = (1 << 4),
};

#if IS_ENABLED(CONFIG_MMPROFILE)
#include <mmprofile.h>

static const char *smi_mmp_name[NR_SYSS] = {
	[SYS_DIS] = "DIS", [SYS_VDE] = "VDE",
	[SYS_ISP] = "ISP", [SYS_CAM] = "CAM", [SYS_VEN] = "VEN",
};
static mmp_event smi_mmp_event[NR_SYSS];
#endif

static const bool
SMI_COMM_BUS_SEL[SMI_COMM_MASTER_NUM] = {0, 1, 1, 0, 0, 1, 0, 1,};

static const u32
SMI_LARB_L1ARB[SMI_LARB_NUM] = {0, 7, 5, 6, 1, 2, 3,};
#endif