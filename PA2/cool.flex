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

extern int curr_lineno;
extern int verbose_flag;

extern YYSTYPE cool_yylval;

int comment_level = 0; /*number of nested comments*/
char string_buf[MAX_STR_CONST]; /*a buffer to assemble string constants*/
int string_len; /*the length of the current string*/
bool string_overflow; /*true if the current string is overflown*/
bool string_contains_null; /*true if the current string contains a null*/

%}

 /* Basic stuff */
DIGIT           [0-9]
UPPERCASE       [A-Z]
LOWERCASE       [a-z]
ALPHANUMERIC    [A-Za-z0-9_]

 /*Integers, Identifiers, and Special Notation */
INTEGER         {DIGIT}+
TYPEID          {UPPERCASE}{ALPHANUMERIC}*
OBJECTID        {LOWERCASE}{ALPHANUMERIC}*
DARROW          \=\>
LE              \<\=
ASSIGN          \<\-

 /*Strings: No regex is required */

 /* Comments */
OPEN_COMMENT    \(\*
CLOSE_COMMENT   \*\)
LINE_COMMENT    \-\-.*

 /*  Keywords */
CLASS           (?i:class)
ELSE            (?i:else)
FI              (?i:fi)
IF              (?i:if)
IN              (?i:in)
INHERITS        (?i:inherits)
ISVOID          (?i:isvoid)
LET             (?i:let)
LOOP            (?i:loop)
POOL            (?i:pool)
THEN            (?i:then)
WHILE           (?i:while)
CASE            (?i:case)
ESAC            (?i:esac)
NEW             (?i:new)
OF              (?i:of)
NOT             (?i:not)
BOOL_TRUE       t(?i:rue)
BOOL_FALSE      f(?i:alse)

 /* White Space */
WHITESPACE      [ \f\t\v\r]
ENDLINE         \n

%x COMMENT
%x STRING
%%

 /* Comment related rules */
{LINE_COMMENT}  { }
{OPEN_COMMENT}  { comment_level++; BEGIN(COMMENT); }
{CLOSE_COMMENT} {
    cool_yylval.error_msg = strdup("Unmatched *)");
    return (ERROR);
}
<COMMENT>{OPEN_COMMENT} { comment_level++; }
<COMMENT>{CLOSE_COMMENT} {
        if (--comment_level == 0)
                BEGIN(INITIAL);
}
<COMMENT>{ENDLINE} { curr_lineno++; }
<COMMENT><<EOF>>   {
    BEGIN(INITIAL);
    cool_yylval.error_msg = strdup("EOF in comment");
    return (ERROR);
}
<COMMENT>.      { }

 /* Multiple-character operators */
{DARROW}        { return (DARROW); }
{LE}            { return (LE); }
{ASSIGN}        { return (ASSIGN); }

 /* Single-character operators */
[\;\:\,\.\@\~\<\=] { return yytext[0]; }
[\-\+\*\/\{\}\(\)] { return yytext[0]; }

 /*
  * Keywords
  * Note: keywords are case-insensitive except for the values
  * true and false, which must begin with a lower-case letter.
  */
{CLASS}         { return (CLASS); }
{ELSE}          { return (ELSE); }
{FI}            { return (FI); }
{IF}            { return (IF); }
{IN}            { return (IN); }
{INHERITS}      { return (INHERITS); }
{ISVOID}        { return (ISVOID); }
{LET}           { return (LET); }
{LOOP}          { return (LOOP); }
{POOL}          { return (POOL); }
{THEN}          { return (THEN); }
{WHILE}         { return (WHILE); }
{CASE}          { return (CASE); }
{ESAC}          { return (ESAC); }
{NEW}           { return (NEW); }
{OF}            { return (OF); }
{NOT}           { return (NOT); }
{BOOL_TRUE}     {
    cool_yylval.boolean = true;
    return (BOOL_CONST);
}
{BOOL_FALSE}    {
    cool_yylval.boolean = false;
    return (BOOL_CONST);
}

 /* Constants and identifiers */
{INTEGER}       {
    cool_yylval.symbol = inttable.add_string(yytext);
    return (INT_CONST);
}
{TYPEID}        {
    cool_yylval.symbol = idtable.add_string(yytext);
    return (TYPEID);
}
{OBJECTID}      {
    cool_yylval.symbol = idtable.add_string(yytext);
    return (OBJECTID);
}

 /*
  * String constant related rules
  * Note: constants are in C syntax; escape sequence
  * \c is accepted for all characters c. Except for 
  * \n \t \b \f, the result is c.
  */
\"              {
    BEGIN(STRING);
    string_len = 0;
    string_overflow = false;
    string_contains_null = false;
}
<STRING>\\\"    {
    if (string_len == 1024) string_overflow = true;
    else string_buf[string_len++] = '"';
}
<STRING>\\{ENDLINE} { 
    curr_lineno++;
    if (string_len == 1024) string_overflow = true;
    else string_buf[string_len++] = '\n';
}
<STRING>{ENDLINE} {
    curr_lineno++;
    BEGIN(INITIAL);
    if (string_contains_null)
        cool_yylval.error_msg = strdup("String contains null character");
    else
        cool_yylval.error_msg = strdup("Unterminated string constant");
    return (ERROR);
}
<STRING>\\[btnf] {
    if (string_len == 1024) string_overflow = true;
    else {
        switch (yytext[1]) {
            case 'b':
                string_buf[string_len++] = '\b';
                break;
            case 't':
                string_buf[string_len++] = '\t';
                break;
            case 'n':
                string_buf[string_len++] = '\n';
                break;
            case 'f':
                string_buf[string_len++] = '\f';
                break;
            default:
              break;
        }
    }
}
<STRING>\\.     {
    if (string_len == 1024) string_overflow = true;
    else string_buf[string_len++] = yytext[1];
}
<STRING>\"      {
    BEGIN(INITIAL);
    if (string_contains_null) {
        cool_yylval.error_msg = strdup("String contains null character");
        return (ERROR);
    }
    if (string_overflow) {
        cool_yylval.error_msg = strdup("String constant too long");
        return (ERROR);
    }
    else {
        string_buf[string_len] = 0;
        cool_yylval.symbol = stringtable.add_string(string_buf);
        return (STR_CONST);
    }
}
<STRING>\0      {
    string_contains_null = true;
    if (string_len == 1024) string_overflow = true;
    else string_buf[string_len++] = yytext[0];
}
<STRING><<EOF>> {
    BEGIN(INITIAL);
    cool_yylval.error_msg = strdup("EOF in string constant");
    return (ERROR);
}
<STRING>.       {
    if (string_len == 1024) string_overflow = true;
    else string_buf[string_len++] = yytext[0];
}

 /* Rules to soak up everything else */
{ENDLINE}       { curr_lineno++; }
{WHITESPACE}+   { }
.               {
    cool_yylval.error_msg = strdup(yytext);
    return (ERROR);
}

%%

