/* A Bison parser, made from config/actions/actions.y, by GNU bison 1.75.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Written by Richard Stallman by simplifying the original so called
   ``semantic'' parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON	1

/* Pure parsers.  */
#define YYPURE	0

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     T_END_RULE = 258,
     T_MIN_OP = 259,
     T_PLS_OP = 260,
     T_LOG_LEX = 261,
     T_VAR_LEX = 262,
     T_EVENT_LEX = 263,
     T_ONLINE_LEX = 264,
     T_FOR_LEX = 265,
     T_MESSAGE_LEX = 266,
     T_STOP_LEX = 267,
     T_MAIL_LEX = 268,
     T_RUN_LEX = 269,
     T_WITH_LEX = 270,
     T_OFFLINE_LEX = 271,
     T_SAVEBASIC_LEX = 272,
     T_SEARCH_LEX = 273,
     T_STATUS_LEX = 274,
     T_ANY_LEX = 275,
     T_REGISTRATION_LEX = 276,
     T_IDENT = 277,
     T_DIGITS = 278,
     T_WRONG_IDENT = 279,
     T_STRING = 280,
     T_NONE = 281,
     T_USING_LEX = 282,
     T_S_ONLINE = 283,
     T_S_OFFLINE = 284,
     T_S_NA = 285,
     T_S_AWAY = 286,
     T_S_DND = 287,
     T_S_PRIVACY = 288,
     T_S_FFCH = 289,
     T_S_OCCUPIED = 290,
     T_RDBMS_FAIL_LEX = 291,
     T_INTERNAL_ERR_LEX = 292,
     T_PPHUNG_LEX = 293,
     T_RDBMS_ERR_LEX = 294,
     T_RATE_LEX = 295,
     T_CLASS_LEX = 296,
     T_SNAC_LEX = 297,
     T_FAMILY_LEX = 298,
     T_INDEX_LEX = 299,
     T_VERSION_LEX = 300,
     T_SUBTYPE_LEX = 301,
     T_DIGITS16 = 302,
     T_NUMBER_LEX = 303,
     T_END_RULE1 = 304,
     T_CLASSES_LEX = 305
   };
#endif
#define T_END_RULE 258
#define T_MIN_OP 259
#define T_PLS_OP 260
#define T_LOG_LEX 261
#define T_VAR_LEX 262
#define T_EVENT_LEX 263
#define T_ONLINE_LEX 264
#define T_FOR_LEX 265
#define T_MESSAGE_LEX 266
#define T_STOP_LEX 267
#define T_MAIL_LEX 268
#define T_RUN_LEX 269
#define T_WITH_LEX 270
#define T_OFFLINE_LEX 271
#define T_SAVEBASIC_LEX 272
#define T_SEARCH_LEX 273
#define T_STATUS_LEX 274
#define T_ANY_LEX 275
#define T_REGISTRATION_LEX 276
#define T_IDENT 277
#define T_DIGITS 278
#define T_WRONG_IDENT 279
#define T_STRING 280
#define T_NONE 281
#define T_USING_LEX 282
#define T_S_ONLINE 283
#define T_S_OFFLINE 284
#define T_S_NA 285
#define T_S_AWAY 286
#define T_S_DND 287
#define T_S_PRIVACY 288
#define T_S_FFCH 289
#define T_S_OCCUPIED 290
#define T_RDBMS_FAIL_LEX 291
#define T_INTERNAL_ERR_LEX 292
#define T_PPHUNG_LEX 293
#define T_RDBMS_ERR_LEX 294
#define T_RATE_LEX 295
#define T_CLASS_LEX 296
#define T_SNAC_LEX 297
#define T_FAMILY_LEX 298
#define T_INDEX_LEX 299
#define T_VERSION_LEX 300
#define T_SUBTYPE_LEX 301
#define T_DIGITS16 302
#define T_NUMBER_LEX 303
#define T_END_RULE1 304
#define T_CLASSES_LEX 305




/* Copy the first part of user declarations.  */
#line 1 "config/actions/actions.y"


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



/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

#ifndef YYSTYPE
typedef int yystype;
# define YYSTYPE yystype
# define YYSTYPE_IS_TRIVIAL 1
#endif

#ifndef YYLTYPE
typedef struct yyltype
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} yyltype;
# define YYLTYPE yyltype
# define YYLTYPE_IS_TRIVIAL 1
#endif

/* Copy the second part of user declarations.  */


/* Line 213 of /usr/local/share/bison/yacc.c.  */
#line 223 "y.tab.c"

#if ! defined (yyoverflow) || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# if YYSTACK_USE_ALLOCA
#  define YYSTACK_ALLOC alloca
# else
#  ifndef YYSTACK_USE_ALLOCA
#   if defined (alloca) || defined (_ALLOCA_H)
#    define YYSTACK_ALLOC alloca
#   else
#    ifdef __GNUC__
#     define YYSTACK_ALLOC __builtin_alloca
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#  if defined (__STDC__) || defined (__cplusplus)
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   define YYSIZE_T size_t
#  endif
#  define YYSTACK_ALLOC malloc
#  define YYSTACK_FREE free
# endif
#endif /* ! defined (yyoverflow) || YYERROR_VERBOSE */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (YYLTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAX (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE))				\
      + YYSTACK_GAP_MAX)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  register YYSIZE_T yyi;		\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];	\
	}					\
      while (0)
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAX;	\
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif

#if defined (__STDC__) || defined (__cplusplus)
   typedef signed char yysigned_char;
#else
   typedef short yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  42
#define YYLAST   303

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  59
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  98
/* YYNRULES -- Number of rules. */
#define YYNRULES  174
/* YYNRULES -- Number of states. */
#define YYNSTATES  319

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   305

