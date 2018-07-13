#ifndef __REG_ARM64_H_INCLUDE__
#define __REG_ARM64_H_INCLUDE__

#include <target/config.h>
#include <target/types.h>
#include <asm/io.h>
#include <asm/mach/reg.h>

/* =================================================================
 * D.7.2 General system control registers
 * ================================================================= */
/* D.7.2.18 CPTR_EL2/3, Architectural Feature Trap Register */
#define CPTR_TFP		_BV(10)
#define CPTR_TTA		_BV(20)
#define CPTR_TCPAC		_BV(31)

#define CPTR_EL2_RES0				\
	(_BV(11)|_BV(14)|_BV(15)|_BV(16)|	\
	 _BV(17)|_BV(18)|_BV(19)|_BV(21)|	\
	 _BV(22)|_BV(23)|_BV(24)|_BV(25)|	\
	 _BV(26)|_BV(27)|_BV(28)|_BV(29)|_BV(30))
#define CPTR_EL2_RES1				\
	(_BV(0)|_BV(1)|_BV(2)|_BV(3)|		\
	 _BV(4)|_BV(5)|_BV(6)|_BV(7)|		\
	 _BV(8)|_BV(9)|_BV(12)|_BV(13))
#define CPTR_EL3_RES0				\
	(CPTR_EL2_RES0|CPTR_EL2_RES1)

/* D.7.2.33 HCR_EL2, Hypervisor Configuration Register */
#define	HCR_FV(name, value)	_FV(HCR_##name, value)

#define HCR_VM			_BV(0) /* Enable virtualization */
#define HCR_SWIO		_BV(1) /* Set/way invalidation override */
#define HCR_PTW			_BV(2) /* Protected table walk */
#define HCR_FMO			_BV(3) /* Physical FIQ routing */
#define HCR_IMO			_BV(4) /* Physical IRQ routing */
#define HCR_AMO			_BV(5) /* Async external abort and SError IRQ routing */
#define HCR_VF			_BV(6) /* Virtual FIQ */
#define HCR_VI			_BV(7) /* Virtual IRQ */
#define HCR_VSE			_BV(8) /* Virtual system error or async abort */
#define HCR_FB			_BV(9) /* Force broadcast */

#define HCR_BSU_OFFSET		10
#define HCR_BSU_MASK		0x03
#define HCR_BSU(value)		HCR_FV(BSU, value) /* Barrier sharebility upgrade */
#define HCR_BSU_NO_EFFECT	0
#define HCR_BSU_INNER_SHAREABLE	1
#define HCR_BSU_OUTER_SHAREABLE	2
#define HCR_BSU_FULL_SYSTEM	3

#define HCR_DC			_DV(12) /* Default cacheable */
#define HCR_TWI			_BV(13) /* Trap WFI */
#define HCR_TWE			_BV(14) /* Trap WFE */
#define HCR_TID0		_BV(15) /* Trap ID group 0 */
#define HCR_TID1		_BV(16) /* Trap ID group 1 */
#define HCR_TID2		_BV(17) /* Trap ID group 2 */
#define HCR_TID3		_BV(18) /* Trap ID group 3 */
#define HCR_TSC			_BV(19) /* Trap SMC */
#define HCR_TIDCP		_BV(20) /* Trap implementation defined functionality */
#define HCR_TACR		_BV(21) /* Trap Auxiliary Control Register */
#define HCR_TSW			_BV(22) /* Trap data or cache maintenance by set/way */
#define HCR_TPC			_BV(23) /* Trap data or cache maintenance of POC */
#define HCR_TPU			_BV(24) /* Trap cache maintenance */
#define HCR_TTLB		_BV(25) /* Trap TLB maintenance */
#define HCR_TVM			_BV(26) /* Trap virtual memory controls */
#define HCR_TGE			_BV(27) /* Trap general exception */
#define HCR_TDZ			_BV(28) /* Trap DC ZVA */
#define HCR_HCD			_BV(29) /* Disable HVC */
#define HCR_TRVM		_BV(30) /* Trap virtual memory control reads */
#define HCR_RW			_BV(31) /* Execution state for lower level */
#define HCR_CD			_BV(32) /* Disable data cache */
#define HCR_ID			_BV(33) /* Disable instruction cache */
#define HCR_MIOCNCE		_BV(38) /* Enable mismatched Inner/Outer Cacheable Non-Coherency */

#define HCR_EL2_RES0				\
	(_BV(63)|_BV(62)|_BV(61)|_BV(60)|	\
	 _BV(59)|_BV(58)|_BV(57)|_BV(56)|	\
	 _BV(55)|_BV(54)|_BV(53)|_BV(52)|	\
	 _BV(51)|_BV(50)|_BV(49)|_BV(48)|	\
	 _BV(47)|_BV(46)|_BV(45)|_BV(44)|	\
	 _BV(43)|_BV(42)|_BV(41)|_BV(40)|	\
	 _BV(39)|_BV(37)|_BV(36)|_BV(35)|	\
	 _BV(34))

