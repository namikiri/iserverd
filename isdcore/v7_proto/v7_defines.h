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
/* Defines for OSCAR (v7) protocol 					  */
/*                                                                        */
/**************************************************************************/

#ifndef _V7_DEFINES_H
#define _V7_DEFINES_H

#define FLAP_ID_BYTE   0x2A
#define FLAP_HRD_SIZE  0x06
#define FLAP_VERSION   0x01

/*--[ authorization error codes ]-----------------------------------------*/
#define AUTH_ERR_PASS		0x0004	/* err: password incorrect    	  */
#define AUTH_ERR_PASS2		0x0005  /* err: password incorrect    	  */
#define AUTH_INVALID_PACKET	0x0006  /* err: invalid pack from client  */
#define AUTH_ERR_UIN		0x0007  /* err: ICQ# doesn't exist    	  */
#define AUTH_ERR_UIN2		0x0008  /* err: ICQ# doesn't exist    	  */
#define AUTH_ACCOUNT_SUSPENDED	0x0011	/* err: your account suspended    */
#define AUTH_TEMP_DOWN		0x0014	/* err: temporarily unavailable   */
#define AUTH_UIN_MAX		0x0016	/* err: max logins from same IP	  */
#define AUTH_UIN_MAX2		0x0017	/* err: max logins from same IP   */
#define AUTH_RATE_LIM		0x0018  /* err: rate limit excedeed   	  */
#define AUTH_RATE_LIM2		0x001D  /* err: rate limit excedeed   	  */
#define AUTH_OLD_VERS 		0x001B	/* err: you are using old ver     */
#define AUTH_OLD_VERS2		0x001C	/* err: you are using old ver 	  */
#define AUTH_SERV_FAIL		0x001E	/* err: login failed	      	  */
#define AUTH_INVALID_SECUREID	0x0020  /* err: invalid SecurID value     */

/**************************************************************************/
/*--[ SNAC basic types ]--------------------------------------------------*/
#define SN_TYP_NUMBER		17
#define SN_TYP_NEGOTIATION	0x0000	/* connection negotiation     	  */
#define SN_TYP_GENERIC		0x0001  /* generic service controls       */
#define SN_TYP_LOCATION		0x0002  /* location services          	  */
#define SN_TYP_BUDDYLIST	0x0003  /* buddy list management      	  */
#define SN_TYP_MESSAGING	0x0004  /* aim messaging services     	  */
#define SN_TYP_ADVERT		0x0005  /* advertisments service      	  */
#define SN_TYP_INVITATION	0x0006	/* invitation services        	  */
#define SN_TYP_ADMINISTRATIVE	0x0007	/* administrative services    	  */
#define SN_TYP_POPUPxNOTICES	0x0008	/* popup notices 	       	  */
#define SN_TYP_BOS		0x0009	/* privacy management services	  */
#define SN_TYP_AIMxSEARCH	0x000A	/* aim user search services   	  */
#define SN_TYP_STATS		0x000B	/* stats services 	      	  */
#define SN_TYP_TRANSLATE	0x000C	/* translating services       	  */
#define SN_TYP_CHATxNAVIG	0x000D	/* chat navigation services   	  */
#define SN_TYP_CHAT		0x000E	/* chat services	      	  */
#define SN_TYP_SSI		0x0013	/* server-side info service   	  */
#define SN_TYP_ICQxMESSxEXT	0x0015	/* AIM ICQ message extensions 	  */
#define SN_TYP_REGISTRATION	0x0017	/* ICQ registration services  	  */

/*--[ SNACs(1,x) Generic serivices ]--------------------------------------*/
#define SN_GEN_SUBS_NUMBER	24
#define SN_GEN_ERROR		0x0001	/* x: protocol error snac packet  */
#define SN_GEN_CLIENTxREADY	0x0002	/* c: client ready snac packet	  */
#define SN_GEN_SERVERxFAMILIES	0x0003	/* s: server ready snac packet    */
#define SN_GEN_SERVICExREQ	0x0004	/* c: ack BOS for new service	  */
#define SN_GEN_REDIRECT		0x0005  /* s: redirect for new service    */
#define SN_GEN_REQUESTxRATE	0x0006	/* c: request for rate-limit info */
#define SN_GEN_RATExRESPONSE	0x0007	/* s: rate-limit server response  */
#define SN_GEN_RATExACK		0x0008	/* c: rate-limit ack packet       */
#define SN_GEN_RATExCHANGED	0x000A	/* s: rate-limit info changed	  */
#define SN_GEN_PAUSE		0x000B	/* s: pause command from server   */
#define SN_GEN_PAUSE_ACK	0x000C	/* c: pause ack - cli families    */
#define SN_GEN_RESUME		0x000D	/* s: resume command from server  */
#define SN_GEN_INFOxREQUEST	0x000E	/* c: request personal info	  */
#define SN_GEN_INFOxRESPONSE	0x000F	/* s: personal info response snac */
#define SN_GEN_EVIL		0x0010	/* s: evil notification		  */
#define SN_GEN_SETxIDLETIME	0x0011	/* c: set idle time snac	  */
#define SN_GEN_MIGRATION	0x0012	/* s: migration notice & info	  */
#define SN_GEN_MOTD		0x0013	/* s: message of the day	  */
#define SN_GEN_SETxPRIVxFLAGS	0x0014	/* c: set privacy flags 	  */
#define SN_GEN_WELLxKNOWNxURLS	0x0015	/* s: well known URLs		  */
#define SN_GEN_NOP		0x0016	/* s: no operation - ping ? 	  */
#define SN_GEN_REQUESTxVERS	0x0017	/* c: request service versions	  */
#define SN_GEN_VERSxRESPONSE	0x0018	/* s: server service versions     */
#define SN_GEN_SETxSTATUS	0x001E	/* c: set status/dir conn info    */
#define SN_GEN_CHECKxCLIENT	0x001F	/* s: client verification	  */

