/* touch_sw49105.h
 *
 * Copyright (C) 2015 LGE.
 *
 * Author: PH1-BSP-Touch@lge.com
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef LGE_TOUCH_SW49105_H
#define LGE_TOUCH_SW49105_H

//#define SW49105_TCL_OFF_VIA_MIPI //[bringup] needed to check
//#define SW49105_ESD_SKIP_WHILE_TOUCH_ON //[bringup] needed to check

/* debug info */
#define DEBUG_PROTOCOL_PACKET_NUM	1
#define DEBUG_BUF_SIZE 112

struct sw49105_touch_debug {
	u32 padding[3];	/* packet structure 4+4+4+12*10+12+112 */
	u8 protocol_ver;
	u8 reserved_1;
	u32 frame_cnt;
	u8 rn_max_bfl;
	u8 rn_max_afl;
	u8 rn_min_bfl;
	u8 rn_min_afl;
	u8 rn_max_afl_x;
	u8 rn_max_afl_y;
	s8 lf_oft[18];
	u8 seg1_cnt:4;
	u8 seg2_cnt:4;
	u8 seg1_thr;
	u8 rebase[8];
	u8 rn_pos_cnt;
	u8 rn_neg_cnt;
	u8 rn_pos_sum;
	u8 rn_neg_sum;
	u8 rn_stable;
	u8 track_bit[10];
	u8 rn_max_tobj[12];
	u8 palm[8];
	u8 noise_detect[8];
	u8 reserved_2[29];
} __packed;

/* report packet */

struct sw49105_touch_data {
	u8 tool_type:4;
	u8 event:4;
	s8 track_id;
	u16 x;
	u16 y;
	u8 pressure;
	u8 angle;
	u16 width_major;
	u16 width_minor;
} __packed;

struct sw49105_touch_info {
	u32 ic_status;
	u32 device_status;
	u32 wakeup_type:8;
	u32 touch_cnt:5;
	u32 button_cnt:3;
	u32 palm_bit:16;
	struct sw49105_touch_data data[10];
	struct sw49105_touch_debug debug[DEBUG_PROTOCOL_PACKET_NUM];
} __packed;

#define PALM_ID				15

// [bringup] start
#define VCHIP_VAL		10
#define VPROTO_VAL		4
// [bringup] end

/* device control */
#define tc_version				(0x242)
#define tc_product_id1			(0x244)
#define tc_product_id2			(0x245)
#define tc_ic_status			(0x200) /* sw49105_touch_info base addr*/
#define tc_status				(0x201)

#define spr_subdisp_st			(0x021)
#define spr_boot_st				(0x011)

#define tc_device_ctl			(0xC00)
#define tc_interrupt_ctl		(0xC01)
#define tc_interrupt_status		(0xC02)
#define tc_drive_ctl			(0xC03)

#define info_chip_revision		(0x27C)
#define info_lot_num			(0x27E)
#define info_serial_num			(0x27F)
#define info_date				(0x280)
#define info_time				(0x281)

#define TCI_ENABLE_W			(0xC20)
#define TAP_COUNT_W				(0xC21)
#define MIN_INTERTAP_W			(0xC22)
#define MAX_INTERTAP_W			(0xC23)
#define TOUCH_SLOP_W			(0xC24)
#define TAP_DISTANCE_W			(0xC25)
#define INT_DELAY_W				(0xC26)
#define ACT_AREA_X1_W			(0xC27)
#define ACT_AREA_Y1_W			(0xC28)
#define ACT_AREA_X2_W			(0xC29)
#define ACT_AREA_Y2_W			(0xC2A)
#define ACT_SENSELESS_AREA_W		(0x21) // 33 pixel == 3mm