#define YYTRANSLATE(X) \
  ((unsigned)(X) <= YYMAXUTOK ? yytranslate[X] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,    54,     2,     2,     2,
       2,     2,     2,     2,    57,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    51,     2,     2,    58,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    55,     2,    56,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    52,     2,    53,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     5,     7,     9,    12,    14,    16,    18,
      21,    27,    28,    35,    39,    42,    44,    47,    48,    49,
      56,    57,    63,    67,    70,    72,    75,    80,    85,    87,
      88,    94,    98,   100,   103,   107,   109,   111,   113,   116,
     118,   120,   122,   124,   126,   128,   130,   132,   134,   136,
     138,   140,   143,   144,   150,   151,   157,   158,   167,   168,
     174,   175,   181,   182,   188,   189,   195,   196,   202,   203,
     209,   210,   216,   217,   223,   224,   230,   231,   237,   238,
     244,   248,   251,   255,   258,   262,   265,   267,   270,   272,
     275,   277,   280,   281,   293,   294,   304,   312,   313,   314,
     327,   328,   338,   340,   341,   351,   352,   360,   366,   367,
     368,   379,   380,   388,   390,   391,   399,   400,   406,   409,
     410,   411,   420,   421,   427,   429,   432,   434,   436,   438,
     439,   442,   443,   446,   448,   451,   452,   455,   458,   459,
     462,   463,   466,   468,   471,   475,   479,   483,   485,   489,
     491,   495,   497,   501,   502,   507,   509,   511,   513,   515,
     517,   519,   521,   523,   525,   527,   528,   530,   532,   534,
     536,   538,   541,   544,   545
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
      60,     0,    -1,    82,    -1,    61,    -1,    62,    -1,    61,
      62,    -1,    63,    -1,    64,    -1,    71,    -1,     1,    49,
      -1,     7,    22,    51,    25,    49,    -1,    -1,    40,    41,
      44,    23,    65,    66,    -1,    52,    67,    53,    -1,    52,
      53,    -1,    68,    -1,    67,    68,    -1,    -1,    -1,    22,
      69,    51,    81,    70,    49,    -1,    -1,    42,    43,    81,
      72,    73,    -1,    52,    74,    53,    -1,    52,    53,    -1,
      75,    -1,    74,    75,    -1,    45,    51,    81,    49,    -1,
      22,    51,    81,    49,    -1,    76,    -1,    -1,    40,    41,
      23,    77,    78,    -1,    52,    79,    53,    -1,    80,    -1,
      79,    80,    -1,    46,    81,    49,    -1,    23,    -1,    47,
      -1,    83,    -1,    82,    83,    -1,    84,    -1,    89,    -1,
      91,    -1,    93,    -1,    95,    -1,    97,    -1,    99,    -1,
     101,    -1,   103,    -1,   105,    -1,   107,    -1,   150,    -1,
       1,     3,    -1,    -1,     7,    22,    23,    85,     3,    -1,
      -1,     7,    22,    25,    86,     3,    -1,    -1,     7,    22,
      87,    54,    52,    22,    53,     3,    -1,    -1,     7,    22,
      88,   138,     3,    -1,    -1,     8,    38,    90,   111,     3,
      -1,    -1,     8,    36,    92,   111,     3,    -1,    -1,     8,
      39,    94,   111,     3,    -1,    -1,     8,    37,    96,   111,
       3,    -1,    -1,     8,     9,    98,   109,     3,    -1,    -1,
       8,    16,   100,   109,     3,    -1,    -1,     8,    17,   102,
     109,     3,    -1,    -1,     8,    18,   104,   109,     3,    -1,
      -1,     8,    19,   106,   110,     3,    -1,    -1,     8,    21,
     108,   111,     3,    -1,    52,   113,    53,    -1,    52,    53,
      -1,    52,   112,    53,    -1,    52,    53,    -1,    52,   114,
      53,    -1,    52,    53,    -1,   115,    -1,   112,   115,    -1,
     121,    -1,   113,   121,    -1,   127,    -1,   114,   127,    -1,
      -1,    10,   133,    15,   149,    11,   134,   156,   152,   116,
     151,     3,    -1,    -1,    10,   133,    15,   149,     6,   152,
     117,   151,     3,    -1,    10,   133,    15,   149,    12,   151,
       3,    -1,    -1,    -1,    10,   133,    15,   149,    13,   152,
     118,   156,   152,   119,   151,     3,    -1,    -1,    10,   133,
      15,   149,    14,   152,   120,   151,     3,    -1,   121,    -1,
      -1,    10,   133,    11,   134,   156,   152,   122,   151,     3,
      -1,    -1,    10,   133,     6,   152,   123,   151,     3,    -1,
      10,   133,    12,   151,     3,    -1,    -1,    -1,    10,   133,
      13,   152,   124,   156,   152,   125,   151,     3,    -1,    -1,
      10,   133,    14,   152,   126,   151,     3,    -1,   127,    -1,
      -1,    11,   134,   156,   152,   128,   151,     3,    -1,    -1,
       6,   152,   129,   151,     3,    -1,    12,     3,    -1,    -1,
      -1,    13,   152,   130,   156,   152,   131,   151,     3,    -1,
      -1,    14,   152,   132,   151,     3,    -1,   150,    -1,     1,
       3,    -1,   135,    -1,   140,    -1,    20,    -1,    -1,   136,
     153,    -1,    -1,   137,   143,    -1,   155,    -1,     1,     3,
      -1,    -1,   139,   143,    -1,     1,     3,    -1,    -1,   141,
     153,    -1,    -1,   142,   144,    -1,   155,    -1,     1,     3,
      -1,    55,   145,    56,    -1,    55,    20,    56,    -1,    55,
     146,    56,    -1,   153,    -1,   145,    57,   153,    -1,   147,
      -1,   145,    57,   147,    -1,   153,    -1,   146,    57,   153,
      -1,    -1,   153,   148,     4,   153,    -1,    28,    -1,    29,
      -1,    30,    -1,    31,    -1,    32,    -1,    33,    -1,    34,
      -1,    35,    -1,    20,    -1,     3,    -1,    -1,    12,    -1,
     154,    -1,    25,    -1,   154,    -1,    23,    -1,    54,    22,
      -1,    58,    22,    -1,    -1,    27,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,    46,    46,    47,    54,    55,    59,    60,    61,    62,
      66,    73,    73,    77,    78,    82,    83,    87,    87,    87,
      94,    94,    98,    99,   103,   104,   108,   109,   110,   114,
     114,   118,   122,   123,   127,   134,   135,   142,   143,   147,
     148,   149,   150,   151,   152,   153,   154,   155,   156,   157,
     158,   159,   165,   165,   170,   170,   175,   175,   187,   187,
     194,   194,   198,   198,   202,   202,   206,   206,   210,   210,
     214,   214,   218,   218,   222,   222,   226,   226,   230,   230,
     234,   235,   239,   240,   244,   245,   249,   250,   254,   255,
     259,   260,   264,   264,   268,   268,   272,   276,   276,   276,
     280,   280,   284,   288,   288,   292,   292,   296,   300,   300,
     300,   304,   304,   308,   312,   312,   316,   316,   320,   324,
     324,   324,   328,   328,   332,   333,   337,   341,   345,   346,
     346,   351,   351,   352,   368,   378,   378,   379,   389,   389,
     394,   394,   395,   411,   421,   422,   430,   434,   439,   444,
     445,   449,   454,   462,   462,   473,   474,   475,   476,   477,
     478,   479,   480,   481,   485,   489,   490,   494,   517,   521,
     547,   551,   555,   559,   560
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "T_END_RULE", "T_MIN_OP", "T_PLS_OP", 
  "T_LOG_LEX", "T_VAR_LEX", "T_EVENT_LEX", "T_ONLINE_LEX", "T_FOR_LEX", 
  "T_MESSAGE_LEX", "T_STOP_LEX", "T_MAIL_LEX", "T_RUN_LEX", "T_WITH_LEX", 
  "T_OFFLINE_LEX", "T_SAVEBASIC_LEX", "T_SEARCH_LEX", "T_STATUS_LEX", 
  "T_ANY_LEX", "T_REGISTRATION_LEX", "T_IDENT", "T_DIGITS", 
  "T_WRONG_IDENT", "T_STRING", "T_NONE", "T_USING_LEX", "T_S_ONLINE", 
  "T_S_OFFLINE", "T_S_NA", "T_S_AWAY", "T_S_DND", "T_S_PRIVACY", 
  "T_S_FFCH", "T_S_OCCUPIED", "T_RDBMS_FAIL_LEX", "T_INTERNAL_ERR_LEX", 
  "T_PPHUNG_LEX", "T_RDBMS_ERR_LEX", "T_RATE_LEX", "T_CLASS_LEX", 
  "T_SNAC_LEX", "T_FAMILY_LEX", "T_INDEX_LEX", "T_VERSION_LEX", 
  "T_SUBTYPE_LEX", "T_DIGITS16", "T_NUMBER_LEX", "T_END_RULE1", 
  "T_CLASSES_LEX", "'='", "'{'", "'}'", "'$'", "'['", "']'", "','", "'@'", 
  "$accept", "config.file", "aim_proto.conf", "aim_statement", 
  "aim.variable.stmt", "rate.class.idx.stmt", "@1", "class.index.body", 
  "class.index.stmt.list", "class.index.stmt", "@2", "@3", 
  "snac.family.stmt", "@4", "snac.family.body", 
  "snac.family.body.stmt.list", "snac.family.body.stmt", 
  "rate.class.with.subtypes", "@5", "rate.class.with.subtypes.body", 
  "rate.class.with.subtypes.stmt.list", "rate.class.with.subtypes.stmt", 
  "global.number", "actions.conf", "statement", 
  "variable.declaration.stmt", "@6", "@7", "@8", "@9", 
  "event.pphung.section", "@10", "event.rdbms_fail.section", "@11", 
  "event.rdbms_error.section", "@12", "event.int_err.section", "@13", 
  "event.online.section", "@14", "event.offline.section", "@15", 
  "event.savebasic.section", "@16", "event.search.section", "@17", 
  "event.status.section", "@18", "event.registration.section", "@19", 
  "section1.body", "section2.body", "section3.body", 
  "event.commands2.stmt.list", "event.commands.stmt.list", 
  "event.special.commands.stmt.list", "event.commands2.stmt", "@20", 
  "@21", "@22", "@23", "@24", "event.commands.stmt", "@25", "@26", "@27", 
  "@28", "@29", "event.special.commands.stmt", "@30", "@31", "@32", "@33", 
  "@34", "uin.section.n1", "uin.section.n2", "uin.section.wrap", "@35", 
  "@36", "uin.section.wrap2", "@37", "uin.section.wrap3", "@38", "@39", 
  "uin.list", "uin.list2", "uin.list.internal", "uin.list.internal2", 
  "uin.range", "@40", "status.vals", "empty.stmt", "astop.stmt", 
  "nstring", "ndigits", "variable.name", "list.name", "using.stmt", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,    61,   123,   125,    36,    91,    93,    44,    64
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    59,    60,    60,    61,    61,    62,    62,    62,    62,
      63,    65,    64,    66,    66,    67,    67,    69,    70,    68,
      72,    71,    73,    73,    74,    74,    75,    75,    75,    77,
      76,    78,    79,    79,    80,    81,    81,    82,    82,    83,
      83,    83,    83,    83,    83,    83,    83,    83,    83,    83,
      83,    83,    85,    84,    86,    84,    87,    84,    88,    84,
      90,    89,    92,    91,    94,    93,    96,    95,    98,    97,
     100,    99,   102,   101,   104,   103,   106,   105,   108,   107,
     109,   109,   110,   110,   111,   111,   112,   112,   113,   113,
     114,   114,   116,   115,   117,   115,   115,   118,   119,   115,
     120,   115,   115,   122,   121,   123,   121,   121,   124,   125,
     121,   126,   121,   121,   128,   127,   129,   127,   127,   130,
     131,   127,   132,   127,   127,   127,   133,   134,   135,   136,
     135,   137,   135,   135,   135,   139,   138,   138,   141,   140,
     142,   140,   140,   140,   143,   143,   144,   145,   145,   145,
     145,   146,   146,   148,   147,   149,   149,   149,   149,   149,
     149,   149,   149,   149,   150,   151,   151,   152,   152,   153,
     153,   154,   155,   156,   156
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     1,     1,     2,     1,     1,     1,     2,
       5,     0,     6,     3,     2,     1,     2,     0,     0,     6,
       0,     5,     3,     2,     1,     2,     4,     4,     1,     0,
       5,     3,     1,     2,     3,     1,     1,     1,     2,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     2,     0,     5,     0,     5,     0,     8,     0,     5,
       0,     5,     0,     5,     0,     5,     0,     5,     0,     5,
       0,     5,     0,     5,     0,     5,     0,     5,     0,     5,
       3,     2,     3,     2,     3,     2,     1,     2,     1,     2,
       1,     2,     0,    11,     0,     9,     7,     0,     0,    12,
       0,     9,     1,     0,     9,     0,     7,     5,     0,     0,
      10,     0,     7,     1,     0,     7,     0,     5,     2,     0,
       0,     8,     0,     5,     1,     2,     1,     1,     1,     0,
       2,     0,     2,     1,     2,     0,     2,     2,     0,     2,
       0,     2,     1,     2,     3,     3,     3,     1,     3,     1,
       3,     1,     3,     0,     4,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     0,     1,     1,     1,     1,
       1,     2,     2,     0,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       0,     0,   164,     0,     0,     0,     0,     0,     0,     4,
       6,     7,     8,     0,    37,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,     9,    58,
      68,    70,    72,    74,    76,    78,    62,    66,    60,    64,
       0,     0,     1,     0,     0,     5,     0,     0,    38,    52,
      54,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    35,    36,    20,     0,    58,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,     0,    53,    55,    10,     0,   137,    59,     0,   136,
       0,     0,     0,     0,     0,     0,     0,    81,     0,    88,
     113,   124,    69,    71,    73,    75,     0,    83,     0,    86,
     102,    77,    85,     0,    90,    79,    63,    67,    61,    65,
       0,     0,    21,     0,     0,   170,     0,     0,   149,   147,
     169,   125,   168,   116,   167,     0,   128,     0,     0,   126,
       0,     0,   133,     0,   173,   127,     0,     0,   142,   118,
     119,   122,    80,    89,     0,    82,    87,    84,    91,     0,
      12,     0,     0,     0,    23,     0,    24,    28,     0,   145,
     171,   144,     0,     0,   165,   134,   172,     0,     0,   165,
       0,     0,   130,   132,   143,   174,     0,   139,     0,   141,
     173,   165,     0,    17,    14,     0,    15,     0,     0,     0,
      22,    25,    57,   150,   148,     0,   166,     0,   105,   173,
       0,   108,   111,   114,     0,   151,     0,     0,   163,   155,
     156,   157,   158,   159,   160,   161,   162,     0,     0,    13,
      16,     0,    29,     0,   154,   117,   165,     0,   107,   173,
     165,   165,   146,     0,   120,   123,     0,     0,   165,     0,
       0,     0,    27,     0,    26,     0,   103,     0,     0,     0,
     152,   165,    94,   173,     0,    97,   100,    18,     0,    30,
     106,   165,   109,   112,   115,     0,   165,     0,    96,   173,
     165,     0,     0,     0,    32,     0,   165,   121,     0,    92,
       0,     0,    19,     0,    31,    33,   104,     0,    95,   165,
      98,   101,    34,   110,     0,   165,    93,     0,    99
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,     7,     8,     9,    10,    11,   130,   170,   205,   206,
     238,   291,    12,    91,   132,   175,   176,   177,   263,   279,
     293,   294,    67,    13,    14,    15,    70,    71,    52,    53,
      16,    62,    17,    60,    18,    63,    19,    61,    20,    54,
      21,    55,    22,    56,    23,    57,    24,    58,    25,    59,
      78,    83,    85,   118,   108,   123,   119,   309,   286,   289,
     315,   290,   120,   281,   246,   249,   296,   250,   110,   251,
     184,   200,   271,   201,   148,   154,   149,   150,   151,    75,
      76,   155,   156,   157,    99,   199,   137,   224,   138,   183,
     237,   111,   217,   143,   139,   144,   158,   196
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -199
static const short yypact[] =
{
      89,     7,  -199,    59,   201,    -3,    41,    95,    58,  -199,
    -199,  -199,  -199,   149,  -199,  -199,  -199,  -199,  -199,  -199,
    -199,  -199,  -199,  -199,  -199,  -199,  -199,  -199,  -199,    -8,
    -199,  -199,  -199,  -199,  -199,  -199,  -199,  -199,  -199,  -199,
      57,    55,  -199,    61,    90,  -199,   108,    94,  -199,  -199,
    -199,    92,    74,     5,    78,    78,    78,    78,    80,    83,
      83,    83,    83,    83,   114,  -199,  -199,  -199,    97,    84,
     137,   138,   104,   111,   157,   163,   122,   112,   164,   176,
     178,   181,   133,   182,   191,   190,   196,   197,   209,   210,
    -199,   162,  -199,  -199,  -199,   193,  -199,  -199,    -7,  -199,
     213,    16,    17,    25,   218,    16,    16,  -199,   158,  -199,
    -199,  -199,  -199,  -199,  -199,  -199,    17,  -199,   177,  -199,
    -199,  -199,  -199,   195,  -199,  -199,  -199,  -199,  -199,  -199,
     171,    -1,  -199,   172,   168,  -199,   205,   -32,  -199,   224,
    -199,  -199,  -199,  -199,  -199,   226,  -199,   214,   251,  -199,
      -9,   122,  -199,   239,   216,  -199,    -9,   192,  -199,  -199,
    -199,  -199,  -199,  -199,   220,  -199,  -199,  -199,  -199,    13,
    -199,   194,   217,   208,  -199,     9,  -199,  -199,   243,  -199,
    -199,  -199,    -9,   256,   258,  -199,  -199,    16,    25,   258,
      16,    16,  -199,  -199,  -199,  -199,    16,  -199,    -9,  -199,
     216,   258,   221,  -199,  -199,    20,  -199,    55,   248,    55,
    -199,  -199,  -199,  -199,   224,    -9,  -199,   269,  -199,   216,
     270,  -199,  -199,  -199,   -24,  -199,    16,   271,  -199,  -199,
    -199,  -199,  -199,  -199,  -199,  -199,  -199,   255,   225,  -199,
    -199,   228,  -199,   229,  -199,  -199,   258,    16,  -199,   216,
     258,   258,  -199,    -9,  -199,  -199,    16,    25,   258,    16,
      16,    55,  -199,   223,  -199,   276,  -199,    16,   277,   278,
    -199,   258,  -199,   216,   279,  -199,  -199,  -199,   237,  -199,
    -199,   258,  -199,  -199,  -199,   281,   258,    16,  -199,   216,
     258,   236,    55,   -26,  -199,   283,   258,  -199,   284,  -199,
      16,   285,  -199,   240,  -199,  -199,  -199,   287,  -199,   258,
    -199,  -199,  -199,  -199,   288,   258,  -199,   289,  -199
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -199,  -199,  -199,   286,  -199,  -199,  -199,  -199,  -199,    88,
    -199,  -199,  -199,  -199,  -199,  -199,   120,  -199,  -199,  -199,
    -199,     3,  -198,  -199,   290,  -199,  -199,  -199,  -199,  -199,
    -199,  -199,  -199,  -199,  -199,  -199,  -199,  -199,  -199,  -199,
    -199,  -199,  -199,  -199,  -199,  -199,  -199,  -199,  -199,  -199,
     -27,  -199,   113,  -199,  -199,  -199,   179,  -199,  -199,  -199,
    -199,  -199,   -20,  -199,  -199,  -199,  -199,  -199,   -72,  -199,
    -199,  -199,  -199,  -199,   183,  -183,  -199,  -199,  -199,  -199,
    -199,  -199,  -199,  -199,   147,  -199,  -199,  -199,   118,  -199,
    -199,   106,  -182,  -105,  -148,   -95,   -80,  -196
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, parse error.  */
#define YYTABLE_NINF -154
static const short yytable[] =
{
     160,   161,   192,   140,   226,   219,    74,   220,   197,   241,
      27,   243,   124,   134,   135,    49,   135,    50,   145,   227,
     292,   171,   152,   247,   181,   182,   153,   304,    79,    80,
      81,   171,   252,   253,   214,   203,   152,   146,    40,   172,
    -129,   142,   203,    51,   173,   136,   -56,   136,  -138,   172,
     225,   168,   174,   267,   173,   140,    28,   109,    -3,    43,
    -135,   140,   210,   277,   265,    44,   204,   244,   268,   269,
     136,  -129,  -131,   239,   273,   147,   274,   287,    65,  -138,
    -140,    29,   218,   147,    41,   221,   222,   140,   163,   285,
       1,   223,     2,   300,   303,    42,     3,     4,     5,   295,
       6,    64,    66,   140,   298,   270,    26,    49,   301,    50,
      28,    27,    68,   100,   307,     2,    69,    72,   101,    26,
     140,   254,   102,   103,   104,   105,   106,   314,    73,     5,
      77,     6,    82,   317,   100,    84,     2,    90,   -56,   101,
      92,    93,   266,   116,   103,   104,   105,   106,    51,    -2,
      46,   272,     2,    94,   275,   276,    47,     4,   140,   100,
      96,     2,   282,    95,   101,   107,    97,   112,   102,   103,
     104,   105,   106,    86,    87,    88,    89,    98,   100,   113,
       2,   114,   299,   101,   115,   121,   117,   116,   103,   104,
     105,   106,   100,   125,     2,   310,   100,   101,     2,   126,
     127,   101,   103,   104,   105,   106,   103,   104,   105,   106,
      30,   162,   128,   129,   131,   133,   141,    31,    32,    33,
      34,   159,    35,   169,   179,   178,   187,   180,  -153,   185,
     165,   188,   189,   190,   191,   202,   186,    36,    37,    38,
      39,   228,   194,   195,   122,   207,   212,   198,   167,   229,
     230,   231,   232,   233,   234,   235,   236,   187,   208,   209,
     215,   256,   188,   189,   190,   191,   257,   258,   259,   260,
     216,   242,   245,   248,   255,   278,   261,   262,   264,   280,
     283,   284,   288,   292,   297,   302,   306,   308,   311,   312,
     313,   316,   318,   240,    45,   211,   305,   166,   193,   164,
     213,     0,     0,    48
};

static const short yycheck[] =
{
     105,   106,   150,    98,   200,   188,     1,   189,   156,   207,
       3,   209,    84,    20,    23,    23,    23,    25,     1,   201,
      46,    22,   102,   219,    56,    57,     1,    53,    55,    56,
      57,    22,    56,    57,   182,    22,   116,    20,    41,    40,
      23,    25,    22,    51,    45,    54,    54,    54,    23,    40,
     198,   123,    53,   249,    45,   150,    49,    77,     0,     1,
      55,   156,    53,   261,   246,     7,    53,   215,   250,   251,
      54,    54,    55,    53,   257,    58,   258,   273,    23,    54,
      55,    22,   187,    58,    43,   190,   191,   182,   108,   271,
       1,   196,     3,   289,   292,     0,     7,     8,    40,   281,
      42,    44,    47,   198,   286,   253,     0,    23,   290,    25,
      49,     3,    22,     1,   296,     3,    22,    25,     6,    13,
     215,   226,    10,    11,    12,    13,    14,   309,    54,    40,
      52,    42,    52,   315,     1,    52,     3,    23,    54,     6,
       3,     3,   247,    10,    11,    12,    13,    14,    51,     0,
       1,   256,     3,    49,   259,   260,     7,     8,   253,     1,
       3,     3,   267,    52,     6,    53,     3,     3,    10,    11,
      12,    13,    14,    60,    61,    62,    63,    55,     1,     3,
       3,     3,   287,     6,     3,     3,    53,    10,    11,    12,
      13,    14,     1,     3,     3,   300,     1,     6,     3,     3,
       3,     6,    11,    12,    13,    14,    11,    12,    13,    14,
       9,    53,     3,     3,    52,    22,     3,    16,    17,    18,
      19,     3,    21,    52,    56,    53,     6,    22,     4,     3,
      53,    11,    12,    13,    14,    15,    22,    36,    37,    38,
      39,    20,     3,    27,    53,    51,     3,    55,    53,    28,
      29,    30,    31,    32,    33,    34,    35,     6,    41,    51,
       4,     6,    11,    12,    13,    14,    11,    12,    13,    14,
      12,    23,     3,     3,     3,    52,    51,    49,    49,     3,
       3,     3,     3,    46,     3,    49,     3,     3,     3,    49,
       3,     3,     3,   205,     8,   175,   293,   118,   151,   116,
     182,    -1,    -1,    13
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,     1,     3,     7,     8,    40,    42,    60,    61,    62,
      63,    64,    71,    82,    83,    84,    89,    91,    93,    95,
      97,    99,   101,   103,   105,   107,   150,     3,    49,    22,
       9,    16,    17,    18,    19,    21,    36,    37,    38,    39,
      41,    43,     0,     1,     7,    62,     1,     7,    83,    23,
      25,    51,    87,    88,    98,   100,   102,   104,   106,   108,
      92,    96,    90,    94,    44,    23,    47,    81,    22,    22,
      85,    86,    25,    54,     1,   138,   139,    52,   109,   109,
     109,   109,    52,   110,    52,   111,   111,   111,   111,   111,
      23,    72,     3,     3,    49,    52,     3,     3,    55,   143,
       1,     6,    10,    11,    12,    13,    14,    53,   113,   121,
     127,   150,     3,     3,     3,     3,    10,    53,   112,   115,
     121,     3,    53,   114,   127,     3,     3,     3,     3,     3,
      65,    52,    73,    22,    20,    23,    54,   145,   147,   153,
     154,     3,    25,   152,   154,     1,    20,    58,   133,   135,
     136,   137,   155,     1,   134,   140,   141,   142,   155,     3,
     152,   152,    53,   121,   133,    53,   115,    53,   127,    52,
      66,    22,    40,    45,    53,    74,    75,    76,    53,    56,
      22,    56,    57,   148,   129,     3,    22,     6,    11,    12,
      13,    14,   153,   143,     3,    27,   156,   153,    55,   144,
     130,   132,    15,    22,    53,    67,    68,    51,    41,    51,
      53,    75,     3,   147,   153,     4,    12,   151,   152,   134,
     151,   152,   152,   152,   146,   153,   156,   151,    20,    28,
      29,    30,    31,    32,    33,    34,    35,   149,    69,    53,
      68,    81,    23,    81,   153,     3,   123,   156,     3,   124,
     126,   128,    56,    57,   152,     3,     6,    11,    12,    13,
      14,    51,    49,    77,    49,   151,   152,   156,   151,   151,
     153,   131,   152,   134,   151,   152,   152,    81,    52,    78,
       3,   122,   152,     3,     3,   151,   117,   156,     3,   118,
     120,    70,    46,    79,    80,   151,   125,     3,   151,   152,
     156,   151,    49,    81,    53,    80,     3,   151,     3,   116,
     152,     3,    49,     3,   151,   119,     3,   151,     3
};

#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# if defined (__STDC__) || defined (__cplusplus)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrlab1

/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror ("syntax error: cannot back up");			\
      YYERROR;							\
    }								\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)           \
  Current.first_line   = Rhs[1].first_line;      \
  Current.first_column = Rhs[1].first_column;    \
  Current.last_line    = Rhs[N].last_line;       \
  Current.last_column  = Rhs[N].last_column;
