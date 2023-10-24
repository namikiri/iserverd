%{

#include "includes.h"

void yyerror(char *s, ...)
{
   va_list ap;
   char res_str[255];
   
   /* Well, if it is simply "syntax error" from parser we */
   /* should find out information about error location    */
   
   va_start(ap, s);
   vsnprintf(res_str, 255, s, ap);
   va_end(ap);
  
   
   printf("Error: %s\n", res_str);
   printf("Error: line number = %d. Rule skiped.\n", line_number);
   
   LOG_SYS(0, ("Init: AP/AIM parser: %s\n", res_str));
   LOG_SYS(0, ("Init: config line number = %d. Rule skiped.\n", line_number));
   
}

int yylex();
int yyparse();

%}

%token	T_END_RULE T_MIN_OP T_PLS_OP T_LOG_LEX T_VAR_LEX T_EVENT_LEX
%token	T_ONLINE_LEX T_FOR_LEX T_MESSAGE_LEX T_STOP_LEX
%token	T_MAIL_LEX T_RUN_LEX T_WITH_LEX T_OFFLINE_LEX T_SAVEBASIC_LEX
%token	T_SEARCH_LEX T_STATUS_LEX T_ANY_LEX T_REGISTRATION_LEX 
%token	T_IDENT T_DIGITS T_WRONG_IDENT T_STRING T_NONE T_USING_LEX
%token  T_S_ONLINE T_S_OFFLINE T_S_NA T_S_AWAY T_S_DND T_S_PRIVACY T_S_FFCH T_S_OCCUPIED
%token  T_RDBMS_FAIL_LEX T_INTERNAL_ERR_LEX T_PPHUNG_LEX T_RDBMS_ERR_LEX

/* for aim_proto config file */
%token  T_RATE_LEX T_CLASS_LEX T_SNAC_LEX T_FAMILY_LEX T_INDEX_LEX T_VERSION_LEX 
%token  T_SUBTYPE_LEX T_DIGITS16 T_NUMBER_LEX T_END_RULE1 T_CLASSES_LEX

%%

config.file
    : actions.conf
    | aim_proto.conf
    ;

/* -----------------------------------------------------------------*/    
/* --[ aim_proto.conf statements ]----------------------------------*/
/* -----------------------------------------------------------------*/    
aim_proto.conf
    : aim_statement
    | aim_proto.conf aim_statement
    ;

aim_statement
    : aim.variable.stmt
    | rate.class.idx.stmt
    | snac.family.stmt
    | error T_END_RULE1
    ;

aim.variable.stmt
    : T_VAR_LEX T_IDENT '=' T_STRING T_END_RULE1
    {
       aim_var2list(aim_create_variable(iname,us_token));
    }
    ;

rate.class.idx.stmt
    : T_RATE_LEX T_CLASS_LEX T_INDEX_LEX T_DIGITS { rsindex=atoul(us_token); add_rate_class(rsindex); } class.index.body
    ;

class.index.body
    : '{' class.index.stmt.list '}'
    | '{' '}'
    ;
    
class.index.stmt.list
    : class.index.stmt
    | class.index.stmt.list class.index.stmt
    ;

class.index.stmt
    : T_IDENT { strncpy(pname, us_token, 254); } '=' global.number { pvalue = atoul(us_token); } T_END_RULE1
    { 
       add_rate_variable(rsindex, pname, pvalue); 
    } 
    ;

snac.family.stmt
    : T_SNAC_LEX T_FAMILY_LEX global.number { family = snumber; add_snac_family(family); } snac.family.body
    ;

snac.family.body
    : '{' snac.family.body.stmt.list '}'
    | '{' '}'
    ;

snac.family.body.stmt.list
    : snac.family.body.stmt
    | snac.family.body.stmt.list snac.family.body.stmt
    ;

snac.family.body.stmt
    : T_VERSION_LEX '=' global.number T_END_RULE1 { family_set_version(family, snumber); }
    | T_IDENT '=' global.number T_END_RULE1
    | rate.class.with.subtypes
    ;

rate.class.with.subtypes
    : T_RATE_LEX T_CLASS_LEX T_DIGITS { rsindex = rate_check(atoul(us_token)); } rate.class.with.subtypes.body 
    ;

