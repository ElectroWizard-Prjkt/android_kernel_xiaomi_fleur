/*
 * Copyright (C) 2015 MediaTek Inc.
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

#include <linux/sched.h>
#include <linux/dma-mapping.h>
#include "val_types_private.h"
/*#include "val_log.h"*/
#include "drv_api.h"

/* #define VCODEC_DEBUG */
#ifdef VCODEC_DEBUG
#undef VCODEC_DEBUG
#define VCODEC_DEBUG pr_debug
#undef pr_debug
#define pr_debug pr_debug
#else
#define VCODEC_DEBUG(...)
#undef pr_debug
#define pr_debug(...)
#endif

/* ============================================================== */
/* For Hybrid HW */
/* spinlock : OalHWContextLock */
struct VAL_VCODEC_OAL_HW_CONTEXT_T oal_hw_context[VCODEC_MULTIPLE_INSTANCE_NUM];
/* mutex : NonCacheMemoryListLock */
struct VAL_NON_CACHE_MEMORY_LIST_T grNonCacheMemoryList[VCODEC_MULTIPLE_INSTANCE_NUM_x_10];

/* For both hybrid and pure HW */
struct VAL_VCODEC_HW_LOCK_T grVcodecHWLock;	/* mutex : HWLock */

unsigned int gu4LockDecHWCount;	/* spinlock : LockDecHWCountLock */
unsigned int gu4LockEncHWCount;	/* spinlock : LockEncHWCountLock */
unsigned int gu4DecISRCount;	/* spinlock : DecISRCountLock */
unsigned int gu4EncISRCount;	/* spinlock : EncISRCountLock */


int search_HWLockSlot_ByTID(unsigned long ulpa, unsigned int curr_tid)
{
	int i;
	int j;

	for (i = 0; i < VCODEC_MULTIPLE_INSTANCE_NUM; i++) {
		if (oal_hw_context[i].u4VCodecThreadNum != VCODEC_THREAD_MAX_NUM) {
			for (j = 0; j < oal_hw_context[i].u4VCodecThreadNum; j++) {
				if (oal_hw_context[i].u4VCodecThreadID[j] == curr_tid) {
					pr_debug("[VCODEC][%s]\n",
						__func__);
					pr_debug("Lookup curr HW Locker is ObjId %d in index%d\n",
						 curr_tid, i);
					return i;
				}
			}
		}
	}

	return -1;
}

int search_HWLockSlot_ByHandle(unsigned long ulpa, unsigned long handle)
{
	int i;

	for (i = 0; i < VCODEC_MULTIPLE_INSTANCE_NUM; i++) {
		if (oal_hw_context[i].pvHandle == handle) {
			/* Add one line comment for avoid kernel coding style, WARNING:BRACES: */
			return i;
		}
	}

	/* dump debug info */
	pr_debug("search_HWLockSlot_ByHandle");
	for (i = 0; i < VCODEC_MULTIPLE_INSTANCE_NUM / 2; i++) {
/* Add one line comment for avoid kernel coding style, WARNING:BRACES: */
		pr_debug("[%d] 0x%lx", i, oal_hw_context[i].pvHandle);
	}

	return -1;
}

struct VAL_VCODEC_OAL_HW_CONTEXT_T *setCurr_HWLockSlot(unsigned long ulpa,
	unsigned int tid)
{

	int i, j;

	/* Dump current ObjId */
	for (i = 0; i < VCODEC_MULTIPLE_INSTANCE_NUM; i++) {
		/* Add one line comment for avoid kernel coding style, WARNING:BRACES: */
		pr_debug("[VCODEC] Dump curr slot %d ObjId 0x%lx\n",
			i, oal_hw_context[i].ObjId);
	}

	/* check if current ObjId exist in oal_hw_context[i].ObjId */
	for (i = 0; i < VCODEC_MULTIPLE_INSTANCE_NUM; i++) {
		if (oal_hw_context[i].ObjId == ulpa) {
			pr_debug("[VCODEC] Curr Already exist in %d Slot\n", i);
			return &oal_hw_context[i];
		}
	}

	/* if not exist in table,  find a new free slot and put it */
	for (i = 0; i < VCODEC_MULTIPLE_INSTANCE_NUM; i++) {
		if (oal_hw_context[i].u4VCodecThreadNum != VCODEC_THREAD_MAX_NUM) {
			for (j = 0; j < oal_hw_context[i].u4VCodecThreadNum; j++) {
				if (oal_hw_context[i].u4VCodecThreadID[j] == current->pid) {
					oal_hw_context[i].ObjId = ulpa;
					pr_debug("[VCODEC][%s] setCurr %d Slot\n",
						__func__, i);
					return &oal_hw_context[i];
				}
			}
		}
	}

	pr_err("[VCODEC][ERROR] %s All %d Slots unavaliable\n",
		__func__, VCODEC_MULTIPLE_INSTANCE_NUM);
	oal_hw_context[0].u4VCodecThreadNum = VCODEC_THREAD_MAX_NUM - 1;
	for (i = 0; i < oal_hw_context[0].u4VCodecThreadNum; i++) {
		/* Add one line comment for avoid kernel coding style, WARNING:BRACES: */
		oal_hw_context[0].u4VCodecThreadID[i] = current->pid;
	}
	return &oal_hw_context[0];
}


