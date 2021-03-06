/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2019 Western Digital Corporation or its affiliates.
 *
 * Authors:
 *   Anup Patel <anup.patel@wdc.com>
 */

#ifndef __SBI_PLATFORM_H__
#define __SBI_PLATFORM_H__

/** OpenSBI 32-bit platform version with:
 *  1. upper 16-bits as major number
 *  2. lower 16-bits as minor number
 */
#define SBI_PLATFORM_VERSION(Major, Minor) ((Major << 16) | Minor)

#ifndef __ASSEMBLY__
/** Possible feature flags of a platform */
enum sbi_platform_features {
	/** Platform has timer value */
	SBI_PLATFORM_HAS_TIMER_VALUE = (1 << 0),
	/** Platform has HART hotplug support */
	SBI_PLATFORM_HAS_HART_HOTPLUG = (1 << 1),
	/** Platform has PMP support */
	SBI_PLATFORM_HAS_PMP = (1 << 2),
	/** Platform has S-mode counter enable */
	SBI_PLATFORM_HAS_SCOUNTEREN = (1 << 3),
	/** Platform has M-mode counter enable */
	SBI_PLATFORM_HAS_MCOUNTEREN = (1 << 4),
	/** Platform has fault delegation support */
	SBI_PLATFORM_HAS_MFAULTS_DELEGATION = (1 << 5),
};

/** Default feature set for a platform */
#define SBI_PLATFORM_DEFAULT_FEATURES                                \
	(SBI_PLATFORM_HAS_TIMER_VALUE | SBI_PLATFORM_HAS_PMP |       \
	 SBI_PLATFORM_HAS_SCOUNTEREN | SBI_PLATFORM_HAS_MCOUNTEREN | \
	 SBI_PLATFORM_HAS_MFAULTS_DELEGATION)

/** Platform functions */
struct sbi_platform_operations {
	/** Platform early initialization */
	int (*early_init)(bool cold_boot);
	/** Platform final initialization */
	int (*final_init)(bool cold_boot);

	/** Get number of PMP regions for given HART */
	u32 (*pmp_region_count)(u32 hartid);
	/**
	 * Get PMP regions details (namely: protection, base address,
	 * and size) for given HART
	 */
	int (*pmp_region_info)(u32 hartid, u32 index, ulong *prot, ulong *addr,
			       ulong *log2size);

	/** Initialize the platform interrupt controller for current HART */
	int (*irqchip_init)(bool cold_boot);

	/** Send IPI to a target HART */
	void (*ipi_send)(u32 target_hart);
	/** Wait for target HART to acknowledge IPI */
	void (*ipi_sync)(u32 target_hart);
	/** Clear IPI for a target HART */
	void (*ipi_clear)(u32 target_hart);
	/** Initialize IPI for current HART */
	int (*ipi_init)(bool cold_boot);

	/** Get platform timer value */
	u64 (*timer_value)(void);
	/** Start platform timer event for current HART */
	void (*timer_event_start)(u64 next_event);
	/** Stop platform timer event for current HART */
	void (*timer_event_stop)(void);
	/** Initialize platform timer for current HART */
	int (*timer_init)(bool cold_boot);

	/** Reboot the platform */
	int (*system_reboot)(u32 type);
	/** Shutdown or poweroff the platform */
	int (*system_shutdown)(u32 type);
} __packed;

/** Representation of a platform */
struct sbi_platform {
	/**
	 * OpenSBI version this sbi_platform is based on.
	 * It's a 32-bit value where upper 16-bits are major number
	 * and lower 16-bits are minor number
	 */
	u32 opensbi_version;
	/**
	 * OpenSBI platform version released by vendor.
	 * It's a 32-bit value where upper 16-bits are major number
	 * and lower 16-bits are minor number
	 */
	u32 platform_version;
	/** Name of the platform */
	char name[64];
	/** Supported features */
	u64 features;
	/** Mask representing the set of disabled HARTs */
	u64 disabled_hart_mask;
	/** Pointer to sbi platform operations */
	unsigned long platform_ops_addr;
	/** Pointer to system firmware specific context */
	unsigned long firmware_context;
} __packed;

/** Get pointer to sbi_platform for sbi_scratch pointer */
#define sbi_platform_ptr(__s) \
	((const struct sbi_platform *)((__s)->platform_addr))
/** Get pointer to sbi_platform for current HART */
#define sbi_platform_thishart_ptr() ((const struct sbi_platform *) \
	(sbi_scratch_thishart_ptr()->platform_addr))
