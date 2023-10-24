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
/* Defines for V3 transport						  */
/*                                                                        */
/**************************************************************************/

#ifndef V3_DEFINES_H
#define V3_DEFINES_H

/* UDP server commands */
const unsigned short ICQ_CMDxSND_ACK               	= 0x000A; /* 0010 */
const unsigned short ICQ_CMDxSND_SETxOFFLINE       	= 0x0028; /* 0040 */
const unsigned short ICQ_CMDxSND_USERxDEPS_LIST		= 0x0032; /* 0050 */
const unsigned short ICQ_CMDxSND_HELLO             	= 0x005A; /* 0090 */
const unsigned short ICQ_CMDxSND_WRONGxPASSWD      	= 0x0064; /* 0100 */
const unsigned short ICQ_CMDxSND_USERxONLINE       	= 0x006E; /* 0110 */
const unsigned short ICQ_CMDxSND_USERxOFFLINE      	= 0x0078; /* 0120 */
const unsigned short ICQ_CMDxSND_USERxDEPS_LIST1	= 0x0082; /* 0130 */
const unsigned short ICQ_CMDxSND_SEARCHxFOUND      	= 0x008C; /* 0140 */
const unsigned short ICQ_CMDxSND_SEARCHxDONE       	= 0x00A0; /* 0160 */
const unsigned short ICQ_CMDxSND_SYSxMSGxOFFLINE   	= 0x00DC; /* 0220 */
const unsigned short ICQ_CMDxSND_SYSxMSGxDONE      	= 0x00E6; /* 0230 */
const unsigned short ICQ_CMDxSND_ERR_NOT_CONNECTED	= 0x00F0; /* 0240 */
const unsigned short ICQ_CMDxSND_BUSY              	= 0x00FA; /* 0250 */
const unsigned short ICQ_CMDxSND_SYSxMSGxONLINE    	= 0x0104; /* 0260 */
const unsigned short ICQ_CMDxSND_USERxINFO_SINGLE	= 0x0118; /* 0280 */
const unsigned short ICQ_CMDxSND_USERxINVALIDxUIN  	= 0x02EE; /* 0300 */
const unsigned short ICQ_CMDxSND_USERxSET_PASSWORD_OK	= 0x0140; /* 0320 */
const unsigned short ICQ_CMDxSND_USERxSTATUS       	= 0x01A4; /* 0420 */
const unsigned short ICQ_CMDxSND_USERxSET_BASIC_INFO_OK = 0x01E0; /* 0480 */
const unsigned short ICQ_CMDxSND_USERxSET_AUTH_OK	= 0x01F4; /* 0500 */
const unsigned short ICQ_CMDxSND_USERxLISTxDONE    	= 0x021C; /* 0540 */
const unsigned short ICQ_CMDxSND_USERxSET_WORK_PAGE_OK  = 0x0258; /* 0600 */
const unsigned short ICQ_CMDxSND_USERxSET_WORK_INFO_OK  = 0x026C; /* 0620 */
const unsigned short ICQ_CMDxSND_USERxSET_HOME_INFO_OK  = 0x0280; /* 0640 */
const unsigned short ICQ_CMDxSND_USERxSET_HOME_PAGE_OK  = 0x0294; /* 0660 */
const unsigned short ICQ_CMDxSND_USERxSET_NOTES_OK	= 0x02A8; /* 0680 */
const unsigned short ICQ_CMDxSND_USERxINFO_BASIC       	= 0x02E4; /* 0740 */
const unsigned short ICQ_CMDxSND_USERxINFO_WORK         = 0x02F8; /* 0760 */
const unsigned short ICQ_CMDxSND_USERxINFO_WWEB         = 0x030C; /* 0780 */
const unsigned short ICQ_CMDxSND_USERxINFO_HOME         = 0x0320; /* 0800 */
const unsigned short ICQ_CMDxSND_USERxINFO_HWEB         = 0x0334; /* 0820 */
const unsigned short ICQ_CMDxSND_USERxNOTES		= 0x0352; /* 0850 */
const unsigned short ICQ_CMDxSND_SENDxFRAGMENT		= 0x0366; /* 0870 */
const unsigned short ICQ_CMDxSND_LOGIN_ERR		= 0x0370; /* 0880 */
const unsigned short ICQ_CMDxSND_REGISTER_INFO		= 0x037A; /* 0890 */
const unsigned short ICQ_CMDxSND_REGISTRATIONxOK	= 0x0384; /* 0900 */

