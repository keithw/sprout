#ifndef SAMPLEDFUNCTION_HH
#define SAMPLEDFUNCTION_HH

#include <vector>
#include <functional>
#include <limits.h>

static double BIG = 1.e6;

class SampledFunction {
private:
  const double _offset;
  const double _bin_width;
  std::vector< double > _function;

  unsigned int to_bin( double x ) const { int ret = (x - _offset) / _bin_width; if ( ret < 0 ) { return 0; } else if ( ret >= (int)_function.size() ) { return _function.size() - 1; } else { return ret; } }

  double from_bin_floor( unsigned int bin ) const { if ( bin <= 0 ) { return -BIG; } else { return bin * _bin_width + _offset; } }
  double from_bin_ceil( unsigned int bin ) const { if ( bin >= _function.size() - 1 ) { return BIG; } else { return (bin + 1) * _bin_width + _offset; } }
  double from_bin_mid( unsigned int bin ) const { if ( bin == 0 ) { return _offset; } return (bin + 0.5) * _bin_width + _offset; }

public:
  SampledFunction( const int num_samples, const double maximum_value, const double minimum_value );

  unsigned int size( void ) const { return _function.size(); }
  unsigned int index( const double x ) const { return to_bin( x ); }
  double & operator[]( const double x ) { return _function[ to_bin( x ) ]; }
  const double & operator[]( const double x ) const { return _function[ to_bin( x ) ]; }

  double sample_floor( double x ) const { return from_bin_floor( to_bin( x ) ); }
  double sample_ceil( double x ) const { return from_bin_ceil( to_bin( x ) ); }

  void for_each( const std::function< void( const double midpoint,
					    double & value,
					    const unsigned int index ) > f );
  void for_each( const std::function< void( const double midpoint,
					    const double & value,
					    const unsigned int index ) > f ) const;
  void for_range( const double min, const double max,
		  const std::function< void( const double midpoint,
					     double & value,
					     const unsigned int index ) > f );

  const SampledFunction & operator=( const SampledFunction & other );

  double lower_quantile( const double x ) const;

  double summation( const std::vector< std::vector< double > > & count_probability,
		    const int count ) const;
};

#endif