/*--[ SNACs(2,x) LOCation services ]--------------------------------------*/
#define SN_LOC_SUBS_NUMBER	8
#define SN_LOC_ERROR		0x0001	/* x: server/client error	  */
#define SN_LOC_RIGHTSxREQUEST	0x0002	/* c: request limitations/params  */
#define SN_LOC_RIGHTSxRESPONSE	0x0003	/* s: limitations/params response */
#define SN_LOC_SETxUSERINFO	0x0004	/* c: set user information	  */
#define SN_LOC_REQxUSERINFO	0x0005	/* c: request user info		  */
#define SN_LOC_USERINFO		0x0006	/* s: user information response   */
#define SN_LOC_WATCHERxSUBREQ	0x0007	/* c: watcher sub request	  */
#define SN_LOC_WATCHERxNOTICE	0x0008  /* s: watcher notification	  */
#define SN_LOC_UPDT_DIRxREQ	0x0009	/* c: request to update dir info  */
#define SN_LOC_UPDT_DIRxREPLY	0x000A  /* s: server reply for dir update */
#define SN_LOC_UPDT_INTxREQ	0x000F  /* c: update interests request    */
#define SN_LOC_UPDT_INTxREPLY	0x0010  /* s: server reply for int update */
#define SN_LOC_REQxUSERINFO2	0x0015  /* c: unified userinfo request    */

/*--[ SNACs(3,x) Buddy List Management ]----------------------------------*/
#define SN_BLM_SUBS_NUMBER      12
#define SN_BLM_ERROR		0x0001	/* x: client/server error	  */
#define SN_BLM_RIGHTSxREQUEST	0x0002	/* c: request limitations/params  */
#define SN_BLM_RIGHTSxRESPONSE  0x0003  /* s: limitations/params response */
#define SN_BLM_ADDxCONTACT	0x0004	/* c: add buddy to contact list   */
#define SN_BLM_DELxCONTACT	0x0005	/* c: remove buddy from contact   */
#define SN_BLM_WATCHERxQUERY	0x0006	/* c: query for list of watchers  */
#define SN_BLM_WATCHERxRESPONSE	0x0007	/* s: requested watchers list     */
#define SN_BLM_WATCHERxSUBREQ	0x0008	/* c: watcher sub request   	  */
#define SN_BLM_WATCHERxNOTIFY	0x0009	/* s: watcher notification	  */
#define SN_BLM_NOTIFYxREJECTED	0x000A	/* s: can't send notification     */
#define SN_BLM_ONCOMINGxBUDDY	0x000B	/* s: your contact user is online */
#define SN_BLM_OFFGOINGxBUDDY	0x000C	/* s: your contact user is offline*/

/*--[ SNACs(4,x) ICBM service ]-------------------------------------------*/
#define SN_MSG_SUBS_NUMBER      12
#define SN_MSG_ERROR		0x0001	/* x: client/server error	  */
#define SN_MSG_ADDxICBMxPARAM	0x0002	/* c: add ICBM parameter	  */
#define SN_MSG_DELxICBMxPARAM	0x0003  /* c: delete ICBM parameter	  */
#define SN_MSG_PARAMxREQUEST	0x0004	/* c: request parameter info	  */
#define SN_MSG_PARAMxRESPONSE	0x0005	/* s: requested parameter info	  */
#define SN_MSG_SENDxMESSAGE	0x0006	/* c: send message thru server	  */
#define SN_MSG_RECVxMESSAGE	0x0007	/* s: message for client from srv */
#define SN_MSG_EVILxREQUEST	0x0008	/* c: evil request		  */
#define SN_MSG_EVILxREPLY	0x0009	/* s: server evil reply		  */
#define SN_MSG_MISSEDxCALLS	0x000A	/* s: missed calls 		  */
#define SN_MSG_MESSxACK		0x000B	/* x: client/server message ack	  */
#define SN_MSG_SRVxMESSxACK	0x000C	/* s: server message type-2 ack   */
#define SN_MSG_MTN		0x0014  /* x: mini typing notification    */