#define SWIPE_ENABLE_W			(0xC30)
#define SWIPE_DIST_W			(0xC31)
#define SWIPE_RATIO_THR_W		(0xC32)
#define SWIPE_RATIO_PERIOD_W	(0xC34)
#define SWIPE_RATIO_DIST_W		(0xC33)
#define SWIPE_TIME_MIN_W		(0xC35)
#define SWIPE_TIME_MAX_W		(0xC36)
#define SWIPE_ACT_AREA_X1_W		(0xC37)
#define SWIPE_ACT_AREA_Y1_W		(0xC38)
#define SWIPE_ACT_AREA_X2_W		(0xC39)
#define SWIPE_ACT_AREA_Y2_W		(0xC3A)

#define LPWG_DEBUG_CTRL			(0xC70)
#define TCI_DEBUG_FAIL_BIT_CTRL	(0xC71)
#define SWIPE_DEBUG_FAIL_BIT_CTRL	(0xC72)
#define LPWG_DEBUG_FAIL_STATUS		(0xC73)
#define TCI_DEBUG_FAILREASON_BUFFER		(0xC74)
#define SWIPE_DEBUG_FAILREASON_BUFFER	(0xC75)

#define REG_IME_STATE			(0xC51)
#define REG_CALL_STATE			(0xC54)
#define REG_QMEMO_STATE		(0xC55)
#define R_HEADER_SIZE			(0)
#define W_HEADER_SIZE			(2)

#define CMD_DIS			0xAA
#define CMD_ENA			0xAB
#define CMD_CLK_ON		0x83
#define CMD_CLK_OFF		0x82
#define CMD_OSC_ON		0x81
#define CMD_OSC_OFF		0x80
#define CMD_RESET_LOW	0x84
#define CMD_RESET_HIGH	0x85


#define MASK_NORMAL_DEVICE_STATUS 		(0x5000E0) // [bringup] (0x5080E)
/* (1 << 0) || (1 << 3) */
#define MASK_ABNORMAL_IC_STATUS 		(0x009)
/* (1 << 9) || (1 << 10) */
#define MASK_ABNORMAL_DEVICE_STATUS		(0x600)
/* (1 << 13)|(1 << 15)|(1 << 20)|(1 << 22) */
#define INT_LOGGING_CLR_BIT				(0x50A000)


/* charger status */
#define SPR_CHARGER_STS			(0xC50)

#define CONNECT_NONE			(0x00)
#define CONNECT_USB			(0x01)
#define CONNECT_TA			(0x02)
#define CONNECT_OTG			(0x03)
#define CONNECT_WIRELESS		(0x10)
/* ASC */
#define MAX_DELTA			(0x2AD)
#define TOUCH_MAX_W			(0xC53)
#define TOUCH_MAX_R			(0x2A1)

//----Added for FW Upgrade------------
#define CFG_MAGIC_CODE		0xCACACACA
#define CFG_CHIP_ID			49105
#define CFG_C_MAX_SIZE		2048
#define CFG_S_MAX_SIZE		4048
//---------------------------------


enum {
	SW_RESET = 0,
	HW_RESET_ASYNC,
	HW_RESET_SYNC,
};

enum {
	TOUCHSTS_IDLE = 0,
	TOUCHSTS_DOWN,
	TOUCHSTS_MOVE,
	TOUCHSTS_UP,
};

enum {
	ABS_MODE = 0,
	KNOCK_1,
	KNOCK_2,
	SWIPE_RIGHT,
	SWIPE_LEFT,
	CUSTOM_DEBUG = 200,
	KNOCK_OVERTAP = 201,
};

enum {
	LCD_MODE_U0 = 0,
	LCD_MODE_U2_UNBLANK,
	LCD_MODE_U2,
	LCD_MODE_U3,
	LCD_MODE_U3_PARTIAL,
	LCD_MODE_U3_QUICKCOVER,
	LCD_MODE_STOP,
};

enum {
	SWIPE_R = 0,
	SWIPE_L,
};

enum {
	SWIPE_RIGHT_BIT	= 1,
	SWIPE_LEFT_BIT	= 1 << 16,
};

