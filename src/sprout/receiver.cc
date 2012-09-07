#include <assert.h>
#include <stdio.h>

#include "receiver.hh"

Receiver::Receiver()
  : _process( MAX_ARRIVAL_RATE,
	      BROWNIAN_MOTION_RATE,
	      OUTAGE_ESCAPE_RATE,
	      NUM_BINS ),
    _forecastr(),
    _time( -1 ),
    _count_this_tick( 0 ),
    _cached_forecast(),
    _recv_queue()
{
  for ( int i = 0; i < NUM_TICKS; i++ ) {
    ProcessForecastInterval one_forecast( .001 * TICK_LENGTH,
					  _process,
					  MAX_ARRIVALS_PER_TICK,
					  i + 1 );
    _forecastr.push_back( one_forecast );
  }
}

void Receiver::advance_to( const uint64_t time )
{
  assert( time >= _time );

  while ( _time + TICK_LENGTH < time ) {
    _process.evolve( .001 * TICK_LENGTH );
    _process.observe( .001 * TICK_LENGTH, _count_this_tick );
    _count_this_tick = 0;
    _time += TICK_LENGTH;
  }
}

void Receiver::recv( const uint64_t seq, const uint16_t throwaway_window )
{
  _count_this_tick++;
  _recv_queue.recv( seq, throwaway_window );
}

Sprout::DeliveryForecast Receiver::forecast( void )
{
  if ( _cached_forecast.time() == _time ) {
    return _cached_forecast;
  } else {
    std::vector< int > counts;

    _process.normalize();

    _cached_forecast.set_received_or_lost_count( _recv_queue.packet_count() );
    _cached_forecast.set_time( _time );
    _cached_forecast.clear_counts();

    for ( auto it = _forecastr.begin(); it != _forecastr.end(); it++ ) {
      _cached_forecast.add_counts( it->lower_quantile( _process, 0.05 ) );
    }

    return _cached_forecast;
  }
}

void Receiver::RecvQueue::recv( const uint64_t seq, const uint16_t throwaway_window )
{
  received_sequence_numbers.push( seq );
  throwaway_before = seq - throwaway_window;
}

uint64_t Receiver::RecvQueue::packet_count( void )
{
  /* returns cumulative count of packets received or lost */
  while ( !received_sequence_numbers.empty() ) {
    if ( received_sequence_numbers.top() < throwaway_before ) {
      received_sequence_numbers.pop();
    } else {
      break;
    }
  }

  return throwaway_before + received_sequence_numbers.size();
}