/*--[ SNACs(5,x) ADVertisment service ]-----------------------------------*/
#define SN_ADV_SUBS_NUMBER      3
#define SN_ADV_ERROR		0x0001	/* x: client/server error	  */
#define SN_ADV_REQUESTxADS	0x0002	/* c: client request ads 	  */
#define SN_ADV_RESPONSExADS	0x0003	/* s: server returned ads (GIFs)  */

/*--[ SNACs(6,x) INVitation service ]-------------------------------------*/
#define SN_INV_SUBS_NUMBER      2
#define SN_INV_JOIN		0x0002	/* c: invite a friend to join AIM */
#define SN_INV_JOINxACK		0x0003	/* s: invitation server ack	  */

/*--[ SNACs(7,x) ADMinistrative service ]---------------------------------*/
#define SN_ADM_SUBS_NUMBER      7
#define SN_ADM_ERROR		0x0001	/* x: client/server error 	  */
#define SN_ADM_REQUESTxINFO	0x0002	/* c: request service information */
#define SN_ADM_INFOxRESPONSE	0x0003	/* s: returned service info	  */
#define SN_ADM_INFOxCHANGExREQ	0x0004	/* c: client info change request  */
#define SN_ADM_INFOxCHANGED   	0x0005	/* s: info change reply	 	  */
#define SN_ADM_ACCOUNTxCONFIRM	0x0006	/* c: account confirm request 	  */
#define SN_ADM_ACCxCONFIRMxACK	0x0007	/* s: account confirm reply 	  */

/*--[ SNACs(8,x) POPup notices service ]----------------------------------*/
#define SN_POP_SUBS_NUMBER      2
#define SN_POP_ERROR		0x0001	/* x: client/server error	  */
#define SN_POP_DISPLAY		0x0002	/* s: display popup message 	  */

/*--[ SNACs(9,x) privacy management service ]-----------------------------*/
#define SN_BOS_SUBS_NUMBER      11
#define SN_BOS_ERROR		0x0001	/* x: client/server error	  */
#define SN_BOS_RIGHTSxREQUEST	0x0002	/* c: request service parameters  */
#define SN_BOS_RIGHTSxRESPONSE	0x0003	/* s: service parameters response */
#define SN_BOS_SETxPERMSxMASK	0x0004	/* c: set group permissions mask  */
#define SN_BOS_ADDxVISIBLExLIST	0x0005	/* c: add to visible list	  */
#define SN_BOS_DELxVISIBLExLIST	0x0006	/* c: delete from visible list    */
#define SN_BOS_ADDxINVISxLIST	0x0007	/* c: add to invisible list       */
#define SN_BOS_DELxINVISxLIST	0x0008	/* c: delete from invisible list  */
#define SN_BOS_SERVICExERROR	0x0009	/* s: BOS error			  */
#define SN_BOS_ADDxVISxLIST2	0x000A	/* c: add to visible list #2      */
#define SN_BOS_DELxVISxLIST2	0x000B	/* c: delete from visible list #2 */

/*--[ SNACs(A,x) User Lookup Service ]------------------------------------*/
#define SN_ULS_SUBS_NUMBER      3
#define SN_ULS_ERROR		0x0001  /* x: search failed/client error  */
#define SN_ULS_SEARCHxBYxEMAIL	0x0002	/* c: search by email 		  */
#define SN_ULS_SEARCHxRESPONSE	0x0003	/* s: search response (found user)*/

/*--[ SNACs(B,x) STats Service ]------------------------------------------*/
#define SN_STS_SUBS_NUMBER      4
#define SN_STS_ERROR		0x0001	/* x: client/server error	  */
#define SN_STS_SETxREPORTxINT	0x0002	/* s: set report interval	  */
#define SN_STS_STATSxREPORT	0x0003	/* c: stats report 		  */
#define SN_STS_STATSxREPORTxACK	0x0004	/* s: stats report ack snac	  */

/*--[ SNACs(C,x) TRanslation Service ]------------------------------------*/
#define SN_TRS_SUBS_NUMBER      3
#define SN_TRS_ERROR		0x0001	/* x: client/server error	  */
#define SN_TRS_TRANSLATExREQ	0x0002	/* c: client request translation  */
#define SN_TRS_TRANSLATExRESP	0x0003	/* s: translation req response	  */

