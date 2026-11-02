/*
 * @file quectel_phy.c
 * @brief Quectel PHY driver.
 *
 * @note
 */
/*=============================================================================
 *Copyright (c) 2018 Quectel Wireless Solution, Co., Ltd.  All Rights Reserved.
 *Quectel Wireless Solution Proprietary and Confidential.
 *=============================================================================
 */

/*=====================================================================
 *
 *		EDIT HISTORY FOR MODULE
 *
 *This section contains comments describing changes made to the module.
 *Notice that changes are listed in reverse chronological order.
 *
 *WHEN             WHO         WHAT, WHERE, WHY
 *------------     -------     ----------------------------------------
 *2022/05/11       eyelyn        Support Quectel PHY
 *=======================================================================
 */
#include <linux/phy.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/module.h>
#include <linux/of.h>

#ifndef LINUX_VERSION_CODE
#include <linux/version.h>
#else
#define KERNEL_VERSION(a, b, c) (((a) << 16) + ((b) << 8) + (c))
#endif

#define QUECTEL_PHY_ID                  0x12345678
#define PHY_ID_YT8521                   0x0000011a
#define PHY_ID_YT8010                   0x00000309
#define PHY_ID_YT8011                   0x4f51eb01
#define PHY_ID_YT8010AN                 0x4f51eb02
#define PHY_ID_AR8033                   0x004dd074
#define PHY_ID_BCM89834                 0x35905050
#define PHY_ID_JL311A1                  0x937c4018
#define PHY_ID_RTL9000BRG               0x001ccb00
#define PHY_ID_DP83TC812                0x2000a271
#define PHY_ID_JL3101                   0x937c4010

#define YT_SOFTWARE_RESET               0x8000
#define YT8521_EXTREG_SLEEP_CONTROL1    0x27
#define YT8521_EN_SLEEP_SW_BIT          15
#define REG_DEBUG_ADDR_OFFSET           0x1e
#define REG_DEBUG_DATA                  0x1f
#define AR_REG_DEBUG_ADDR_OFFSET        0x1d
#define AR_REG_DEBUG_DATA               0x1e
#define REG_MII_MMD_CTRL                0x0D
#define REG_MII_MMD_DATA                0x0E
#define REG_PHY_SPEC_STATUS             0x11
#define REG_PHY_BASIC_STATUS            0x01

#define JL3XXX_UNKNOWN                  0x0000
#define JL3XXX_100BASE_T1               0x0001
#define JL3XXX_1000BASE_T1              0x0002
#define JL3XXX_PMA_CTL_ADDR             0x0001
#define JL3XXX_PMA_CTL                  0x0834
#define JL3XXX_MASTER_MODE              BIT(14)
#define JL3XXX_PMA_BASE                 0x0000
#define JL3XXX_PMA_RESET                BIT(15)
#define JL3XXX_PHYSID1                  0x0002
#define JL3XXX_PHYSID2                  0x0003
#define JL3XXX_PCS_CTL_ADDR             0x0001
#define JL3XXX_PCS_CTL                  0x0000
#define JL3XXX_PCS_SPEED_LSB            BIT(13)
#define JL3XXX_PCS_SPEED_MSB            BIT(6)
#define JL3XXX_IEEE_CTL_ADDR            0x0003
#define JL3XXX_IEEE_STATUS              0x0001
#define JL3XXX_IEEE_CTL                 0x0001
#define JL3XXX_GLOBAL_LINK              BIT(2)
#define JL3XXX_RGMII_CFG                0x8800
#define JL3XXX_RGMII_TX_DLY             BIT(15)
#define JL3XXX_RGMII_RX_DLY             BIT(14)
#define JL3XXX_RGMII_TX_DLY_LEVEL_0     BIT(10)
#define JL3XXX_RGMII_TX_DLY_LEVEL_1     BIT(11)

#define YTXXXX_SPEED_MODE		0xc000
#define YTXXXX_SPEED_MODE_BIT		14
#define YTXXXX_DUPLEX_BIT		13
#define YTXXXX_LINK_STATUS_BIT		10

struct virtual_phy_priv {
	int virtual_phy_speed;
	int virtual_phy_link;
};

/* user system mode  */
/* #define JL3XXX_FORCE_LINK_STATUS */
/* #define JL3XXX_RGMII_TO_SGMII_PHY */

enum jl3xxx_mode {
	PHY_MODE_UNKNOWN,
	PHY_MODE_MASTER,
	PHY_MODE_SLAVE,
	PHY_MODE_AUTONEGO,
};

enum jl3xxx_speed {
	PHY_SPEED_UNKNOWN,
	PHY_SPEED_100M,
	PHY_SPEED_1000M,
};

enum jl3xxx_inter {
	PHY_SGMII,
	PHY_RGMII,
	PHY_RMII,
	PHY_RGMII_SGMII,
};

/*eyelyn  20220707 add Troubleshoot an unmatched PHY, causing dump*/
typedef struct {
	int phy_id;
	int clause;
} quectel_phy_struct;

#if (KERNEL_VERSION(5, 5, 0) > LINUX_VERSION_CODE)
static inline void phy_lock_mdio_bus(struct phy_device *phydev)
{
#if (KERNEL_VERSION(4, 5, 0) > LINUX_VERSION_CODE)
	mutex_lock(&phydev->bus->mdio_lock);
#else
	mutex_lock(&phydev->mdio.bus->mdio_lock);
#endif
}

static inline void phy_unlock_mdio_bus(struct phy_device *phydev)
{
#if (KERNEL_VERSION(4, 5, 0) > LINUX_VERSION_CODE)
	mutex_unlock(&phydev->bus->mdio_lock);
#else
	mutex_unlock(&phydev->mdio.bus->mdio_lock);
#endif
}
#endif

#if (KERNEL_VERSION(4, 16, 0) > LINUX_VERSION_CODE)
static inline int __phy_read(struct phy_device *phydev, u32 regnum)
{
#if (KERNEL_VERSION(4, 5, 0) > LINUX_VERSION_CODE)
	struct mii_bus *bus = phydev->bus;
	int addr = phydev->addr;
	return bus->read(bus, phydev->addr, regnum);
#else
	struct mii_bus *bus = phydev->mdio.bus;
	int addr = phydev->mdio.addr;
#endif
	return bus->read(bus, addr, regnum);
}

static inline int __phy_write(struct phy_device *phydev, u32 regnum, u16 val)
{
#if (KERNEL_VERSION(4, 5, 0) > LINUX_VERSION_CODE)
	struct mii_bus *bus = phydev->bus;
	int addr = phydev->addr;
#else
	struct mii_bus *bus = phydev->mdio.bus;
	int addr = phydev->mdio.addr;
#endif
	return bus->write(bus, addr, regnum, val);
}
#endif

__attribute__((unused)) static int __ytphy_read_ext(struct phy_device *phydev,
						    u32 regnum)
{
	int ret;

	ret = __phy_write(phydev, REG_DEBUG_ADDR_OFFSET, regnum);
	if (ret < 0)
		return ret;

	return __phy_read(phydev, REG_DEBUG_DATA);
}

static int ytphy_read_ext(struct phy_device *phydev, u32 regnum)
{
	int ret;

	phy_lock_mdio_bus(phydev);
	ret = __phy_write(phydev, REG_DEBUG_ADDR_OFFSET, regnum);
	if (ret < 0)
		goto err_handle;

	ret = __phy_read(phydev, REG_DEBUG_DATA);
	if (ret < 0)
		goto err_handle;

err_handle:
	phy_unlock_mdio_bus(phydev);

	return ret;
}

static int ytphy_write_ext(struct phy_device *phydev, u32 regnum, u16 val)
{
	int ret;

	phy_lock_mdio_bus(phydev);
	ret = __phy_write(phydev, REG_DEBUG_ADDR_OFFSET, regnum);
	if (ret < 0)
		goto err_handle;

	ret = __phy_write(phydev, REG_DEBUG_DATA, val);
	if (ret < 0)
		goto err_handle;

err_handle:
	phy_unlock_mdio_bus(phydev);

	return ret;
}

__attribute__((unused)) static int __ytphy_write_ext(struct phy_device *phydev,
						     u32 regnum, u16 val)
{
	int ret;

	ret = __phy_write(phydev, REG_DEBUG_ADDR_OFFSET, regnum);
	if (ret < 0)
		return ret;

	ret = __phy_write(phydev, REG_DEBUG_DATA, val);
	if (ret < 0)
		return ret;

	return 0;
}