/** Get pointer to platform_ops_addr from platform pointer **/
#define sbi_platform_ops(__p) \
	((const struct sbi_platform_operations *)(__p)->platform_ops_addr)

/** Check whether the platform supports timer value */
#define sbi_platform_has_timer_value(__p) \
	((__p)->features & SBI_PLATFORM_HAS_TIMER_VALUE)
/** Check whether the platform supports HART hotplug */
#define sbi_platform_has_hart_hotplug(__p) \
	((__p)->features & SBI_PLATFORM_HAS_HART_HOTPLUG)
/** Check whether the platform has PMP support */
#define sbi_platform_has_pmp(__p) ((__p)->features & SBI_PLATFORM_HAS_PMP)
/** Check whether the platform supports scounteren CSR */
#define sbi_platform_has_scounteren(__p) \
	((__p)->features & SBI_PLATFORM_HAS_SCOUNTEREN)
/** Check whether the platform supports mcounteren CSR */
#define sbi_platform_has_mcounteren(__p) \
	((__p)->features & SBI_PLATFORM_HAS_MCOUNTEREN)
/** Check whether the platform supports fault delegation */
#define sbi_platform_has_mfaults_delegation(__p) \
	((__p)->features & SBI_PLATFORM_HAS_MFAULTS_DELEGATION)

/**
 * Get name of the platform
 *
 * @param plat pointer to struct sbi_platform
 *
 * @return pointer to platform name on success and NULL on failure
 */
static inline const char *sbi_platform_name(const struct sbi_platform *plat)
{
	if (plat)
		return plat->name;
	return NULL;
}

/**
 * Check whether the given HART is disabled
 *
 * @param plat pointer to struct sbi_platform
 * @param hartid HART ID
 *
 * @return TRUE if HART is disabled and FALSE otherwise
 */
static inline bool sbi_platform_hart_disabled(const struct sbi_platform *plat,
					      u32 hartid)
{
	if (plat && (plat->disabled_hart_mask & (1 << hartid)))
		return true;
	return false;
}

/**
 * Early initialization for current HART
 *
 * @param plat pointer to struct sbi_platform
 * @param cold_boot whether cold boot (TRUE) or warm_boot (FALSE)
 *
 * @return 0 on success and negative error code on failure
 */
static inline int sbi_platform_early_init(const struct sbi_platform *plat,
					  bool cold_boot)
{
	if (plat && sbi_platform_ops(plat)->early_init)
		return sbi_platform_ops(plat)->early_init(cold_boot);
	return 0;
}

/**
 * Final initialization for current HART
 *
 * @param plat pointer to struct sbi_platform
 * @param cold_boot whether cold boot (TRUE) or warm_boot (FALSE)
 *
 * @return 0 on success and negative error code on failure
 */
static inline int sbi_platform_final_init(const struct sbi_platform *plat,
					  bool cold_boot)
{
	if (plat && sbi_platform_ops(plat)->final_init)
		return sbi_platform_ops(plat)->final_init(cold_boot);
	return 0;
}

/**
 * Get the number of PMP regions of a HART
 *
 * @param plat pointer to struct sbi_platform
 * @param hartid HART ID
 *
 * @return number of PMP regions
 */
static inline u32 sbi_platform_pmp_region_count(const struct sbi_platform *plat,
						u32 hartid)
{
	if (plat && sbi_platform_ops(plat)->pmp_region_count)
		return sbi_platform_ops(plat)->pmp_region_count(hartid);
	return 0;
}

/**
 * Get PMP regions details (namely: protection, base address,
 * and size) of a HART
 *
 * @param plat pointer to struct sbi_platform
 * @param hartid HART ID
 * @param index index of PMP region for which we want details
 * @param prot output pointer for PMP region protection
 * @param addr output pointer for PMP region base address
 * @param log2size output pointer for log-of-2 PMP region size
 *
 * @return 0 on success and negative error code on failure
 */
static inline int sbi_platform_pmp_region_info(const struct sbi_platform *plat,
						u32 hartid, u32 index,
						ulong *prot, ulong *addr,
						ulong *log2size)
{
	if (plat && sbi_platform_ops(plat)->pmp_region_info)
		return sbi_platform_ops(plat)->pmp_region_info(hartid,
							       index,
							       prot,
							       addr,
							       log2size);
	return 0;
}

/**
 * Initialize the platform interrupt controller for current HART
 *
 * @param plat pointer to struct sbi_platform
 * @param cold_boot whether cold boot (TRUE) or warm_boot (FALSE)
 *
 * @return 0 on success and negative error code on failure
 */