#endif

/* YYLEX -- calling `yylex' with the right arguments.  */

#define YYLEX	yylex ()

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (0)
# define YYDSYMPRINT(Args)			\
do {						\
  if (yydebug)					\
    yysymprint Args;				\
} while (0)
/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YYDSYMPRINT(Args)
#endif /* !YYDEBUG */

/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#if YYMAXDEPTH == 0
# undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  register const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  register char *yyd = yydest;
  register const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

#endif /* !YYERROR_VERBOSE */



#if YYDEBUG
/*-----------------------------.
| Print this symbol on YYOUT.  |
`-----------------------------*/

static void
#if defined (__STDC__) || defined (__cplusplus)
yysymprint (FILE* yyout, int yytype, YYSTYPE yyvalue)
#else
yysymprint (yyout, yytype, yyvalue)
    FILE* yyout;
    int yytype;
    YYSTYPE yyvalue;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvalue;

  if (yytype < YYNTOKENS)
    {
      YYFPRINTF (yyout, "token %s (", yytname[yytype]);
# ifdef YYPRINT
      YYPRINT (yyout, yytoknum[yytype], yyvalue);
# endif
    }
  else
    YYFPRINTF (yyout, "nterm %s (", yytname[yytype]);

  switch (yytype)
    {
      default:
        break;
    }
  YYFPRINTF (yyout, ")");
}
#endif /* YYDEBUG. */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
#if defined (__STDC__) || defined (__cplusplus)
yydestruct (int yytype, YYSTYPE yyvalue)
#else
yydestruct (yytype, yyvalue)
    int yytype;
    YYSTYPE yyvalue;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvalue;

  switch (yytype)
    {
      default:
        break;
    }
}