/*--[ SNACs(D,x) CHat Navigation ]----------------------------------------*/
#define SN_CHN_SUBS_NUMBER      9
#define SN_CHN_ERROR		0x0001	/* x: client/server error	  */
#define SN_CHN_RIGHTSxREQ	0x0002	/* c: request rigths information  */
#define SN_CHN_EXCHANGExREQ	0x0003  /* c: request exchange info       */
#define SN_CHN_ROOMxINFOxREQ	0x0004  /* c: request room information    */
#define SN_CHN_EXTROOMxINFOxREQ 0x0005  /* c: request extended room info  */
#define SN_CHN_PEOPLExREQ	0x0006  /* c: request room people list    */
#define SN_CHN_ROOMxSEARCH	0x0007  /* c: search for room	          */
#define SN_CHN_ROOMxCREATE	0x0008  /* c: create chat room		  */
#define SN_CHN_REQUESTEDxINFO	0x0009  /* s: requested data		  */

/*--[ SNACs(E,x) CHat Service ]-------------------------------------------*/
#define SN_CHS_SUBS_NUMBER      9
#define SN_CHS_ERROR		0x0001	/* x: client/server error 	  */
#define SN_CHS_ROOMxINFOxUPDATE 0x0002	/* s: room information updated 	  */
#define SN_CHS_USERSxJOINED	0x0003	/* s: users joined chat room	  */
#define SN_CHS_USERSxLEFT	0x0004	/* s: users left chat room 	  */
#define SN_CHS_MESSxFROMxCLIENT 0x0005	/* c: send message to channel	  */
#define SN_CHS_MESSxTOxCLIENT	0x0006	/* s: message from chat to client */
#define SN_CHS_EVILxREQUEST	0x0007	/* c: evil request		  */
#define SN_CHS_EVILxREPLY	0x0008	/* s: server evil reply	 	  */
#define SN_CHS_CLIENTxERROR	0x0009	/* c: client error		  */

/*--[ SNACs(13,x) Server-Side Information Service ]-----------------------*/
#define SN_SSI_SUBS_NUMBER      28
#define SN_SSI_ERROR		0x0001	/* x: client/server error	  */
#define SN_SSI_PARAMxREQUEST	0x0002	/* c: request service parameters  */
#define SN_SSI_PARAMxREPLY	0x0003	/* s: requested service params    */
#define SN_SSI_ROASTERxREQUEST	0x0004	/* c: request ssi from server     */
#define SN_SSI_CHECKOUT		0x0005	/* c: checkout ssi 		  */
#define SN_SSI_ROASTERxREPLY  	0x0006	/* s: ssi reply			  */
#define SN_SSI_ACTIVATE 	0x0007	/* c: activate ssi		  */
#define SN_SSI_ITEMxADD		0x0008	/* c: add item to ssi	 	  */
#define SN_SSI_ITEMxUPDATE	0x0009	/* c: update ssi item		  */
#define SN_SSI_ITEMxREMOVE	0x000A	/* c: remove ssi item	 	  */
#define SN_SSI_CHANGExACK	0x000E	/* s: ack for ssi change commands */
#define SN_SSI_UPxTOxDATE	0x000F  /* s: client ssi is up-to-date    */
#define SN_SSI_EDITxBEGIN 	0x0011	/* c: ssi edit begin 		  */
#define SN_SSI_EDITxEND		0x0012	/* c: ssi edit end		  */
#define SN_SSI_AUTHxGRANT	0x0014	/* c: grant authorization	  */
#define SN_SSI_AUTHxGRANTED	0x0015	/* s: authorization granted	  */
#define SN_SSI_AUTHxSENDxREQ	0x0018	/* c: send auth request		  */
#define SN_SSI_AUTHxREQ		0x0019	/* s: auth request		  */
#define SN_SSI_AUTHxSENDxREPLY	0x001A	/* c: send auth reply		  */
#define SN_SSI_AUTHxREPLY	0x001B	/* s: auth reply		  */
#define SN_SSI_YOUxWERExADDED   0x001C	/* s: you were added message      */

/*--[ SNACs(15,x) Icq Meta Extensions ]-----------------------------------*/
#define SN_IME_SUBS_NUMBER      3
#define SN_IME_ERROR		0x0001  /* x: client/server error         */
#define SN_IME_MULTIxMESSxREQ	0x0002	/* c: multi-purpose request	  */
#define SN_IME_MULTIxMESSxRESP	0x0003	/* s: multi-purpose response	  */