rate.class.with.subtypes.body
    : '{' rate.class.with.subtypes.stmt.list '}'
    ;

rate.class.with.subtypes.stmt.list
    : rate.class.with.subtypes.stmt
    | rate.class.with.subtypes.stmt.list rate.class.with.subtypes.stmt
    ;

rate.class.with.subtypes.stmt
    : T_SUBTYPE_LEX global.number T_END_RULE1 
    {
       add_family_subtype(family, rsindex, snumber);
    }
    ;

global.number
    : T_DIGITS   { snumber = atoul(us_token);    }
    | T_DIGITS16 { snumber = ahextoul(us_token); }
    ;

/* -----------------------------------------------------------------*/
/* --[ actions.conf statements ]------------------------------------*/
/* -----------------------------------------------------------------*/
actions.conf
    : statement
    | actions.conf statement
    ;

statement
    : variable.declaration.stmt
    | event.pphung.section
    | event.rdbms_fail.section
    | event.rdbms_error.section
    | event.int_err.section
    | event.online.section
    | event.offline.section
    | event.savebasic.section
    | event.search.section
    | event.status.section
    | event.registration.section
    | empty.stmt
    | error T_END_RULE { yyerrok; }
    ;


/* variable declaration statement */
variable.declaration.stmt
    : T_VAR_LEX T_IDENT T_DIGITS {strncpy(par2,us_token,255); } T_END_RULE 
    {
       uin = atoul(par2);
       add_var2list(create_variableA(iname,par2,uin,VAR_NUM));
    }
    | T_VAR_LEX T_IDENT T_STRING {strncpy(par2,us_token,255); } T_END_RULE 
    {
       uin = atoul(par2);
       add_var2list(create_variableA(iname,par2,uin,VAR_STR));
    }
    | T_VAR_LEX T_IDENT { strncpy(par1, us_token, 255); } '$' '{' T_IDENT '}' T_END_RULE 
    {
       var1 = variable_lookup(us_token);
       if (var1 != NULL)
       {
          add_var2list(create_variableA(par1,var1->str_val,var1->num_val,var1->type));
       }
       else
       {
          yyerror("Variable \"%s\" is not defined.", us_token);
       }    
    }
    | T_VAR_LEX T_IDENT { strncpy(par1, us_token, 255); } uin.section.wrap2 T_END_RULE
    {
       add_var2list(create_variableB(iname,uins_node));
    }
    ;

event.pphung.section
    : T_EVENT_LEX T_PPHUNG_LEX { rsection = ACT_PPHUNG; } section3.body T_END_RULE
    ;

event.rdbms_fail.section
    : T_EVENT_LEX T_RDBMS_FAIL_LEX { rsection = ACT_RDBMSFAIL; } section3.body T_END_RULE
    ;

event.rdbms_error.section
    : T_EVENT_LEX T_RDBMS_ERR_LEX { rsection = ACT_RDBMSERR; } section3.body T_END_RULE
    ;

event.int_err.section
    : T_EVENT_LEX T_INTERNAL_ERR_LEX { rsection = ACT_INTERR; } section3.body T_END_RULE
    ;
    
event.online.section
    : T_EVENT_LEX T_ONLINE_LEX { rsection = ACT_ONLINE; } section1.body T_END_RULE
    ;

event.offline.section
    : T_EVENT_LEX T_OFFLINE_LEX { rsection = ACT_OFFLINE; } section1.body T_END_RULE
    ;

event.savebasic.section
    : T_EVENT_LEX T_SAVEBASIC_LEX { rsection = ACT_SAVEBASIC; } section1.body T_END_RULE
    ;

event.search.section
    : T_EVENT_LEX T_SEARCH_LEX { rsection = ACT_SEARCH; } section1.body T_END_RULE
    ;

event.status.section
    : T_EVENT_LEX T_STATUS_LEX { rsection = ACT_STATUS; } section2.body T_END_RULE
    ;

event.registration.section
    : T_EVENT_LEX T_REGISTRATION_LEX { rsection = ACT_REGISTR; } section3.body T_END_RULE
    ;

section1.body
    : '{' event.commands.stmt.list '}'
    | '{' '}'
    ;
    
