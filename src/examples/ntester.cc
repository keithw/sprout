/*
    Mosh: the mobile shell
    Copyright 2012 Keith Winstein

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    In addition, as a special exception, the copyright holders give
    permission to link the code of portions of this program with the
    OpenSSL library under certain conditions as described in each
    individual source file, and distribute linked combinations including
    the two.

    You must obey the GNU General Public License in all respects for all
    of the code used other than OpenSSL. If you modify file(s) with this
    exception, you may extend this exception to your version of the
    file(s), but you are not obligated to do so. If you do not wish to do
    so, delete this exception statement from your version. If you delete
    this exception statement from all source files in the program, then
    also delete it here.
*/

#include "flood.h"
#include "networktransport.cc"
#include "select.h"

using namespace Network;

int main( int argc, char *argv[] )
{
  char *ip;
  int port;

  Flood me, remote;

  Transport<Flood, Flood> *n;

  if ( argc > 1 ) {
    /* client */
    
    ip = argv[ 1 ];
    port = atoi( argv[ 2 ] );
      
    n = new Transport<Flood, Flood>( me, remote, "4h/Td1v//4jkYhqhLGgegw", ip, port );
  } else {
    n = new Transport<Flood, Flood>( me, remote, NULL, NULL );
  }
  
  fprintf( stderr, "Port bound is %d\n", n->port() );

  Select &sel = Select::get_instance();
  sel.add_fd( n->fd() );

  while ( 1 ) {
    int active_fds = sel.select( n->wait_time() );
    if ( active_fds < 0 ) {
      perror( "select" );
      exit( 1 );
    }
    
    n->tick();
    
    if ( sel.read( n->fd() ) ) {
      n->recv();
    }
  }
 
  delete n;
}