/* swipe */
enum {
	SWIPE_ENABLE_CTRL = 0,
	SWIPE_DISABLE_CTRL,
	SWIPE_DIST_CTRL,
	SWIPE_RATIO_THR_CTRL,
	SWIPE_RATIO_PERIOD_CTRL,
	SWIPE_RATIO_DIST_CTRL,
	SWIPE_TIME_MIN_CTRL,
	SWIPE_TIME_MAX_CTRL,
	SWIPE_AREA_CTRL,
};

enum {
	IC_INIT_NEED = 0,
	IC_INIT_DONE,
};

enum {
	REV_0 = 0,
	REV_6,     /* revision 6   */
	REV_8 = 8, /* revision 7/8 */
	REV_9,     /* revision 9   */
};

/* SPR control */

/* Firmware control */
#define sys_rst_ctl			(0x006)
#define sys_boot_ctl		(0x00F)
#define sys_sram_ctl		(0x010)
#define spr_code_offset		(0x07D)//(0x078)
#define spr_data_offset		(0x082)//(0x07B)
#define tc_flash_dn_ctl		(0xC05)
#define tc_flash_dn_sts		(0x247)
#define tc_confdn_base_addr	(0x2F9)
#define code_access_addr	(0xFD0)//(0x300)
#define data_access_addr	(0xFD1)//(0x301)

#define MAX_RW_SIZE			(1 * 1024)
#define FLASH_FW_SIZE		(80*1024)//(69 * 1024)
#define FLASH_CONF_SIZE		(1 * 1024)
#define FLASH_OTP_SIZE		(4 * 1024)


#define FLASH_KEY_CODE_CMD		0xDFC1
#define FLASH_KEY_CONF_CMD		0xE87B
#define FLASH_KEY_OTPM_CMD      0xAE71
#define FLASH_BOOTCHK_VALUE		0x0A0A0000

//changed as per SW49105 datasheet
#define FLASH_CODE_DNCHK_VALUE		0x42// [bringup] 0x03//0x42-->0x03
#define FLASH_CONF_DNCHK_VALUE		0x84// [bringup] 0x0D//0x8C-->0x0D
#define FLASH_OTP_DNCHK_VALUE		0xD8// [bringup] 0x09


//-------Added for Fw Upgrade---------
enum {
	E_FW_CODE_SIZE_ERR = 1,
	E_FW_CODE_ONLY_VALID = 2,
	E_FW_CODE_AND_CFG_VALID = 3,
	E_FW_CODE_CFG_ERR = 4
};

typedef union {
	struct {
		unsigned common_cfg_size : 16;
		unsigned specific_cfg_size : 16;
	} b;
	u32 w;
} t_cfg_size;

typedef union
{
struct {
	unsigned chip_rev : 8;
	unsigned model_id : 8;
	unsigned lcm_id : 8;
	unsigned fpcb_id : 8;
} b;
uint32_t w;
} t_cfg_specific_info1;

typedef union
{
struct {
unsigned lot_id : 8;
unsigned reserved : 24;
} b;
uint32_t w;
} t_cfg_specific_info2;

typedef struct
{
	t_cfg_specific_info1 cfg_specific_info1; //0x0000
	t_cfg_specific_info2 cfg_specific_info2; //0x0001
	uint32_t cfg_specific_version; 			//0x0002
	uint32_t cfg_model_name; 	   			//0x0003
	uint32_t cfg_header_reserved1; 			//0x0004
	uint32_t cfg_header_reserved2; 			//0x0005
	uint32_t cfg_header_reserved3; 			//0x0006
	uint32_t cfg_header_reserved4; 			//0x0007
}t_cfg_s_header_def;

//needed to check if really exist or not
typedef struct {
	u32 cfg_common_ver;
} t_cfg_c_header_def;

typedef struct
{
	uint32_t cfg_magic_code;      //0x0000
	uint32_t cfg_info_reserved0;  //0x0001
	uint32_t cfg_chip_id;         //0x0002
	uint32_t cfg_struct_version;  //0x0003
	uint32_t cfg_specific_cnt;    //0x0004
	t_cfg_size cfg_size;          //0x0005
	uint32_t cfg_global_date; 	  //0x0006
	uint32_t cfg_global_time;     //0x0007
	uint32_t cfg_info_reserved1;  //0x0008
	uint32_t cfg_info_reserved2;  //0x0009
	uint32_t cfg_info_reserved3;  //0x000A
	uint32_t cfg_info_reserved4;  //0x000B
}t_cfg_info_def;
//-------------------------------------------