__attribute__((unused)) static int ytphy_read_mmd(struct phy_device* phydev,
						  u16 device, u16 reg)
{
	int val;

	phy_lock_mdio_bus(phydev);

	__phy_write(phydev, REG_MII_MMD_CTRL, device);
	__phy_write(phydev, REG_MII_MMD_DATA, reg);
	__phy_write(phydev, REG_MII_MMD_CTRL, device | 0x4000);
	val = __phy_read(phydev, REG_MII_MMD_DATA);
	if (val < 0) {
#if (KERNEL_VERSION(4, 5, 0) > LINUX_VERSION_CODE)
		dev_err(&phydev->dev, "error read mmd device(%u) reg (%u)\n",
			device, reg);
#else
		dev_err(&phydev->mdio.dev,
			"error read mmd device(%u) reg (%u)\n", device, reg);
#endif

		goto err_handle;
	}

err_handle:
	phy_unlock_mdio_bus(phydev);

	return val;
}

__attribute__((unused)) static int ytphy_write_mmd(struct phy_device* phydev,
						   u16 device, u16 reg,
						   u16 value)
{
	int ret = 0;

	phy_lock_mdio_bus(phydev);

	__phy_write(phydev, REG_MII_MMD_CTRL, device);
	__phy_write(phydev, REG_MII_MMD_DATA, reg);
	__phy_write(phydev, REG_MII_MMD_CTRL, device | 0x4000);
	__phy_write(phydev, REG_MII_MMD_DATA, value);

	phy_unlock_mdio_bus(phydev);

	return ret;
}

__attribute__((unused)) static int __ytphy_read_mmd(struct phy_device* phydev,
						    u16 device, u16 reg)
{
	int val;

	__phy_write(phydev, REG_MII_MMD_CTRL, device);
	__phy_write(phydev, REG_MII_MMD_DATA, reg);
	__phy_write(phydev, REG_MII_MMD_CTRL, device | 0x4000);
	val = __phy_read(phydev, REG_MII_MMD_DATA);

	return val;
}

__attribute__((unused)) static int __ytphy_write_mmd(struct phy_device* phydev,
						     u16 device, u16 reg,
						     u16 value)
{
	__phy_write(phydev, REG_MII_MMD_CTRL, device);
	__phy_write(phydev, REG_MII_MMD_DATA, reg);
	__phy_write(phydev, REG_MII_MMD_CTRL, device | 0x4000);
	__phy_write(phydev, REG_MII_MMD_DATA, value);

	return 0;
}

__attribute__((unused)) static int
	__ytphy_soft_reset(struct phy_device *phydev)
{
	int ret = 0, val = 0;

	val = __phy_read(phydev, MII_BMCR);
	if (val < 0)
		return val;

	ret = __phy_write(phydev, MII_BMCR, val | BMCR_RESET);
	if (ret < 0)
		return ret;

	return ret;
}

static int ytphy_soft_reset(struct phy_device *phydev)
{
	int ret = 0, val = 0;

	val = phy_read(phydev, MII_BMCR);
	if (val < 0)
		return val;

	ret = phy_write(phydev, MII_BMCR, val | BMCR_RESET);
	if (ret < 0)
		return ret;

	return ret;
}

/*The unit is 150 ps*/
static int yt8521_set_tx_delay(struct phy_device *phydev, int delay)
{
	int val;

	val = ytphy_read_ext(phydev, 0xa003);
	val &= ~0xf;
	val |= (delay & 0xf);
	ytphy_write_ext(phydev, 0xa003, val);

	return 0;
}

static int yt8011_read_status(struct phy_device *phydev)
{
	int ret;
	int val;
	int link;

	/* UTP */
	ret = ytphy_write_ext(phydev, 0x9000, 0x0);
	if (ret < 0)
		return ret;

	val = phy_read(phydev, REG_PHY_SPEC_STATUS);
	if (val < 0)
		return val;

	link = val & (BIT(YTXXXX_LINK_STATUS_BIT));
	if (link)
		phydev->link = 1;
	else
		phydev->link = 0;

	return 0;
}

static int yt8010_read_status(struct phy_device *phydev)
{
	int val;
	int link;

	val = phy_read(phydev, REG_PHY_BASIC_STATUS);
	if (val < 0)
		return val;

	link = val & (BIT(2));
	if (link)
		phydev->link = 1;
	else
		phydev->link = 0;

	return 0;
}

static int ytxxxx_automotive_adjust_status(struct phy_device *phydev, int val)
{
	int speed_mode;
#if (KERNEL_VERSION(4, 0, 0) > LINUX_VERSION_CODE)
	int speed = -1;
#else
	int speed = SPEED_UNKNOWN;
#endif

	speed_mode = (val & YTXXXX_SPEED_MODE) >> YTXXXX_SPEED_MODE_BIT;
	switch (speed_mode) {
	case 1:
		speed = SPEED_100;
		break;
	case 2:
		speed = SPEED_1000;
		break;
	default:
#if (KERNEL_VERSION(4, 0, 0) > LINUX_VERSION_CODE)
		speed = -1;
#else
		speed = SPEED_UNKNOWN;
#endif
		break;
	}

	phydev->speed = speed;
	phydev->duplex = DUPLEX_FULL;

	return 0;
}
static int yt8010an_read_status(struct phy_device *phydev)
{
	int ret;
	int val;
	int link;
	int link_utp = 0;

	/* UTP */
	ret = ytphy_write_ext(phydev, 0x9000, 0x0);
	if (ret < 0)
		return ret;

	val = phy_read(phydev, REG_PHY_SPEC_STATUS);
	if (val < 0)
		return val;

	link = val & (BIT(YTXXXX_LINK_STATUS_BIT));
	if (link) {
		link_utp = 1;
		ytxxxx_automotive_adjust_status(phydev, val);
	} else {
		link_utp = 0;
	}

	if (link_utp) {
		if (phydev->link == 0)
#if (KERNEL_VERSION(4, 5, 0) > LINUX_VERSION_CODE)
			netdev_info(phydev->attached_dev,
				    "%s, phy addr: %d, link up, media: UTP, mii reg 0x11 = 0x%x\n",
				    __func__, phydev->addr, (unsigned int)val);
#else
			netdev_info(phydev->attached_dev,
				    "%s, phy addr: %d, link up, media: UTP, mii reg 0x11 = 0x%x\n",
				    __func__, phydev->mdio.addr,
				    (unsigned int)val);
#endif
		phydev->link = 1;
	} else {
		if (phydev->link == 1)
#if (KERNEL_VERSION(4, 5, 0) > LINUX_VERSION_CODE)
			netdev_info(phydev->attached_dev,
				    "%s, phy addr: %d, link down\n",
				    __func__, phydev->addr);
#else
			netdev_info(phydev->attached_dev,
				    "%s, phy addr: %d, link down\n",
				    __func__, phydev->mdio.addr);
#endif
		phydev->link = 0;
	}

	if (link_utp)
		ytphy_write_ext(phydev, 0x9000, 0x0);

	return phydev->link;
}


static int arphy_write_ext(struct phy_device *phydev, u32 regnum, u16 val)
{
	int ret;

	ret = phy_write(phydev, AR_REG_DEBUG_ADDR_OFFSET, regnum);
	if (ret < 0)
		return ret;

	ret = phy_write(phydev, AR_REG_DEBUG_DATA, val);

	return ret;
}

static int arphy_read_ext(struct phy_device *phydev, u32 regnum)
{
	int ret;
	int val;

	ret = phy_write(phydev, AR_REG_DEBUG_ADDR_OFFSET, regnum);
	if (ret < 0)
		return ret;

	val = phy_read(phydev, AR_REG_DEBUG_DATA);

	return val;
}

static int yt8521_config_init(struct phy_device *phydev)
{
	int ret;
	int val;

	//phydev->irq = PHY_POLL;

	printk("enter %s\n", __func__);
	ytphy_write_ext(phydev, 0xa000, 0);

	val = ytphy_read_ext(phydev, YT8521_EXTREG_SLEEP_CONTROL1);
	if (val < 0)
		return val;

	val &= (~BIT(YT8521_EN_SLEEP_SW_BIT));

	ret = ytphy_write_ext(phydev, YT8521_EXTREG_SLEEP_CONTROL1, val);
	if (ret < 0)
		return ret;

	/* enable RXC clock when no wire plug */
	ret = ytphy_write_ext(phydev, 0xa000, 0);
	if (ret < 0)
		return ret;

	val = ytphy_read_ext(phydev, 0xc);
	if (val < 0)
		return val;
	val &= ~(1 << 12);
	ret = ytphy_write_ext(phydev, 0xc, val);
	if (ret < 0)
		return ret;

	val = phy_read(phydev, MII_BMCR);
	if (val < 0)
		return val;

	val |= YT_SOFTWARE_RESET;
	ret = phy_write(phydev, MII_BMCR, val);

	//eyelyn.tang 20220425 add rgmii
	/* set tx clock delay to 1.65ns */
	yt8521_set_tx_delay(phydev, 0x0b);
	//end
	return ret;
}

