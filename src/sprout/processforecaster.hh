#ifndef PROCESSFORECASTER_HH
#define PROCESSFORECASTER_HH

#include "process.hh"

#include <vector>

class ProcessForecastTick
{
private:
  std::vector< std::vector< double > > _count_probability;

public:
  ProcessForecastTick( const double tick_time, const Process & example, const unsigned int upper_limit );

  double probability( unsigned int component, unsigned int count ) const { return _count_probability[ component ][ count ]; }
  double probability( const Process & ensemble, unsigned int count ) const;

  static std::vector< Process > make_components( const Process & example );
};

class ProcessForecastInterval
{
private:
  std::vector< std::vector< double > > _count_probability;

  static std::vector< double > convolve( const std::vector< double > & old_count_probabilities,
					 const std::vector< double > & this_tick );

public:
  ProcessForecastInterval( const double tick_time,
			   const Process & example,
			   const unsigned int tick_upper_limit,
			   const unsigned int num_ticks );

  double probability( const Process & ensemble, unsigned int count ) const;

  unsigned int lower_quantile( const Process & ensemble, const double x ) const;
};

#endif
