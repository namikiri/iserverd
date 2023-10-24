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

#ifndef BISON_Y_TAB_H
# define BISON_Y_TAB_H

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




#ifndef YYSTYPE
typedef int yystype;
# define YYSTYPE yystype
#endif

extern YYSTYPE yylval;


#endif /* not BISON_Y_TAB_H */

