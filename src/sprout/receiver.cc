#include <assert.h>
#include <stdio.h>

#include "receiver.hh"

Receiver::Receiver( const double s_time )
  : _process( MAX_ARRIVAL_RATE,
	      BROWNIAN_MOTION_RATE,
	      OUTAGE_ESCAPE_RATE,
	      NUM_BINS ),
    _forecastr(),
    _time( s_time ),
    _count_this_tick( 0 )
{
  for ( int i = 0; i < NUM_TICKS; i++ ) {
    ProcessForecastInterval one_forecast( TICK_LENGTH,
					  _process,
					  MAX_ARRIVALS_PER_TICK,
					  i + 1 );
    _forecastr.push_back( one_forecast );
  }
}

void Receiver::advance_to( const double time )
{
  assert( time >= _time );

  while ( _time + TICK_LENGTH < time ) {
    _process.evolve( TICK_LENGTH );
    if ( _count_this_tick ) {
      _process.observe( TICK_LENGTH, _count_this_tick );
      _count_this_tick = 0;
    }
    _time += TICK_LENGTH;
  }
}

void Receiver::recv( void )
{
  _count_this_tick++;
}

DeliveryForecast Receiver::forecast( void )
{
  std::vector< int > ret;

  _process.normalize();

  for ( auto it = _forecastr.begin(); it != _forecastr.end(); it++ ) {
    ret.push_back( it->lower_quantile( _process, 0.05 ) );
  }

  return DeliveryForecast( TICK_LENGTH, ret );
}
