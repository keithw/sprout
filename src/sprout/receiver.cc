#include <assert.h>
#include <stdio.h>

#include "receiver.hh"

Receiver::Receiver()
  : _process( MAX_ARRIVAL_RATE,
	      BROWNIAN_MOTION_RATE,
	      OUTAGE_ESCAPE_RATE,
	      NUM_BINS ),
    _forecastr(),
    _time( 0 ),
    _score_time( -1 ),
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
    if ( (_time >= _score_time) || (_count_this_tick > 0) ) {
      int discrete_observe = int( _count_this_tick + 0.5 );
      if ( _count_this_tick > 0 && _count_this_tick < 1 ) {
	discrete_observe = 1;
      }
      _process.observe( .001 * TICK_LENGTH, discrete_observe );
      //      fprintf( stderr, "tick(%f) ", _count_this_tick );
      _count_this_tick = 0;
    } else {
      //      fprintf( stderr, "-SKIP-" );
    }
    _time += TICK_LENGTH;
  }
}

void Receiver::recv( const uint64_t seq, const uint16_t throwaway_window, const uint16_t time_to_next, const size_t len )
{
  _count_this_tick += len / 1400.0;
  _recv_queue.recv( seq, throwaway_window, len );
  _score_time = std::max( _time + time_to_next, _score_time );
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

void Receiver::RecvQueue::recv( const uint64_t seq, const uint16_t throwaway_window, const int len )
{
  received_sequence_numbers.push( PacketLen( seq, len ) );
  throwaway_before = std::max( throwaway_before, seq - throwaway_window );
}

uint64_t Receiver::RecvQueue::packet_count( void )
{
  /* returns cumulative count of bytes received or lost */
  while ( !received_sequence_numbers.empty() ) {
    if ( received_sequence_numbers.top().seq < throwaway_before ) {
      received_sequence_numbers.pop();
    } else {
      break;
    }
  }

  std::priority_queue< PacketLen, std::deque<PacketLen>, PacketLen > copy( received_sequence_numbers );

  int buffer_sum = 0;
  while ( !copy.empty() ) {
    buffer_sum += copy.top().len;
    copy.pop();
  }

  return throwaway_before + buffer_sum;
}