//gabriel.li 20220929 add rmii YT8010
static int yt8010_config_init(struct phy_device *phydev)
{
	int ret;
	int val;

	printk("enter %s\n", __func__);
	ret = ytphy_write_ext(phydev, 0x1023, 0xfc00);
	if (ret < 0)
		return ret;
	ret = ytphy_write_ext(phydev, 0x101d, 0x12c0);
	if (ret < 0)
		return ret;
	val = ytphy_read_ext(phydev, 0x1000);
	val &= ~(0x1 << 7);
	ret = ytphy_write_ext(phydev, 0x1000, val);
	if (ret < 0)
		return ret;
	ret = ytphy_write_ext(phydev, 0x101d, 0x12c0);
	if (ret < 0)
		return ret;
	ret = ytphy_write_ext(phydev, 0x101e, 0x1900);
	if (ret < 0)
		return ret;
	ret = ytphy_write_ext(phydev, 0x101f, 0x1900);
	if (ret < 0)
		return ret;
	ret = ytphy_write_ext(phydev, 0x4083, 0x4327);
	if (ret < 0)
		return ret;
	ret = ytphy_write_ext(phydev, 0x4082, 0xc20);
	if (ret < 0)
		return ret;

	ret = ytphy_write_ext(phydev, 0x4000, 0x2000);   //mode select
	if (ret < 0)
		return ret;

	//OPEN interrupt and Turn off all interrupt triggers
	val = ytphy_read_ext(phydev, 0x4001);
	val |= (0x1 << 14);
	ret = ytphy_write_ext(phydev, 0x4001, val);
	if (ret < 0)
		return ret;

	//reset
	val = phy_read(phydev, MII_BMCR);
	val |= YT_SOFTWARE_RESET;
	ret = phy_write(phydev, MII_BMCR, val);
	if (ret < 0)
		return ret;
	printk("yt8010_config_init finish\n");

	return ret;
}

/* #define YT801X_RGMII_DVDDIO_3V3 */
/* #define YT801X_RGMII_DVDDIO_2V5 */
#define YT801X_RGMII_DVDDIO_1V8 
static int yt8010an_config_init(struct phy_device *phydev)
{
	int chip_config = ytphy_read_ext(phydev, 0x9030);
	unsigned char chip_mode = chip_config & 0x01;

	phydev->autoneg = AUTONEG_DISABLE;

	/* UTP */
	ytphy_write_ext(phydev, 0x9000, 0x0);

	ytphy_write_ext(phydev, 0x1008, 0x2119);
	ytphy_write_ext(phydev, 0x1092, 0x712);
	ytphy_write_ext(phydev, 0x90bc, 0x6661);
	ytphy_write_ext(phydev, 0x90b9, 0x620b);
	ytphy_write_ext(phydev, 0x2001, 0x6418);
	ytphy_write_ext(phydev, 0x1019, 0x3712);
	ytphy_write_ext(phydev, 0x101a, 0x3713);
	ytphy_write_ext(phydev, 0x2005, 0x810);
	ytphy_write_ext(phydev, 0x2015, 0x1012);
	ytphy_write_ext(phydev, 0x2013, 0xff06);
	ytphy_write_ext(phydev, 0x3017, 0x4);
	ytphy_write_ext(phydev, 0x3027, 0xffe8);
	ytphy_write_ext(phydev, 0x3026, 0x1b58);
	ytphy_write_ext(phydev, 0x301e, 0xb40);
	ytphy_write_ext(phydev, 0x3019, 0xffd4);
	ytphy_write_ext(phydev, 0x3014, 0x1115);
	ytphy_write_ext(phydev, 0x301a, 0x7800);
	ytphy_write_ext(phydev, 0x1000, 0x28);
	ytphy_write_ext(phydev, 0x1053, 0xf);
	ytphy_write_ext(phydev, 0x105e, 0xa46c);
	ytphy_write_ext(phydev, 0x1088, 0x2b);
	ytphy_write_ext(phydev, 0x1088, 0x2b);
	ytphy_write_ext(phydev, 0x1088, 0xb);
	ytphy_write_ext(phydev, 0x3008, 0x141);
	ytphy_write_ext(phydev, 0x3009, 0x1918);
	ytphy_write_ext(phydev, 0x9095, 0x1a1a);
	ytphy_write_ext(phydev, 0x9096, 0x1a10);
	ytphy_write_ext(phydev, 0x9097, 0x101a);
	ytphy_write_ext(phydev, 0x9098, 0x01ff);
	netdev_info(phydev->attached_dev, "%s done, phy chip_mod: %d\n",
		    __func__, chip_mode);
	if (!(chip_mode)) {	/* rgmii config */
#if defined (YT801X_RGMII_DVDDIO_3V3)
		ytphy_write_ext(phydev, 0x9000, 0x8000);
		ytphy_write_ext(phydev, 0x0062, 0x0000);
		ytphy_write_ext(phydev, 0x9000, 0x0000);
		ytphy_write_ext(phydev, 0x9031, 0xb200);
		ytphy_write_ext(phydev, 0x903b, 0x0040);
		ytphy_write_ext(phydev, 0x903e, 0x3b3b);
		ytphy_write_ext(phydev, 0x903c, 0xf);
		ytphy_write_ext(phydev, 0x903d, 0x1000);
		ytphy_write_ext(phydev, 0x9038, 0x0000);
	netdev_info(phydev->attached_dev, "%s done, 1 phy chip_mod: %d\n",
		    __func__, chip_mode);
#elif defined (YT801X_RGMII_DVDDIO_2V5)
		ytphy_write_ext(phydev, 0x9000, 0x8000);
		ytphy_write_ext(phydev, 0x0062, 0x0000);
		ytphy_write_ext(phydev, 0x9000, 0x0000);
		ytphy_write_ext(phydev, 0x9031, 0xb200);
		ytphy_write_ext(phydev, 0x9111, 0x5);
		ytphy_write_ext(phydev, 0x9114, 0x3939);
		ytphy_write_ext(phydev, 0x9112, 0xf);
		ytphy_write_ext(phydev, 0x9110, 0x0);
		ytphy_write_ext(phydev, 0x9113, 0x10);
		ytphy_write_ext(phydev, 0x903d, 0x2);
	netdev_info(phydev->attached_dev, "%s done, 2 phy chip_mod: %d\n",
		    __func__, chip_mode);
#elif defined (YT801X_RGMII_DVDDIO_1V8)
		ytphy_write_ext(phydev, 0x9000, 0x8000);
		ytphy_write_ext(phydev, 0x0062, 0x0000);
		ytphy_write_ext(phydev, 0x9000, 0x0000);
		ytphy_write_ext(phydev, 0x9031, 0xb200);
		ytphy_write_ext(phydev, 0x9116, 0x6);
		ytphy_write_ext(phydev, 0x9119, 0x3939);
		ytphy_write_ext(phydev, 0x9117, 0xf);
		ytphy_write_ext(phydev, 0x9115, 0x0);
		ytphy_write_ext(phydev, 0x9118, 0x20);
		ytphy_write_ext(phydev, 0x903d, 0x3);
	netdev_info(phydev->attached_dev, "%s done, 3 phy chip_mod: %d\n",
		    __func__, chip_mode);
#endif
	}

	if (phydev->phy_id == PHY_ID_YT8011)
		ytphy_soft_reset(phydev);
	else if (phydev->phy_id == PHY_ID_YT8010AN)
	{
		ytphy_write_mmd(phydev, 0x1, 0x834, 0x8000);
		
		phy_write(phydev, MII_BMCR, 0xa100);
		msleep(20); /* 等待 RESET 完成 */
	}

#if (KERNEL_VERSION(4, 5, 0) > LINUX_VERSION_CODE)
	netdev_info(phydev->attached_dev, "%s done, phy addr: %d\n",
		    __func__, phydev->addr);
#else
	netdev_info(phydev->attached_dev, "%s done, phy addr: %d\n",
		    __func__, phydev->mdio.addr);
#endif

	return 0;
}

