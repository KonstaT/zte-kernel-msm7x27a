/* Copyright (c) 2008-2011, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "msm_fb.h"
#include "mipi_dsi.h"
#include "mipi_lead.h"
#include <mach/gpio.h>
#ifdef CONFIG_ZTE_PLATFORM
#include <mach/zte_memlog.h>
#endif
static struct dsi_buf lead_tx_buf;
static struct dsi_buf lead_rx_buf;
extern u32 LcdPanleID;
static int lcd_bkl_ctl=97;
#define GPIO_LCD_RESET 129

static bool onewiremode = false;

/*ic define*/
#define HIMAX_8363 		1
#define HIMAX_8369 		2
#define NOVATEK_35510	3

#define HIMAX8369_TIANMA_TN_ID		0xB1
#define HIMAX8369_TIANMA_IPS_ID		0xA5
#define HIMAX8369_LEAD_ID				0
#define HIMAX8369_LEAD_HANNSTAR_ID	0x88
#define NT35510_YUSHUN_ID				0
#define NT35510_LEAD_ID				0xA0

/*about icchip sleep and display on */
static char display_off[2] = {0x28, 0x00};
static char enter_sleep[2] = {0x10, 0x00};
//static char exit_sleep[2] = {0x11, 0x00};
//static char display_on[2] = {0x29, 0x00};
static struct dsi_cmd_desc display_off_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 50, sizeof(display_off), display_off},
	{DTYPE_DCS_WRITE, 1, 0, 0, 120, sizeof(enter_sleep), enter_sleep}
};


/*about himax8363 chip id */
static char hx8363_setpassword_para[4]={0xB9,0xFF,0x83,0x63};
static char hx8363_icid_rd_para[2] = {0xB9, 0x00}; 
   
static struct dsi_cmd_desc hx8363_icid_rd_cmd = 
{
	DTYPE_DCS_READ, 1, 0, 0, 0, sizeof(hx8363_icid_rd_para), hx8363_icid_rd_para
};
static struct dsi_cmd_desc hx8363_setpassword_cmd[] = 
{	
	{DTYPE_DCS_LWRITE, 1, 0, 0, 1, sizeof(hx8363_setpassword_para),hx8363_setpassword_para},

};

/*about himax8369 chip id */
static char hx8369_setpassword_para[4]={0xB9,0xFF,0x83,0x69};
static char hx8369_icid_rd_para[2] = {0xB9, 0x00}; 
static char hx8369_panleid_rd_para[2] = {0xda, 0x00};    


static struct dsi_cmd_desc hx8369_icid_rd_cmd = 
{
	DTYPE_DCS_READ, 1, 0, 0, 0, sizeof(hx8369_icid_rd_para), hx8369_icid_rd_para
};
static struct dsi_cmd_desc hx8369_setpassword_cmd[] = 
{	
	{DTYPE_DCS_LWRITE, 1, 0, 0, 1, sizeof(hx8369_setpassword_para),hx8369_setpassword_para},

};
static struct dsi_cmd_desc hx8369_panleid_rd_cmd = 
{
	DTYPE_DCS_READ, 1, 0, 0, 0, sizeof(hx8369_panleid_rd_para), hx8369_panleid_rd_para
};


/*about Novatek3511 chip id */
static char nt3511_page_ff[5] = {0xff, 0xaa,0x55,0x25,0x01};
static char nt3511_page_f8[20] = {0xf8, 0x01,0x12,0x00,0x20,0x33,0x13,0x00,0x40,0x00,0x00,0x23,0x01,0x99,0xc8,0x00,0x00,0x01,0x00,0x00};
static char nt3511_icid_rd_para[2] = {0xc5, 0x00}; 
static char nt3511_panleid_rd_para[2] = {0xDA, 0x00};    //added by zte_gequn091966 for lead_nt35510,20111226

static struct dsi_cmd_desc nt3511_setpassword_cmd[] = {	
	{DTYPE_DCS_LWRITE, 1, 0, 0, 1, sizeof(nt3511_page_ff),nt3511_page_ff},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 1, sizeof(nt3511_page_f8),nt3511_page_f8}
};
static struct dsi_cmd_desc nt3511_icid_rd_cmd = {
	DTYPE_DCS_READ, 1, 0, 0, 0, sizeof(nt3511_icid_rd_para), nt3511_icid_rd_para};


static struct dsi_cmd_desc nt3511_panleid_rd_cmd = {
	DTYPE_DCS_READ, 1, 0, 0, 0, sizeof(nt3511_panleid_rd_para), nt3511_panleid_rd_para
};   //added by zte_gequn091966 for lead_nt35510,20111226

