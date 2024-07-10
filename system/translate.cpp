/**************************************************************************/
/* Many parts of this source code were 'inspired' by the ircII4.4 	  */
/* translat.c source.							  */
/* RIPPED FROM KVirc: http://www.kvirc.org				  */
/* Original by Szymon Stefanek (kvirc@tin.it).				  */
/* Modified by Andrew Frolov (dron@linuxer.net)				  */
/* Modified by Alexandr Shutko (AVShutko@mail.khstu.ru)			  */
/*									  */
/**************************************************************************/

#include "includes.h"

/**************************************************************************/
/* Class constructor							  */
/**************************************************************************/
ITranslator::ITranslator()
{
	setDefaultTranslationMap();
	memset(m_szMapFileName, 0, 255);
}


/**************************************************************************/
/* Class destructor							  */
/**************************************************************************/
ITranslator::~ITranslator()
{
}


/**************************************************************************/
/* set default translation map - translation disabled 			  */
/**************************************************************************/
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


/**************************************************************************/
/* Set specified translation map					  */
/**************************************************************************/
bool ITranslator::setTranslationMap(const char *szMapFileName)
{
   FILE *fhdl;

   if(strcsequal(szMapFileName,"DEFAULT"))
   {
      setDefaultTranslationMap();
      return true;
   }

   if ((fhdl = fopen(szMapFileName, "r")) == NULL)
   {
      /* "Could not open the translation file for reading." */
      LOG_SYS(0, ("Can't open translation file: %s\n", szMapFileName));
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
      setDefaultTranslationMap();
      return false;
   }

   m_bDefault=false;
   memset(m_szMapFileName, 0, 255);
   snprintf(m_szMapFileName, 255, szMapFileName);

   return true;
}


/**************************************************************************/
/* Translate client data to server format				  */
/**************************************************************************/
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


/**************************************************************************/
/* Translate server data to client 					  */
/**************************************************************************/
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