/* D.7.2.73/74/75 RMR_EL1/2/3, Reset Management Register */
#define RMR_AA64		_BV(0)
#define RMR_RR			_BV(1)
#define RMR_RES0		\
	(_BV(31)|_BV(30)|_BV(29)|_BV(28)|	\
	 _BV(27)|_BV(26)|_BV(25)|_BV(24)|	\
	 _BV(23)|_BV(22)|_BV(21)|_BV(20)|	\
	 _BV(19)|_BV(18)|_BV(17)|_BV(16)|	\
	 _BV(15)|_BV(14)|_BV(13)|_BV(12)|	\
	 _BV(11)|_BV(10)|_BV(9)|_BV(8)|		\
	 _BV(7)|_BV(6)|_BV(5)|_BV(4)|		\
	 _BV(3)|_BV(2))
#define RMR_EL1_RES0		RMR_RES0
#define RMR_EL2_RES0		RMR_RES0
#define RMR_EL3_RES0		RMR_RES0

/* D.7.20.80 SCR_EL3, Secure Configuration Register */
#define SCR_NS			_BV(0) /* Non-secure */
#define SCR_IRQ			_BV(1) /* Physical IRQ routing */
#define SCR_FIQ			_BV(2) /* Physical FIQ routing */
#define SCR_EA			_BV(3) /* External ABT and SError IRQ routing */
#define SCR_SMD			_BV(7) /* Disable SMC */
#define SCR_HCE			_BV(8) /* Disable HVC */
#define SCR_SIF			_BV(9) /* Secure instruction fetch */
#define SCR_RW			_BV(10) /* Execution state for lower level */
#define SCR_ST			_BV(11) /* Secure timer - trap EL1 CNTPS_ */
#define SCR_TWI			_BV(12) /* Trap WFI */
#define SCR_TWE			_BV(13) /* Trap WFE */

#define SCR_EL3_RES0				\
	(_BV(31)|_BV(30)|_BV(29)|_BV(28)|	\
	 _BV(27)|_BV(26)|_BV(25)|_BV(24)|	\
	 _BV(23)|_BV(22)|_BV(21)|_BV(20)|	\
	 _BV(19)|_BV(18)|_BV(17)|_BV(16)|	\
	 _BV(15)|_BV(14)|_BV(6))
#define SCR_EL3_RES1		(_BV(5)|_BV(4))

/* D.7.2.81/82/83 SCTLR_EL1/2/3, System Control Register */
/* EL1 only: */
#define SCTLR_SA0		_BV(4)	/* Check EL0 stack alignment */
#define SCTLR_CP16BEN		_BV(5)	/* Enable CP15 memory barrier */
#define SCTLR_IT		_BV(7)	/* Trap EL0 IT */
#define SCTLR_SED		_BV(8)	/* Trap EL0 SETEND */
#define SCTLR_UMA		_BV(9)	/* Trap EL0 DAIF access */
#define SCTLR_DZE		_BV(14)	/* Trap EL0 DC ZVA */
#define SCTLR_UCT		_BV(15)	/* Trap EL0 CTR_EL0 access */
#define SCTLR_nTWI		_BV(16)	/* Trap EL0 WFI */
#define SCTLR_nTWE		_BV(18)	/* Trap EL0 WFE */
#define SCTLR_E0E		_BV(24)	/* EL0 endianness */
#define SCTLR_UCI		_BV(26)	/* Trap EL0 cache maintenance */

#define SCTLR_EL1_RES0				\
	(_BV(31)|_BV(30)|_BV(27)|_BV(21)|	\
	 _BV(17)|_BV(13)|_BV(10)|_BV(6))
#define SCTLR_EL1_RES1				\
	(_BV(29)|_BV(28)|_BV(23)|_BV(22)|	\
	 _BV(20)|_BV(11))

/* EL3/EL2/EL1: */
#define SCTLR_M			_BV(0)	/* Enable MMU */
#define SCTLR_A			_BV(1)	/* Check alignment */
#define SCTLR_C			_BV(2)	/* Cacheability */
#define SCTLR_SA		_BV(3)	/* Check stack alignment */
#define SCTLR_I			_BV(12)	/* Instruction access cacheability */
#define SCTLR_WXN		_BV(19)	/* Write permission Execute-Never */
#define SCTLR_EE		_BV(25)	/* Endianness */
#define SCTLR_RES0				\
	(_BV(31)|_BV(30)|_BV(27)|_BV(26)|	\
	 _BV(24)|_BV(21)|_BV(20)|_BV(17)|	\
	 _BV(15)|_BV(14)|_BV(13)|_BV(10)|	\
	 _BV(9)|_BV(8)|_BV(7)|_BV(6))
#define SCTLR_RES1				\
	(_BV(29)|_BV(28)|_BV(23)|_BV(22)|	\
	 _BV(18)|_BV(16)|_BV(11)|_BV(5)|_BV(4))
#define SCTLR_EL2_RES0		SCTLR_RES0
#define SCTLR_EL2_RES1		SCTLR_RES1
#define SCTLR_EL3_RES0		SCTLR_RES0
#define SCTLR_EL3_RES1		SCTLR_RES1

#endif /* __REG_ARM64_H_INCLUDE__ */