//gabriel.li 20221011 add rgmii AR8033
static int ar8033_config_init(struct phy_device *phydev)
{
	int ret;

	printk("enter %s\n", __func__);

	//Mode Settings and open the ext register
	ret = phy_write(phydev, 0x1f, 0x8100);
	if (ret < 0)
		return ret;

	//Open the RGMII Tx clock delay enable
	ret = arphy_write_ext(phydev, 0x05, 0x100);
	if (ret < 0)
		return ret;

	//Change IO voltage 1.5V to 1.8V
	ret = arphy_write_ext(phydev, 0x1f, 0x8);
	if (ret < 0)
		return ret;

	//reset and Set 100 M
	ret = phy_write(phydev, 0x00, 0xa100);
	if (ret < 0)
		return ret;

	return ret;
}

//gabriel.li 20230421 add rgmii BCM89834
static int bcm89834_rdb_write_reg(struct phy_device *phydev, int rdb_address, int value)
{
	phy_write(phydev, 0x1E, rdb_address);
	udelay(5);
	phy_write(phydev, 0x1F, value);
	udelay(5);
	return 0;
}

static int bcm89834_rdb_read_reg(struct phy_device *phydev, int rdb_address)
{
	int reg = 0;

	phy_write(phydev, 0x1E, rdb_address);
	udelay(5);
	reg = phy_read(phydev, 0x1F);
	udelay(5);
	return reg;
}

static int bcm89834_c45_by_c22_read_reg(struct phy_device *phydev, int devad, int address)
{
	int reg = 0;

	phy_write(phydev, MII_MMD_CTRL, devad);
	udelay(5);
	phy_write(phydev, MII_MMD_DATA, address);
	udelay(5);
	phy_write(phydev, MII_MMD_CTRL, 0x4000 | devad); //Data mode
	udelay(5);
	reg = phy_read(phydev, MII_MMD_DATA);
	udelay(5);
	return reg;
}

static int bcm89834_config_init(struct phy_device *phydev)
{
	int ret = 0;
	//int val;
	//int reg;

	printk("enter %s %d\n", __func__, __LINE__);

	// For 1000M set TXCLK_DELAY and RXCLK_DELAY

	//reg = bcm89834_rdb_read_reg(phydev, 0x0013);
	//val = reg | (1<<9);		//shifted 90 degrees
	//val = reg & (~(1<<9));		//shifted 0 degrees
	//bcm89834_rdb_write_reg(phydev, 0x0013, val);

	//reg = bcm89834_rdb_read_reg(phydev, 0x002f);
	//val = reg | (1<<8);		//shifted 90 degrees
	//val = reg & (~(1<<8));		//shifted 0 degrees
	//bcm89834_rdb_write_reg(phydev, 0x002f, val);

	return ret;
}

//gabriel add 20230818 Adaptation module phy jl3113
static int jlphy_mmd_read_ext(struct phy_device *phydev, int devad, u16 devnum)
{
	phy_write(phydev, 13, devad);
	phy_write(phydev, 14, devnum);
	phy_write(phydev, 13, 0x4000 | devad);
	return phy_read(phydev, 14);
}

static int jlphy_mmd_write_ext(struct phy_device *phydev,
			       int devad, u16 devnum, u16 val)
{
	phy_write(phydev, 13, devad);
	phy_write(phydev, 14, devnum);
	phy_write(phydev, 13, 0x4000 | devad);
	phy_write(phydev, 14, val);
	return 0;
}

static int jl3xxx_mmd_modify(struct phy_device *phydev,
			     int devad, u16 reg, u16 mask, u16 bits)
{
	int old, val, ret;

	old = jlphy_mmd_read_ext(phydev, devad, reg);
	if (old < 0)
		return old;

	val = (old & ~mask) | bits;
	if (val == old)
		return 0;

	ret = jlphy_mmd_write_ext(phydev, devad, reg, val);

	return ret < 0 ? ret : 1;
}

static int jl3xxx_fetch_mmd_bit(struct phy_device *phydev,
				int devad, u16 reg, u16 bit)
{
	int val;

	val = jlphy_mmd_read_ext(phydev, devad, reg);
	if (val < 0)
		return val;

	return ((val & bit) == bit) ? 1 : 0;
}

static int jl3xxx_set_mmd_bits(struct phy_device *phydev,
			       int devad, u16 reg, u16 bits)
{
	return jl3xxx_mmd_modify(phydev, devad, reg, 0, bits);
}

static int jl3xxx_clear_mmd_bits(struct phy_device *phydev,
				 int devad, u16 reg, u16 bits)
{
	return jl3xxx_mmd_modify(phydev, devad, reg, bits, 0);
}

static int jl3xxx_check_link(struct phy_device *phydev)
{
	return jl3xxx_fetch_mmd_bit(phydev, JL3XXX_IEEE_STATUS,
				    JL3XXX_IEEE_CTL, JL3XXX_GLOBAL_LINK);
}

static int jl3xxx_set_mode(struct phy_device *phydev, enum jl3xxx_mode mode)
{
	if (mode == PHY_MODE_MASTER)
		jl3xxx_set_mmd_bits(phydev, JL3XXX_PMA_CTL_ADDR,
				    JL3XXX_PMA_CTL, JL3XXX_MASTER_MODE);
	else if (mode == PHY_MODE_SLAVE)
		jl3xxx_clear_mmd_bits(phydev, JL3XXX_PMA_CTL_ADDR,
				      JL3XXX_PMA_CTL, JL3XXX_MASTER_MODE);
	else
		dev_warn(&phydev->mdio.dev,
			 "Automatic negotiation is not supported at present");

	return 0;
}

static int jl3xxx_set_speed(struct phy_device *phydev, enum jl3xxx_speed speed)
{
	if (speed == PHY_SPEED_1000M)
		jl3xxx_mmd_modify(phydev, JL3XXX_PCS_CTL_ADDR, JL3XXX_PCS_CTL,
				  JL3XXX_PCS_SPEED_LSB, JL3XXX_PCS_SPEED_MSB);
	else
		jl3xxx_mmd_modify(phydev, JL3XXX_PCS_CTL_ADDR, JL3XXX_PCS_CTL,
				  JL3XXX_PCS_SPEED_MSB, JL3XXX_PCS_SPEED_LSB);

	return 0;
}

static int jl3xxx_set_phy_inter(struct phy_device *phydev, enum jl3xxx_inter inter)
{
	u16 ret = 0;

	if (inter == PHY_SGMII) {
		jl3xxx_mmd_modify(phydev, 3, 0x8000, 0xf, BIT(0));
		jlphy_mmd_write_ext(phydev, 3, 0x8000, 0x0245);
	} else if (inter == PHY_RGMII_SGMII) {
		jl3xxx_mmd_modify(phydev, 3, 0x8000, 0xf, BIT(0));
		jlphy_mmd_write_ext(phydev, 3, 0x8000, 0x0244);
	} else if (inter == PHY_RGMII) {
		jl3xxx_clear_mmd_bits(phydev, 3, 0x8000, BIT(0));
		jlphy_mmd_write_ext(phydev, 3, 0x8000, 0x0240);
	} else if (inter == PHY_RMII) {
		jl3xxx_mmd_modify(phydev, 3, 0x8000, 0xf, (BIT(1) | BIT(2)));
		jl3xxx_set_mmd_bits(phydev, 3, 0x8800, BIT(1));
	} else {
		printk("%s %d\n unknown jlphy interface", __func__, __LINE__);
	}
	return 0;
}

static int jl3xxx_get_speed(struct phy_device *phydev)
{
	int msb, lsb;

	msb = jl3xxx_fetch_mmd_bit(phydev, JL3XXX_PCS_CTL_ADDR,
				   JL3XXX_PCS_CTL, JL3XXX_PCS_SPEED_MSB);

	lsb = jl3xxx_fetch_mmd_bit(phydev, JL3XXX_PCS_CTL_ADDR,
				   JL3XXX_PCS_CTL, JL3XXX_PCS_SPEED_LSB);

	if (~lsb & msb)
		return JL3XXX_1000BASE_T1;
	else
		return JL3XXX_100BASE_T1;
}

