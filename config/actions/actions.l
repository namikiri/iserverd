%{

#include "includes.h"

void skipcmnts ();
void skipcmnts2 ();

typedef struct include_struct
{
   YY_BUFFER_STATE buffer;
   char filename[255];
   int line_number;
   
} include_struct;


include_struct include_stack[MAX_INCLUDE_DEPTH];

%}

%option noyywrap
%x incl

digit      [0-9]
letter     [_a-zA-Z]
hex_dig    [0-9a-fA-F]
extletter  [_\-0-9a-zA-Z]
ident      {letter}{extletter}*
digits     ({digit}{1,10})
digits16   0x{hex_dig}{1,8}
string     \"[^\"][^\"]*\"
wrident    [0-9]*[0-9a-zA-Z_]*
whitespace [ \t\n]

%%

\/\/[^\n]*\n    {   /* COMMENT */; line_number++; }
"#"     	{   skipcmnts2(); }

"/*"            {   skipcmnts (); }
";"             {   if (config_file_type == CONFIG_TYPE_ACTIONS) 
                    {
		       return(T_END_RULE);
		    }
		    else
		    {
		       return(T_END_RULE1);
		    }
		}
		      
"$"             {   return('$'); }
"@"             {   return('@'); }
"["		{   return('['); }
"]"             {   return(']'); }
"{"             {   return('{'); }
"}"             {   return('}'); }
","             {   return(','); }
"="		{   return('='); }
"-"             {   return(T_MIN_OP); }
"+"             {   return(T_PLS_OP); }

<incl>[ \t]*    {/* eat the whitespace */}
<incl>{string}  { 

   /* got the include file name */
   if ( include_stack_ptr >= MAX_INCLUDE_DEPTH )
   {
      yyerror("Includes nested too deeply. Operator skiped.");
      BEGIN(INITIAL);
   }
   else
   {
      strncpy(us_token, yytext+1, 255); 
      us_token[strlen(us_token)-1] = '\0';
      yyin_temp = fopen(us_token, "r");
   
      if ( ! yyin_temp )
      {
         yyerror("Warning, can't open include file (%s).", us_token);
      }
      else
      {
         include_stack[include_stack_ptr++].buffer = YY_CURRENT_BUFFER;
	 include_stack[include_stack_ptr-1].line_number = line_number;
	 strncpy(include_stack[include_stack_ptr-1].filename, filename, 255);
	 
	 strncpy(filename, us_token, 255);
	 
         yyin = yyin_temp;
         yy_switch_to_buffer(yy_create_buffer( yyin, YY_BUF_SIZE ) );
      }
      
      BEGIN(INITIAL);
   }
}

{string}        {   strncpy(us_token, yytext+1, 255); 
		    us_token[strlen(us_token)-1] = '\0';
		    return(T_STRING); 
		}
"log to"        {   if (config_file_type == CONFIG_TYPE_ACTIONS) return(T_LOG_LEX); }
"log"           {   if (config_file_type == CONFIG_TYPE_ACTIONS) return(T_LOG_LEX); }
"var"           {   return(T_VAR_LEX); }
"event"         {   if (config_file_type == CONFIG_TYPE_ACTIONS) return(T_EVENT_LEX); }
"Online"        {   if (config_file_type == CONFIG_TYPE_ACTIONS) return(T_ONLINE_LEX); }
"for"           {   if (config_file_type == CONFIG_TYPE_ACTIONS) return(T_FOR_LEX); }
"msg to"        {   if (config_file_type == CONFIG_TYPE_ACTIONS) return(T_MESSAGE_LEX); }
"msg"           {   if (config_file_type == CONFIG_TYPE_ACTIONS) return(T_MESSAGE_LEX); }
"message to"    {   if (config_file_type == CONFIG_TYPE_ACTIONS) return(T_MESSAGE_LEX); }
"message"       {   if (config_file_type == CONFIG_TYPE_ACTIONS) return(T_MESSAGE_LEX); }
"stop"          {   if (config_file_type == CONFIG_TYPE_ACTIONS) return(T_STOP_LEX); }
"mail to"	{   if (config_file_type == CONFIG_TYPE_ACTIONS) return(T_MAIL_LEX); }
"mail"          {   if (config_file_type == CONFIG_TYPE_ACTIONS) return(T_MAIL_LEX); }
"run"           {   if (config_file_type == CONFIG_TYPE_ACTIONS) return(T_RUN_LEX); }
"with"          {   if (config_file_type == CONFIG_TYPE_ACTIONS) return(T_WITH_LEX); }
"using"		{   if (config_file_type == CONFIG_TYPE_ACTIONS) return(T_USING_LEX); }
"Offline"       {   if (config_file_type == CONFIG_TYPE_ACTIONS) return(T_OFFLINE_LEX); }
"SaveBasicInfo" {   if (config_file_type == CONFIG_TYPE_ACTIONS) return(T_SAVEBASIC_LEX); }
"Search"        {   if (config_file_type == CONFIG_TYPE_ACTIONS) return(T_SEARCH_LEX); }
"StatusChange"  {   if (config_file_type == CONFIG_TYPE_ACTIONS) return(T_STATUS_LEX); }
"any"           {   if (config_file_type == CONFIG_TYPE_ACTIONS) return(T_ANY_LEX); }
"Registration"  {   if (config_file_type == CONFIG_TYPE_ACTIONS) return(T_REGISTRATION_LEX); }
"RDBMS_fail"	{   if (config_file_type == CONFIG_TYPE_ACTIONS) return(T_RDBMS_FAIL_LEX); }
"RDBMS_error"   {   if (config_file_type == CONFIG_TYPE_ACTIONS) return(T_RDBMS_ERR_LEX); }
"Internal_err"  {   if (config_file_type == CONFIG_TYPE_ACTIONS) return(T_INTERNAL_ERR_LEX); }
"PPHung"	{   if (config_file_type == CONFIG_TYPE_ACTIONS) return(T_PPHUNG_LEX); }
"S_ONLINE"	{   if (config_file_type == CONFIG_TYPE_ACTIONS) return(T_S_ONLINE); }
"S_OFFLINE"     {   if (config_file_type == CONFIG_TYPE_ACTIONS) return(T_S_OFFLINE); }
"S_FFCH"        {   if (config_file_type == CONFIG_TYPE_ACTIONS) return(T_S_FFCH); }
"S_PRIVACY"     {   if (config_file_type == CONFIG_TYPE_ACTIONS) return(T_S_PRIVACY); }
"S_DND"         {   if (config_file_type == CONFIG_TYPE_ACTIONS) return(T_S_DND); }
"S_AWAY"        {   if (config_file_type == CONFIG_TYPE_ACTIONS) return(T_S_AWAY); }
"S_OCCUPIED"    {   if (config_file_type == CONFIG_TYPE_ACTIONS) return(T_S_OCCUPIED); }
"S_NA"          {   if (config_file_type == CONFIG_TYPE_ACTIONS) return(T_S_NA); }
"S_ANY"         {   if (config_file_type == CONFIG_TYPE_ACTIONS) return(T_ANY_LEX); }

