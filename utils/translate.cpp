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
/**************************************************************************/

#include "includes.h"

ITranslator::ITranslator()
{
	setDefaultTranslationMap();
	memset(m_szMapFileName, 0, 255);
}


ITranslator::~ITranslator()
{
}


void ITranslator::setDefaultTranslationMap()
{
   for(int i=0;i<256;i++)
   {
      serverToClientTab[i]=i;
      clientToServerTab[i]=i;
   }

   m_bDefault=true;
   memset(m_szMapFileName, 0, 255);
}


bool ITranslator::setTranslationMap(const char *szMapFileName)
{
   FILE *fhdl;

   if(strcmp(szMapFileName, "DEFAULT"))
   {
      printf("Setting default translation map...\n");
      setDefaultTranslationMap();
      return true;
   }
   if ((fhdl = fopen(szMapFileName, "r")) == NULL)
   {
      /* "Could not open the translation file for reading." */
      printf("Can't open translation file for reading...\n");
      setDefaultTranslationMap();
      return false;
   }

   char buffer[80];
   unsigned int inputs[8];
   int finished = 0;
   unsigned char temp_table[512];
   int c=0;

   while((!finished) && (c<512))
   {
      for (int i=0; i<80; i++)
      {
        if (fread(&buffer[i], 1, 1, fhdl) == 0) finished = 1;
        if (buffer[i] == 0x0a) {buffer[i] = 0; break;}
      }
      if(
   	sscanf(buffer, "0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x",
	inputs+0, inputs+1, inputs+2, inputs+3,
	inputs+4, inputs+5, inputs+6, inputs+7)   <8
      ) {
        /* "Syntax error in translation file." */
	printf("Syntax error in translation file....\n");
        setDefaultTranslationMap();
        fclose(fhdl);
	return false;
      }

      for (int j = 0; j<8; j++)temp_table[c++] = (unsigned char) inputs[j];
   }

   fclose(fhdl);

   if(c==512)
   {
      for (c = 0; c < 256; c++)
      {
         serverToClientTab[c] = temp_table[c];
         clientToServerTab[c] = temp_table[c | 256];
      }
   }
   else
   {
      /* "Translation file corrupted." */
      printf("Translation file corrupted....\n");
      setDefaultTranslationMap();
      return false;
   }

   m_bDefault=false;
   memset(m_szMapFileName, 0, 255);
   snprintf(m_szMapFileName, 255, szMapFileName);
   printf("Translation map initialization complete...\n");

   return true;
}


void ITranslator::translateToServer(char *szString)
{
   if(m_bDefault)return;
   char *pC=szString;
   while(*pC)
   {
      (*pC)=serverToClientTab[(unsigned char)(*pC)];
      pC++;
   }
}


void ITranslator::translateToClient(char *szString)
{
   if (m_bDefault) return;
   char *pC=szString;

   while(*pC)
   {
      (*pC)=clientToServerTab[(unsigned char)(*pC)];
      pC++;
   }
}