/*--[ SNACs(17,x) Oscar Authorization Service ]---------------------------*/
#define SN_IES_SUBS_NUMBER      7
#define SN_IES_ERROR		0x0001  /* x: client/server error         */
#define SN_IES_AUTHxLOGIN	0x0002	/* c: start md5 auth 		  */
#define SN_IES_LOGINxREPLY	0x0003	/* s: server login reply	  */
#define SN_IES_REQxNEWxUIN	0x0004	/* c: request new uin number 	  */
#define SN_IES_SRVxNEWxUIN	0x0005	/* s: new uin number		  */
#define SN_IES_AUTHxREQUEST	0x0006  /* c: request auth key from serv  */
#define SN_IES_AUTHxKEY		0x0007	/* s: auth key from server        */
#define SN_IES_SECURIDxREQ	0x000A  /* s: server securid request      */
#define SN_IES_SECURIDxREPLY    0x000B  /* c: securid reply from client   */
#define SN_IES_REQxIMAGE	0x000C  /* c: image request from client   */
#define SN_IES_REQxIMAGExREPLY	0x000D  /* s: image request reply	  */ 

/*==[ Multi-request meta commands ]=======================================*/
#define META_REQ_OFFLINE_MSG	0x003C	/* request offline messages       */
#define META_ACK_OFFLINE_MSG	0x003E	/* ack offline messages 	  */
#define OFFLINE_MSG_RESPONSE    0x0041	/* offline message response       */
#define META_INFO_MESSAGES_EOF  0x0042  /* end of offline messages        */
#define META_REQ_INFORMATION	0x07D0	/* request various information    */
#define META_RESP_INFORMATION	0x07DA	/* information response           */

/*==[ Multi-request info meta subcommands ]===============================*/
#define META_INFO_REQ_HOMEINFO  0x04D0  /* request home information       */
#define META_INFO_REQ_SHORTINFO 0x04BA  /* request short information      */
#define META_INFO_REQ_FULLINFO  0x04B2  /* request full user info	  */
#define META_INFO_REQ_WP_UIN    0x051F  /* req white pages info by uin    */
#define META_INFO_SEARCH_REQ	0x0FA0	/* new utf8 info/search request   */
#define META_INFO_SEARCH_RESP	0x0FB4	/* new utf8 info/search resp	  */

#define META_STAT_0A8C		0x0A8C  /* some stats report		  */
#define META_STAT_0A96		0x0A96  /* some stats report              */
#define META_STAT_0AAA		0x0AAA  /* some stats report              */
#define META_STAT_0AB4		0x0AB4  /* some stats report              */
#define META_STAT_0AB9		0x0AB9  /* some stats report              */
#define META_STAT_0ABE		0x0ABE  /* some stats report              */
#define META_STAT_0AC8		0x0AC8  /* some stats report              */
#define META_STAT_0ACD		0x0ACD  /* some stats report              */
#define META_STAT_0AD2		0x0AD2  /* some stats report              */
#define META_STAT_0AD7		0x0AD7  /* some stats report              */

#define META_SEARCH_BY_UIN2     0x0569  /* tlv based search by uin	  */
#define META_SEARCH_BY_EMAIL    0x0529  /* search by email request        */
#define META_SEARCH_BY_EMAIL2   0x0547  /* wildcard search by email       */
#define META_SEARCH_BY_EMAIL3   0x0573  /* 2001/2002 search by email      */
#define META_SEARCH_BY_DETAILS  0x0515  /* search by details request      */
#define META_SEARCH_BY_DETAILS2 0x053D  /* wildcard search by details     */
#define META_SEARCH_WHITE	0x0533	/* white pages search request     */
#define META_SEARCH_WHITE2	0x0551	/* wildcards white pages search   */
#define META_SEARCH_WHITE3	0x055F	/* new white pages search   	  */
#define META_SEARCH_RANDOM	0x074E	/* search for random chat user    */
#define META_XML_REQ_DATA	0x0898  /* reuest data via xml            */
#define META_UNREGISTER_USER    0x04C4  /* remove user from database      */

#define META_INFO_SET_PASSWORD  0x042E	/* set password request           */
#define META_INFO_SET_PERMS  	0x0424  /* set permissions request	  */
#define META_INFO_SET_HOMEINFO  0x03EA	/* set home info request          */
#define META_INFO_SET_MOREINFO  0x03FD  /* set more info request          */
#define META_INFO_SET_NOTESINFO 0x0406  /* set about info request         */
#define META_INFO_SET_WORKINFO  0x03F3	/* set work info request 	  */
#define META_INFO_SET_EMAILINFO 0x040B	/* set additional email addresses */
#define META_INFO_SET_INTINFO   0x0410	/* set interests request          */
#define META_INFO_SET_AFFILAT   0x041A  /* set affilations info 	  */
#define META_INFO_SET_HPCAT     0x0442  /* set home page category info 	  */
#define META_INFO_SET_ICQPHONE  0x0654  /* set icqphone info packet       */
#define META_INFO_SET_RANDOM	0x0758  /* set random search information  */
#define META_INFO_SET_INFO	0x0C3A  /* ICQLite use this to save info  */
#define META_INFO_SET_RESULT	0x0C3F  /* Info update result (for 0C3A)  */
#define META_INFO_SET_REQ	0x0FD2  /* New utf8 info update		  */