#if 1
/**************************************
trulylg HX8357C initial
***************************************/
static char hx8357c_trulylg_para_0xb9[4]={0xB9,0xff,0x83,0x57};  
static char hx8357c_trulylg_para_0xb3[5]={0xb3,0x43,0x00,0x06,0x06}; 
static char hx8357c_trulylg_para_0x3a[2]={0x3a,0x77};  
static char hx8357c_trulylg_para_0xe9[2]={0xe9,0x20};
static char hx8357c_trulylg_para_0xcc[2]={0xcc,0x07};  
static char hx8357c_trulylg_para_0xb6[2]={0xb6,0x50};  
static char hx8357c_trulylg_para_0xb1[7]={0xb1,0x00,0x15,0x1c,0x1c,0x83,0xaa};  
static char hx8357c_trulylg_para_0xb4[8]={0xb4,0x02,0x40,0x00,0x2a,0x2a,0x0d,0x4f}; 
static char hx8357c_trulylg_para_0xb0[3]={0xb0,0x68,0x01};  
static char hx8357c_trulylg_para_0xc0[7]={0xc0,0x33,0x50,0x01,0x7c,0x1e,0x08}; 
static char hx8357c_trulylg_para_0xba[17] = {0xba,0x00,0x56,0xd4,0x00,0x0a,0x00,0x10,0x32,0x6e,0x04,0x05,0x9a,0x14,0x19,0x10,0x40};
static char hx8357c_trulylg_para_0xe0[68]={0xe0,0x00,0x00,0x02,0x00,0x0A,0x00,0x11,0x00,0x1D,0x00,0x23,0x00,0x35,0x00,0x41,0x00,0x4B,0x00,0x4B,0x00,0x42,0x00,0x3A,0x00,0x27,0x00,0x1B,0x00,0x12,0x00,0x0C,0x00,0x03,0x01,0x02,0x00,0x0A,0x00,0x11,0x00,0x1D,0x00,0x23,0x00,0x35,0x00,0x41,0x00,0x4B,0x00,0x4B,0x00,0x42,0x00,0x3A,0x00,0x27,0x00,0x1B,0x00,0x12,0x00,0x0C,0x00,0x03,0x00,0x00}; 
static char hx8357c_trulylg_para_0x11[2]={0x11,0x00}; 
static char hx8357c_trulylg_para_0x29[2]={0x29,0x00}; 



static struct dsi_cmd_desc hx8357c_trulylg_display_on_cmds[] = 
{

	{DTYPE_DCS_LWRITE, 1, 0, 0, 10, sizeof(hx8357c_trulylg_para_0xb9), hx8357c_trulylg_para_0xb9},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8357c_trulylg_para_0xb3), hx8357c_trulylg_para_0xb3},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(hx8357c_trulylg_para_0x3a), hx8357c_trulylg_para_0x3a},	
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(hx8357c_trulylg_para_0xe9), hx8357c_trulylg_para_0xe9},	
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(hx8357c_trulylg_para_0xcc),hx8357c_trulylg_para_0xcc},	
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(hx8357c_trulylg_para_0xb6), hx8357c_trulylg_para_0xb6},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8357c_trulylg_para_0xb1), hx8357c_trulylg_para_0xb1},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8357c_trulylg_para_0xb4), hx8357c_trulylg_para_0xb4},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8357c_trulylg_para_0xb0), hx8357c_trulylg_para_0xb0},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8357c_trulylg_para_0xc0), hx8357c_trulylg_para_0xc0},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 10, sizeof(hx8357c_trulylg_para_0xba), hx8357c_trulylg_para_0xba},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 10, sizeof(hx8357c_trulylg_para_0xe0), hx8357c_trulylg_para_0xe0},
	{DTYPE_DCS_WRITE, 1, 0, 0, 120, sizeof(hx8357c_trulylg_para_0x11), hx8357c_trulylg_para_0x11},	
	{DTYPE_DCS_WRITE, 1, 0, 0, 10, sizeof(hx8357c_trulylg_para_0x29), hx8357c_trulylg_para_0x29},	

};
#endif

