#ifndef SPROUTCONN_H
#define SPROUTCONN_H

#include "network.h"
#include "deliveryforecast.pb.h"

namespace Network {
  class SproutConnection
  {
  private:
    class ForecastPacket
    {
    private:
      string _forecast, _data;
      
    public:
      ForecastPacket() : _forecast(), _data() {}
      
      /* No forecast update */
      ForecastPacket( bool, const string & s_data )
	: _forecast(),
	  _data( s_data )
      {}
      
      void add_forecast( const Sprout::DeliveryForecast & s_forecast )
      {
	_forecast = s_forecast.SerializeAsString();
	assert( _forecast.size() <= 65535 );
      }
      
      ForecastPacket( const string & incoming )
	: _forecast( incoming.substr( sizeof( uint16_t ), *(uint16_t *) incoming.data() ) ),
	  _data( incoming.substr( sizeof( uint16_t ) + _forecast.size() ) )
      {}
      
      string tostring( void ) const
      {
	uint16_t forecast_size = _forecast.size();
	string ret( (char *) & forecast_size, sizeof( forecast_size ) );
	if ( forecast_size ) {
	  ret.append( _forecast );
	}
	ret.append( _data );
	
	return ret;
      }
      
      bool has_forecast( void ) const { return _forecast.size() > 0; }
      
      Sprout::DeliveryForecast forecast( void ) const
      {
	assert( has_forecast() );
	
	Sprout::DeliveryForecast ret;
	assert( ret.ParseFromString( _forecast ) );
	
	return ret;
      }

      string data( void ) const { return _data; }
    };      

    Connection conn;

    static const int TARGET_DELAY_TICKS = 5;
    uint64_t local_forecast_time;
    uint64_t remote_forecast_time;
    bool last_outgoing_ended_flight;
    int current_queue_bytes_estimate;
    int current_forecast_tick;

    Sprout::DeliveryForecast operative_forecast;

    void update_queue_estimate( void );

    std::deque< std::pair< const string, uint16_t > > outgoing_queue;

  public:
    SproutConnection( const char *desired_ip, const char *desired_port ); /* server */
    SproutConnection( const char *key_str, const char *ip, int port ); /* client */

    void send( const string & s, uint16_t time_to_next = 0 );
    void queue_to_send( const string & s, uint16_t time_to_next = 0 );
    string recv( void );

    int fd( void ) const { return conn.fd(); }
    int get_MTU( void ) const { return conn.get_MTU(); }

    int port( void ) const { return conn.port(); }
    bool get_has_remote_addr( void ) const { return conn.get_has_remote_addr(); }

    uint64_t timeout( void ) const { return conn.timeout(); }
    double get_SRTT( void ) const { return conn.get_SRTT(); }

    uint64_t get_next_seq( void ) const { return conn.get_next_seq(); }
    int get_tick_length( void ) const { return conn.get_tick_length(); }

    int window_size( void );

    void tick( void );
  };
}

#endif