#define META_INFO_USER_INFO2    0x00C8  /* home information response      */
#define META_INFO_MORE	  	0x00DC  /* more information response      */
#define META_INFO_EMAIL_MORE	0x00EB  /* extended email information     */
#define META_INFO_HPAGE_CAT   	0x010E  /* home page category info        */
#define META_INFO_WORK        	0x00D2  /* user work information          */
#define META_INFO_ABOUT       	0x00E6  /* user about information         */
#define META_INFO_INTERESTS   	0x00F0  /* user interests information     */
#define META_INFO_AFFILATIONS 	0x00FA  /* user affilations  		  */
#define META_INFO_SHORT		0x0104	/* short user information resp    */

#define META_INFO_PERMS_ACK	0x00A0  /* ack for set perms packet       */
#define META_INFO_PASS_ACK      0x00AA  /* ack for set password packet    */
#define META_INFO_SETHOME_ACK   0x0064	/* ack for set home info packet   */
#define META_INFO_SETWORK_ACK   0x006E	/* ack for set work info packet   */
#define META_INFO_SETMORE_ACK   0x0078	/* ack for set more info packet   */
#define META_INFO_SETNOTES_ACK  0x0082	/* ack for set notes info packet  */
#define META_INFO_SETEMAIL_ACK	0x0087	/* ack for set email info packet  */
#define META_INFO_SETINT_ACK	0x008C  /* ack for set int info packet    */
#define META_INFO_SETAFF_ACK	0x0096  /* ack for set aff info packet    */
#define META_INFO_SETHPCAT_ACK  0x00BE  /* ack for set hpage cat packet   */
#define META_UNREGISTER_ACK     0x00B4  /* ack for unregister user packet */
#define META_INFO_ICQPHONE_ACK  0x031E  /* ack for set icqphone info pack */
#define META_PROCESS_ERROR	0x0001  /* error processing meta commands */

#define META_WHITE_FOUND        0x01A4  /* wp user found packet 	  */
#define META_RANDOM_FOUND	0x0366  /* random chat user found reply   */
#define META_WHITE_LAST_FOUND   0x01AE  /* wp user found last packet 	  */
#define META_XML_DATA		0x08A2	/* data requested by xml 	  */
#define META_SEND_SMS		0x1482  /* client wants to send SMS       */

/*==[ ICQ password decryption data ]======================================*/
const unsigned char v7_cryptpass [] =
{
   0xF3, 0x26, 0x81, 0xC4, 0x39, 0x86, 0xDB, 0x92, 
   0x71, 0xA3, 0xB9, 0xE6, 0x53, 0x7A, 0x95, 0x7C
};

/*==[ Structure for SNAC header ]=========================================*/
#ifndef _SNAC_HEADER
#define _SNAC_HEADER
typedef struct snac_header
{
   unsigned short family;
   unsigned short subfamily;
   unsigned short flags;
   unsigned  long id;
   
} snac_header;
#endif /* _SNAC_HEADER */

#define META_SUCCESS 0x0a
#define META_FAIL    0x14
#define META_EMPTY   0x32

#define MSG_KIND_TXT 0x0001  /* message channel 1 */
#define MSG_KIND_ADV 0x0002  /* message channel 2 */
#define MSG_KIND_SYS 0x0004  /* message channel 4 */

#define MSG_TYPE_1 0x0001    /* message channel 1 */
#define MSG_TYPE_2 0x0002    /* message channel 2 */
#define MSG_TYPE_4 0x0004    /* message channel 4 */

/* Family error list for SNAC(xx,01) */
#define ERR_SNAC_INVALID	 0x0001
#define ERR_RATE_LIMIT		 0x0002
#define ERR_CLI_RATE_LIMIT	 0x0003
#define ERR_RECIPIENT_OFFLINE	 0x0004
#define ERR_SERVICE_UNAVAILABLE  0x0005
#define ERR_SERVICE_NON_DEFINED  0x0006
#define ERR_OBSOLETE_SNAC	 0x0007
#define ERR_SVR_NON_SUPPORTED	 0x0008
#define ERR_CLI_NON_SUPPORTED	 0x0009
#define ERR_REFUSED_BY_CLIENT	 0x000A
#define ERR_REPLY_TOO_BIG	 0x000B
#define ERR_RESPONSES_LOST	 0x000C
#define ERR_REQUEST_DENIED	 0x000D
#define ERR_SNAC_DATA_INVALID	 0x000E
#define ERR_INSUFFICIENT_RIGHTS	 0x000F
#define ERR_RECIPIENT_BLOCKED	 0x0010
#define ERR_SENDER_TOO_EVIL	 0x0011
#define ERR_RECIPIENT_TOO_EVIL	 0x0012
#define ERR_USR_UNAVAILABLE	 0x0013
#define ERR_NO_MATCH		 0x0014
#define ERR_LIST_OVERFLOW	 0x0015
#define ERR_REQUEST_AMBIGUOUS	 0x0016
#define ERR_SERVER_QUEUE_FULL	 0x0017
#define ERR_NOT_WHILE_ON_AOL	 0x0018