static int jl3xxx_set_rgmii_dly(struct phy_device *phydev, bool tx_on, bool rx_on)
{
	if (tx_on)
		jl3xxx_set_mmd_bits(phydev, JL3XXX_IEEE_CTL_ADDR,
				    JL3XXX_RGMII_CFG, JL3XXX_RGMII_TX_DLY);
	else
		jl3xxx_clear_mmd_bits(phydev, JL3XXX_IEEE_CTL_ADDR,
				      JL3XXX_RGMII_CFG, JL3XXX_RGMII_TX_DLY);
	if (rx_on)
		jl3xxx_set_mmd_bits(phydev, JL3XXX_IEEE_CTL_ADDR,
				    JL3XXX_RGMII_CFG, JL3XXX_RGMII_RX_DLY);
	else
		jl3xxx_clear_mmd_bits(phydev, JL3XXX_IEEE_CTL_ADDR,
				      JL3XXX_RGMII_CFG, JL3XXX_RGMII_RX_DLY);
	return 0;
}

static int jl3xxx_config_init(struct phy_device *phydev)
{
	int err;

	err = jl3xxx_set_speed(phydev, PHY_SPEED_1000M);
	if (err < 0)
		return err;

	err = jl3xxx_set_mode(phydev, PHY_MODE_MASTER);
	if (err < 0)
		return err;

	if (phydev->interface == PHY_INTERFACE_MODE_SGMII) {
		err = jl3xxx_set_phy_inter(phydev, PHY_SGMII);
		if (err < 0)
			return err;
	} else {
		err = jl3xxx_set_phy_inter(phydev, PHY_RGMII);
		if (err < 0)
			return err;
	}

	//1000M  TX or RX delay enabled and TX CLOCK DELAY LEVEL
	//jl3xxx_set_rgmii_dly(phydev, true, true);

	//TX CLOCK DELAY LEVEL 1ns
	//jl3xxx_clear_mmd_bits(phydev, JL3XXX_IEEE_CTL_ADDR,
	//			JL3XXX_RGMII_CFG, JL3XXX_RGMII_TX_DLY_LEVEL_0);
	//jl3xxx_clear_mmd_bits(phydev, JL3XXX_IEEE_CTL_ADDR,
	//			JL3XXX_RGMII_CFG, JL3XXX_RGMII_TX_DLY_LEVEL_1);

	//TX CLOCK DELAY LEVEL 1.2ns
	//jl3xxx_set_mmd_bits(phydev, JL3XXX_IEEE_CTL_ADDR,
	//			JL3XXX_RGMII_CFG, JL3XXX_RGMII_TX_DLY_LEVEL_0);
	//jl3xxx_clear_mmd_bits(phydev, JL3XXX_IEEE_CTL_ADDR,
	//			JL3XXX_RGMII_CFG, JL3XXX_RGMII_TX_DLY_LEVEL_1);

	//TX CLOCK DELAY LEVEL 1.5ns
	//jl3xxx_clear_mmd_bits(phydev, JL3XXX_IEEE_CTL_ADDR,
	//			JL3XXX_RGMII_CFG, JL3XXX_RGMII_TX_DLY_LEVEL_0);
	//jl3xxx_set_mmd_bits(phydev, JL3XXX_IEEE_CTL_ADDR,
	//			JL3XXX_RGMII_CFG, JL3XXX_RGMII_TX_DLY_LEVEL_1);

	//TX CLOCK DELAY LEVEL 1.8ns
	//jl3xxx_set_mmd_bits(phydev, JL3XXX_IEEE_CTL_ADDR,
	//			JL3XXX_RGMII_CFG, JL3XXX_RGMII_TX_DLY_LEVEL_0);
	//jl3xxx_set_mmd_bits(phydev, JL3XXX_IEEE_CTL_ADDR,
	//			JL3XXX_RGMII_CFG, JL3XXX_RGMII_TX_DLY_LEVEL_1);

	// soft reset
	jl3xxx_set_mmd_bits(phydev, 1, 0, BIT(15));
	msleep(20);

	return 0;
}

static int yt8011_config_init(struct phy_device *phydev)
{
	int ret;
	int val;

	printk("enter %s\n", __func__);
	/* UTP */
	ytphy_write_ext(phydev, 0x9000, 0x0);

	ytphy_write_ext(phydev, 0x1008, 0x2119);
	ytphy_write_ext(phydev, 0x1092, 0x712);
	ytphy_write_ext(phydev, 0x90bc, 0x6661);
	ytphy_write_ext(phydev, 0x90b9, 0x620b);
	ytphy_write_ext(phydev, 0x2001, 0x6418);
	ytphy_write_ext(phydev, 0x1019, 0x3712);
	ytphy_write_ext(phydev, 0x101a, 0x3713);
	ytphy_write_ext(phydev, 0x2015, 0x1012);
	ytphy_write_ext(phydev, 0x2005, 0x810);
	ytphy_write_ext(phydev, 0x2013, 0xff06);
	ytphy_write_ext(phydev, 0x1000, 0x0028);
	ytphy_write_ext(phydev, 0x1002, 0x3800);
	ytphy_write_ext(phydev, 0x1090, 0x1012);
	ytphy_write_ext(phydev, 0x1091, 0x1013);
	ytphy_write_ext(phydev, 0x1100, 0x2020);
	ytphy_write_mmd(phydev, 0x7, 0x1f42, 0x0016);
	ytphy_write_ext(phydev, 0x1088, 0x002b);
	ytphy_write_ext(phydev, 0x1088, 0x022b);
	ytphy_write_ext(phydev, 0x1088, 0x020b);
	ytphy_write_ext(phydev, 0x3008, 0x143);
	ytphy_write_ext(phydev, 0x3009, 0x1918);
	//1.8V RGMII set
	ytphy_write_ext(phydev, 0x9000, 0x8000);
	ytphy_write_ext(phydev, 0x0062, 0x0000);
	ytphy_write_ext(phydev, 0x9000, 0x0000);
	ytphy_write_ext(phydev, 0x9031, 0xb200);
	ytphy_write_ext(phydev, 0x9116, 0x6);
	ytphy_write_ext(phydev, 0x9119, 0x3939);
	ytphy_write_ext(phydev, 0x9117, 0xf);
	ytphy_write_ext(phydev, 0x9115, 0x0);
	ytphy_write_ext(phydev, 0x9118, 0x20);
	ytphy_write_ext(phydev, 0x903d, 0x3);

	//reset
	val = phy_read(phydev, MII_BMCR);
	val |= YT_SOFTWARE_RESET;
	ret = phy_write(phydev, MII_BMCR, val);
	if (ret < 0)
		return ret;

	//clock delay  Max. 2ns
	/*	ETX 0X9001
	*	BIT 3:0 RX DELAY
	*	BIT 7:4 TX DELAY
	*	SEL:125 ps
	*/
	ytphy_write_ext(phydev, 0x9001, 0x1f0);

	printk("yt8011_config_init finish\n");

	return ret;
}

static int rtlphy_read_spec(struct phy_device *phydev, u32 regnum)
{
	int ret;
	int val;

	ret = phy_write(phydev, 0x1B, regnum);
	if (ret < 0)
		return ret;

	val = phy_read(phydev, 0x1C);

	return val;
}

static int rtlphy_write_spec(struct phy_device *phydev, u32 regnum, u16 val)
{
	int ret;

	ret = phy_write(phydev, 0x1B, regnum);
	if (ret < 0)
		return ret;

	ret = phy_write(phydev, 0x1C, val);
	if (ret < 0)
		return ret;

	return ret;
}

static int rtlphy_read_normal(struct phy_device *phydev, u32 regnum, u16 page)
{
	int ret;
	int val;

	ret = phy_write(phydev, 0x1F, page);
	if (ret < 0)
		return ret;

	val = phy_read(phydev, regnum);

	return val;

}

static int rtlphy_write_normal(struct phy_device *phydev, u32 regnum, u16 val, u16 page)
{
	int ret;

	ret = phy_write(phydev, 0x1F, page);
	if (ret < 0)
		return ret;

	ret = phy_write(phydev, regnum, val);
	if (ret < 0)
		return ret;

	return ret;
}

