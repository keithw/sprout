#include "sproutconn.h"

using namespace Network;

SproutConnection::SproutConnection( const char *desired_ip, const char *desired_port )
  : conn( desired_ip, desired_port ),
    local_forecast_time( 0 ),
    remote_forecast_time( 0 ),
    last_outgoing_ended_flight( true ),
    operative_forecast( conn.forecast() ) /* something reasonable */
{}

SproutConnection::SproutConnection( const char *key_str, const char *ip, int port )
  : conn( key_str, ip, port ),
    local_forecast_time( 0 ),
    remote_forecast_time( 0 ),
    last_outgoing_ended_flight( true ),
    operative_forecast( conn.forecast() ) /* something reasonable */
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

  conn.send( to_send.tostring(), time_to_next );
}

string SproutConnection::recv( void )
{
  ForecastPacket packet( conn.recv() );

  if ( packet.has_forecast() ) {
    operative_forecast = packet.forecast();
    remote_forecast_time = timestamp(); /* - (SRTT/2 ?? XXX) */
  }

  return packet.data();
}

int SproutConnection::window_size( void ) const
{
  uint64_t now = timestamp();
  uint64_t delayed_queue_estimate = conn.get_next_seq() - operative_forecast.received_or_lost_count();

  int current_forecast_tick = (now - remote_forecast_time) / conn.get_tick_length();

  if ( current_forecast_tick < 0 ) {
    current_forecast_tick = 0;
  } else if ( current_forecast_tick >= operative_forecast.counts_size() ) {
    current_forecast_tick = operative_forecast.counts_size() - 1;
  }

  int current_queue_estimate = delayed_queue_estimate - operative_forecast.counts( current_forecast_tick );
  if ( current_queue_estimate < 0 ) {
    current_queue_estimate = 0;
  }

  int cumulative_delivery_tick = current_forecast_tick + TARGET_DELAY_TICKS;
  if ( cumulative_delivery_tick >= operative_forecast.counts_size() ) {
    cumulative_delivery_tick = operative_forecast.counts_size() - 1;
  }

  int cumulative_delivery_forecast = operative_forecast.counts( cumulative_delivery_tick ) - operative_forecast.counts( current_forecast_tick );

  int packets_to_send = cumulative_delivery_forecast - current_queue_estimate;

  if ( packets_to_send < 0 ) {
    packets_to_send = 0;
  }

  return packets_to_send * 1440;
}