/* Missed message reason */
#define MISSED_MSG_INVALID       0x0000
#define MISSED_MSG_TOO_LARGE     0x0001
#define MISSED_MSG_RATE_EXCEEDED 0x0002
#define MISSED_MSG_SENDER_EVIL	 0x0003
#define MISSED_MSG_RECEIVER_EVIL 0x0004

/* String for MD5 login */
#define AIM_MD5_STRING "AOL Instant Messenger (SM)"

/* Message types */
#define MSG_TYPE_PLAIN		 0x0001
#define MSG_TYPE_CHAT_REQ	 0x0002
#define MSG_TYPE_FILE_REQ	 0x0003
#define MSG_TYPE_FILE_OK	 0x0003
#define MSG_TYPE_URL		 0x0004
#define MSG_TYPE_AUTH_REQ	 0x0006
#define MSG_TYPE_AUTH_DENIED	 0x0007
#define MSG_TYPE_AUTH_GRANTED	 0x0008
#define MSG_TYPE_ADMIN		 0x0009
#define MSG_TYPE_ADDED		 0x000C
#define MSG_TYPE_WWP		 0x000D
#define MSG_TYPE_EMAILEXPRESS	 0x000E
#define MSG_TYPE_CONTACTS	 0x0013
#define MSG_TYPE_PLUGIN		 0x001A
#define MSG_TYPE_AUTO_AWAY	 0x00E8
#define MSG_TYPE_AUTO_OCCUPIED   0x00E9
#define MSG_TYPE_AUTO_NA	 0x00EA
#define MSG_TYPE_AUTO_DND	 0x00EB
#define MSG_TYPE_AUTO_FREE4CHAT  0x00EC

/* Internal SSI action code list */
#define SSI_ADD			 0x0001
#define SSI_UPDATE		 0x0002
#define SSI_REMOVE		 0x0003

/* SSI update ack result code list */
#define SSI_UPDATE_SUCCESS	 0x0000
#define SSI_UPDATE_NOT_FOUND	 0x0002
#define SSI_UPDATE_EXISTS	 0x0003
#define SSI_UPDATE_ERROR	 0x000A
#define SSI_UPDATE_LIMIT	 0x000C
#define SSI_UPDATE_ITEM_INVALID	 0x000D
#define SSI_UPDATE_AUTH_REQUIRED 0x000E

/* SSI additional TLVs type list */
#define SSI_TLV_AUTH		 0x0066
#define SSI_TLV_GROUP		 0x00C8
#define SSI_TLV_IDLEPERMS	 0x00C9
#define SSI_TLV_PRIVACY		 0x00CA
#define SSI_TLV_RIGHTS		 0x00CB
#define SSI_TLV_PRESENSE	 0x00CC
#define SSI_TLV_IMPORTTIME	 0x00D4
#define SSI_TLV_BICONxINFO	 0x00D5
#define SSI_TLV_NICKNAME	 0x0131
#define SSI_TLV_EMAIL		 0x0137
#define SSI_TLV_SMSNUMBER	 0x013A
#define SSI_TLV_COMMENT		 0x013C
#define SSI_TLV_ALERTS		 0x013D
#define SSI_TLV_SOUND		 0x013E
#define SSI_TLV_LASTACCESS	 0x0145

/* Info TLV type list */
#define INF_TLV_FIRSTNAME	 0x0140   /* 320 - first name             */
#define INF_TLV_LASTNAME	 0x014A   /* 330 - last name              */
#define INF_TLV_NICKNAME	 0x0154   /* 340 - nick name              */
#define INF_TLV_EMAIL            0x015E   /* 350 - email info             */
#define INF_TLV_AGE_RANGE        0x0168   /* 360 - age range (min-max)    */
#define INF_TLV_AGE              0x0172   /* 370 - age (max 255)          */
#define INF_TLV_GENDER           0x017C   /* 380 - gender                 */
#define INF_TLV_SPOKENLANG       0x0186   /* 390 - spoken language        */
#define INF_TLV_HOMECITY         0x0190   /* 400 - home city              */
#define INF_TLV_HOMESTATE        0x019A   /* 410 - home state             */
#define INF_TLV_HOMECOUNTRY      0x01A4   /* 420 - home country code      */
#define INF_TLV_WCOMPANY         0x01AE   /* 430 - work company           */
#define INF_TLV_WDEPARTMENT      0x01B8   /* 440 - work department        */
#define INF_TLV_WPOSITION        0x01C2   /* 450 - work position          */
#define INF_TLV_WOCUPATION       0x01CC   /* 460 - work ocupation code    */
#define INF_TLV_AFFILATIONS      0x01D6   /* 470 - affilations            */