section2.body
    : '{' event.commands2.stmt.list '}'
    | '{' '}'
    ;

section3.body
    : '{' event.special.commands.stmt.list '}'
    | '{' '}'
    ;

event.commands2.stmt.list
    : event.commands2.stmt
    | event.commands2.stmt.list event.commands2.stmt
    ;

event.commands.stmt.list
    : event.commands.stmt
    | event.commands.stmt.list event.commands.stmt
    ;

event.special.commands.stmt.list 
    : event.special.commands.stmt
    | event.special.commands.stmt.list event.special.commands.stmt
    ;

event.commands2.stmt
    : T_FOR_LEX uin.section.n1 T_WITH_LEX status.vals T_MESSAGE_LEX uin.section.n2 using.stmt nstring { strncpy(par2, us_token, 255); } astop.stmt T_END_RULE
    { 
       add_rule_to_list(rsection, A_MSG, NULL, par2, NULL, uins_lst1, uins_lst2, rstatus, astop);
    } 
    | T_FOR_LEX uin.section.n1 T_WITH_LEX status.vals T_LOG_LEX nstring { strncpy(par2, us_token, 255); } astop.stmt T_END_RULE
    { 
       add_rule_to_list(rsection, A_LOG, NULL, par2, NULL, uins_lst1, NULL, rstatus, astop);
    }
    | T_FOR_LEX uin.section.n1 T_WITH_LEX status.vals T_STOP_LEX astop.stmt T_END_RULE
    { 
       add_rule_to_list(rsection, A_STOP, NULL, NULL, NULL, uins_lst1, NULL, rstatus, astop);
    }
    | T_FOR_LEX uin.section.n1 T_WITH_LEX status.vals T_MAIL_LEX nstring { strncpy(par1, us_token, 255); } using.stmt nstring { strncpy(par2, us_token, 255); } astop.stmt T_END_RULE
    { 
       add_rule_to_list(rsection, A_MAIL, par1, par2, NULL, uins_lst1, NULL, rstatus, astop);
    }
    | T_FOR_LEX uin.section.n1 T_WITH_LEX status.vals T_RUN_LEX nstring { strncpy(par1, us_token, 255); } astop.stmt T_END_RULE
    { 
       add_rule_to_list(rsection, A_RUN, par1, par2, NULL, uins_lst1, NULL, rstatus, astop);
    }
    | event.commands.stmt
    ;

event.commands.stmt
    : T_FOR_LEX uin.section.n1 T_MESSAGE_LEX uin.section.n2 using.stmt nstring { strncpy(par2, us_token, 255); } astop.stmt T_END_RULE
    { 
       add_rule_to_list(rsection, A_MSG, NULL, par2, NULL, uins_lst1, uins_lst2, 0, astop);
    } 
    | T_FOR_LEX uin.section.n1 T_LOG_LEX nstring { strncpy(par2, us_token, 255); } astop.stmt T_END_RULE
    { 
       add_rule_to_list(rsection, A_LOG, NULL, par2, NULL, uins_lst1, NULL, 0, astop);
    }
    | T_FOR_LEX uin.section.n1 T_STOP_LEX astop.stmt T_END_RULE
    { 
       add_rule_to_list(rsection, A_STOP, NULL, NULL, NULL, uins_lst1, NULL, 0, astop);
    }
    | T_FOR_LEX uin.section.n1 T_MAIL_LEX nstring { strncpy(par1, us_token, 255); } using.stmt nstring { strncpy(par2, us_token, 255); } astop.stmt T_END_RULE
    { 
       add_rule_to_list(rsection, A_MAIL, par1, par2, NULL, uins_lst1, NULL, 0, astop);
    }
    | T_FOR_LEX uin.section.n1 T_RUN_LEX nstring { strncpy(par1, us_token, 255); } astop.stmt T_END_RULE
    { 
       add_rule_to_list(rsection, A_RUN, par1, par2, NULL, uins_lst1, NULL, 0, astop);
    }
    | event.special.commands.stmt
    ;