#if 1
static char hx8357c_tianma_para_0xb9[4] = {0xb9,0xff,0x83,0x57};
static char hx8357c_tianma_para_0xb6[2] = {0xb6, 0x4d};
static char hx8357c_tianma_para_0xb0[2] = {0xb0,0x68};
static char hx8357c_tianma_para_0xb1[7] = {0xb1,0x00,0x25,0x30,0x30,0x83,0xaa};
static char hx8357c_tianma_para_0xb3[5] = {0xb3,0x43,0x0f,0x06,0x06};
static char hx8357c_tianma_para_0xb4[8] = {0xb4,0x02,0x40,0x00,0x2a,0x2a,0x0d,0x4f};
static char hx8357c_tianma_para_0xb5[4] = {0xb5,0x01,0x01,0x67};
static char hx8357c_tianma_para_0xc0[7] = {0xc0,0x24,0x50,0x01,0x3c,0x1E,0x08};
static char hx8357c_tianma_para_0xe0[68] = {0xe0,0x00,0x00,0x01,0x00,0x0E,0x00,0x1E,0x00,0x21,0x00,0x21,0x00,0x41,0x00,0x4E,0x00,0x56,0x00,0x45,0x00,0x3E,0x00,0x3A,0x00,0x31,0x00,0x2E,0x00,0x2A,0x00,0x29,0x00,0x21,0x01,0x01,0x00,0x0E,0x00,0x1E,0x00,0x21,0x00,0x21,0x00,0x41,0x00,0x4E,0x00,0x56,0x00,0x45,0x00,0x3E,0x00,0x3A,0x00,0x31,0x00,0x2E,0x00,0x2A,0x00,0x29,0x00,0x21,0x00,0x44};


static char hx8357c_tianma_para_0xcc[2] = {0xcc,0x09};
static char hx8357c_tianma_para_0x3a[2] = {0x3a,0x77};
static char hx8357c_tianma_para_0xe9[2] = {0xe9,0x20};
static char hx8357c_tianma_para_0xba[17] = {0xba,0x00,0x56,0xd4,0x00,0x0a,0x00,0x10,0x32,0x6e,0x04,0x05,0x9a,0x14,0x19,0x10,0x40};
static char hx8357c_tianma_para_0x11[2] = {0x11, 0x00};
static char hx8357c_tianma_para_0x29[2] = {0x29, 0x00};
static char hx8357c_tianma_para_0x2c[2] = {0x2C, 0x00};


static struct dsi_cmd_desc hx8357c_tianma_display_on_cmds[] = 
{

	{DTYPE_DCS_LWRITE, 1, 0, 0, 10, sizeof(hx8357c_tianma_para_0xb9), hx8357c_tianma_para_0xb9},
	
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(hx8357c_tianma_para_0xb6),hx8357c_tianma_para_0xb6},	
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(hx8357c_tianma_para_0xb0), hx8357c_tianma_para_0xb0},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8357c_tianma_para_0xb1), hx8357c_tianma_para_0xb1},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8357c_tianma_para_0xb3), hx8357c_tianma_para_0xb3},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8357c_tianma_para_0xb4), hx8357c_tianma_para_0xb4},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8357c_tianma_para_0xb5), hx8357c_tianma_para_0xb5},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8357c_tianma_para_0xc0), hx8357c_tianma_para_0xc0},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8357c_tianma_para_0xe0), hx8357c_tianma_para_0xe0},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(hx8357c_tianma_para_0xcc), hx8357c_tianma_para_0xcc},	
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(hx8357c_tianma_para_0x3a), hx8357c_tianma_para_0x3a},	
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(hx8357c_tianma_para_0xe9), hx8357c_tianma_para_0xe9},	
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8357c_tianma_para_0xba), hx8357c_tianma_para_0xba},
	{DTYPE_DCS_WRITE, 1, 0, 0, 120, sizeof(hx8357c_tianma_para_0x11), hx8357c_tianma_para_0x11},	
	{DTYPE_DCS_WRITE, 1, 0, 0, 10, sizeof(hx8357c_tianma_para_0x29), hx8357c_tianma_para_0x29},	
	{DTYPE_DCS_WRITE, 1, 0, 0, 10, sizeof(hx8357c_tianma_para_0x2c), hx8357c_tianma_para_0x2c},	

};
#endif
#if 1

