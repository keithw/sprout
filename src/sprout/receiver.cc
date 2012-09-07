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
    _expected_seq( 0 )
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

void Receiver::recv( const uint64_t seq )
{
  _count_this_tick++;

  if ( seq + 1 > _expected_seq ) {
    _expected_seq = seq + 1;
  }
}

Sprout::DeliveryForecast Receiver::forecast( void )
{
  if ( _cached_forecast.time() == _time ) {
    return _cached_forecast;
  } else {
    std::vector< int > counts;

    _process.normalize();

    _cached_forecast.set_last_seq( _expected_seq - 1 );
    _cached_forecast.set_time( _time );
    _cached_forecast.clear_counts();

    for ( auto it = _forecastr.begin(); it != _forecastr.end(); it++ ) {
      _cached_forecast.add_counts( it->lower_quantile( _process, 0.05 ) );
    }

    return _cached_forecast;
  }
}