event.special.commands.stmt
    : T_MESSAGE_LEX uin.section.n2 using.stmt nstring { strncpy(par2, us_token, 255); } astop.stmt T_END_RULE
    { 
       add_rule_to_list(rsection, A_MSG, "", par2, NULL, NULL, uins_lst2, 0, astop);
    }
    | T_LOG_LEX nstring { strncpy(par2, us_token, 255); } astop.stmt T_END_RULE                
    { 
       add_rule_to_list(rsection, A_LOG, "", par2, NULL, NULL, NULL, 0, astop);
    }
    | T_STOP_LEX T_END_RULE                                 
    { 
       add_rule_to_list(rsection, A_STOP, NULL, NULL, NULL, NULL, NULL, 0, 0);
    }
    | T_MAIL_LEX nstring { strncpy(par1, us_token, 255); } using.stmt nstring { strncpy(par2, us_token, 255); } astop.stmt T_END_RULE 
    { 
       add_rule_to_list(rsection, A_MAIL, par1, par2, NULL, NULL, NULL, 0, astop);
    }
    | T_RUN_LEX nstring { strncpy(par1, us_token, 255); } astop.stmt T_END_RULE
    { 
       add_rule_to_list(rsection, A_RUN, par1, NULL, NULL, NULL, NULL, 0, astop);
    }
    | empty.stmt
    | error T_END_RULE { yyerrok; }
    ;

uin.section.n1
    : uin.section.wrap { uins_lst1 = uins_node;}
    ;

uin.section.n2
    : uin.section.wrap3 { uins_lst2 = uins_node;}
    ;

uin.section.wrap
    : T_ANY_LEX { uins_node = NULL; }
    | { uins_node = create_uin_node(); } ndigits
    { 
       uin = atoul(us_token);
       add_uin2node(uins_node, uin, UIN_NONE);       
    }
    | { uins_node = create_uin_node(); } uin.list
    | list.name
    {
       /* here we should find variable and copy its value (list) to */
       /* uin_node, but do this only if variable has list type      */
       var1 = variable_lookup(iname);
       if ((var1 == NULL) | (var1->type != VAR_LST))
       {
          yyerror("Variable \"%s\" should have list type.", us_token);
          
          YYERROR;
       }
       else
       {
          uins_node = var1->uins;
       }
    }
    | error T_END_RULE 
    { 
       delete_uin_node(uins_node); 
       delete_uin_node(uins_lst1); 
       delete_uin_node(uins_lst2); 
       yyerrok; 
    }
    ;

uin.section.wrap2
    : { uins_node = create_uin_node(); } uin.list
    | error T_END_RULE 
    { 
       delete_uin_node(uins_node); 
       delete_uin_node(uins_lst1); 
       delete_uin_node(uins_lst2); 
       yyerrok; 
    }
    ;

uin.section.wrap3
    : { uins_node = create_uin_node(); } ndigits
    { 
       uin = atoul(us_token);
       add_uin2node(uins_node, uin, UIN_NONE);       
    }
    | { uins_node = create_uin_node(); } uin.list2
    | list.name
    {
       /* here we should find variable and copy its value (list) to */
       /* uin_node, but do this only if variable has list type      */
       var1 = variable_lookup(iname);
       if ((var1 == NULL) | (var1->type != VAR_LST))
       {
          yyerror("Variable \"%s\" should have list type.", us_token);
          
          YYERROR;
       }
       else
       {
          uins_node = var1->uins;
       }
    }
    | error T_END_RULE 
    { 
       delete_uin_node(uins_node); 
       delete_uin_node(uins_lst1); 
       delete_uin_node(uins_lst2); 
       yyerrok; 
    }
    ;

uin.list
    : '[' uin.list.internal ']' 
    | '[' T_ANY_LEX ']' 
    { 
       delete_uin_node(uins_node); 
       uins_node = NULL; 
    }
    ;

uin.list2
    : '[' uin.list.internal2 ']' 
    ;

uin.list.internal
    : ndigits
    {
       uin = atoul(us_token);
       add_uin2node(uins_node, uin, UIN_NONE);
    }
    | uin.list.internal ',' ndigits
    {
       uin = atoul(us_token);
       add_uin2node(uins_node, uin, UIN_NONE);
    }
    | uin.range
    | uin.list.internal ',' uin.range
    ; 