/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
#  define YYPARSE_PARAM_ARG void *YYPARSE_PARAM
#  define YYPARSE_PARAM_DECL
# else
#  define YYPARSE_PARAM_ARG YYPARSE_PARAM
#  define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
# endif
#else /* !YYPARSE_PARAM */
# define YYPARSE_PARAM_ARG
# define YYPARSE_PARAM_DECL
#endif /* !YYPARSE_PARAM */

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
# ifdef YYPARSE_PARAM
int yyparse (void *);
# else
int yyparse (void);
# endif
#endif


/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of parse errors so far.  */
int yynerrs;


int
yyparse (YYPARSE_PARAM_ARG)
     YYPARSE_PARAM_DECL
{
  
  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Lookahead token as an internal (translated) token number.  */
  int yychar1 = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  short	yyssa[YYINITDEPTH];
  short *yyss = yyssa;
  register short *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;



#define YYPOPSTACK   (yyvsp--, yyssp--)

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* When reducing, the number of symbols on the RHS of the reduced
     rule.  */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;

      {
	short *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with.  */

  if (yychar <= 0)		/* This means end of input.  */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more.  */

      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yychar1 = YYTRANSLATE (yychar);

      /* We have to keep this `#if YYDEBUG', since we use variables
	 which are defined only if `YYDEBUG' is set.  */
      YYDPRINTF ((stderr, "Next token is "));
      YYDSYMPRINT ((stderr, yychar1, yylval));
      YYDPRINTF ((stderr, "\n"));
    }

  /* If the proper action on seeing token YYCHAR1 is to reduce or to
     detect an error, take that action.  */
  yyn += yychar1;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yychar1)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */
  YYDPRINTF ((stderr, "Shifting token %d (%s), ",
	      yychar, yytname[yychar1]));

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;


  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];



