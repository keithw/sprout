#include <stdint.h>

#include "process.hh"
#include "processforecaster.hh"

#ifndef RECEIVER_HH
#define RECEIVER_HH

class DeliveryForecast
{
public:
  const uint64_t TICK_LENGTH;
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
  static const int TICK_LENGTH = 20;
  static const int MAX_ARRIVALS_PER_TICK = 30;
  static const int NUM_TICKS = 10;

  Process _process;

  std::vector< ProcessForecastInterval > _forecastr;

  uint64_t _time;

  int _count_this_tick;

public:

  Receiver();
  void warp_to( const uint64_t time ) { _time = time; }
  void advance_to( const uint64_t time );
  void recv( void );

  DeliveryForecast forecast( void );
};

#endif