struct VAL_VCODEC_OAL_HW_CONTEXT_T *setCurr_HWLockSlot_Thread_ID(
	struct VAL_VCODEC_THREAD_ID_T a_prVcodecThreadID,
	unsigned int *a_prIndex)
{
	int i;
	int j;
	int k;

	/* Dump current tids */
	for (i = 0; i < VCODEC_MULTIPLE_INSTANCE_NUM; i++) {
		if (oal_hw_context[i].u4VCodecThreadNum != VCODEC_THREAD_MAX_NUM) {
			for (j = 0; j < oal_hw_context[i].u4VCodecThreadNum; j++) {
				pr_debug("[VCODEC][%s]\n", __func__);
				pr_debug("Dump curr slot %d, ThreadID[%d] = %d\n",
					i, j,
					oal_hw_context[i].u4VCodecThreadID[j]);
			}
		}
	}

	for (i = 0; i < a_prVcodecThreadID.u4VCodecThreadNum; i++) {
		pr_debug("[VCODEC][%s] VCodecThreadNum = %d, VCodecThreadID = %d\n",
		     __func__, a_prVcodecThreadID.u4VCodecThreadNum,
		     a_prVcodecThreadID.u4VCodecThreadID[i]);
	}

	/* check if current tids exist in oal_hw_context[i].ObjId */
	for (i = 0; i < VCODEC_MULTIPLE_INSTANCE_NUM; i++) {
		if (oal_hw_context[i].u4VCodecThreadNum != VCODEC_THREAD_MAX_NUM) {
			for (j = 0; j < oal_hw_context[i].u4VCodecThreadNum; j++) {
				for (k = 0; k < a_prVcodecThreadID.u4VCodecThreadNum; k++) {
					if (oal_hw_context[i].u4VCodecThreadID[j] ==
					    a_prVcodecThreadID.u4VCodecThreadID[k]) {
						pr_debug("[VCODEC][%s]\n",
							__func__);
						pr_debug("Curr Already exist in %d Slot\n",
							i);
						*a_prIndex = i;
						return &oal_hw_context[i];
					}
				}
			}
		}
	}

	/* if not exist in table,  find a new free slot and put it */
	for (i = 0; i < VCODEC_MULTIPLE_INSTANCE_NUM; i++) {
		if (oal_hw_context[i].u4VCodecThreadNum == VCODEC_THREAD_MAX_NUM) {
			oal_hw_context[i].u4VCodecThreadNum = a_prVcodecThreadID.u4VCodecThreadNum;
			for (j = 0; j < a_prVcodecThreadID.u4VCodecThreadNum; j++) {
				oal_hw_context[i].u4VCodecThreadID[j] =
				    a_prVcodecThreadID.u4VCodecThreadID[j];
				pr_debug("[VCODEC][%s] setCurr %d Slot, %d\n",
					__func__, i,
					oal_hw_context[i].u4VCodecThreadID[j]);
			}
			*a_prIndex = i;
			return &oal_hw_context[i];
		}
	}

	{
		pr_err("[VCODEC][ERROR] %s All %d Slots unavaliable\n",
			__func__, VCODEC_MULTIPLE_INSTANCE_NUM);
		oal_hw_context[0].u4VCodecThreadNum = a_prVcodecThreadID.u4VCodecThreadNum;
		for (i = 0; i < oal_hw_context[0].u4VCodecThreadNum; i++) {
			/* Add one line comment for avoid kernel coding style, WARNING:BRACES: */
			oal_hw_context[0].u4VCodecThreadID[i] =
			    a_prVcodecThreadID.u4VCodecThreadID[i];
		}
		*a_prIndex = 0;
		return &oal_hw_context[0];
	}
}