static char hx8357c_lead_para_0xb9[4] = {0xb9,0xff,0x83,0x57};
static char hx8357c_lead_para_0xb6[2] = {0xb6, 0x3e};
static char hx8357c_lead_para_0xba[17] = {0xba,0x00,0x56,0xd4,0x00,0x0a,0x00,0x10,0x32,0x6e,0x04,0x05,0x9a,0x14,0x19,0x10,0x40};
static char hx8357c_lead_para_0xe0[68] = {0xe0,0x00,0x00,0x00,0x00,0x04,0x00,0x09,0x00,0x12,0x00,0x25,0x00,0x3c,0x00,0x47,0x00,0x50,0x00,0x4a,0x00,0x44,0x00,0x3f,0x00,0x36,0x00,0x34,0x00,0x2f,0x00,0x30,0x00,0x2c,0x01,0x00,0x00,0x04,0x00,0x09,0x00,0x12,0x00,0x25,0x00,0x3c,0x00,0x47,0x00,0x50,0x00,0x4a,0x00,0x44,0x00,0x3f,0x00,0x36,0x00,0x34,0x00,0x2f,0x00,0x30,0x00,0x2c,0x00,0x00};
static char hx8357c_lead_para_0xb0[2] = {0xb0,0x68};
static char hx8357c_lead_para_0x11[2] = {0x11, 0x00};
static char hx8357c_lead_para_0x3a[2] = {0x3a,0x77};
static char hx8357c_lead_para_0xe9[2] = {0xe9,0x20};
static char hx8357c_lead_para_0xcc[2] = {0xcc,0x09};
static char hx8357c_lead_para_0xb3[5] = {0xb3,0x43,0x00,0x06,0x06};
static char hx8357c_lead_para_0xb1[7] = {0xb1,0x00,0x15,0x1c,0x1c,0x83,0xaa};
static char hx8357c_lead_para_0xc0[7] = {0xc0,0x24,0x24,0x01,0x3c,0x1e,0x08};
static char hx8357c_lead_para_0xb4[8] = {0xb4,0x02,0x40,0x00,0x2a,0x2a,0x0d,0x4f};
static char hx8357c_lead_para_0x2a[5] = {0x2a,0x00,0x00,0x3f,0x01}; 
static char hx8357c_lead_para_0x2b[5] = {0x2b,0x00,0x00,0xdf,0x01}; 
static char hx8357c_lead_para_0x29[2] = {0x29, 0x00};
static char hx8357c_lead_para_0x2c[2] = {0x2c,0x00}; 


static struct dsi_cmd_desc hx8357c_lead_display_on_cmds[] = {

	{DTYPE_DCS_LWRITE, 1, 0, 0, 10, sizeof(hx8357c_lead_para_0xb9), hx8357c_lead_para_0xb9},	
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(hx8357c_lead_para_0xb6),hx8357c_lead_para_0xb6},	
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8357c_lead_para_0xba), hx8357c_lead_para_0xba},	
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8357c_lead_para_0xe0), hx8357c_lead_para_0xe0},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(hx8357c_lead_para_0xb0), hx8357c_lead_para_0xb0},
	{DTYPE_DCS_WRITE, 1, 0, 0, 120, sizeof(hx8357c_lead_para_0x11), hx8357c_lead_para_0x11},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(hx8357c_lead_para_0x3a), hx8357c_lead_para_0x3a},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(hx8357c_lead_para_0xe9), hx8357c_lead_para_0xe9},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(hx8357c_lead_para_0xcc), hx8357c_lead_para_0xcc},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8357c_lead_para_0xb3), hx8357c_lead_para_0xb3},	
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8357c_lead_para_0xb1), hx8357c_lead_para_0xb1},	
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8357c_lead_para_0xc0), hx8357c_lead_para_0xc0},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8357c_lead_para_0xb4), hx8357c_lead_para_0xb4},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8357c_lead_para_0x2a), hx8357c_lead_para_0x2a},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8357c_lead_para_0x2b), hx8357c_lead_para_0x2b},
	{DTYPE_DCS_WRITE, 1, 0, 0, 10, sizeof(hx8357c_lead_para_0x29), hx8357c_lead_para_0x29},
	{DTYPE_DCS_WRITE, 1, 0, 0, 10, sizeof(hx8357c_lead_para_0x2c), hx8357c_lead_para_0x2c},

};


#endif

#if 1

static char hx8357c_yushun_para_0xb9[4] = {0xb9,0xff,0x83,0x57};
static char hx8357c_yushun_para_0x3a[2] = {0x3a,0x77};
static char hx8357c_yushun_para_0xe9[2] = {0xe9,0x20};
static char hx8357c_yushun_para_0xcc[2] = {0xcc,0x09};
//static char hx8357c_yushun_para_0xe3[2] = {0xe3,0x10};
static char hx8357c_yushun_para_0xb3[5] = {0xb3,0x43,0x00,0x06,0x06};
static char hx8357c_yushun_para_0xb6[2] = {0xb6, 0x3b};
static char hx8357c_yushun_para_0xb1[7] = {0xb1,0x00,0x11,0x1b,0x1b,0x83,0xaa};
static char hx8357c_yushun_para_0xc0[7] = {0xc0,0x76,0x50,0x01,0x3c,0xc8,0x08};
static char hx8357c_yushun_para_0xb4[8] = {0xb4,0x02,0x40,0x00,0x2a,0x2a,0x0d,0x4f};
static char hx8357c_yushun_para_0xe0[68] = {0xe0,0x00,0x00,0x02,0x00,0x13,0x00,0x14,0x00,0x10,0x00,0x3a,0x00,0x48,0x00,0x50,0x00,0x59,0x00,0x42,0x00,0x3b,0x00,0x38,0x00,0x2D,0x00,0x2d,0x00,0x21,0x00,0x17,0x00,0x01,0x01,0x02,0x00,0x13,0x00,0x14,0x00,0x10,0x00,0x3a,0x00,0x48,0x00,0x50,0x00,0x59,0x00,0x42,0x00,0x3b,0x00,0x38,0x00,0x2D,0x00,0x2d,0x00,0x21,0x00,0x17,0x00,0x01,0x00,0x00};
static char hx8357c_yushun_para_0xba[17] = {0xba,0x00,0x56,0xd4,0x00,0x0a,0x00,0x10,0x32,0x6e,0x04,0x05,0x9a,0x14,0x19,0x10,0x40};
static char hx8357c_yushun_para_0x11[2] = {0x11, 0x00};
static char hx8357c_yushun_para_0x29[2] = {0x29, 0x00};
static char hx8357c_yushun_para_0x2c[2] = {0x2c, 0x00};