static int rtl9000_config_init(struct phy_device *phydev)
{
	int ret;
	int val;
	int retry = 100;

	printk("enter %s %d \n", __func__, __LINE__);

	//phy ready
	while (val != 0x0003) {
		val = (rtlphy_read_normal(phydev, 0x10, 0x0A42)) & 0x0007;
		udelay(10);
		retry--;
		if (retry == 0)
			goto phy_init_fail;
	}

	rtlphy_write_spec(phydev, 0x8017, 0xFB03);
	rtlphy_write_spec(phydev, 0x8092, 0x6000);
	rtlphy_write_spec(phydev, 0x80AF, 0x6000);
	rtlphy_write_spec(phydev, 0x807D, 0x4443);
	rtlphy_write_spec(phydev, 0x809A, 0x4443);
	rtlphy_write_spec(phydev, 0x81A3, 0x0F00);
	rtlphy_write_normal(phydev, 0x12, 0x0004, 0x0A81);
	rtlphy_write_spec(phydev, 0x83C8, 0x0005);
	rtlphy_write_normal(phydev, 0x12, 0x0003, 0x0A5C);
	rtlphy_write_spec(phydev, 0xB820, 0x0010);
	rtlphy_write_spec(phydev, 0xB830, 0x8000);
	retry = 100;
	do {
		val = (rtlphy_read_spec(phydev, 0xB800)) & 0x40;
		udelay(10);
		retry--;
		if (retry == 0)
			goto phy_init_fail;
	} while (val == 0);

	rtlphy_write_spec(phydev, 0x8020, 0x3300);
	rtlphy_write_spec(phydev, 0xB82E, 0x0001);
	rtlphy_write_spec(phydev, 0xB820, 0x0290);
	rtlphy_write_spec(phydev, 0xA012, 0x0000);
	rtlphy_write_spec(phydev, 0xA014, 0x401C);
	phy_write(phydev, 0x1C, 0xA610);
	phy_write(phydev, 0x1C, 0x8520);
	phy_write(phydev, 0x1C, 0xA510);
	phy_write(phydev, 0x1C, 0x21F4);
	rtlphy_write_spec(phydev, 0xA01A, 0x0000);
	rtlphy_write_spec(phydev, 0xA000, 0x11EF);
	rtlphy_write_spec(phydev, 0xB820, 0x0210);
	rtlphy_write_spec(phydev, 0xB82E, 0x0000);
	rtlphy_write_spec(phydev, 0x8020, 0x0000);
	rtlphy_write_spec(phydev, 0xB820, 0x0000);
	retry = 100;
	do {
		val = (rtlphy_read_spec(phydev, 0xB800)) & 0x40;
		udelay(10);
		retry--;
		if (retry == 0)
			goto phy_init_fail;
	} while (val != 0);

	// I/O voltage 1.8V RGMII set
	rtlphy_write_normal(phydev, 0x12, 0x20FF, 0x0A4C);

	//default slave
	//master: phy_write(phydev, 0x9, (val | (1 << 11)));

	val = phy_read(phydev, 0x9);
	phy_write(phydev, 0x9, (val & (~(1 << 11))));

	//clock delay  default
	/*	spec 0xD082
	*	BIT 3:0 RX DELAY
	*	BIT 9:8 TX DELAY
	*	SEL:4 ns
	*/
	rtlphy_write_spec(phydev, 0xD012, 0x10);
	rtlphy_write_spec(phydev, 0xD082, 0x4018);


	//software reset
	val = phy_read(phydev, MII_BMCR);
	val |= YT_SOFTWARE_RESET;
	ret = phy_write(phydev, MII_BMCR, val);
	if (ret < 0)
		return ret;

	printk("%s finish\n", __func__);

	return ret;

phy_init_fail:
	printk("%s fail\n", __func__);
	return 0;
}

static int dp83tc812_config_init(struct phy_device *phydev)
{
	int ret;
	int val;
	unsigned int mask = 1 << 15;

	val = jlphy_mmd_read_ext(phydev, 0x1f, 0x1f);
	val = val | mask;
	ret = jlphy_mmd_write_ext(phydev, 0x1f, 0x1f, val);
	msleep(100);
	if (ret)
		printk("enter %s %d write reg fail\n", __func__, __LINE__);

	ret = jlphy_mmd_write_ext(phydev, 0x1, 0x0834, 0x8001);
	if (ret)
		printk("enter %s %d write reg fail\n", __func__, __LINE__);

	ret = jlphy_mmd_write_ext(phydev, 0x1F, 0x001F, 0x8000);
	if (ret)
		printk("enter %s %d \n", __func__, __LINE__);

	ret = jlphy_mmd_write_ext(phydev, 0x1F, 0x0523, 0x0001);
	if (ret)
		printk("enter %s %d write reg fail\n", __func__, __LINE__);

	ret = jlphy_mmd_write_ext(phydev, 0x1, 0x0834, 0x8001);
	if (ret)
		printk("enter %s %d \n", __func__, __LINE__);

	ret = jlphy_mmd_write_ext(phydev, 0x1F, 0x0873, 0x0821);
	if (ret)
		printk("enter %s %d write reg fail\n", __func__, __LINE__);

	ret = jlphy_mmd_write_ext(phydev, 0x1F, 0x0896, 0x22FF);
	if (ret)
		printk("enter %s %d write reg fail\n", __func__, __LINE__);

	ret = jlphy_mmd_write_ext(phydev, 0x1F, 0x089E, 0x0000);
	if (ret)
		printk("enter %s %d write reg fail\n", __func__, __LINE__);

	ret = jlphy_mmd_write_ext(phydev, 0x1F, 0x001F, 0x4000);
	if (ret)
		printk("enter %s %d write reg fail\n", __func__, __LINE__);

	ret = jlphy_mmd_write_ext(phydev, 0x1F, 0x0523, 0x0000);
	if (ret)
		printk("enter %s %d write reg fail\n", __func__, __LINE__);

	ret = jlphy_mmd_write_ext(phydev, 0x1F, 0x600, 0x38);	//enable rgmii
	if (ret)
		printk("enter %s %d write reg fail\n", __func__, __LINE__);

	//clock delay
	/*	REG 0x602
	*	BIT 1 RX DELAY
	*	BIT 0 TX DELAY
	*	SEL: 90 degrees
	*/
	ret = jlphy_mmd_write_ext(phydev, 0x1F, 0x602, 0x3);	//enable rgmii delay	
	if (ret)
		printk("enter %s %d write reg fail\n", __func__, __LINE__);

	ret = jlphy_mmd_write_ext(phydev, 0x1F, 0x608, 0x7b);	//disable sgmii
	if (ret)
		printk("enter %s %d write reg fail\n", __func__, __LINE__);

	val = jlphy_mmd_read_ext(phydev, 0x1f, 0x018B);
	mask = 1 << 6;
	val = val | mask;
	ret = jlphy_mmd_write_ext(phydev, 0x1f, 0x018B, val);
	if (ret)
		printk("enter %s %d write reg fail\n", __func__, __LINE__);

	val = jlphy_mmd_read_ext(phydev, 0x1f, 0x1f);
	mask = 1 << 14;
	val = val | mask;
	ret = jlphy_mmd_write_ext(phydev, 0x1f, 0x1f, val);
	if (ret)
		printk("enter %s %d write reg fail\n", __func__, __LINE__);
	msleep(100);

	printk("enter %s %d finish \n", __func__, __LINE__);
	return 0;
}