#if YYDEBUG
  /* We have to keep this `#if YYDEBUG', since we use variables which
     are defined only if `YYDEBUG' is set.  */
  if (yydebug)
    {
      int yyi;

      YYFPRINTF (stderr, "Reducing via rule %d (line %d), ",
		 yyn - 1, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (yyi = yyprhs[yyn]; yyrhs[yyi] >= 0; yyi++)
	YYFPRINTF (stderr, "%s ", yytname[yyrhs[yyi]]);
      YYFPRINTF (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif
  switch (yyn)
    {
        case 10:
#line 67 "config/actions/actions.y"
    {
       aim_var2list(aim_create_variable(iname,us_token));
    }
    break;

  case 11:
#line 73 "config/actions/actions.y"
    { rsindex=atoul(us_token); add_rate_class(rsindex); }
    break;

  case 17:
#line 87 "config/actions/actions.y"
    { strncpy(pname, us_token, 254); }
    break;

  case 18:
#line 87 "config/actions/actions.y"
    { pvalue = atoul(us_token); }
    break;

  case 19:
#line 88 "config/actions/actions.y"
    { 
       add_rate_variable(rsindex, pname, pvalue); 
    }
    break;

  case 20:
#line 94 "config/actions/actions.y"
    { family = snumber; add_snac_family(family); }
    break;

  case 26:
#line 108 "config/actions/actions.y"
    { family_set_version(family, snumber); }
    break;

  case 29:
#line 114 "config/actions/actions.y"
    { rsindex = rate_check(atoul(us_token)); }
    break;

  case 34:
#line 128 "config/actions/actions.y"
    {
       add_family_subtype(family, rsindex, snumber);
    }
    break;

  case 35:
#line 134 "config/actions/actions.y"
    { snumber = atoul(us_token);    }
    break;

  case 36:
#line 135 "config/actions/actions.y"
    { snumber = ahextoul(us_token); }
    break;

  case 51:
#line 159 "config/actions/actions.y"
    { yyerrok; }
    break;

  case 52:
#line 165 "config/actions/actions.y"
    {strncpy(par2,us_token,255); }
    break;

  case 53:
#line 166 "config/actions/actions.y"
    {
       uin = atoul(par2);
       add_var2list(create_variableA(iname,par2,uin,VAR_NUM));
    }
    break;

  case 54:
#line 170 "config/actions/actions.y"
    {strncpy(par2,us_token,255); }
    break;

  case 55:
#line 171 "config/actions/actions.y"
    {
       uin = atoul(par2);
       add_var2list(create_variableA(iname,par2,uin,VAR_STR));
    }
    break;

  case 56:
#line 175 "config/actions/actions.y"
    { strncpy(par1, us_token, 255); }
    break;

  case 57:
#line 176 "config/actions/actions.y"
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
    break;

  case 58:
#line 187 "config/actions/actions.y"
    { strncpy(par1, us_token, 255); }
    break;

  case 59:
#line 188 "config/actions/actions.y"
    {
       add_var2list(create_variableB(iname,uins_node));
    }
    break;

  case 60:
#line 194 "config/actions/actions.y"
    { rsection = ACT_PPHUNG; }
    break;

  case 62:
#line 198 "config/actions/actions.y"
    { rsection = ACT_RDBMSFAIL; }
    break;

  case 64:
#line 202 "config/actions/actions.y"
    { rsection = ACT_RDBMSERR; }
    break;

  case 66:
#line 206 "config/actions/actions.y"
    { rsection = ACT_INTERR; }
    break;

  case 68:
#line 210 "config/actions/actions.y"
    { rsection = ACT_ONLINE; }
    break;

  case 70:
#line 214 "config/actions/actions.y"
    { rsection = ACT_OFFLINE; }
    break;

  case 72:
#line 218 "config/actions/actions.y"
    { rsection = ACT_SAVEBASIC; }
    break;

  case 74:
#line 222 "config/actions/actions.y"
    { rsection = ACT_SEARCH; }
    break;

  case 76:
#line 226 "config/actions/actions.y"
    { rsection = ACT_STATUS; }
    break;

  case 78:
#line 230 "config/actions/actions.y"
    { rsection = ACT_REGISTR; }
    break;

  case 92:
#line 264 "config/actions/actions.y"
    { strncpy(par2, us_token, 255); }
    break;

  case 93:
#line 265 "config/actions/actions.y"
    { 
       add_rule_to_list(rsection, A_MSG, NULL, par2, NULL, uins_lst1, uins_lst2, rstatus, astop);
    }
    break;

  case 94:
#line 268 "config/actions/actions.y"
    { strncpy(par2, us_token, 255); }
    break;

  case 95:
#line 269 "config/actions/actions.y"
    { 
       add_rule_to_list(rsection, A_LOG, NULL, par2, NULL, uins_lst1, NULL, rstatus, astop);
    }
    break;

  case 96:
#line 273 "config/actions/actions.y"
    { 
       add_rule_to_list(rsection, A_STOP, NULL, NULL, NULL, uins_lst1, NULL, rstatus, astop);
    }
    break;

  case 97:
#line 276 "config/actions/actions.y"
    { strncpy(par1, us_token, 255); }
    break;

  case 98:
#line 276 "config/actions/actions.y"
    { strncpy(par2, us_token, 255); }
    break;

  case 99:
#line 277 "config/actions/actions.y"
    { 
       add_rule_to_list(rsection, A_MAIL, par1, par2, NULL, uins_lst1, NULL, rstatus, astop);
    }
    break;

  case 100:
#line 280 "config/actions/actions.y"
    { strncpy(par1, us_token, 255); }
    break;

  case 101:
#line 281 "config/actions/actions.y"
    { 
       add_rule_to_list(rsection, A_RUN, par1, par2, NULL, uins_lst1, NULL, rstatus, astop);
    }
    break;

  case 103:
#line 288 "config/actions/actions.y"
    { strncpy(par2, us_token, 255); }
    break;

  case 104:
#line 289 "config/actions/actions.y"
    { 
       add_rule_to_list(rsection, A_MSG, NULL, par2, NULL, uins_lst1, uins_lst2, 0, astop);
    }
    break;

  case 105:
#line 292 "config/actions/actions.y"
    { strncpy(par2, us_token, 255); }
    break;

  case 106:
#line 293 "config/actions/actions.y"
    { 
       add_rule_to_list(rsection, A_LOG, NULL, par2, NULL, uins_lst1, NULL, 0, astop);
    }
    break;

  case 107:
#line 297 "config/actions/actions.y"
    { 
       add_rule_to_list(rsection, A_STOP, NULL, NULL, NULL, uins_lst1, NULL, 0, astop);
    }
    break;

  case 108:
#line 300 "config/actions/actions.y"
    { strncpy(par1, us_token, 255); }
    break;

  case 109:
#line 300 "config/actions/actions.y"
    { strncpy(par2, us_token, 255); }
    break;

  case 110:
#line 301 "config/actions/actions.y"
    { 
       add_rule_to_list(rsection, A_MAIL, par1, par2, NULL, uins_lst1, NULL, 0, astop);
    }
    break;

  case 111:
#line 304 "config/actions/actions.y"
    { strncpy(par1, us_token, 255); }
    break;

  case 112:
#line 305 "config/actions/actions.y"
    { 
       add_rule_to_list(rsection, A_RUN, par1, par2, NULL, uins_lst1, NULL, 0, astop);
    }
    break;

  case 114:
#line 312 "config/actions/actions.y"
    { strncpy(par2, us_token, 255); }
    break;

  case 115:
#line 313 "config/actions/actions.y"
    { 
       add_rule_to_list(rsection, A_MSG, "", par2, NULL, NULL, uins_lst2, 0, astop);
    }
    break;

  case 116:
#line 316 "config/actions/actions.y"
    { strncpy(par2, us_token, 255); }
    break;

  case 117:
#line 317 "config/actions/actions.y"
    { 
       add_rule_to_list(rsection, A_LOG, "", par2, NULL, NULL, NULL, 0, astop);
    }
    break;

  case 118:
#line 321 "config/actions/actions.y"
    { 
       add_rule_to_list(rsection, A_STOP, NULL, NULL, NULL, NULL, NULL, 0, 0);
    }
    break;

  case 119:
#line 324 "config/actions/actions.y"
    { strncpy(par1, us_token, 255); }
    break;

  case 120:
#line 324 "config/actions/actions.y"
    { strncpy(par2, us_token, 255); }
    break;

  case 121:
#line 325 "config/actions/actions.y"
    { 
       add_rule_to_list(rsection, A_MAIL, par1, par2, NULL, NULL, NULL, 0, astop);
    }
    break;

  case 122:
#line 328 "config/actions/actions.y"
    { strncpy(par1, us_token, 255); }
    break;

  case 123:
#line 329 "config/actions/actions.y"
    { 
       add_rule_to_list(rsection, A_RUN, par1, NULL, NULL, NULL, NULL, 0, astop);
    }
    break;

  case 125:
#line 333 "config/actions/actions.y"
    { yyerrok; }
    break;

  case 126:
#line 337 "config/actions/actions.y"
    { uins_lst1 = uins_node;}
    break;

  case 127:
#line 341 "config/actions/actions.y"
    { uins_lst2 = uins_node;}
    break;

  case 128:
#line 345 "config/actions/actions.y"
    { uins_node = NULL; }
    break;

  case 129:
#line 346 "config/actions/actions.y"
    { uins_node = create_uin_node(); }
    break;

  case 130:
#line 347 "config/actions/actions.y"
    { 
       uin = atoul(us_token);
       add_uin2node(uins_node, uin, UIN_NONE);       
    }
    break;

  case 131:
#line 351 "config/actions/actions.y"
    { uins_node = create_uin_node(); }
    break;

  case 133:
#line 353 "config/actions/actions.y"
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
    break;

  case 134:
#line 369 "config/actions/actions.y"
    { 
       delete_uin_node(uins_node); 
       delete_uin_node(uins_lst1); 
       delete_uin_node(uins_lst2); 
       yyerrok; 
    }
    break;

  case 135:
#line 378 "config/actions/actions.y"
    { uins_node = create_uin_node(); }
    break;

  case 137:
#line 380 "config/actions/actions.y"
    { 
       delete_uin_node(uins_node); 
       delete_uin_node(uins_lst1); 
       delete_uin_node(uins_lst2); 
       yyerrok; 
    }
    break;

  case 138:
#line 389 "config/actions/actions.y"
    { uins_node = create_uin_node(); }
    break;

  case 139:
#line 390 "config/actions/actions.y"
    { 
       uin = atoul(us_token);
       add_uin2node(uins_node, uin, UIN_NONE);       
    }
    break;

  case 140:
#line 394 "config/actions/actions.y"
    { uins_node = create_uin_node(); }
    break;

  case 142:
#line 396 "config/actions/actions.y"
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
    break;

  case 143:
#line 412 "config/actions/actions.y"
    { 
       delete_uin_node(uins_node); 
       delete_uin_node(uins_lst1); 
       delete_uin_node(uins_lst2); 
       yyerrok; 
    }
    break;

  case 145:
#line 423 "config/actions/actions.y"
    { 
       delete_uin_node(uins_node); 
       uins_node = NULL; 
    }
    break;

  case 147:
#line 435 "config/actions/actions.y"
    {
       uin = atoul(us_token);
       add_uin2node(uins_node, uin, UIN_NONE);
    }
    break;

  case 148:
#line 440 "config/actions/actions.y"
    {
       uin = atoul(us_token);
       add_uin2node(uins_node, uin, UIN_NONE);
    }
    break;

  case 151:
#line 450 "config/actions/actions.y"
    {
       uin = atoul(us_token);
       add_uin2node(uins_node, uin, UIN_NONE);
    }
    break;

  case 152:
#line 455 "config/actions/actions.y"
    {
       uin = atoul(us_token);
       add_uin2node(uins_node, uin, UIN_NONE);
    }
    break;

  case 153:
#line 462 "config/actions/actions.y"
    { uin1 = atoul(us_token); }
    break;

  case 154:
#line 464 "config/actions/actions.y"
    { 
       uin2 = atoul(us_token);
       if (uin1 > uin2) { uin  = uin1; uin1 = uin2; uin2 = uin; }
       if (uin1 == 0)   { uin1 = 1; }
       add_uin2node(uins_node, uin1, uin2);
    }
    break;

  case 155:
#line 473 "config/actions/actions.y"
    { rstatus = ICQ_STATUS_ONLINE; }
    break;

  case 156:
#line 474 "config/actions/actions.y"
    { rstatus = ICQ_STATUS_OFFLINE; }
    break;

  case 157:
#line 475 "config/actions/actions.y"
    { rstatus = ICQ_STATUS_NA; }
    break;

  case 158:
#line 476 "config/actions/actions.y"
    { rstatus = ICQ_STATUS_AWAY; }
    break;

  case 159:
#line 477 "config/actions/actions.y"
    { rstatus = ICQ_STATUS_DND; }
    break;

  case 160:
#line 478 "config/actions/actions.y"
    { rstatus = ICQ_STATUS_PRIVATE; }
    break;

  case 161:
#line 479 "config/actions/actions.y"
    { rstatus = ICQ_STATUS_FFC; }
    break;

  case 162:
#line 480 "config/actions/actions.y"
    { rstatus = ICQ_STATUS_OCCUPIED; }
    break;

  case 163:
#line 481 "config/actions/actions.y"
    { rstatus = ICQ_STATUS_ANY; }
    break;

  case 165:
#line 489 "config/actions/actions.y"
    { astop = 0; }
    break;

  case 166:
#line 490 "config/actions/actions.y"
    { astop = 1; }
    break;

  case 167:
#line 495 "config/actions/actions.y"
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
    break;

  case 169:
#line 522 "config/actions/actions.y"
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
    break;


    }

/* Line 1016 of /usr/local/share/bison/yacc.c.  */
#line 2008 "y.tab.c"

  yyvsp -= yylen;
  yyssp -= yylen;


#if YYDEBUG
  if (yydebug)
    {
      short *yyssp1 = yyss - 1;
      YYFPRINTF (stderr, "state stack now");
      while (yyssp1 != yyssp)
	YYFPRINTF (stderr, " %d", *++yyssp1);
      YYFPRINTF (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (YYPACT_NINF < yyn && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  int yytype = YYTRANSLATE (yychar);
	  char *yymsg;
	  int yyx, yycount;

	  yycount = 0;
	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  for (yyx = yyn < 0 ? -yyn : 0;
	       yyx < (int) (sizeof (yytname) / sizeof (char *)); yyx++)
	    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	      yysize += yystrlen (yytname[yyx]) + 15, yycount++;
	  yysize += yystrlen ("parse error, unexpected ") + 1;
	  yysize += yystrlen (yytname[yytype]);
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "parse error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[yytype]);

	      if (yycount < 5)
		{
		  yycount = 0;
		  for (yyx = yyn < 0 ? -yyn : 0;
		       yyx < (int) (sizeof (yytname) / sizeof (char *));
		       yyx++)
		    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
		      {
			const char *yyq = ! yycount ? ", expecting " : " or ";
			yyp = yystpcpy (yyp, yyq);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yycount++;
		      }
		}
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exhausted");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror ("parse error");
    }
  goto yyerrlab1;


/*----------------------------------------------------.
| yyerrlab1 -- error raised explicitly by an action.  |
`----------------------------------------------------*/
yyerrlab1:
  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      /* Return failure if at end of input.  */
      if (yychar == YYEOF)
        {
	  /* Pop the error token.  */
          YYPOPSTACK;
	  /* Pop the rest of the stack.  */
	  while (yyssp > yyss)
	    {
	      YYDPRINTF ((stderr, "Error: popping "));
	      YYDSYMPRINT ((stderr,
			    yystos[*yyssp],
			    *yyvsp));
	      YYDPRINTF ((stderr, "\n"));
	      yydestruct (yystos[*yyssp], *yyvsp);
	      YYPOPSTACK;
	    }
	  YYABORT;
        }

      YYDPRINTF ((stderr, "Discarding token %d (%s).\n",
		  yychar, yytname[yychar1]));
      yydestruct (yychar1, yylval);
      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */

  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;

      YYDPRINTF ((stderr, "Error: popping "));
      YYDSYMPRINT ((stderr,
		    yystos[*yyssp], *yyvsp));
      YYDPRINTF ((stderr, "\n"));

      yydestruct (yystos[yystate], *yyvsp);
      yyvsp--;
      yystate = *--yyssp;


#if YYDEBUG
      if (yydebug)
	{
	  short *yyssp1 = yyss - 1;
	  YYFPRINTF (stderr, "Error: state stack now");
	  while (yyssp1 != yyssp)
	    YYFPRINTF (stderr, " %d", *++yyssp1);
	  YYFPRINTF (stderr, "\n");
	}
#endif
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  YYDPRINTF ((stderr, "Shifting error token, "));

  *++yyvsp = yylval;


  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*----------------------------------------------.
| yyoverflowlab -- parser overflow comes here.  |
`----------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}


#line 563 "config/actions/actions.y"


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


