/*
 * Read a number from a file.
 */
int fread_number( FILE *fp )
{
    int number;
    bool sign;
    char c;

    do
    {
    c = getc( fp );
    }
    while ( isspace(c) );

    number = 0;

    sign   = FALSE;
    if ( c == '+' )
    {
    c = getc( fp );
    }
    else if ( c == '-' )
    {
    sign = TRUE;
    c = getc( fp );
    }

    if ( !isdigit(c) )
    {
    bug( "Fread_number: bad format.", 0 );
    exit( 1 );
    }

    while ( isdigit(c) )
    {
    number = number * 10 + c - '0';
    c      = getc( fp );
    }

    if ( sign )
    number = 0 - number;

    if ( c == '|' )
    number += fread_number( fp );
    else if ( c != ' ' )
    ungetc( c, fp );

    return number;
}