static struct dsi_cmd_desc hx8357c_yushun_display_on_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 120, sizeof(hx8357c_yushun_para_0x11), hx8357c_yushun_para_0x11},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 10, sizeof(hx8357c_yushun_para_0xb9), hx8357c_yushun_para_0xb9},	
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8357c_yushun_para_0xba), hx8357c_yushun_para_0xba},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(hx8357c_yushun_para_0x3a), hx8357c_yushun_para_0x3a},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(hx8357c_yushun_para_0xe9), hx8357c_yushun_para_0xe9},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(hx8357c_yushun_para_0xcc), hx8357c_yushun_para_0xcc},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8357c_yushun_para_0xb3), hx8357c_yushun_para_0xb3},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(hx8357c_yushun_para_0xb6), hx8357c_yushun_para_0xb6},	
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8357c_yushun_para_0xb1), hx8357c_yushun_para_0xb1},	
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8357c_yushun_para_0xc0), hx8357c_yushun_para_0xc0},	
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8357c_yushun_para_0xb4), hx8357c_yushun_para_0xb4},	
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8357c_yushun_para_0xe0), hx8357c_yushun_para_0xe0},	
	//{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(hx8357c_yushun_para_0xba), hx8357c_yushun_para_0xba},
	//{DTYPE_DCS_WRITE, 1, 0, 0, 120, sizeof(hx8357c_yushun_para_0x11), hx8357c_yushun_para_0x11},
	{DTYPE_DCS_WRITE, 1, 0, 0, 10, sizeof(hx8357c_yushun_para_0x29), hx8357c_yushun_para_0x29},
	{DTYPE_DCS_WRITE, 1, 0, 0, 10, sizeof(hx8357c_yushun_para_0x2c), hx8357c_yushun_para_0x2c},

};
#endif

void myudelay(unsigned int usec)
{
udelay(usec);
}
static void lcd_panle_reset(void)
{
	gpio_direction_output(GPIO_LCD_RESET,1);
	msleep(10);
	gpio_direction_output(GPIO_LCD_RESET,0);
	msleep(10);
	gpio_direction_output(GPIO_LCD_RESET,1);
	msleep(50);
}

struct mipi_manufacture_ic {
	struct dsi_cmd_desc *readid_tx;
	int readid_len_tx;
	struct dsi_cmd_desc *readid_rx;
	int readid_len_rx;
	int mode;
};

static int mipi_get_manufacture_icid(struct msm_fb_data_type *mfd)
{
	uint32 icid = 0;
	int i ;
	

	 struct mipi_manufacture_ic mipi_manufacture_icid[3] = {
		{hx8363_setpassword_cmd,ARRAY_SIZE(hx8363_setpassword_cmd),&hx8363_icid_rd_cmd,3,1},
		{nt3511_setpassword_cmd,ARRAY_SIZE(nt3511_setpassword_cmd),&nt3511_icid_rd_cmd,3,0},
		{hx8369_setpassword_cmd,ARRAY_SIZE(hx8369_setpassword_cmd),&hx8369_icid_rd_cmd,3,1},

	 };

	for(i = 0; i < ARRAY_SIZE(mipi_manufacture_icid) ; i++)
	{	lcd_panle_reset();	
		mipi_dsi_buf_init(&lead_tx_buf);
		mipi_dsi_buf_init(&lead_rx_buf);
		mipi_set_tx_power_mode(1);	
		
		mipi_dsi_cmds_tx(mfd, &lead_tx_buf, mipi_manufacture_icid[i].readid_tx,mipi_manufacture_icid[i].readid_len_tx);
		mipi_dsi_cmd_bta_sw_trigger(); 
		
		if(!mipi_manufacture_icid[i].mode)
			mipi_set_tx_power_mode(0);	
		
		mipi_dsi_cmds_rx(mfd,&lead_tx_buf, &lead_rx_buf, mipi_manufacture_icid[i].readid_rx,mipi_manufacture_icid[i].readid_len_rx);

		if(mipi_manufacture_icid[i].mode)
			mipi_set_tx_power_mode(0);
		
		icid = *(uint32 *)(lead_rx_buf.data);
		
		printk("debug read icid is %x\n",icid & 0xffffff);

		switch(icid & 0xffffff){
			case 0x1055:
						return NOVATEK_35510;
			case 0x6383ff:
						return HIMAX_8363;
						
			case 0x6983ff:
						return HIMAX_8369;
						
			default:
						break;			
		}

	}
	return 0;
}

