/***********************************************************
 * nvsetenv for AIX 4.3 and later                          *
 *                                                         *
 * get / set NVRAM variables a la nvsetenv under PPC Linux *
 *                                                         *
 * version 0.1, (c) 2005 Chris Brown, code@thebrown.net    *
 ***********************************************************/


#include <stdio.h>
#include <stdarg.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mdio.h>

#define BUFFER_SIZE 102400

void nvram_get_var( const char *name, char *value, unsigned long *length );
void nvram_set_var( const char* name, const char *value );
void nvram_print_vars( char *buffer, unsigned long length );
void nvram_error( const char * msg, ... );


int main( int argc, char *argv[] )
{
  char buffer[ BUFFER_SIZE ];
  unsigned long length = BUFFER_SIZE;

  /* no arguments supplied means get all variables */
  if( argc == 1 ) {
    nvram_get_var( "*", buffer, &length );
    /* print them */
    nvram_print_vars( buffer, length );
    return 0;
  }

  /* an option? */
  if( argv[1][0] == '-' ) {
    /* delete variable? */
    if( argv[1][1] == 'd' && argc > 2 ) {
      /* done by setting it to an empty string */
      nvram_set_var( argv[2], "" );
      printf( "Variable %s deleted.\n", argv[2] );
      return 0;

    } else {
      /* must need help */
      printf( "nvsetenv - get / set NVRAM variables\n"
              "Usage:\n"
              "  print a variable:    nvsetenv varname\n"
              "  set a variable:      nvsetenv varname value\n"
              "  delete a variable:   nvsetenv -d varname\n"
              "  print all variables: nvsetenv\n" );
      return 0;
    }
  } 

  /* setting a variable? */
  if( argc > 2 )
    nvram_set_var( argv[1], argv[2] );

  /* get a specific variable */
  nvram_get_var( argv[1], buffer, &length );

  /* and print it */
  nvram_print_vars( buffer, length );

  return 0;
}


/* get a variable from the NVRAM */
void nvram_get_var( const char *name, char *value, unsigned long *length )
{ 
  MACH_DD_IO nvram_data;
  int nvram_f;
  char varname[BUFFER_SIZE];

  nvram_f = open( "/dev/nvram", O_RDONLY );
  if( nvram_f == -1 ) {
    nvram_error( "Cannot open /dev/nvram - permission denied! Are you root?" );
    exit( 1 );
  }

  /* the request is of the form "varname=" */
  sprintf( varname, "%s=", name );
  nvram_data.md_addr = (unsigned long) varname;

  /* how much buffer is available */
  nvram_data.md_size = *length;

  /* address of the return buffer */
  nvram_data.md_data = value;

  /* pointer to receive how much data was returned -
     re-use the length parameter */
  nvram_data.md_length = length;

  /* unused */
  nvram_data.md_sla = 0;
  nvram_data.md_incr = 0;

  /* to be safe, zero return value */
  length = 0;

  /* get variable */
  if( ioctl( nvram_f, MIOGEARD, &nvram_data ) ) {
    nvram_error( "Failed to read variable %s: error number %d.", name, errno );
    close( nvram_f );
    exit( 1 );
  }

  close( nvram_f );
}


/* set a variable in the NVRAM */
void nvram_set_var( const char* name, const char *value )
{
  MACH_DD_IO nvram_data;
  int nvram_f;
  char varname[BUFFER_SIZE];
  unsigned long junk;

  nvram_f = open( "/dev/nvram", O_RDWR );
  if( nvram_f == -1 ) {
    nvram_error( "Cannot open /dev/nvram - permission denied! Are you root?" );
    exit( 1 );
  }

  /* form a string name=value */
  sprintf( varname, "%s=%s", name, value );
  nvram_data.md_addr = (unsigned long) varname;

  /* unused */
  nvram_data.md_size = 0;
  nvram_data.md_data = 0;
  nvram_data.md_length = &junk;
  nvram_data.md_sla = 0;
  nvram_data.md_incr = 0;

  /* set variable */
  if( ioctl( nvram_f, MIOGEAUPD, &nvram_data ) ) {
    nvram_error( "Failed to set variable %s: error number %d.", name, errno );
    close( nvram_f );
    exit( 1 );
  }
}


/* print a \0 separated list of strings */
void nvram_print_vars( char *buffer, unsigned long length )
{
  int i = 0;

  do {
    /* print up to the next \0 */
    printf( "%s\n", &buffer[i] );

    /* skip the string just printed */
    i += strlen( &buffer[i] ) + 1;

  } while( i < length );
}


void nvram_error( const char * msg, ... )
{
  va_list ap;

  fprintf( stderr, "nvsetenv: " );

  va_start( ap, msg );
  vfprintf( stderr, msg, ap );

  fprintf( stderr, "\n" );

  va_end( ap );
  exit( 1 );
}