static inline int sbi_platform_irqchip_init(const struct sbi_platform *plat,
					    bool cold_boot)
{
	if (plat && sbi_platform_ops(plat)->irqchip_init)
		return sbi_platform_ops(plat)->irqchip_init(cold_boot);
	return 0;
}

/**
 * Send IPI to a target HART
 *
 * @param plat pointer to struct sbi_platform
 * @param target_hart HART ID of IPI target
 */
static inline void sbi_platform_ipi_send(const struct sbi_platform *plat,
					 u32 target_hart)
{
	if (plat && sbi_platform_ops(plat)->ipi_send)
		sbi_platform_ops(plat)->ipi_send(target_hart);
}

/**
 * Wait for target HART to acknowledge IPI
 *
 * @param plat pointer to struct sbi_platform
 * @param target_hart HART ID of IPI target
 */
static inline void sbi_platform_ipi_sync(const struct sbi_platform *plat,
					 u32 target_hart)
{
	if (plat && sbi_platform_ops(plat)->ipi_sync)
		sbi_platform_ops(plat)->ipi_sync(target_hart);
}

/**
 * Clear IPI for a target HART
 *
 * @param plat pointer to struct sbi_platform
 * @param target_hart HART ID of IPI target
 */
static inline void sbi_platform_ipi_clear(const struct sbi_platform *plat,
					  u32 target_hart)
{
	if (plat && sbi_platform_ops(plat)->ipi_clear)
		sbi_platform_ops(plat)->ipi_clear(target_hart);
}

/**
 * Initialize the platform IPI support for current HART
 *
 * @param plat pointer to struct sbi_platform
 * @param cold_boot whether cold boot (TRUE) or warm_boot (FALSE)
 *
 * @return 0 on success and negative error code on failure
 */
static inline int sbi_platform_ipi_init(const struct sbi_platform *plat,
					bool cold_boot)
{
	if (plat && sbi_platform_ops(plat)->ipi_init)
		return sbi_platform_ops(plat)->ipi_init(cold_boot);
	return 0;
}

/**
 * Get platform timer value
 *
 * @param plat pointer to struct sbi_platform
 *
 * @return 64bit timer value
 */
static inline u64 sbi_platform_timer_value(const struct sbi_platform *plat)
{
	if (plat && sbi_platform_ops(plat)->timer_value)
		return sbi_platform_ops(plat)->timer_value();
	return 0;
}

/**
 * Start platform timer event for current HART
 *
 * @param plat pointer to struct struct sbi_platform
 * @param next_event timer value when timer event will happen
 */
static inline void
sbi_platform_timer_event_start(const struct sbi_platform *plat, u64 next_event)
{
	if (plat && sbi_platform_ops(plat)->timer_event_start)
		sbi_platform_ops(plat)->timer_event_start(next_event);
}

/**
 * Stop platform timer event for current HART
 *
 * @param plat pointer to struct sbi_platform
 */
static inline void
sbi_platform_timer_event_stop(const struct sbi_platform *plat)
{
	if (plat && sbi_platform_ops(plat)->timer_event_stop)
		sbi_platform_ops(plat)->timer_event_stop();
}

/**
 * Initialize the platform timer for current HART
 *
 * @param plat pointer to struct sbi_platform
 * @param cold_boot whether cold boot (TRUE) or warm_boot (FALSE)
 *
 * @return 0 on success and negative error code on failure
 */
static inline int sbi_platform_timer_init(const struct sbi_platform *plat,
					  bool cold_boot)
{
	if (plat && sbi_platform_ops(plat)->timer_init)
		return sbi_platform_ops(plat)->timer_init(cold_boot);
	return 0;
}

/**
 * Reboot the platform
 *
 * @param plat pointer to struct sbi_platform
 * @param type type of reboot
 *
 * @return 0 on success and negative error code on failure
 */
static inline int sbi_platform_system_reboot(const struct sbi_platform *plat,
					     u32 type)
{
	if (plat && sbi_platform_ops(plat)->system_reboot)
		return sbi_platform_ops(plat)->system_reboot(type);
	return 0;
}

/**
 * Shutdown or poweroff the platform
 *
 * @param plat pointer to struct sbi_platform
 * @param type type of shutdown or poweroff
 *
 * @return 0 on success and negative error code on failure
 */
static inline int sbi_platform_system_shutdown(const struct sbi_platform *plat,
					       u32 type)
{
	if (plat && sbi_platform_ops(plat)->system_shutdown)
		return sbi_platform_ops(plat)->system_shutdown(type);
	return 0;
}
#endif

#endif