static uint32 mipi_get_commic_panleid(struct msm_fb_data_type *mfd,struct dsi_cmd_desc *para,uint32 len,int mode)
{
	uint32 panelid = 0;
	mipi_dsi_buf_init(&lead_tx_buf);
	mipi_dsi_buf_init(&lead_rx_buf);
	mipi_dsi_cmd_bta_sw_trigger(); 
	if(mode)
		mipi_set_tx_power_mode(1);
	else 
		mipi_set_tx_power_mode(0);
	mipi_dsi_cmds_rx(mfd,&lead_tx_buf, &lead_rx_buf, para,len);
	if(mode)
		mipi_set_tx_power_mode(0);
	panelid = *(uint32 *)(lead_rx_buf.data);
	printk("debug read panelid is %x\n",panelid & 0xffffffff);
	return panelid & 0xff;
}

static uint32 mipi_get_himax8369_panleid(struct msm_fb_data_type *mfd)
{
	uint32 panleid;
	
	panleid =  mipi_get_commic_panleid(mfd,&hx8369_panleid_rd_cmd,1,1);
	switch(panleid){
		case HIMAX8369_TIANMA_TN_ID:
				return (u32)LCD_PANEL_4P0_HIMAX8369_TIANMA_TN;
		case HIMAX8369_TIANMA_IPS_ID:
				return (u32)LCD_PANEL_4P0_HIMAX8369_TIANMA_IPS;
		case HIMAX8369_LEAD_ID:
				return (u32)LCD_PANEL_4P0_HIMAX8369_LEAD;
		case HIMAX8369_LEAD_HANNSTAR_ID:
				return (u32)LCD_PANEL_4P0_HIMAX8369_LEAD_HANNSTAR;
		default:
				return (u32)LCD_PANEL_NOPANEL;
	}
}




static uint32 mipi_get_nt35510_panleid(struct msm_fb_data_type *mfd)
{
	uint32 panleid =  mipi_get_commic_panleid(mfd,&nt3511_panleid_rd_cmd,1,0);
	switch(panleid){
		case NT35510_YUSHUN_ID:
				return  (u32)LCD_PANEL_4P0_NT35510_HYDIS_YUSHUN	;
		case NT35510_LEAD_ID:
				return (u32)LCD_PANEL_4P0_NT35510_LEAD;
		default:
				return (u32)LCD_PANEL_NOPANEL;
	}
}

static uint32 mipi_get_icpanleid(struct msm_fb_data_type *mfd )
{
	int icid = 0;

	lcd_panle_reset();
	icid = mipi_get_manufacture_icid(mfd);


	switch(icid){
		case HIMAX_8363:		
					LcdPanleID = LCD_PANEL_4P0_HX8363_CMI_YASSY;
					break;
		case HIMAX_8369:
					LcdPanleID = mipi_get_himax8369_panleid(mfd);
					break;
		case NOVATEK_35510:
					LcdPanleID = mipi_get_nt35510_panleid(mfd);
					break;
		default:
					LcdPanleID = (u32)LCD_PANEL_NOPANEL;
					printk("warnning cann't indentify the chip\n");
					break;
	}
		
	return LcdPanleID;
}

#ifdef CONFIG_ZTE_PLATFORM
static u32 __init get_lcdpanleid_from_bootloader(void)
{
	smem_global*	msm_lcd_global = (smem_global*) ioremap(SMEM_LOG_GLOBAL_BASE, sizeof(smem_global));
	
	printk("debug chip id 0x%x\n",msm_lcd_global->lcd_id);
	
	if (((msm_lcd_global->lcd_id) & 0xffff0000) == 0x09830000) 
	{
		
		switch(msm_lcd_global->lcd_id & 0x0000ffff)
		{	
			case 0x0001:
				return (u32)LCD_PANEL_3P95_HX8357_BOE_BOE;
			case 0x0002:
				return (u32)LCD_PANEL_3P95_HX8357_IVO_YUSHUN;
			case 0x0003:
				return (u32)LCD_PANEL_3P95_HX8357_HANSTAR_LEAD;
			case 0x0004:
				return (u32)LCD_PANEL_3P95_HX8357_TIANMA_TIANMA;
			case 0x0005:
				return 5;
			case 0x0006:
				return 6;
			case 0x0007:
				return 7;
			default:
				break;
		}		
	}
	return (u32)LCD_PANEL_NOPANEL;
}
#endif

