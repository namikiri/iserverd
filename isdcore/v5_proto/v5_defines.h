/**************************************************************************/
/*									  */
/* Copyright (c) 2000-2005 by Alexandr V. Shutko, Khabarovsk, Russia	  */
/* All rights reserved.							  */
/*									  */
/* Redistribution and use in source and binary forms, with or without	  */
/* modification, are permitted provided that the following conditions	  */
/* are met:								  */
/* 1. Redistributions of source code must retain the above copyright	  */
/*    notice, this list of conditions and the following disclaimer.	  */
/* 2. Redistributions in binary form must reproduce the above copyright	  */
/*    notice, this list of conditions and the following disclaimer in 	  */
/*    the documentation and/or other materials provided with the 	  */
/*    distribution.							  */
/*									  */
/* THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND */
/* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE  */
/* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 	  */
/* PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS  */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, 	  */
/* OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT   */
/* OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR 	  */
/* BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,  */
/* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE   */
/* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, 	  */
/* EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.			  */
/*									  */
/* Defines for V5 transport (commands and meta-commands) 		  */
/*                                                                        */
/**************************************************************************/

#ifndef V5_DEFINES_H
#define V5_DEFINES_H

const unsigned short ICQ_CMDxSND_ACK               	= 0x000A; /* 0010 */
const unsigned short ICQ_CMDxSND_USERxLMETA		= 0x001E; /* 0030 */
const unsigned short ICQ_CMDxSND_NEWxUIN                = 0x0046; /* 0070 */
const unsigned short ICQ_CMDxSND_HELLO             	= 0x005A; /* 0090 */
const unsigned short ICQ_CMDxSND_WRONGxPASSWD      	= 0x0064; /* 0100 */
const unsigned short ICQ_CMDxSND_USERxONLINE       	= 0x006E; /* 0110 */
const unsigned short ICQ_CMDxSND_USERxOFFLINE      	= 0x0078; /* 0120 */
const unsigned short ICQ_CMDxSND_OLDxSEARCHxFIND	= 0x008C; /* 0140 */
const unsigned short ICQ_CMDxSND_OLDxSEARCHxEND		= 0x00A0; /* 0160 */
const unsigned short ICQ_CMDxSND_SYSxMSGxOFFLINE        = 0x00DC; /* 0220 */
const unsigned short ICQ_CMDxSND_ENDxSYSTEMxMESSAGES	= 0x00E6; /* 0230 */
const unsigned short ICQ_CMDxSND_ERR_NOT_CONNECTED	= 0x00F0; /* 0240 */
const unsigned short ICQ_CMDxSND_BUSY                   = 0x00FA; /* 0250 */
const unsigned short ICQ_CMDxSND_SYSxMSGxONLINE    	= 0x0104; /* 0260 */
const unsigned short ICQ_CMDxSND_USERxINFO_SINGLE       = 0x0118; /* 0280 */
const unsigned short ICQ_CMDxSND_OLD_INFO_EXT		= 0x0122; /* 0290 */
const unsigned short ICQ_CMDxSND_USERxINVALIDxUIN       = 0x012C; /* 0300 */
const unsigned short ICQ_CMDxSND_USERxSTATUS       	= 0x01A4; /* 0420 */
const unsigned short ICQ_CMDxSND_USERxLISTxDONE    	= 0x021C; /* 0540 */
const unsigned short ICQ_CMDxSND_LOGIN_ERR		= 0x0370; /* 0880 */
const unsigned short ICQ_CMDxSND_METAxUSER		= 0x03DE; /* 0990 */
const unsigned short ICQ_CMDxSND_ACKxNEWxUIN		= 0x03FC; /* 1020 */

const unsigned short ICQ_CMDxRCV_ACK               	= 0x000A; /* 0010 */
const unsigned short ICQ_CMDxRCV_THRUxSERVER       	= 0x010E; /* 0270 */

const unsigned short ICQ_CMDxRCV_LOGON             	= 0x03E8; /* 1000 */
const unsigned short ICQ_CMDxRCV_GETxDEPS		= 0x03F2; /* 1010 */
const unsigned short ICQ_CMDxRCV_USERxLIST         	= 0x0406; /* 1030 */
const unsigned short ICQ_CMDxRCV_OLDxSEARCHxUIN		= 0x041A; /* 1050 */
const unsigned short ICQ_CMDxRCV_OLDxSEARCH		= 0x0424; /* 1060 */
const unsigned short ICQ_CMDxRCV_PING              	= 0x042E; /* 1070 */
const unsigned short ICQ_CMDxRCV_LOGOFF            	= 0x0438; /* 1080 */
const unsigned short ICQ_CMDxRCV_SYSxMSGxDONExACK  	= 0x0442; /* 1090 */
const unsigned short ICQ_CMDxRCV_SYSxMSGxREQUEST	= 0x044C; /* 1100 */
const unsigned short ICQ_CMDxRCV_THRUxSERVER2       	= 0x0456; /* 1110 */
const unsigned short ICQ_CMDxRCV_USER_INFO_OLD          = 0x0460; /* 1120 */
const unsigned short ICQ_CMDxRCV_USER_INFO_OLD_EXT	= 0x046A; /* 1130 */
const unsigned short ICQ_CMDxRCV_SETxSTATUS             = 0x04D8; /* 1240 */
const unsigned short ICQ_CMDxRCV_FIRSTxLOGIN		= 0x04EC; /* 1260 */
const unsigned short ICQ_CMDxRCV_PING2              	= 0x051E; /* 1310 */
const unsigned short ICQ_CMDxRCV_USERxADD               = 0x053C; /* 1340 */
const unsigned short ICQ_CMDxRCV_METAxUSER            	= 0x064A; /* 1610 */
const unsigned short ICQ_CMDxRCV_VISIBLExLIST		= 0x06AE; /* 1710 */
const unsigned short ICQ_CMDxRCV_INVISIBLExLIST		= 0x06A4; /* 1700 */
const unsigned short ICQ_CMDxRCV_CHANGExVILISTS		= 0x06B8; /* 1720 */