struct VAL_VCODEC_OAL_HW_CONTEXT_T *freeCurr_HWLockSlot(unsigned long ulpa)
{
	int i;
	int j;

	/* check if current ObjId exist in oal_hw_context[i].ObjId */

	for (i = 0; i < VCODEC_MULTIPLE_INSTANCE_NUM; i++) {
		if (oal_hw_context[i].ObjId == ulpa) {
			oal_hw_context[i].ObjId = -1L;
			for (j = 0; j < oal_hw_context[i].u4VCodecThreadNum; j++) {
				/* Add one line comment for avoid kernel coding style, WARNING:BRACES: */
				oal_hw_context[i].u4VCodecThreadID[j] = -1;
			}
			oal_hw_context[i].u4VCodecThreadNum =
				VCODEC_THREAD_MAX_NUM;
			oal_hw_context[i].Oal_HW_reg =
				(struct VAL_VCODEC_OAL_HW_REGISTER_T  *)0;
			pr_debug("[VCODEC] %s %d Slot\n", __func__, i);
			return &oal_hw_context[i];
		}
	}

	pr_err("[VCODEC][ERROR] %s can't find pid in HWLockSlot\n", __func__);
	return 0;
}


void Add_NonCacheMemoryList(unsigned long a_ulKVA,
			    unsigned long a_ulKPA,
			    unsigned long a_ulSize,
			    unsigned int a_u4VCodecThreadNum,
			    unsigned int *a_pu4VCodecThreadID)
{
	unsigned int u4I = 0;
	unsigned int u4J = 0;

	pr_debug("[VCODEC] %s +, KVA = 0x%lx, KPA = 0x%lx, Size = 0x%lx\n",
		__func__, a_ulKVA, a_ulKPA, a_ulSize);

	for (u4I = 0; u4I < VCODEC_MULTIPLE_INSTANCE_NUM_x_10; u4I++) {
		if ((grNonCacheMemoryList[u4I].ulKVA == -1L)
		    && (grNonCacheMemoryList[u4I].ulKPA == -1L)) {
			pr_debug("[VCODEC] %s index = %d, VCodecThreadNum = %d, curr_tid = %d\n",
				__func__, u4I, a_u4VCodecThreadNum,
				current->pid);

			grNonCacheMemoryList[u4I].u4VCodecThreadNum = a_u4VCodecThreadNum;
			for (u4J = 0; u4J < grNonCacheMemoryList[u4I].u4VCodecThreadNum; u4J++) {
				grNonCacheMemoryList[u4I].u4VCodecThreadID[u4J] =
				    *(a_pu4VCodecThreadID + u4J);
				pr_debug("[VCODEC][%s] VCodecThreadNum = %d, VCodecThreadID = %d\n",
				     __func__,
				grNonCacheMemoryList[u4I].u4VCodecThreadNum,
			grNonCacheMemoryList[u4I].u4VCodecThreadID[u4J]);
			}

			grNonCacheMemoryList[u4I].ulKVA = a_ulKVA;
			grNonCacheMemoryList[u4I].ulKPA = a_ulKPA;
			grNonCacheMemoryList[u4I].ulSize = a_ulSize;
			break;
		}
	}

	if (u4I == VCODEC_MULTIPLE_INSTANCE_NUM_x_10) {
		/* Add one line comment for avoid kernel coding style, WARNING:BRACES: */
		pr_err("[VCODEC][ERROR] CAN'T ADD %s, List is FULL!!\n",
			__func__);
	}

	pr_debug("[VCODEC] %s -\n", __func__);
}

void Free_NonCacheMemoryList(unsigned long a_ulKVA, unsigned long a_ulKPA)
{
	unsigned int u4I = 0;
	unsigned int u4J = 0;

	pr_debug("[VCODEC] %s +, KVA = 0x%lx, KPA = 0x%lx\n",
		__func__, a_ulKVA, a_ulKPA);

	for (u4I = 0; u4I < VCODEC_MULTIPLE_INSTANCE_NUM_x_10; u4I++) {
		if ((grNonCacheMemoryList[u4I].ulKVA == a_ulKVA)
		    && (grNonCacheMemoryList[u4I].ulKPA == a_ulKPA)) {
			pr_debug("[VCODEC] Free %s index = %d\n",
				__func__, u4I);
			grNonCacheMemoryList[u4I].u4VCodecThreadNum = VCODEC_THREAD_MAX_NUM;
			for (u4J = 0; u4J < VCODEC_THREAD_MAX_NUM; u4J++) {
				/* Add one line comment for avoid kernel coding style, WARNING:BRACES: */
				grNonCacheMemoryList[u4I].u4VCodecThreadID[u4J] = 0xffffffff;
			}

			grNonCacheMemoryList[u4I].ulKVA = -1L;
			grNonCacheMemoryList[u4I].ulKPA = -1L;
			grNonCacheMemoryList[u4I].ulSize = -1L;
			break;
		}
	}

	if (u4I == VCODEC_MULTIPLE_INSTANCE_NUM_x_10) {
		/* Add one line comment for avoid kernel coding style, WARNING:BRACES: */
		pr_err("[VCODEC][ERROR] CAN'T Free %s, Address is not find!!\n",
			__func__);
	}

	pr_debug("[VCODEC]%s -\n", __func__);
}