#define INF_TLV_INTERESTS        0x01EA   /* 490 - interests info         */
#define INF_TLV_PASTINFO         0x01FE   /* 510 - past information       */

#define INF_TLV_HOMEPAGECAT      0x0212   /* 530 - homepage category      */
#define INF_TLV_HOMEWEBPAGE      0x0213   /* 531 - web homepage url       */

#define INF_TLV_SEARCHKEYWORD    0x0226   /* 550 - whitepage search key   */
#define INF_TLV_SEARCHONLINE     0x0230   /* 560 - search online flag     */
#define INF_TLV_BIRTHDAY         0x023A   /* 570 - birthday (year,mon,day)*/

#define INF_TLV_USERNOTES        0x0258   /* 600 - user notes text        */
#define INF_TLV_HOMEADDR         0x0262   /* 610 - home street address    */
#define INF_TLV_HOMEZIP          0x026C   /* 620 - home zip code          */
#define INF_TLV_HOMEZIP2         0x026D   /* 621 - new home zip code      */
#define INF_TLV_HOMEPHONE	 0x0276   /* 630 - home phone number      */
#define INF_TLV_HOMEFAX          0x0280   /* 640 - home fax number        */
#define INF_TLV_CELLULAR         0x028A   /* 650 - private cellular phone */
#define INF_TLV_WORKADDR         0x0294   /* 660 - work street address    */
#define INF_TLV_WORKCITY         0x029E   /* 670 - work city              */
#define INF_TLV_WORKSTATE        0x02A8   /* 680 - work state abbr        */
#define INF_TLV_WORKCOUNTRY      0x02B2   /* 690 - work country code      */
#define INF_TLV_WORKZIP          0x02BC   /* 700 - work zip code          */
#define INF_TLV_WORKZIP2         0x02BD   /* 701 - new work zip code      */
#define INF_TLV_WORKPHONE        0x02C6   /* 710 - work phone number      */
#define INF_TLV_WORKFAX          0x02D0   /* 720 - work fax number        */
#define INF_TLV_WORKWEBPAGE      0x02DA   /* 730 - work webpage url       */


#define INF_TLV_WEBPERMS         0x02F8   /* 760 - allow web view status  */

#define INF_TLV_AUTH             0x030C   /* 780 - authorization perms    */
#define INF_TLV_GMTOFFSET        0x0316   /* 790 - local gmt offset       */
#define INF_TLV_OCITY            0x0320   /* 800 - originally from city   */
#define INF_TLV_OSTATE           0x032A   /* 810 - originally from state  */
#define INF_TLV_OCOUNTRY         0x0334   /* 820 - orig from country      */
#define INF_TLV_MARTIAL          0x033E   /* 830 - martial status         */

/* Message flags masks */
#define ICBM_FLG_MTN         0x00000008   /* mtn messages allowed         */
#define ICBM_FLG_MISSED      0x00000002   /* missed notices allowed       */
#define ICBM_FLG_BASE        0x00000001   /* base messages allowed        */

/* Message channel indices */
#define MCH4 3
#define MCH3 2
#define MCH2 1
#define MCH1 0

/* user class defines */
#define CLASS_UNCONFIRMED	0x0001     /* AOL unconfirmed user flag   */
#define CLASS_ADMINISTRATOR	0x0002     /* AOL administrator flag 	  */
#define CLASS_AOL		0x0004     /* AOL staff user flag 	  */
#define CLASS_COMMERCIAL	0x0008	   /* AOL commercial account flag */
#define CLASS_FREE		0x0010	   /* non-commercial account flag */
#define CLASS_AWAY		0x0020	   /* Away status flag 		  */
#define CLASS_ICQ		0x0040	   /* ICQ user sign 		  */
#define CLASS_WIRELESS		0x0080	   /* AOL wireless user 	  */

/* location info mask values */
#define INFO_EMPTY		0x00000000 /* user fixed online info      */
#define INFO_PROF		0x00000001 /* user profile request        */
#define INFO_AWAY		0x00000002 /* user away info request      */
#define INFO_CAPS		0x00000004 /* user caps block request     */
#define INFO_CERT		0x00000008 /* user cert info reqeust      */

#endif /* _V7_DEFINES_H */

