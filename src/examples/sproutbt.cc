#include <unistd.h>

#include "network.h"
#include "select.h"

#include "deliveryforecast.pb.h"

int main( int argc, char *argv[] )
{
  char *key;
  char *ip;
  int port;

  Network::Connection *net;

  if ( argc > 1 ) {
    /* client */

    key = argv[ 1 ];
    ip = argv[ 2 ];
    port = atoi( argv[ 3 ] );

    net = new Network::Connection( key, ip, port );
  } else {
    net = new Network::Connection( NULL, NULL );
  }

  fprintf( stderr, "Port bound is %d, key is %s\n", net->port(), net->get_key().c_str() );

  Select &sel = Select::get_instance();
  sel.add_fd( net->fd() );
}
