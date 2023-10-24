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
/* Defines for AIM/ICQ capabilities 	 				  */
/*                                                                        */
/**************************************************************************/

#ifndef _CAPABILITIES_H
#define _CAPABILITIES_H

extern char aim_caps[][16];

/* indexes in aim_caps array (capabilities.cpp) */
#define AIM_CAPS_CHAT               0     
#define AIM_CAPS_VOICE              1 
#define AIM_CAPS_SENDxFILE          2 
#define AIM_CAPS_ISxICQ             3 
#define AIM_CAPS_IMxIMAGE           4 
#define AIM_CAPS_BUDDYxICON         5 
#define AIM_CAPS_SAVESTOCKS         6 
#define AIM_CAPS_GETxFILE           7 
#define AIM_CAPS_ICQxEXTxMSG	    8 
#define AIM_CAPS_GAMES              9 
#define AIM_CAPS_GAMES2            10 
#define AIM_CAPS_SENDxBUDDYxLIST   11 
#define AIM_CAPS_ICQxUNKNOWN       12 
#define AIM_CAPS_ICQxUNKNOWN2      13 
#define AIM_CAPS_ICQxUNKNOWN3      14 
#define AIM_CAPS_TRILLIANxCRYPT    15 
#define AIM_CAPS_APxINFO           16
#define AIM_CAPS_MACxICQ	   17
 
#endif /* _CAPABILITIES_H */