"rate"		{   if (config_file_type == CONFIG_TYPE_AIM) return(T_RATE_LEX); }
"class"		{   if (config_file_type == CONFIG_TYPE_AIM) return(T_CLASS_LEX); }
"classes"       {   if (config_file_type == CONFIG_TYPE_AIM) return(T_CLASSES_LEX); }
"snac"		{   if (config_file_type == CONFIG_TYPE_AIM) return(T_SNAC_LEX); }
"family"	{   if (config_file_type == CONFIG_TYPE_AIM) return(T_FAMILY_LEX); }
"index"		{   if (config_file_type == CONFIG_TYPE_AIM) return(T_INDEX_LEX); }
"version"	{   if (config_file_type == CONFIG_TYPE_AIM) return(T_VERSION_LEX); }
"subtype"	{   if (config_file_type == CONFIG_TYPE_AIM) return(T_SUBTYPE_LEX); }
"number"	{   if (config_file_type == CONFIG_TYPE_AIM) return(T_NUMBER_LEX); }
"num"           {   if (config_file_type == CONFIG_TYPE_AIM) return(T_NUMBER_LEX); }
"True"		{   strncpy(us_token, "1", 255); return(T_DIGITS); }
"False"		{   strncpy(us_token, "0", 255); return(T_DIGITS); }

include	        BEGIN(incl);

{ident}         {   strncpy(us_token, yytext, 255); 
		    strncpy(iname, yytext, 255);
		    return(T_IDENT); 
		}
{digits}        {   strncpy(us_token, yytext, 255); return(T_DIGITS); }
{digits16}	{   strncpy(us_token, yytext, 255); return(T_DIGITS16); }
{wrident}       {   strncpy(us_token, yytext, 255); return(T_WRONG_IDENT); }
\n              {   /* SKIPED */; line_number++; }
" "             {   /* SKIPED */; }

<<EOF>> {

   if ( --include_stack_ptr < 0 )
   {
      yyterminate();
   }
   else
   {
      yy_delete_buffer( YY_CURRENT_BUFFER ); fclose(yyin);
      yy_switch_to_buffer(include_stack[include_stack_ptr].buffer);
      line_number = include_stack[include_stack_ptr].line_number;
      strncpy(filename, include_stack[include_stack_ptr].filename, 255);
   }
}

%%
void skipcmnts ()
{
   int lexem;
   for (;;) 
   {
      lexem = input();
      while (lexem != '*') 
      {
         if (lexem == '\n') line_number++;
         lexem = input();
      }
      
      lexem = input();
      if (lexem == '\n') line_number++;
      
      if (lexem != '/') 
      {
         unput(yytext[yyleng-1]);
      }
      else return;
   }
}

void skipcmnts2 ()
{
   for (;;) 
   {
      while (input() != '\n');
      line_number++;
      return;
   }
}
