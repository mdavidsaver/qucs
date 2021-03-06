/* -*-c-*- */

%{
/*
 * scan_csv.l - scanner for CSV files
 *
 * Copyright (C) 2007 Stefan Jahn <stefan@lkcc.org>
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this package; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street - Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 * $Id$
 *
 */

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef __MINGW32__
#include <io.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "logging.h"
#include "complex.h"
#include "object.h"
#include "vector.h"
#include "dataset.h"
#include "check_csv.h"
#include "tokens_csv.h"

using namespace qucs;

%}

WS       [ \t\n\r]
SEP      [;,]
ARRAY    \[[0-9]+,[0-9]+\]
ID1      [a-zA-Z_][a-zA-Z0-9_.]*{ARRAY}?
ID2      [^\"\n\r]*
DIGIT    [0-9]
EXPONENT [Ee][+-]?{DIGIT}+
INT      [+-]?{DIGIT}+
FLOAT1   [+-]?{DIGIT}+{EXPONENT}
FLOAT2   [+-]?{DIGIT}*"."{DIGIT}+({EXPONENT})?
SPACE    [ \t]

%x COMMENT IDENT
%option yylineno noyywrap nounput prefix="csv_"

%%

<INITIAL>\"    { BEGIN(IDENT); /* pass the '"' to the parser */ return '"'; }
<INITIAL>\r?\n { /* detect end of line */ return Eol; }

<INITIAL>{ID1} { /* identify identifier */
    csv_lval.ident = strdup (csv_text);
    return Identifier;
  }

<IDENT>{ID2} { /* identify identifier */
    csv_lval.ident = strdup (csv_text);
    return Identifier;
  }

<IDENT>\"    { BEGIN(INITIAL); return '"'; }
<IDENT>\r?\n { BEGIN(INITIAL); return Eol; }

<*>({SPACE}|{SEP}) /* skip spaces and separators */

<INITIAL>({FLOAT1}|{FLOAT2}|{INT}) { /* identify float */
    csv_lval.f = strtod (csv_text, NULL);
    return Float;
  }

<INITIAL>. { /* any other character in invalid */
    logprint (LOG_ERROR,
	      "line %d: syntax error, unrecognized character: `%s'\n",
	      csv_lineno, csv_text);
    return InvalidCharacter;
  }

<COMMENT>.     { /* skip any character in here */ }
<COMMENT>\r?\n { BEGIN(INITIAL); /* skipping ends here */ return Eol; }

%%