static int mipi_lcd_off(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;

	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;
	mipi_set_tx_power_mode(1);

	mipi_dsi_cmds_tx(mfd, &lead_tx_buf, display_off_cmds,
			ARRAY_SIZE(display_off_cmds));
	gpio_direction_output(GPIO_LCD_RESET,0);
	msleep(5);
	gpio_direction_output(GPIO_LCD_RESET,1);
	msleep(10);
	gpio_direction_output(121,0);
	return 0;
}

//added by zte_gequn091966,20110428
static void select_1wire_mode(void)
{
	gpio_direction_output(lcd_bkl_ctl, 1);
	myudelay(120);
	gpio_direction_output(lcd_bkl_ctl, 0);
	myudelay(280);				////ZTE_LCD_LUYA_20100226_001
	gpio_direction_output(lcd_bkl_ctl, 1);
	myudelay(650);				////ZTE_LCD_LUYA_20100226_001
	
}


static void send_bkl_address(void)
{
	unsigned int i,j;
	i = 0x72;
	gpio_direction_output(lcd_bkl_ctl, 1);
	myudelay(10);
	printk("[LY] send_bkl_address \n");
	for(j = 0; j < 8; j++)
	{
		if(i & 0x80)
		{
			gpio_direction_output(lcd_bkl_ctl, 0);
			myudelay(10);
			gpio_direction_output(lcd_bkl_ctl, 1);
			myudelay(180);
		}
		else
		{
			gpio_direction_output(lcd_bkl_ctl, 0);
			myudelay(180);
			gpio_direction_output(lcd_bkl_ctl, 1);
			myudelay(10);
		}
		i <<= 1;
	}
	gpio_direction_output(lcd_bkl_ctl, 0);
	myudelay(10);
	gpio_direction_output(lcd_bkl_ctl, 1);

}

static void send_bkl_data(int level)
{
	unsigned int i,j;
	i = level & 0x1F;
	gpio_direction_output(lcd_bkl_ctl, 1);
	myudelay(10);
	printk("[LY] send_bkl_data \n");
	for(j = 0; j < 8; j++)
	{
		if(i & 0x80)
		{
			gpio_direction_output(lcd_bkl_ctl, 0);
			myudelay(10);
			gpio_direction_output(lcd_bkl_ctl, 1);
			myudelay(180);
		}
		else
		{
			gpio_direction_output(lcd_bkl_ctl, 0);
			myudelay(180);
			gpio_direction_output(lcd_bkl_ctl, 1);
			myudelay(10);
		}
		i <<= 1;
	}
	gpio_direction_output(lcd_bkl_ctl, 0);
	myudelay(10);
	gpio_direction_output(lcd_bkl_ctl, 1);

}
static void mipi_zte_set_backlight(struct msm_fb_data_type *mfd)
{
       /*value range is 1--32*/
	 int current_lel = mfd->bl_level;
  	 unsigned long flags;


    	printk("[ZYF] lcdc_set_bl level=%d, %d\n", 
		   current_lel , mfd->panel_power_on);

    	if(!mfd->panel_power_on)
	{
    		gpio_direction_output(lcd_bkl_ctl, 0);
		mdelay(3);
		onewiremode = FALSE;
	    	return;
    	}

    	if(current_lel < 1)
    	{
        	current_lel = 0;
   	 }
		
    	if(current_lel > 32)
    	{
        	current_lel = 32;
    	}
    
    	local_irq_save(flags);
		
   	if(current_lel==0)
    	{
    		gpio_direction_output(lcd_bkl_ctl, 0);
		mdelay(3);
		onewiremode = FALSE;
			
    	}
    	else 
	{
		if(!onewiremode)	
		{
			printk("[LY] before select_1wire_mode\n");
			select_1wire_mode();
			onewiremode = TRUE;
		}
		send_bkl_address();
		send_bkl_data(current_lel-1);

	}
		
    	local_irq_restore(flags);
}