void Force_Free_NonCacheMemoryList(unsigned int a_u4Tid)
{
	unsigned int u4I = 0;
	unsigned int u4J = 0;
	unsigned int u4K = 0;

	pr_debug("[VCODEC] %s +, curr_id = %d", __func__, a_u4Tid);

	for (u4I = 0; u4I < VCODEC_MULTIPLE_INSTANCE_NUM_x_10; u4I++) {
		if (grNonCacheMemoryList[u4I].u4VCodecThreadNum != VCODEC_THREAD_MAX_NUM) {
			for (u4J = 0; u4J < grNonCacheMemoryList[u4I].u4VCodecThreadNum; u4J++) {
				if (grNonCacheMemoryList[u4I].u4VCodecThreadID[u4J] == a_u4Tid) {
					pr_debug("[VCODEC][WARNING] %s\n",
						__func__);
					pr_debug("index = %d, tid = %d, KVA = 0x%lx, KPA = 0x%lx, Size = %lu\n",
					     u4I, a_u4Tid, grNonCacheMemoryList[u4I].ulKVA,
					     grNonCacheMemoryList[u4I].ulKPA,
					     grNonCacheMemoryList[u4I].ulSize);

					dma_free_coherent(0,
							  grNonCacheMemoryList[u4I].ulSize,
							  (void *)grNonCacheMemoryList[u4I].ulKVA,
							  (dma_addr_t) grNonCacheMemoryList[u4I].
							  ulKPA);

					grNonCacheMemoryList[u4I].u4VCodecThreadNum =
					    VCODEC_THREAD_MAX_NUM;
					for (u4K = 0; u4K < VCODEC_THREAD_MAX_NUM; u4K++) {
						/* Add one line comment for avoid kernel coding style,
						 * WARNING:BRACES:
						 */
						grNonCacheMemoryList[u4I].u4VCodecThreadID[u4K] =
						    0xffffffff;
					}
					grNonCacheMemoryList[u4I].ulKVA = -1L;
					grNonCacheMemoryList[u4I].ulKPA = -1L;
					grNonCacheMemoryList[u4I].ulSize = -1L;
					break;
				}
			}
		}
	}

	pr_debug("[VCODEC] %s -, curr_id = %d", __func__, a_u4Tid);
}

unsigned long Search_NonCacheMemoryList_By_KPA(unsigned long a_ulKPA)
{
	unsigned int u4I = 0;
	unsigned long ulVA_Offset = 0;

	ulVA_Offset = a_ulKPA & 0x0000000000000fff;

	pr_debug("[VCODEC] %s +, KPA = 0x%lx, ulVA_Offset = 0x%lx\n", __func__,
		 a_ulKPA, ulVA_Offset);

	for (u4I = 0; u4I < VCODEC_MULTIPLE_INSTANCE_NUM_x_10; u4I++) {
		if (grNonCacheMemoryList[u4I].ulKPA == (a_ulKPA - ulVA_Offset)) {
			pr_debug("[VCODEC] Find %s index = %d\n",
				__func__, u4I);
			break;
		}
	}

	if (u4I == VCODEC_MULTIPLE_INSTANCE_NUM_x_10) {
		pr_err("[VCODEC][ERROR] CAN'T Find %s, Address is not find!!\n",
			__func__);
		return grNonCacheMemoryList[0].ulKVA + ulVA_Offset;
	}

	pr_debug("[VCODEC] %s, ulVA = 0x%lx -\n", __func__,
		 (grNonCacheMemoryList[u4I].ulKVA + ulVA_Offset));

	return grNonCacheMemoryList[u4I].ulKVA + ulVA_Offset;
}