/* test control */

struct sw49105_fw_info {
	u8 version[2];
	u8 product_id[8];
	u8 image_version[2];
	u8 image_product_id[8];
	u8 revision;
};

struct sw49105_asc_info {
	u16 normal_s;
	u16 acute_s;
	u16 obtuse_s;
};

struct swipe_info {
	u8	distance;
	u8	ratio_thres;
	u8	ratio_distance;
	u8	ratio_period;
	u16	min_time;
	u16	max_time;
	struct active_area area;
};

struct swipe_ctrl {
	u32 mode;
	struct swipe_info info[2]; /* down is 0, up 1 - sw49105 use up */
};

struct sw49105_data {
	struct device *dev;
	struct kobject kobj;
	struct sw49105_touch_info info;
	struct sw49105_fw_info fw;
	struct sw49105_asc_info asc;
	u8 lcd_mode;
	u8 u3fake;
	//struct watch_data watch;
	struct swipe_ctrl swipe;
	struct mutex spi_lock;
	struct delayed_work font_download_work;
	struct delayed_work fb_notify_work;
	u32 charger;
	u32 earjack;
	u8 tci_debug_type;
	u8 swipe_debug_type;
	//atomic_t block_watch_cfg;
	atomic_t init;
};

#define TCI_MAX_NUM			2
#define SWIPE_MAX_NUM			2
#define TCI_DEBUG_MAX_NUM			16
#define SWIPE_DEBUG_MAX_NUM			8
#define DISTANCE_INTER_TAP		(0x1 << 1) /* 2 */
#define DISTANCE_TOUCHSLOP		(0x1 << 2) /* 4 */
#define TIMEOUT_INTER_TAP_LONG		(0x1 << 3) /* 8 */
#define MULTI_FINGER			(0x1 << 4) /* 16 */
#define DELAY_TIME			(0x1 << 5) /* 32 */
#define PALM_STATE		(0x1 << 6) /* 64 */
#define OUTOF_AREA			(0x1 << 7) /* 128 */

#define TCI_DEBUG_ALL (DISTANCE_INTER_TAP | DISTANCE_TOUCHSLOP |\
	TIMEOUT_INTER_TAP_LONG | MULTI_FINGER | DELAY_TIME |\
	PALM_STATE | OUTOF_AREA)

static inline struct sw49105_data *to_sw49105_data(struct device *dev)
{
	return (struct sw49105_data *)touch_get_device(to_touch_core(dev));
}

static inline struct sw49105_data *to_sw49105_data_from_kobj(struct kobject *kobj)
{
	return (struct sw49105_data *)container_of(kobj,
			struct sw49105_data, kobj);
}

int sw49105_reg_read(struct device *dev, u16 addr, void *data, int size);
int sw49105_reg_write(struct device *dev, u16 addr, void *data, int size);
int sw49105_ic_info(struct device *dev);
int sw49105_tc_driving(struct device *dev, int mode);
int sw49105_irq_abs(struct device *dev);
int sw49105_irq_lpwg(struct device *dev);
int sw49105_irq_handler(struct device *dev);
int sw49105_check_status(struct device *dev);
int sw49105_debug_info(struct device *dev);
int sw49105_reset_ctrl(struct device *dev, int ctrl);

static inline int sw49105_read_value(struct device *dev,
					u16 addr, u32 *value)
{
	return sw49105_reg_read(dev, addr, value, sizeof(*value));
}

static inline int sw49105_write_value(struct device *dev,
					 u16 addr, u32 value)
{
	return sw49105_reg_write(dev, addr, &value, sizeof(value));
}

#endif /* LGE_TOUCH_SW49105_H */
