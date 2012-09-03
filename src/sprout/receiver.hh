#include "process.hh"
#include "processforecaster.hh"

#ifndef RECEIVER_HH
#define RECEIVER_HH

class DeliveryForecast
{
public:
  const double TICK_LENGTH;
  const std::vector< int > counts;

  DeliveryForecast( const double s_TICK_LENGTH, const std::vector< int > s_counts )
    : TICK_LENGTH( s_TICK_LENGTH ), counts( s_counts ) {}
};

class Receiver
{
private:
  static constexpr double MAX_ARRIVAL_RATE = 2000;
  static constexpr double BROWNIAN_MOTION_RATE = 300;
  static constexpr double OUTAGE_ESCAPE_RATE = 5;
  static const int NUM_BINS = 64;
  static constexpr double TICK_LENGTH = 0.02;
  static const int MAX_ARRIVALS_PER_TICK = 30;
  static const int NUM_TICKS = 10;

  Process _process;

  std::vector< ProcessForecastInterval > _forecastr;

  double _time;

  int _count_this_tick;

public:

  Receiver( const double s_time );
  void advance_to( const double time );
  void recv( void );

  DeliveryForecast forecast( void );
};

#endif
