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

  bool new_tick = false;

  while ( _time + TICK_LENGTH < time ) {
    _process.evolve( TICK_LENGTH );
    fprintf( stderr, "Observing %d packets this tick\n", _count_this_tick );
    _process.observe( TICK_LENGTH, _count_this_tick );
    _count_this_tick = 0;
    fprintf( stderr, "Ticking from %f to %f (target %f)\n",
	     _time, _time + TICK_LENGTH, time );
    _time += TICK_LENGTH;
    new_tick = true;
  }

  if ( new_tick ) {
    DeliveryForecast window_forecast( forecast() );
    fprintf( stderr, "%f forecast: %d %d %d %d %d %d %d %d %d %d\n",
	     time,
	     window_forecast.counts[ 0 ],
	     window_forecast.counts[ 1 ],
	     window_forecast.counts[ 2 ],
	     window_forecast.counts[ 3 ],
	     window_forecast.counts[ 4 ],
	     window_forecast.counts[ 5 ],
	     window_forecast.counts[ 6 ],
	     window_forecast.counts[ 7 ],
	     window_forecast.counts[ 8 ],
	     window_forecast.counts[ 9 ] );
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