/* Client requests */
const unsigned short ICQ_CMDxRCV_ACK               	= 0x000A; /* 0010 */
const unsigned short ICQ_CMDxRCV_THRUxSERVER       	= 0x010E; /* 0270 */
const unsigned short ICQ_CMDxRCV_RECONNECT		= 0x015E; /* 0350 */
const unsigned short ICQ_CMDxRCV_LOGON             	= 0x03E8; /* 1000 */
const unsigned short ICQ_CMDxRCV_GETxDEPS		= 0x03F2; /* 1010 */
const unsigned short ICQ_CMDxRCV_USERxLIST         	= 0x0406; /* 1030 */
const unsigned short ICQ_CMDxRCV_SEARCHxSTART      	= 0x05C8; /* 1060 */
const unsigned short ICQ_CMDxRCV_PING              	= 0x042E; /* 1070 */
const unsigned short ICQ_CMDxRCV_LOGOFF            	= 0x0438; /* 1080 */
const unsigned short ICQ_CMDxRCV_SYSxMSGxDONExACK  	= 0x0442; /* 1090 */
const unsigned short ICQ_CMDxRCV_SYSxMSGxREQ       	= 0x044C; /* 1100 */
const unsigned short ICQ_CMDxRCV_AUTHORIZE         	= 0x0456; /* 1110 */
const unsigned short ICQ_CMDxRCV_USERxGETINFO1		= 0x0460; /* 1120 */
const unsigned short ICQ_CMDxRCV_SET_PASSWORD		= 0x049C; /* 1180 */
const unsigned short ICQ_CMDxRCV_GETxEXTERNALS		= 0x04C4; /* 1220 */
const unsigned short ICQ_CMDxRCV_SETxSTATUS        	= 0x04D8; /* 1240 */
const unsigned short ICQ_CMDxRCV_FIRSTxLOGIN		= 0x04EC; /* 1260 */
const unsigned short ICQ_CMDxRCV_SETxBASIC_INFO		= 0x050A; /* 1290 */
const unsigned short ICQ_CMDxRCV_SET_AUTH		= 0x0514; /* 1300 */
const unsigned short ICQ_CMDxRCV_PING2                  = 0x051E; /* 1310 */
const unsigned short ICQ_CMDxRCV_SETxSTATE		= 0x0528; /* 1320 */
const unsigned short ICQ_CMDxRCV_USAGESTATS		= 0x0532; /* 1330 */
const unsigned short ICQ_CMDxRCV_USERxADD          	= 0x053C; /* 1340 */
const unsigned short ICQ_CMDxRCV_SETxWORK_INFO		= 0x0578; /* 1400 */
const unsigned short ICQ_CMDxRCV_SETxHOME_INFO		= 0x0582; /* 1410 */
const unsigned short ICQ_CMDxRCV_SETxHOME_PAGE		= 0x058C; /* 1420 */
const unsigned short ICQ_CMDxRCV_SETxNOTES		= 0x0596; /* 1430 */
const unsigned short ICQ_CMDxRCV_GETxNOTES		= 0x05AA; /* 1450 */
const unsigned short ICQ_CMDxRCV_SETxWORK_PAGE		= 0x05BE; /* 1470 */
const unsigned short ICQ_CMDxRCV_FRAGMENTED		= 0x05D2; /* 1490 */
const unsigned short ICQ_CMDxRCV_REGxREQUEST_INFO	= 0x05DC; /* 1500 */
const unsigned short ICQ_CMDxRCV_REGxNEWxUSERxINFO	= 0x05E6; /* 1510 */
const unsigned short ICQ_CMDxRCV_GETxDEPS1		= 0x05F0; /* 1520 */
const unsigned short ICQ_CMDxRCV_USERxGETINFO      	= 0x05FA; /* 1530 */
const unsigned short ICQ_CMDxRCV_UKNOWN_DEP		= 0x0604; /* 1540 */
const unsigned short ICQ_CMDxRCV_BROADCAST_MSG_ALL	= 0x060E; /* 1550 */
const unsigned short ICQ_CMDxRCV_BROADCAST_MSG_ONL	= 0x0618; /* 1560 */
const unsigned short ICQ_CMDxRCV_WWP_MSG		= 0x0622; /* 1570 */
const unsigned short ICQ_CMDxRCV_DELUSER_REQ		= 0x2121; /* 8481 */

const unsigned short ICQ_CMDxRCV_VISxLIST	        = 0x06AE; /* 1710 */
const unsigned short ICQ_CMDxRCV_INVISxLIST	       	= 0x06A4; /* 1700 */

/* v3 messages high byte allways = 0 (message flag) */
const unsigned short ICQ_CMDxSND_SMxMSG            	= 0x0001;
const unsigned short ICQ_CMDxSND_SMxURL		        = 0x0004;
const unsigned short ICQ_CMDxSND_SMxREQxAUTH       	= 0x0006;
const unsigned short ICQ_CMDxSND_SMxAUTHxGRANTED 	= 0x0008;
const unsigned short ICQ_CMDxSND_SMxADDED          	= 0x000C;
const unsigned short ICQ_CMDxSND_SMxWWP			= 0x000D;

const unsigned short MAX_MESSAGE_SIZE               	= 450;

/* Search modes (search by) */
#define BY_UIN		0x00
#define BY_NICK		0x01
#define BY_FIRST	0x02	
#define BY_LAST		0x03
#define BY_EMAIL	0x04
#define BY_AGE		0x05
#define BY_WCITY	0x06	
#define BY_WSTATE	0x07	
#define BY_WCOUNTRY	0x08
#define BY_WCOMPANY	0x09
#define BY_WTITLE	0x0A	
#define BY_WDEPART	0x0B	
#define BY_HCITY	0x0C	
#define BY_HSTATE	0x0D	
#define BY_HCOUNTRY	0x0E

/* compare type */
#define MODE_CONTAIN	0x00
#define MODE_NOTCONTAIN 0x01
#define MODE_IS		0x02
#define MODE_ISNOT	0x03
#define MODE_BEGINS	0x04
#define MODE_ENDS	0x05

#endif /* _V3_DEFINES_H */