static int jl3101_config_init(struct phy_device *phydev)
{
	int ret;

	printk("enter %s\n", __func__);

	ret = jl3xxx_set_speed(phydev, PHY_SPEED_100M);
	if (ret < 0)
		return ret;

#if !defined(JL3XXX_RGMII_TO_SGMII_PHY)
	ret = jl3xxx_set_mode(phydev, PHY_MODE_MASTER);
	if (ret < 0)
		return ret;

	ret = jl3xxx_set_phy_inter(phydev, PHY_RGMII);
	if (ret < 0)
		return ret;

	ret = jlphy_mmd_write_ext(phydev, 3, 0x8800, 0xc830);
	if (ret < 0)
		return ret;
#else
	ret = jl3xxx_set_mode(phydev, PHY_MODE_SLAVE);
	if (ret < 0)
		return ret;

	ret = jl3xxx_set_phy_inter(phydev, PHY_RGMII_SGMII);
	if (ret < 0)
		return ret;

	/* SGMII CONFIG: sgmii_ucfg_cfg=1 */
	/* Do not check the status during PHY mode (fore link with Fiber) */
	jl3xxx_set_mmd_bits(phydev, 3, 0x8810, BIT(3)|BIT(2));
#endif

	/* TX or RX delay enabled and TX CLOCK DELAY LEVEL */
	//jl3xxx_set_rgmii_dly(phydev, true, true);

	//TX CLOCK DELAY LEVEL 1ns
	//jl3xxx_clear_mmd_bits(phydev, JL3XXX_IEEE_CTL_ADDR,
	//			JL3XXX_RGMII_CFG, JL3XXX_RGMII_TX_DLY_LEVEL_0);
	//jl3xxx_clear_mmd_bits(phydev, JL3XXX_IEEE_CTL_ADDR,
	//			JL3XXX_RGMII_CFG, JL3XXX_RGMII_TX_DLY_LEVEL_1);

	//TX CLOCK DELAY LEVEL 1.2ns
	//jl3xxx_set_mmd_bits(phydev, JL3XXX_IEEE_CTL_ADDR,
	//			JL3XXX_RGMII_CFG, JL3XXX_RGMII_TX_DLY_LEVEL_0);
	//jl3xxx_clear_mmd_bits(phydev, JL3XXX_IEEE_CTL_ADDR,
	//			JL3XXX_RGMII_CFG, JL3XXX_RGMII_TX_DLY_LEVEL_1);

	//TX CLOCK DELAY LEVEL 1.5ns
	//jl3xxx_clear_mmd_bits(phydev, JL3XXX_IEEE_CTL_ADDR,
	//			JL3XXX_RGMII_CFG, JL3XXX_RGMII_TX_DLY_LEVEL_0);
	//jl3xxx_set_mmd_bits(phydev, JL3XXX_IEEE_CTL_ADDR,
	//			JL3XXX_RGMII_CFG, JL3XXX_RGMII_TX_DLY_LEVEL_1);

	//TX CLOCK DELAY LEVEL 1.8ns
	//jl3xxx_set_mmd_bits(phydev, JL3XXX_IEEE_CTL_ADDR,
	//			JL3XXX_RGMII_CFG, JL3XXX_RGMII_TX_DLY_LEVEL_0);
	//jl3xxx_set_mmd_bits(phydev, JL3XXX_IEEE_CTL_ADDR,
	//			JL3XXX_RGMII_CFG, JL3XXX_RGMII_TX_DLY_LEVEL_1);

	//reset
	jl3xxx_set_mmd_bits(phydev, 1, 0, BIT(15));
	msleep(20);

	printk("jl3101_config_init finish\n");

	return ret;
}

int quecphy_config_aneg(struct phy_device *phydev)
{
	return 0;
}

static int quecphy_read_mmd(struct phy_device *phydev, int devnum, u16 regnum)
{
	return 1;
}

static int quecphy_write_mmd(struct phy_device *phydev, int devnum, u16 regnum, u16 val)
{
	return 1;
}

static int quecphy_read_status(struct phy_device *phydev)
{
	struct virtual_phy_priv *priv = phydev->priv;

	if (phydev->phy_id == PHY_ID_AR8033 || phydev->phy_id == PHY_ID_BCM89834 || phydev->phy_id == PHY_ID_RTL9000BRG
		|| phydev->phy_id == PHY_ID_DP83TC812) {
		phydev->link = 1;
		phydev->speed = 100;
		phydev->duplex = DUPLEX_FULL;
		phydev->autoneg = AUTONEG_DISABLE;
		phydev->pause = 0;
		phydev->asym_pause = 0;
	} else if (phydev->phy_id == PHY_ID_JL3101) {
#if defined(JL3XXX_FORCE_LINK_STATUS)
		phydev->link = 1;
#else
		phydev->link = jl3xxx_check_link(phydev);
#endif
		phydev->speed = 100;
		phydev->duplex = DUPLEX_FULL;
		phydev->autoneg = AUTONEG_DISABLE;
		phydev->pause = 0;
		phydev->asym_pause = 0;
	} else if (phydev->phy_id == PHY_ID_YT8010) {
		// phydev->link = yt8010_read_status(phydev);
		phydev->link = 1;
		phydev->speed = 100;
		phydev->duplex = DUPLEX_FULL;
		phydev->autoneg = AUTONEG_DISABLE;
		phydev->pause = 0;
		phydev->asym_pause = 0;
	} else if ( phydev->phy_id == PHY_ID_YT8010AN) {
		phydev->link = yt8010an_read_status(phydev);
		// phydev->link = 1;
		phydev->speed = 100;
		phydev->duplex = DUPLEX_FULL;
		phydev->autoneg = AUTONEG_DISABLE;
		phydev->pause = 0;
		phydev->asym_pause = 0;
	} else if (phydev->phy_id == PHY_ID_YT8011) {
		// phydev->link = yt8011_read_status(phydev);
		phydev->link = 1;
		phydev->speed = 1000;
		phydev->duplex = DUPLEX_FULL;
		phydev->autoneg = AUTONEG_DISABLE;
		phydev->pause = 0;
		phydev->asym_pause = 0;
	} else if (phydev->phy_id == PHY_ID_YT8521 || phydev->phy_id == PHY_ID_JL311A1) {
		phydev->link = 1;
		phydev->speed = 1000;
		phydev->duplex = DUPLEX_FULL;
		phydev->autoneg = AUTONEG_DISABLE;
		phydev->pause = 0;
		phydev->asym_pause = 0;
	} else {
		phydev->link = priv->virtual_phy_link;
		phydev->speed = priv->virtual_phy_speed;
		phydev->duplex = DUPLEX_FULL;
		phydev->autoneg = AUTONEG_DISABLE;
		phydev->pause = 0;
		phydev->asym_pause = 0;
	}
	return 0;
}

static int quecphy_aneg_done(struct phy_device *phydev)
{
	return BMSR_ANEGCOMPLETE;
}

static int quecphy_config_init(struct phy_device *phydev)
{
	if (phydev->phy_id == PHY_ID_YT8521)
		yt8521_config_init(phydev);
	else if (phydev->phy_id == PHY_ID_YT8010)
		yt8010_config_init(phydev);
	else if (phydev->phy_id == PHY_ID_YT8010AN)
		yt8010an_config_init(phydev);
	else if (phydev->phy_id == PHY_ID_AR8033)
		ar8033_config_init(phydev);
	else if (phydev->phy_id == PHY_ID_BCM89834)
		bcm89834_config_init(phydev);
	else if (phydev->phy_id == PHY_ID_JL311A1)
		jl3xxx_config_init(phydev);
	else if (phydev->phy_id == PHY_ID_YT8011)
		yt8011_config_init(phydev);
	else if (phydev->phy_id == PHY_ID_RTL9000BRG)
		rtl9000_config_init(phydev);
	else if (phydev->phy_id == PHY_ID_DP83TC812)
		dp83tc812_config_init(phydev);
	else if (phydev->phy_id == PHY_ID_JL3101)
		jl3101_config_init(phydev);
	else
		return 0;
	return 0;
}

static int quecphy_soft_reset(struct phy_device *phydev)
{
	return 0;
}

static int quecphy_probe(struct phy_device *phydev)
{
	struct device *dev = &phydev->mdio.dev;
	struct virtual_phy_priv *priv;

	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->virtual_phy_speed = 1000;

	if (of_property_read_bool(dev->of_node, "switch,use-virtual-phy")) {
		/* Use the switch configuration */
		priv->virtual_phy_link = 1;
		if (of_property_read_u32(dev->of_node, "speed", &priv->virtual_phy_speed)) {
			dev_info(dev, "virtual PHY speed not specified, using default 1000M\n");
		}
		dev_info(dev, "virtual PHY enabled with speed %d Mbps\n", priv->virtual_phy_speed);
	} else {
		/* Use the standard default configuration */
		priv->virtual_phy_link = 0;
	}

	phydev->priv = priv;

	return 0;
}

static void quecphy_remove(struct phy_device *phydev)
{

}

