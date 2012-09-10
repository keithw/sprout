#include <unistd.h>
#include <string>
#include <assert.h>
#include <list>
#include <stdio.h>

#include "network.h"
#include "select.h"

using namespace std;
using namespace Network;

class DelayQueue
{
private:
  class DelayedPacket
  {
  public:
    uint64_t entry_time;
    uint64_t release_time;
    string contents;

    DelayedPacket( uint64_t s_e, uint64_t s_r, const string & s_c )
      : entry_time( s_e ), release_time( s_r ), contents( s_c ) {}
  };

  const string _name;

  std::queue< DelayedPacket > _delay;
  std::queue< DelayedPacket > _pdp;

  std::queue< uint64_t > _schedule;

  std::vector< string > _delivered;

  const uint64_t _ms_delay;

  uint64_t _total_occurrences;
  uint64_t _used_occurrences;

  uint64_t _bin_sec;

  void prune_schedule( void );

  void tick( void );

public:
  DelayQueue( const string & s_name, const uint64_t s_ms_delay, const char *filename, const uint64_t base_timestamp );

  int wait_time( void );
  std::vector< string > read( void );
  void write( const string & packet );
};

DelayQueue::DelayQueue( const string & s_name, const uint64_t s_ms_delay, const char *filename, const uint64_t base_timestamp )
  : _name( s_name ),
    _delay(),
    _pdp(),
    _schedule(),
    _delivered(),
    _ms_delay( s_ms_delay ),
    _total_occurrences( 0 ),
    _used_occurrences( 0 ),
    _bin_sec( timestamp() / 1000 )
{
  FILE *f = fopen( filename, "r" );
  if ( f == NULL ) {
    perror( "fopen" );
    exit( 1 );
  }

  while ( 1 ) {
    uint64_t ms;
    int num_matched = fscanf( f, "%lu\n", &ms );
    if ( num_matched != 1 ) {
      break;
    }

    ms += base_timestamp;

    if ( !_schedule.empty() ) {
      assert( ms >= _schedule.back() );
    }

    _schedule.push( ms );
  }

  fprintf( stderr, "Initialized %s queue with %d services.\n", filename, (int)_schedule.size() );
}

int DelayQueue::wait_time( void )
{
  int delay_wait = 100, pdp_wait = 100;

  if ( !_delay.empty() ) {
    delay_wait = _delay.front().release_time - timestamp();
    if ( delay_wait < 0 ) {
      delay_wait = 0;
    }
  }

  prune_schedule();

  if ( !_pdp.empty() ) {
    pdp_wait = _schedule.front() - timestamp();
    assert( pdp_wait >= 0 );
  }

  return std::min( delay_wait, pdp_wait );
}

void DelayQueue::prune_schedule( void )
{
  uint64_t now = timestamp();
  while ( (!_schedule.empty())
	  && (_schedule.front() < now) ) {
    _schedule.pop();
    _total_occurrences++;
  }
}

std::vector< string > DelayQueue::read( void )
{
  tick();

  std::vector< string > ret( _delivered );
  _delivered.clear();

  return ret;
}

void DelayQueue::write( const string & packet )
{
  uint64_t now( timestamp() );
  DelayedPacket p( now, now + _ms_delay, packet );
  _delay.push( p );
}

void DelayQueue::tick( void )
{
  prune_schedule();

  uint64_t now = timestamp();

  while ( (!_delay.empty())
	  && (_delay.front().release_time <= now) ) {
    _pdp.push( _delay.front() );
    _delay.pop();
  }

  while ( (!_pdp.empty())
	  && (!_schedule.empty())
	  && (_schedule.front() <= now) ) {
    DelayedPacket packet = _pdp.front();
    _delivered.push_back( packet.contents );
    _pdp.pop();
    _schedule.pop();
    _total_occurrences++;
    _used_occurrences++;
    fprintf( stderr, "%s %f delivery %d\n", _name.c_str(), now / 1000.0, int(now - packet.entry_time) );
  }

  while ( now / 1000 > _bin_sec ) {
    fprintf( stderr, "%s %ld %ld / %ld = %.1f %%\n", _name.c_str(), _bin_sec, _used_occurrences, _total_occurrences, 100.0 * _used_occurrences / (double) _total_occurrences );
    _total_occurrences = 0;
    _used_occurrences = 0;
    _bin_sec++;
  }
}

int main( int argc, char *argv[] )
{
  char *key;
  char *ip;
  int port;
  char *up_filename, *down_filename;

  assert( argc == 6 );

  key = argv[ 1 ];
  ip = argv[ 2 ];
  port = atoi( argv[ 3 ] );
  up_filename = argv[ 4 ];
  down_filename = argv[ 5 ];

  Network::Connection server( NULL, NULL );
  Network::Connection client( key, ip, port );

  fprintf( stderr, "Port bound is %d, key is %s\n", server.port(), server.get_key().c_str() );

  /* Read in schedule */
  uint64_t now = timestamp();
  DelayQueue uplink( "uplink", 20, up_filename, now );
  DelayQueue downlink( "downlink", 20, down_filename, now );

  Select &sel = Select::get_instance();
  sel.add_fd( server.fd() );
  sel.add_fd( client.fd() );

  while ( 1 ) {
    int wait_time = std::min( uplink.wait_time(), downlink.wait_time() );
    int active_fds = sel.select( wait_time );
    if ( active_fds < 0 ) {
      perror( "select" );
      exit( 1 );
    }

    if ( sel.read( server.fd() ) ) {
      string p( server.recv_raw() );
      uplink.write( p );
    }

    if ( sel.read( client.fd() ) ) {
      string p( client.recv_raw() );
      downlink.write( p );
    }

    std::vector< string > uplink_packets( uplink.read() );
    for ( auto it = uplink_packets.begin(); it != uplink_packets.end(); it++ ) {
      client.send_raw( *it );
    }

    std::vector< string > downlink_packets( downlink.read() );
    for ( auto it = downlink_packets.begin(); it != downlink_packets.end(); it++ ) {
      server.send_raw( *it );
    }
  }
}