uin.list.internal2
    : ndigits
    {
       uin = atoul(us_token);
       add_uin2node(uins_node, uin, UIN_NONE);
    }
    | uin.list.internal2 ',' ndigits
    {
       uin = atoul(us_token);
       add_uin2node(uins_node, uin, UIN_NONE);
    }
    ; 

uin.range
    : ndigits { uin1 = atoul(us_token); } 
      T_MIN_OP ndigits 
    { 
       uin2 = atoul(us_token);
       if (uin1 > uin2) { uin  = uin1; uin1 = uin2; uin2 = uin; }
       if (uin1 == 0)   { uin1 = 1; }
       add_uin2node(uins_node, uin1, uin2);
    }
    ;

status.vals
    : T_S_ONLINE   { rstatus = ICQ_STATUS_ONLINE; }
    | T_S_OFFLINE  { rstatus = ICQ_STATUS_OFFLINE; }
    | T_S_NA       { rstatus = ICQ_STATUS_NA; }
    | T_S_AWAY	   { rstatus = ICQ_STATUS_AWAY; }
    | T_S_DND      { rstatus = ICQ_STATUS_DND; }
    | T_S_PRIVACY  { rstatus = ICQ_STATUS_PRIVATE; }
    | T_S_FFCH     { rstatus = ICQ_STATUS_FFC; }
    | T_S_OCCUPIED { rstatus = ICQ_STATUS_OCCUPIED; }
    | T_ANY_LEX    { rstatus = ICQ_STATUS_ANY; }
    ;

empty.stmt 
    : T_END_RULE
    ;

astop.stmt
    : { astop = 0; }
    | T_STOP_LEX { astop = 1; }
    ;

nstring
    : variable.name 
    {
       var1 = variable_lookup(iname);
       if (var1 != NULL)
       {
          strncpy(us_token, var1->str_val, 255);
	  if (var1->type == VAR_NUM) 
	  {
	     yyerror("Variable ${%s} has numeric type.", iname);
	     YYERROR;
	  }
	  if (var1->type == VAR_LST)
	  {
	     yyerror("Variable ${%s} has list type.", iname);
	     YYERROR;
	  }
       }
       else
       {
          yyerror("Variable ${%s} not defined.", iname);
	  strncpy(us_token, "", 255);
       }
    }
    | T_STRING
    ;

ndigits
    : variable.name 
    {
       var1 = variable_lookup(iname);
       if (var1 != NULL)
       {
          strncpy(us_token, var1->str_val, 255);
	  if (var1->type == VAR_STR) 
	  {
	     yyerror("Fatal error. Variable ${%s} has string type.", iname);
	     
	     YYERROR;
	  }
	  
	  if (var1->type == VAR_LST)
	  {
	     yyerror("Variable ${%s} has list type. Use @%s instead", iname, iname);
	     
	     YYERROR;
	  }
       }
       else
       {
          yyerror("Variable ${%s} not defined.", iname);
	  strncpy(us_token, "", 255);
       }
    }
    | T_DIGITS
    ;

variable.name
    : '$' T_IDENT
    ;

list.name
    : '@' T_IDENT
    ;

using.stmt
    : 
    | T_USING_LEX
    ;

%%

static void parser_cleanup() 
{
   fclose(yyin);
   fclose(yyout);
   include_stack_ptr = 0;
}

int mylexer_init(char *filename2)
{

   if (filename2 == NULL)
   {
      LOG_SYS(0, ("Config filename string is empty.\n"));
      return(-1);
   }
   
   yyin  = fopen(filename2, "r");
   
   if (yyin == NULL) 
   {
      LOG_SYS(0, ("Can't open config file (%s).\n", filename2));
      return (-1);
   }
   
   yyout = fopen("/dev/null", "w");
   strncpy(filename, filename2, 255);
   return(0);
}

void parse_config_file(char *filename2, int file_type)
{
   if (mylexer_init(filename2) != -1)
   {
      if (file_type != CONFIG_TYPE_AIM) 
      {
         config_file_type = CONFIG_TYPE_ACTIONS;
	 init_parse_tree();
      }
      else
      {
         config_file_type = CONFIG_TYPE_AIM;
	 init_aim_tree();
      }
      
      yyparse();
      parser_cleanup();
   }
}