static struct phy_driver quecphy_driver[] = {
{
	.phy_id                 = 0x12345678,
	.name                   = "Quectel PHY",
	.phy_id_mask            = 0xffffffff,
	.probe                  = quecphy_probe,
	.soft_reset             = quecphy_soft_reset,
	.config_init            = quecphy_config_init,
	.read_mmd               = quecphy_read_mmd,
	.write_mmd              = quecphy_write_mmd,
	.features               = PHY_GBIT_FEATURES,
	.flags                  = 0,
	.config_aneg            = quecphy_config_aneg,
	.read_status            = quecphy_read_status,
	.aneg_done              = quecphy_aneg_done,
	.remove                 = quecphy_remove,
},
{
	.phy_id                 = PHY_ID_YT8521,
	.name                   = "YT8521 Ethernet",
	.phy_id_mask            = 0xffffffff,
	.probe                  = quecphy_probe,
	.soft_reset             = quecphy_soft_reset,
	.config_init            = quecphy_config_init,
	.read_mmd               = quecphy_read_mmd,
	.write_mmd              = quecphy_write_mmd,
	.features               = PHY_GBIT_FEATURES,
	.flags                  = 0,
	.config_aneg            = quecphy_config_aneg,
	.read_status            = quecphy_read_status,
	.aneg_done              = quecphy_aneg_done,
	.remove                 = quecphy_remove,
},
{
	.phy_id                 = PHY_ID_YT8010,
	.name                   = "YT8010 Ethernet",
	.phy_id_mask            = 0xffffffff,
	.probe                  = quecphy_probe,
	.soft_reset             = quecphy_soft_reset,
	.config_init            = quecphy_config_init,
	.read_mmd               = quecphy_read_mmd,
	.write_mmd              = quecphy_write_mmd,
	.features               = PHY_GBIT_FEATURES,
	.flags                  = 0,
	.config_aneg            = quecphy_config_aneg,
	.read_status            = quecphy_read_status,
	.aneg_done              = quecphy_aneg_done,
	.remove                 = quecphy_remove,
},
{
	.phy_id                 = PHY_ID_YT8010AN,
	.name                   = "YT8010AN Ethernet",
	.phy_id_mask            = 0xfffffff0,
	.probe                  = quecphy_probe,
	.soft_reset             = quecphy_soft_reset,
	.config_init            = quecphy_config_init,
	.read_mmd               = quecphy_read_mmd,
	.write_mmd              = quecphy_write_mmd,
	.features               = PHY_GBIT_FEATURES,
	.flags                  = 0,
	.config_aneg            = quecphy_config_aneg,
	.read_status            = quecphy_read_status,
	.aneg_done              = quecphy_aneg_done,
	.remove                 = quecphy_remove,
},
{
	.phy_id                 = PHY_ID_AR8033,
	.name                   = "AR8033 Ethernet",
	.phy_id_mask            = 0xffffffff,
	.probe                  = quecphy_probe,
	.soft_reset             = quecphy_soft_reset,
	.config_init            = quecphy_config_init,
	.read_mmd               = quecphy_read_mmd,
	.write_mmd              = quecphy_write_mmd,
	.features               = PHY_GBIT_FEATURES,
	.flags                  = 0,
	.config_aneg            = quecphy_config_aneg,
	.read_status            = quecphy_read_status,
	.aneg_done              = quecphy_aneg_done,
	.remove                 = quecphy_remove,
},
{
	.phy_id                 = PHY_ID_BCM89834,
	.name                   = "BCM89834 Ethernet",
	.phy_id_mask            = 0xffffffff,
	.probe                  = quecphy_probe,
	.soft_reset             = quecphy_soft_reset,
	.config_init            = quecphy_config_init,
	.read_mmd               = quecphy_read_mmd,
	.write_mmd              = quecphy_write_mmd,
	.features               = PHY_GBIT_FEATURES,
	.flags                  = 0,
	.config_aneg            = quecphy_config_aneg,
	.read_status            = quecphy_read_status,
	.aneg_done              = quecphy_aneg_done,
	.remove                 = quecphy_remove,
},
{
	.phy_id                 = PHY_ID_JL311A1,
	.name                   = "JL311A1 Ethernet",
	.phy_id_mask            = 0xffffffff,
	.probe                  = quecphy_probe,
	.soft_reset             = quecphy_soft_reset,
	.config_init            = quecphy_config_init,
	.read_mmd               = quecphy_read_mmd,
	.write_mmd              = quecphy_write_mmd,
	.features               = PHY_GBIT_FEATURES,
	.flags                  = 0,
	.config_aneg            = quecphy_config_aneg,
	.read_status            = quecphy_read_status,
	.aneg_done              = quecphy_aneg_done,
	.remove                 = quecphy_remove,
},
{
	.phy_id                 = PHY_ID_YT8011,
	.name                   = "YT8011 Ethernet",
	.phy_id_mask            = 0xffffffff,
	.probe                  = quecphy_probe,
	.soft_reset             = quecphy_soft_reset,
	.config_init            = quecphy_config_init,
	.read_mmd               = quecphy_read_mmd,
	.write_mmd              = quecphy_write_mmd,
	.features               = PHY_GBIT_FEATURES,
	.flags                  = 0,
	.config_aneg            = quecphy_config_aneg,
	.read_status            = quecphy_read_status,
	.aneg_done              = quecphy_aneg_done,
	.remove                 = quecphy_remove,
},
{
	.phy_id                 = PHY_ID_RTL9000BRG,
	.name                   = "RTL9000BRG Ethernet",
	.phy_id_mask            = 0xffffffff,
	.probe                  = quecphy_probe,
	.soft_reset             = quecphy_soft_reset,
	.config_init            = quecphy_config_init,
	.read_mmd               = quecphy_read_mmd,
	.write_mmd              = quecphy_write_mmd,
	.features               = PHY_GBIT_FEATURES,
	.flags                  = 0,
	.config_aneg            = quecphy_config_aneg,
	.read_status            = quecphy_read_status,
	.aneg_done              = quecphy_aneg_done,
	.remove                 = quecphy_remove,
},
{
	.phy_id                 = PHY_ID_DP83TC812,
	.name                   = "DP83TC812 Ethernet",
	.phy_id_mask            = 0xffffffff,
	.probe                  = quecphy_probe,
	.soft_reset             = quecphy_soft_reset,
	.config_init            = quecphy_config_init,
	.read_mmd               = quecphy_read_mmd,
	.write_mmd              = quecphy_write_mmd,
	.features               = PHY_GBIT_FEATURES,
	.flags                  = 0,
	.config_aneg            = quecphy_config_aneg,
	.read_status            = quecphy_read_status,
	.aneg_done              = quecphy_aneg_done,
	.remove                 = quecphy_remove,
},
{
	.phy_id                 = PHY_ID_JL3101,
	.name                   = "Jl3101 Ethernet",
	.phy_id_mask            = 0xffffffff,
	.probe                  = quecphy_probe,
	.soft_reset             = quecphy_soft_reset,
	.config_init            = quecphy_config_init,
	.read_mmd               = quecphy_read_mmd,
	.write_mmd              = quecphy_write_mmd,
	.features               = PHY_GBIT_FEATURES,
	.flags                  = 0,
	.config_aneg            = quecphy_config_aneg,
	.read_status            = quecphy_read_status,
	.aneg_done              = quecphy_aneg_done,
	.remove                 = quecphy_remove,
},
};

module_phy_driver(quecphy_driver);

static struct mdio_device_id __maybe_unused quecphy_tbl[] = {
	{ QUECTEL_PHY_ID, 0xffffffff },
	{ PHY_ID_YT8521, 0xffffffff },
	{ PHY_ID_YT8010, 0xffffffff },
	{ PHY_ID_YT8010AN, 0xffffffff },
	{ PHY_ID_AR8033, 0xffffffff },
	{ PHY_ID_BCM89834, 0xffffffff },
	{ PHY_ID_JL311A1, 0xffffffff },
	{ PHY_ID_YT8011, 0xffffffff },
	{ PHY_ID_RTL9000BRG, 0xffffffff },
	{ PHY_ID_DP83TC812, 0xffffffff },
	{ PHY_ID_JL3101, 0xffffffff },
};

/*eyelyn  20220707 add Troubleshoot an unmatched PHY, causing dump*/
/* 0: clause22  1: clause45 */
quectel_phy_struct quectel_phy[] = {
	{ QUECTEL_PHY_ID,         0 },
	{ PHY_ID_YT8521,          0 },
	{ PHY_ID_YT8010,          0 },
	{ PHY_ID_YT8010AN,        0 },
	{ PHY_ID_AR8033,          0 },
	{ PHY_ID_BCM89834,        0 },
	{ PHY_ID_JL311A1,         0 },
	{ PHY_ID_YT8011,          0 },
	{ PHY_ID_RTL9000BRG,      0 },
	{ PHY_ID_DP83TC812,       0 },
	{ PHY_ID_JL3101,          0 },
};

int quectel_probe_clause_byphyid(const u32 phy_id)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(quectel_phy); i++) {
		if (phy_id == quectel_phy[i].phy_id)
			return quectel_phy[i].clause;
	}
	return -1;
}
EXPORT_SYMBOL(quectel_probe_clause_byphyid);

MODULE_DEVICE_TABLE(mdio, quecphy_tbl);
