#ifndef RECEIVER_HH
#define RECEIVER_HH

#include <stdint.h>
#include <queue>

#include "process.hh"
#include "processforecaster.hh"

#include "deliveryforecast.pb.h"

class Receiver
{
private:
  static constexpr double MAX_ARRIVAL_RATE = 1000;
  static constexpr double BROWNIAN_MOTION_RATE = 200;
  static constexpr double OUTAGE_ESCAPE_RATE = 1;
  static const int NUM_BINS = 256;
  static const int TICK_LENGTH = 20;
  static const int MAX_ARRIVALS_PER_TICK = 30;
  static const int NUM_TICKS = 8;

  class RecvQueue {
  private:
    class PacketLen {
    public:
      uint64_t seq;
      int len;
      bool operator()( const PacketLen & a, const PacketLen & b ) const
      {
	return a.seq > b.seq;
      }
      PacketLen( const uint64_t s_seq, const int s_len ) : seq( s_seq ), len( s_len ) {}
      PacketLen( void ) : seq( -1 ), len( 0 ) {}
    };

    std::priority_queue< PacketLen, std::deque<PacketLen>, PacketLen > received_sequence_numbers;
    uint64_t throwaway_before;

  public:
    RecvQueue() : received_sequence_numbers(), throwaway_before( 0 ) {}

    void recv( const uint64_t seq, const uint16_t throwaway_window, int len );
    uint64_t packet_count( void );
  };

  Process _process;

  std::vector< ProcessForecastInterval > _forecastr;

  uint64_t _time, _score_time;

  double _count_this_tick;

  Sprout::DeliveryForecast _cached_forecast;

  RecvQueue _recv_queue;

public:

  Receiver();
  void warp_to( const uint64_t time ) { _score_time = _time = time; }
  void advance_to( const uint64_t time );
  void recv( const uint64_t seq, const uint16_t throwaway_window, const uint16_t time_to_next, const size_t len );

  Sprout::DeliveryForecast forecast( void );

  int get_tick_length( void ) const { return TICK_LENGTH; }
};

#endif
