/*
 *  The scanner definition for COOL.
 */

/*
 *  Stuff enclosed in %{ %} in the first section is copied verbatim to the
 *  output, so headers and global definitions are placed here to be visible
 * to the code in the file.  Don't remove anything that was here initially
 */
%{
#include <cool-parse.h>
#include <stringtab.h>
#include <utilities.h>

/* The compiler assumes these identifiers. */
#define yylval cool_yylval
#define yylex  cool_yylex

/* Max size of string constants */
#define MAX_STR_CONST 1025
#define YY_NO_UNPUT   /* keep g++ happy */

extern FILE *fin; /* we read from this file */

/* define YY_INPUT so we read from the FILE fin:
 * This change makes it possible to use this scanner in
 * the Cool compiler.
 */
#undef YY_INPUT
#define YY_INPUT(buf,result,max_size) \
	if ( (result = fread( (char*)buf, sizeof(char), max_size, fin)) < 0) \
		YY_FATAL_ERROR( "read() in flex scanner failed");

char string_buf[MAX_STR_CONST]; /* to assemble string constants */
char *string_buf_ptr;

extern int curr_lineno;
extern int verbose_flag;

extern YYSTYPE cool_yylval;

/*
 *  Add Your own definitions here
 */

int comment_depth = 0;

%}

/*
 * Define names for regular expressions here.
 */

DARROW    =>
LE        <=
ASSIGN    <-
COMMENTL  "(*"
COMMENTR  "*)"
COMMENTN  --
INTCONST  [0-9]+
STRCONST  \"[^"\n]*\"
BOOLCONST t[rR][uU][eE]|f[aA][lL][sS][eE]
TYPEIDENT [A-Z][a-zA-Z0-9_]*
OBJIDENT  [a-z][a-zA-Z0-9_]*
SYMBOL    [-.(){}:@,;+*/~<=]

CLASS    [cC][lL][aA][sS][sS]
ELSE     [eE][lL][sS][eE]
FI       [fF][iI]
IF       [iI][fF]
IN       [iI][nN]
INHERITS [iI][nN][hH][eE][rR][iI][tT][sS]
LET      [lL][eE][tT]
LOOP     [lL][oO][oO][pP]
POOL     [pP][oO][oO][lL]
THEN     [tT][hH][eE][nN]
WHILE    [wW][hH][iI][lL][eE]
CASE     [cC][aA][sS][eE]
ESAC     [eE][sS][aA][cC]
OF       [oO][fF]
NEW      [nN][eE][wW]
ISVOID   [iI][sS][vV][oO][iI][dD]
NOT      [nN][oO][tT]

%x COMMENT_BLOCK COMMENT_LINE STR_BLOCK STR_NUL_ERROR

%%

 /*
  *  Nested comments
  */

{COMMENTN} { BEGIN COMMENT_LINE; }
{COMMENTL} { comment_depth = 1; BEGIN COMMENT_BLOCK; }

<COMMENT_LINE>[^\n]* ;
<COMMENT_LINE>\n {
    curr_lineno++;
    BEGIN 0;
}

<COMMENT_BLOCK>[^*()\n]* ;
<COMMENT_BLOCK>\([^*\n]* ;
<COMMENT_BLOCK>\*[^*)\n]* ;
<COMMENT_BLOCK>\) ;
<COMMENT_BLOCK>\n {
    curr_lineno++;
}
<COMMENT_BLOCK>{COMMENTL} {
    comment_depth++;
}
<COMMENT_BLOCK>{COMMENTR} {
    if (comment_depth == 1) BEGIN 0;
    else if (comment_depth > 1) comment_depth--;
}
<COMMENT_BLOCK><<EOF>> {
    cool_yylval.error_msg = "EOF in comment";
    BEGIN 0;
    return (ERROR);
}
<INITIAL>"*)" {
    cool_yylval.error_msg = "Unmatched *)";
    return (ERROR);
}

 /*
  *  The multiple-character operators.
  */
{DARROW}   { return (DARROW); }
{ASSIGN}   { return (ASSIGN); }
{LE}       { return (LE); }

 /*
  * Keywords are case-insensitive except for the values true and false,
  * which must begin with a lower-case letter.
  */
{CLASS}    { return (CLASS); }
{ELSE}     { return (ELSE); }
{FI}       { return (FI); }
{IF}       { return (IF); }
{IN}       { return (IN); }
{INHERITS} { return (INHERITS); }
{LET}      { return (LET); }
{LOOP}     { return (LOOP); }
{POOL}     { return (POOL); }
{THEN}     { return (THEN); }
{WHILE}    { return (WHILE); }
{CASE}     { return (CASE); }
{ESAC}     { return (ESAC); }
{OF}       { return (OF); }
{NEW}      { return (NEW); }
{ISVOID}   { return (ISVOID); }
{NOT}      { return (NOT); }

{INTCONST} { 
    cool_yylval.symbol = inttable.add_string(yytext);
    return (INT_CONST);
}
{BOOLCONST} {
    for (int i = 0; yytext[i]; i++)
        yytext[i] = tolower(yytext[i]);
    if (strcmp("true", yytext) == 0) { cool_yylval.boolean = true; }
    else { cool_yylval.boolean = false; }
    return (BOOL_CONST);
}

{TYPEIDENT} {
    cool_yylval.symbol = idtable.add_string(yytext);
    return (TYPEID);
}

{OBJIDENT} {
    cool_yylval.symbol = idtable.add_string(yytext);
    return (OBJECTID);
}

{SYMBOL} {
    return (int) yytext[0];
}

 /*
  *  String constants (C syntax)
  *  Escape sequence \c is accepted for all characters c. Except for 
  *  \n \t \b \f, the result is c.
  *
  */

\" {
    string_buf_ptr = string_buf;
    BEGIN STR_BLOCK;
}
<STR_BLOCK>\" {
    BEGIN 0;
    *string_buf_ptr = '\0';
    if (string_buf_ptr >= string_buf + MAX_STR_CONST) {
        cool_yylval.error_msg = "String constant too long";
        return (ERROR);
    }
    else {
        cool_yylval.symbol = stringtable.add_string(string_buf);
        return (STR_CONST);
    }
}

<STR_BLOCK>\\b *string_buf_ptr++ = '\b';
<STR_BLOCK>\\t *string_buf_ptr++ = '\t';
<STR_BLOCK>\\n *string_buf_ptr++ = '\n';
<STR_BLOCK>\\f *string_buf_ptr++ = '\f';
<STR_BLOCK>\\\0 { BEGIN STR_NUL_ERROR; }
<STR_BLOCK>\\(.|\n) { *string_buf_ptr++ = yytext[1]; }
<STR_BLOCK>[^"\\\0\n]* {
    if (string_buf_ptr + sizeof(char) * strlen(yytext) < string_buf + MAX_STR_CONST) {
        strcpy(string_buf_ptr, yytext);
    }
    string_buf_ptr += sizeof(char) * strlen(yytext);
}
<STR_BLOCK>\n {
    cool_yylval.error_msg = "Unterminated string constant";
    BEGIN 0;
    return (ERROR);
}
<STR_BLOCK><<EOF>> {
    cool_yylval.error_msg = "EOF in string constant";
    BEGIN 0;
    return (ERROR);
}
<STR_NUL_ERROR>\" {
    cool_yylval.error_msg = "String contains null character";
    BEGIN 0;
    return (ERROR);
}
\n {
    curr_lineno++;
}

[ \f\r\t\v] ;

. { 
    cool_yylval.error_msg = yytext;
    return (ERROR);
}

%%