static int first_time_panel_on = 1;
static int mipi_lcd_on(struct platform_device *pdev)
{

	struct msm_fb_data_type *mfd = platform_get_drvdata(pdev);

	     printk("gequn mipi_lcd_on\n");

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;

	if(first_time_panel_on){
		first_time_panel_on = 0;
		return 0;
		if(LcdPanleID != (u32)LCD_PANEL_NOPANEL)
			return 0;
		else
			LcdPanleID = mipi_get_icpanleid(mfd);
	}

LcdPanleID = (u32)LCD_PANEL_3P95_HX8357_BOE_BOE;
	
	lcd_panle_reset();
	printk("mipi init start\n");
	mipi_set_tx_power_mode(1);
	switch(LcdPanleID){
		
						
		case (u32)LCD_PANEL_3P95_HX8357_BOE_BOE:
				mipi_dsi_cmds_tx(mfd, &lead_tx_buf, hx8357c_trulylg_display_on_cmds,ARRAY_SIZE(hx8357c_trulylg_display_on_cmds));
				printk("trulylg init ok !!\n");
				break;	
		case (u32)LCD_PANEL_3P95_HX8357_IVO_YUSHUN:
				mipi_dsi_cmds_tx(mfd, &lead_tx_buf, hx8357c_yushun_display_on_cmds,ARRAY_SIZE(hx8357c_yushun_display_on_cmds));
				printk("yushun init ok !!\n");
				break;	
		case (u32)LCD_PANEL_3P95_HX8357_HANSTAR_LEAD:
				mipi_dsi_cmds_tx(mfd, &lead_tx_buf, hx8357c_lead_display_on_cmds,ARRAY_SIZE(hx8357c_lead_display_on_cmds));
				printk("lead init ok !!\n");
				break;
						
		case (u32)LCD_PANEL_3P95_HX8357_TIANMA_TIANMA:
				mipi_dsi_cmds_tx(mfd, &lead_tx_buf, hx8357c_tianma_display_on_cmds,ARRAY_SIZE(hx8357c_tianma_display_on_cmds));
				printk("tianma init ok !!\n");
				break;
		default:
			       mipi_dsi_cmds_tx(mfd, &lead_tx_buf, hx8357c_trulylg_display_on_cmds,ARRAY_SIZE(hx8357c_trulylg_display_on_cmds));
				printk("BOE_hx8357C  init ok !!\n");
				//printk("can't get icpanelid value\n");
				break;
				
	}	
	mipi_set_tx_power_mode(0);
	return 0;
}



static struct msm_fb_panel_data lead_panel_data = {
	.on		= mipi_lcd_on,
	.off		= mipi_lcd_off,
	.set_backlight = mipi_zte_set_backlight,
};



static int ch_used[3];

int mipi_lead_device_register(struct msm_panel_info *pinfo,
					u32 channel, u32 panel)
{
	struct platform_device *pdev = NULL;
	int ret;

	if ((channel >= 3) || ch_used[channel])
		return -ENODEV;

printk(KERN_ERR
		  "%s: gequn mipi_device_register n855!\n", __func__);
	ch_used[channel] = TRUE;

	pdev = platform_device_alloc("mipi_lead", (panel << 8)|channel);
	if (!pdev)
		return -ENOMEM;

	lead_panel_data.panel_info = *pinfo;

	ret = platform_device_add_data(pdev, &lead_panel_data,
		sizeof(lead_panel_data));
	if (ret) {
		printk(KERN_ERR
		  "%s: platform_device_add_data failed!\n", __func__);
		goto err_device_put;
	}

	ret = platform_device_add(pdev);
	if (ret) {
		printk(KERN_ERR
		  "%s: platform_device_register failed!\n", __func__);
		goto err_device_put;
	}

	return 0;

err_device_put:
	platform_device_put(pdev);
	return ret;
}


static int __devinit mipi_lead_lcd_probe(struct platform_device *pdev)
{	
	if (pdev->id == 0) return 0;
	
	mipi_dsi_buf_alloc(&lead_tx_buf, DSI_BUF_SIZE);
	mipi_dsi_buf_alloc(&lead_rx_buf, DSI_BUF_SIZE);
	
#ifdef CONFIG_ZTE_PLATFORM	
	if((LcdPanleID = get_lcdpanleid_from_bootloader() )==(u32)LCD_PANEL_NOPANEL)
		printk("cann't get get_lcdpanelid from bootloader\n");
#endif	


	msm_fb_add_device(pdev);

	return 0;
}

static struct platform_driver this_driver = {
	.probe  = mipi_lead_lcd_probe,
	.driver = {
		.name   = "mipi_lead",
	},
};

static int __init mipi_lcd_init(void)
{
   		printk("gequn mipi_lcd_init\n");

	return platform_driver_register(&this_driver);
}

module_init(mipi_lcd_init);