/*========================================================================*/
/*========================================================================*/

/* CMD_META SUB_CMD's */
const unsigned short CMD_META_SET_BASIC			= 0x03E8; /* 1000 */
const unsigned short CMD_META_SET_BASIC2		= 0x03E9; /* 1001 */
const unsigned short CMD_META_SET_WORK			= 0x03F2; /* 1010 */
const unsigned short CMD_META_SET_WORK2			= 0x03F3; /* 1011 */
const unsigned short CMD_META_SET_MORE			= 0x03FC; /* 1020 */
const unsigned short CMD_META_SET_MORE2			= 0x03FD; /* 1021 */
const unsigned short CMD_META_SET_ABOUT			= 0x0406; /* 1030 */
const unsigned short CMD_META_SET_INTERESTS		= 0x0410; /* 1040 */
const unsigned short CMD_META_SET_AFFLATIONS		= 0x041A; /* 1050 */
const unsigned short CMD_META_SET_SECURITY		= 0x0424; /* 1060 */
const unsigned short CMD_META_SET_PASS			= 0x042E; /* 1070 */
const unsigned short CMD_META_SET_HPCAT			= 0x0442; /* 1090 */
const unsigned short CMD_META_USER_FULLINFO		= 0x04B0; /* 1200 */
const unsigned short CMD_META_USER_INFO2		= 0x04B1; /* 1201 */
const unsigned short CMD_META_USER_INFO			= 0x04BA; /* 1210 */
const unsigned short CMD_META_USER_UNREGISTER		= 0x04C4; /* 1220 */
const unsigned short CMD_META_USER_LOGININFO		= 0x04CE; /* 1230 */
const unsigned short CMD_META_USER_LOGININFO2		= 0x04CF; /* 1231 */
const unsigned short CMD_META_SEARCH_NAME		= 0x0514; /* 1300 */
const unsigned short CMD_META_SEARCH_NAME2              = 0x0515; /* 1301 */
const unsigned short CMD_META_SEARCH_UIN		= 0x051E; /* 1310 */
const unsigned short CMD_META_SEARCH_UIN2               = 0x051F; /* 1311 */
const unsigned short CMD_META_SEARCH_EMAIL		= 0x0528; /* 1320 */
const unsigned short CMD_META_SEARCH_EMAIL2		= 0x0529; /* 1321 */
const unsigned short CMD_META_SEARCH_WHITE		= 0x0532; /* 1330 */
const unsigned short CMD_META_SEARCH_WHITE2		= 0x0533; /* 1331 */
const unsigned short CMD_META_USAGE_STATS		= 0x06B8; /* 1720 */
const unsigned short CMD_META_LOGIN			= 0x07D0; /* 2000 */

/* SRV_META replies SUB_CMD */
const unsigned short SRV_META_SET_BASIC_ACK		= 0x0064; /* 0100 */
const unsigned short SRV_META_SET_WORK_ACK		= 0x006E; /* 0110 */
const unsigned short SRV_META_SET_MORE_ACK		= 0x0078; /* 0120 */
const unsigned short SRV_META_SET_ABOUT_ACK		= 0x0082; /* 0130 */
const unsigned short SRV_META_SET_INTERESTS_ACK		= 0x008C; /* 0140 */
const unsigned short SRV_META_SET_AFFILAT_ACK		= 0x0096; /* 0150 */
const unsigned short SRV_META_SET_SECURE_ACK		= 0x00A0; /* 0160 */
const unsigned short SRV_META_SET_PASS_ACK		= 0x00AA; /* 0170 */
const unsigned short SRV_META_UNREG_ACK			= 0x00B4; /* 0180 */
const unsigned short SRV_META_SET_HPCAT_ACK		= 0x00BE; /* 0190 */
const unsigned short SRV_META_USER_INFO2		= 0x00C8; /* 0200 */
const unsigned short SRV_META_INFO_WORK			= 0x00D2; /* 0210 */
const unsigned short SRV_META_INFO_MORE			= 0x00DC; /* 0220 */
const unsigned short SRV_META_INFO_ABOUT		= 0x00E6; /* 0230 */
const unsigned short SRV_META_INFO_INTERESTS		= 0x00F0; /* 0240 */
const unsigned short SRV_META_INFO_AFFILATIONS		= 0x00FA; /* 0250 */
const unsigned short SRV_META_USER_INFO			= 0x0104; /* 0260 */
const unsigned short SRV_META_INFO_HPAGE_CAT		= 0x010E; /* 0270 */

const unsigned short SRV_META_USER_FOUND		= 0x0190; /* 0400 */
const unsigned short SRV_META_USER_LAST_FOUND		= 0x019A; /* 0410 */
const unsigned short SRV_META_WHITE_FOUND		= 0x01A4; /* 0420 */
const unsigned short SRV_META_WHITE_LAST_FOUND		= 0x01AE; /* 0430 */

#endif /* v5_defines */

