#include "sampledfunction.hh"

#include <assert.h>
#include <stdio.h>

SampledFunction::SampledFunction( const int num_samples,
					   const double maximum_value,
					   const double minimum_value )
  : _offset( minimum_value ),
    _bin_width( (maximum_value - minimum_value) / num_samples ),
    _function( int((maximum_value - minimum_value) / _bin_width) + 1, 1.0 )
{
}

void SampledFunction::for_each( const std::function< void( const double midpoint,
							 double & value,
							 const unsigned int index ) > f )
{
  for ( unsigned int i = 0; i < _function.size(); i++ ) {
    f( from_bin_mid( i ), _function[ i ], i );
  }
}

void SampledFunction::for_each( const std::function< void( const double midpoint,
							   const double & value,
							   const unsigned int index ) > f ) const
{
  for ( unsigned int i = 0; i < _function.size(); i++ ) {
    f( from_bin_mid( i ), _function[ i ], i );
  }
}

void SampledFunction::for_range( const double min,
					  const double max,
					  const std::function< void( const double midpoint,
								     double & value,
								     const unsigned int index ) > f )
{
  const unsigned int limit_high = to_bin( sample_ceil( max ) );
  for ( unsigned int i = to_bin( sample_floor( min ) ); i <= limit_high; i++ ) {
    f( from_bin_mid( i ), _function[ i ], i );
  }
}

const SampledFunction & SampledFunction::operator=( const SampledFunction & other )
{
  assert( _offset == other._offset );
  assert( _bin_width == other._bin_width );
  _function = other._function;

  return *this;
}

double SampledFunction::lower_quantile( const double x ) const
{
  double sum = 0.0;

  for ( unsigned int i = 0; i < _function.size(); i++ ) {
    sum += _function[ i ];
    //    fprintf( stderr, "%d sum=%f\n", i, sum );
    if ( sum >= x ) {
      if ( i == 0 ) { return 0; } else { return from_bin_floor( i ); }
    }
  }
  return from_bin_floor( _function.size() - 1 );
}

double SampledFunction::summation( const std::vector< std::vector< double > > & count_probability,
				   const int count ) const
{
  double ret = 0.0;
  for ( unsigned int i = 0; i < _function.size(); i++ ) {
    ret += _function[ i ] * count_probability[ i ][ count ];
  }
  return ret;
}
