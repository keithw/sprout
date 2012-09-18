#include "sproutconn.h"

using namespace Network;

SproutConnection::SproutConnection( const char *desired_ip, const char *desired_port )
  : conn( desired_ip, desired_port ),
    local_forecast_time( 0 ),
    remote_forecast_time( 0 ),
    last_outgoing_ended_flight( true ),
    current_queue_bytes_estimate( 0 ),
    current_forecast_tick( 0 ),
    operative_forecast( conn.forecast() ), /* something reasonable */
    outgoing_queue()
{}

SproutConnection::SproutConnection( const char *key_str, const char *ip, int port )
  : conn( key_str, ip, port ),
    local_forecast_time( 0 ),
    remote_forecast_time( 0 ),
    last_outgoing_ended_flight( true ),
    current_queue_bytes_estimate( 0 ),
    current_forecast_tick( 0 ),
    operative_forecast( conn.forecast() ), /* something reasonable */
    outgoing_queue()
{}

void SproutConnection::send( const string & s, uint16_t time_to_next )
{
  ForecastPacket to_send( false, s );

  /* consider forecast */
  if ( last_outgoing_ended_flight ) {
    Sprout::DeliveryForecast the_fc = conn.forecast();

    if ( the_fc.time() != local_forecast_time ) {
      to_send.add_forecast( the_fc );
      local_forecast_time = the_fc.time();
    }
  }

  last_outgoing_ended_flight = ( time_to_next > 0 );

  const string outgoing( to_send.tostring() );

  conn.send( outgoing, time_to_next );

  current_queue_bytes_estimate += outgoing.size();
  update_queue_estimate();
}

void SproutConnection::update_queue_estimate( void )
{
  uint64_t now = timestamp();
  /* investigate decrementing current forecast */
  int new_forecast_tick = std::min( int((now - remote_forecast_time) / conn.get_tick_length()),
				    operative_forecast.counts_size() - 1 );

  while ( current_forecast_tick < new_forecast_tick ) {
    current_queue_bytes_estimate -= 1440 * operative_forecast.counts( current_forecast_tick );
    if ( current_queue_bytes_estimate < 0 ) current_queue_bytes_estimate = 0;

    current_forecast_tick++;
  }
}

string SproutConnection::recv( void )
{
  ForecastPacket packet( conn.recv() );

  if ( packet.has_forecast() ) {
    operative_forecast = packet.forecast();
    remote_forecast_time = timestamp(); // - conn.get_SRTT()/4;
    current_queue_bytes_estimate = conn.get_next_seq() - operative_forecast.received_or_lost_count();
    assert( current_queue_bytes_estimate >= 0 );
    current_forecast_tick = 0;

    update_queue_estimate();
  }

  return packet.data();
}

int SproutConnection::window_size( void )
{
  update_queue_estimate();

  int cumulative_delivery_tick = current_forecast_tick + TARGET_DELAY_TICKS;
  if ( cumulative_delivery_tick >= operative_forecast.counts_size() ) {
    cumulative_delivery_tick = operative_forecast.counts_size() - 1;
  }

  int cumulative_delivery_forecast = 1440 * ( operative_forecast.counts( cumulative_delivery_tick )
					      - operative_forecast.counts( current_forecast_tick ) );

  int bytes_to_send = cumulative_delivery_forecast - current_queue_bytes_estimate;

  /* also consider outgoing queue */
  /* a more precise calculation would estimate whether we are going to be
     including a forecast */
  for ( auto it = outgoing_queue.begin(); it != outgoing_queue.end(); it++ ) {
    bytes_to_send -= it->first.size();
  }

  if ( bytes_to_send < 0 ) {
    bytes_to_send = 0;
  }

  /*
  if ( bytes_to_send > 0 ) {
    fprintf( stderr, "From tick %d(%d) => %d(%d), %d bytes to send with %d already sent\n",
	     current_forecast_tick, operative_forecast.counts( current_forecast_tick ),
	     cumulative_delivery_tick, operative_forecast.counts( cumulative_delivery_tick ),
	     bytes_to_send, current_queue_bytes_estimate );
  }
  */

  return bytes_to_send;
}

void SproutConnection::queue_to_send( const string & s, uint16_t time_to_next )
{
  outgoing_queue.push_back( make_pair( s, time_to_next ) );
}

void SproutConnection::tick( void )
{
  if ( outgoing_queue.empty() ) {
    return;
  }

  while ( (!outgoing_queue.empty())
	  && (window_size() >= (int)outgoing_queue.front().first.size()) ) {
    /* send it */
    const string s = outgoing_queue.front().first;
    uint16_t time_to_next = outgoing_queue.front().second;
    outgoing_queue.pop_front();

    /* what should the time-to-next be? */
    /* will we also probably send the next packet? */
    if ( (!outgoing_queue.empty())
	 && (window_size() - s.size() - 20 >= outgoing_queue.front().first.size()) ) {
      /* allowance for overhead */
      time_to_next = 0;
    }

    send( s, time_to_next );
  }
}
