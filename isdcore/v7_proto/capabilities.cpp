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
/* This module utility functions related AIM capabilities 		  */
/*									  */
/**************************************************************************/

#include "includes.h"

/* List of known ICQ/AIM capabilities */
char aim_caps[][16] = 
{
	  /* 00. AIM_CAPS_CHAT */
	 {(char)0x74, (char)0x8F, (char)0x24, (char)0x20, (char)0x62, (char)0x87, (char)0x11, (char)0xD1, 
	  (char)0x82, (char)0x22, (char)0x44, (char)0x45, (char)0x53, (char)0x54, (char)0x00, (char)0x00},

	  /* 01. AIM_CAPS_VOICE */
	 {(char)0x09, (char)0x46, (char)0x13, (char)0x41, (char)0x4C, (char)0x7F, (char)0x11, (char)0xD1, 
	  (char)0x82, (char)0x22, (char)0x44, (char)0x45, (char)0x53, (char)0x54, (char)0x00, (char)0x00},

	  /* 02. AIM_CAPS_SENDxFILE */
	 {(char)0x09, (char)0x46, (char)0x13, (char)0x43, (char)0x4C, (char)0x7F, (char)0x11, (char)0xD1, 
	  (char)0x82, (char)0x22, (char)0x44, (char)0x45, (char)0x53, (char)0x54, (char)0x00, (char)0x00},

	  /* 03. AIM_CAPS_ISxICQ */
	 {(char)0x09, (char)0x46, (char)0x13, (char)0x44, (char)0x4C, (char)0x7F, (char)0x11, (char)0xD1, 
	  (char)0x82, (char)0x22, (char)0x44, (char)0x45, (char)0x53, (char)0x54, (char)0x00, (char)0x00},

	  /* 04. AIM_CAPS_IMxIMAGE */
	 {(char)0x09, (char)0x46, (char)0x13, (char)0x45, (char)0x4C, (char)0x7F, (char)0x11, (char)0xD1, 
	  (char)0x82, (char)0x22, (char)0x44, (char)0x45, (char)0x53, (char)0x54, (char)0x00, (char)0x00},

	  /* 05. AIM_CAPS_BUDDYxICON */
	 {(char)0x09, (char)0x46, (char)0x13, (char)0x46, (char)0x4C, (char)0x7F, (char)0x11, (char)0xD1, 
	  (char)0x82, (char)0x22, (char)0x44, (char)0x45, (char)0x53, (char)0x54, (char)0x00, (char)0x00},

	  /* 06. AIM_CAPS_SAVESTOCKS */
	 {(char)0x09, (char)0x46, (char)0x13, (char)0x47, (char)0x4C, (char)0x7F, (char)0x11, (char)0xD1,
	  (char)0x82, (char)0x22, (char)0x44, (char)0x45, (char)0x53, (char)0x54, (char)0x00, (char)0x00},

	  /* 07. AIM_CAPS_GETxFILE */
	 {(char)0x09, (char)0x46, (char)0x13, (char)0x48, (char)0x4C, (char)0x7F, (char)0x11, (char)0xD1,
	  (char)0x82, (char)0x22, (char)0x44, (char)0x45, (char)0x53, (char)0x54, (char)0x00, (char)0x00},

	  /* 08. AIM_CAPS_ICQxEXTxMSG */
	 {(char)0x09, (char)0x46, (char)0x13, (char)0x49, (char)0x4C, (char)0x7F, (char)0x11, (char)0xD1,
	  (char)0x82, (char)0x22, (char)0x44, (char)0x45, (char)0x53, (char)0x54, (char)0x00, (char)0x00},

	  /* 09. AIM_CAPS_GAMES */
	 {(char)0x09, (char)0x46, (char)0x13, (char)0x4A, (char)0x4C, (char)0x7F, (char)0x11, (char)0xD1,
	  (char)0x82, (char)0x22, (char)0x44, (char)0x45, (char)0x53, (char)0x54, (char)0x00, (char)0x00},
	  
	  /* 10. AIM_CAPS_GAMES2 */
	 {(char)0x09, (char)0x46, (char)0x13, (char)0x4A, (char)0x4C, (char)0x7F, (char)0x11, (char)0xD1,
	  (char)0x22, (char)0x82, (char)0x44, (char)0x45, (char)0x53, (char)0x54, (char)0x00, (char)0x00},

	  /* 11. AIM_CAPS_SENDxBUDDYxLIST */
	 {(char)0x09, (char)0x46, (char)0x13, (char)0x4B, (char)0x4C, (char)0x7F, (char)0x11, (char)0xD1,
	  (char)0x82, (char)0x22, (char)0x44, (char)0x45, (char)0x53, (char)0x54, (char)0x00, (char)0x00},

	  /* 12. AIM_CAPS_ICQxUNKNOWN */
	 {(char)0x09, (char)0x46, (char)0x13, (char)0x4E, (char)0x4C, (char)0x7F, (char)0x11, (char)0xD1,
	  (char)0x82, (char)0x22, (char)0x44, (char)0x45, (char)0x53, (char)0x54, (char)0x00, (char)0x00},

	  /* 13. AIM_CAPS_ICQxUNKNOWN2 */
	 {(char)0x97, (char)0xB1, (char)0x27, (char)0x51, (char)0x24, (char)0x3C, (char)0x43, (char)0x34, 
	  (char)0xAD, (char)0x22, (char)0xD6, (char)0xAB, (char)0xF7, (char)0x3F, (char)0x14, (char)0x92},

	  /* 14. AIM_CAPS_ICQxUNKNOWN3 */
	 {(char)0x2E, (char)0x7A, (char)0x64, (char)0x75, (char)0xFA, (char)0xDF, (char)0x4D, (char)0xC8,
	  (char)0x88, (char)0x6F, (char)0xEA, (char)0x35, (char)0x95, (char)0xFD, (char)0xB6, (char)0xDF},

	  /* 15. AIM_CAPS_TRILLIANxCRYPT */
	 {(char)0xF2, (char)0xE7, (char)0xC7, (char)0xF4, (char)0xFE, (char)0xAD, (char)0x4D, (char)0xFB,
	  (char)0xB2, (char)0x35, (char)0x36, (char)0x79, (char)0x8B, (char)0xDF, (char)0x00, (char)0x00},

	  /* 16. AIM_CAPS_APxINFO */
         {(char)0xAA, (char)0x4A, (char)0x32, (char)0xB5, (char)0xF8, (char)0x84, (char)0x48, (char)0xC6,
	  (char)0xA3, (char)0xD7, (char)0x8C, (char)0x50, (char)0x97, (char)0x19, (char)0xFD, (char)0x5B},
	  
	  /* 17. AIM_CAPS_MACxICQ */
	 {(char)0xDD, (char)0x16, (char)0xF2, (char)0x02, (char)0x84, (char)0xE6, (char)0x11, (char)0xD4,
	  (char)0x90, (char)0xDB, (char)0x00, (char)0x10, (char)0x4B, (char)0x9B, (char)0x4B, (char)0x7D},

	  /* 18. AIM_CAPS_UTF8 */
	 {(char)0x09, (char)0x46, (char)0x13, (char)0x4E, (char)0x4C, (char)0x7F, (char)0x11, (char)0xD1,
	  (char)0x82, (char)0x22, (char)0x44, (char)0x45, (char)0x53, (char)0x54, (char)0x00, (char)0x00}	  
};


/**************************************************************************/
/* Check if user has specified capability				  */
/**************************************************************************/
BOOL user_has_cap(struct online_user *to_user, char *required_cap)
{
   BOOL isok;
   
   for (int i=0; i<to_user->caps_num; i++)
   {
      isok = True;
      
      for (int j=0; j<16; j++)
      {
         if (to_user->caps[i][j] != required_cap[j]) isok = False;
      }
      
      if (isok) 
      {
         DEBUG(100, ("Locate: Found required capability. All is ok.\n"));
         return(True);
      }
   }
   
   return(False);
}


/**************************************************************************/
/* Check caps match							  */
/**************************************************************************/
BOOL caps_match(char *caps1, char *caps2)
{
   BOOL isok;
   isok = True;
   
   for (int j=0; j<16; j++)
   {
      if (caps1[j] != caps2[j]) 
      {
         isok = False;
	 break;
      }
   }
      
   if (isok) 
   {
      DEBUG(100, ("Caps: privided caps match.\n"));
      return(True);
   }
   
   return(False);
}


