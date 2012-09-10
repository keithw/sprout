#include <unistd.h>
#include <string>
#include <assert.h>
#include <list>
#include <stdio.h>
#include <queue>
#include <limits.h>

#include "select.h"
#include "network.h"

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

  class PartialPacket
  {
  public:
    int bytes_earned;
    DelayedPacket packet;
    
    PartialPacket( int s_b_e, const DelayedPacket & s_packet ) : bytes_earned( s_b_e ), packet( s_packet ) {}
  };

  static const int SERVICE_PACKET_SIZE = 1500;

  const string _name;

  std::queue< DelayedPacket > _delay;
  std::queue< DelayedPacket > _pdp;
  std::queue< PartialPacket > _limbo;

  std::queue< uint64_t > _schedule;

  std::vector< string > _delivered;

  const uint64_t _ms_delay;

  uint64_t _total_bytes;
  uint64_t _used_bytes;

  uint64_t _bin_sec;

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
    _limbo(),
    _schedule(),
    _delivered(),
    _ms_delay( s_ms_delay ),
    _total_bytes( 0 ),
    _used_bytes( 0 ),
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
  int delay_wait = INT_MAX, schedule_wait = INT_MAX;

  uint64_t now = timestamp();

  tick();

  if ( !_delay.empty() ) {
    delay_wait = _delay.front().release_time - now;
    if ( delay_wait < 0 ) {
      delay_wait = 0;
    }
  }

  if ( !_schedule.empty() ) {
    schedule_wait = _schedule.front() - now;
    assert( schedule_wait >= 0 );
  }

  return std::min( delay_wait, schedule_wait );
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
  uint64_t now = timestamp();

  /* move packets from end of delay to PDP */
  while ( (!_delay.empty())
	  && (_delay.front().release_time <= now) ) {
    _pdp.push( _delay.front() );
    _delay.pop();
  }

  /* execute packet delivery schedule */
  while ( (!_schedule.empty())
	  && (_schedule.front() <= now) ) {
    /* grab a PDO */
    _schedule.pop();
    int bytes_to_play_with = SERVICE_PACKET_SIZE;

    /* execute limbo queue first */
    if ( !_limbo.empty() ) {
      if ( _limbo.front().bytes_earned + bytes_to_play_with >= (int)_limbo.front().packet.contents.size() ) {
	/* deliver packet */
	_total_bytes += _limbo.front().packet.contents.size();
	_used_bytes += _limbo.front().packet.contents.size();

	fprintf( stderr, "%s %f delivery %d\n", _name.c_str(), now / 1000.0, int(now - _limbo.front().packet.entry_time) );

	_delivered.push_back( _limbo.front().packet.contents );
	bytes_to_play_with -= (_limbo.front().packet.contents.size() - _limbo.front().bytes_earned);
	assert( bytes_to_play_with >= 0 );
	_limbo.pop();
	assert( _limbo.empty() );
      } else {
	_limbo.front().bytes_earned += bytes_to_play_with;
	bytes_to_play_with = 0;
	assert( _limbo.front().bytes_earned < (int)_limbo.front().packet.contents.size() );
      }
    }
    
    /* execute regular queue */
    while ( bytes_to_play_with > 0 ) {
      assert( _limbo.empty() );

      /* will this be an underflow? */
      if ( _pdp.empty() ) {
	_total_bytes += bytes_to_play_with;
	bytes_to_play_with = 0;
	/* underflow */
	//	fprintf( stderr, "%s %f underflow!\n", _name.c_str(), now / 1000.0 );
      } else {
	/* dequeue whole and/or partial packet */
	DelayedPacket packet = _pdp.front();
	_pdp.pop();
	if ( bytes_to_play_with >= (int)packet.contents.size() ) {
	  /* deliver whole packet */
	  _total_bytes += packet.contents.size();
	  _used_bytes += packet.contents.size();

	  fprintf( stderr, "%s %f delivery %d\n", _name.c_str(), now / 1000.0, int(now - packet.entry_time) );

	  _delivered.push_back( packet.contents );
	  bytes_to_play_with -= packet.contents.size();
	} else {
	  /* put packet in limbo */
	  assert( _limbo.empty() );

	  assert( bytes_to_play_with < (int)packet.contents.size() );

	  PartialPacket limbo_packet( bytes_to_play_with, packet );
	  
	  _limbo.push( limbo_packet );
	  bytes_to_play_with -= _limbo.front().bytes_earned;
	  assert( bytes_to_play_with == 0 );
	}
      }
    }
  }

  while ( now / 1000 > _bin_sec ) {
    fprintf( stderr, "%s %ld %ld / %ld = %.1f %%\n", _name.c_str(), _bin_sec, _used_bytes, _total_bytes, 100.0 * _used_bytes / (double) _total_bytes );
    _total_bytes = 0;
    _used_bytes = 0;
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
