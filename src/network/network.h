/*
    Mosh: the mobile shell
    Copyright 2012 Keith Winstein

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    In addition, as a special exception, the copyright holders give
    permission to link the code of portions of this program with the
    OpenSSL library under certain conditions as described in each
    individual source file, and distribute linked combinations including
    the two.

    You must obey the GNU General Public License in all respects for all
    of the code used other than OpenSSL. If you modify file(s) with this
    exception, you may extend this exception to your version of the
    file(s), but you are not obligated to do so. If you do not wish to do
    so, delete this exception statement from your version. If you delete
    this exception statement from all source files in the program, then
    also delete it here.
*/

#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <stdint.h>
#include <deque>
#include <queue>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
#include <math.h>

#include "crypto.h"

#include "receiver.hh"

using namespace Crypto;

namespace Network {
  uint64_t timestamp( void );
  uint16_t timestamp16( void );
  uint16_t timestamp_diff( uint16_t tsnew, uint16_t tsold );

  class NetworkException {
  public:
    string function;
    int the_errno;
    NetworkException( string s_function, int s_errno ) : function( s_function ), the_errno( s_errno ) {}
    NetworkException() : function( "<none>" ), the_errno( 0 ) {}
  };

  enum Direction {
    TO_SERVER = 0,
    TO_CLIENT = 1
  };

  class Packet {
  public:
    uint64_t seq;
    Direction direction;
    uint16_t timestamp, timestamp_reply, throwaway_window, time_to_next;
    string payload;
    
    Packet( uint64_t s_seq, Direction s_direction,
	    uint16_t s_timestamp, uint16_t s_timestamp_reply, uint16_t s_throwaway_window, uint16_t s_time_to_next, const string & s_payload )
      : seq( s_seq ), direction( s_direction ),
      timestamp( s_timestamp ), timestamp_reply( s_timestamp_reply ), throwaway_window( s_throwaway_window ), time_to_next( s_time_to_next ),
	payload( s_payload )
    {}
    
    Packet( string coded_packet, Session *session );
    
    string tostring( Session *session );
  };

  class SendQueue {
  private:
    std::queue< std::pair< uint64_t, uint64_t > > sent_packets; /* seq, ts */

    static const int REORDER_LIMIT = 10; /* ms */

  public:
    SendQueue() : sent_packets() {}

    uint16_t add( const uint64_t seq ); /* returns throwaway */
  };

  class Connection {
  private:
    static const int SEND_MTU = 1480;
    static const uint64_t MIN_RTO = 50; /* ms */
    static const uint64_t MAX_RTO = 5000; /* ms */

    static const int PORT_RANGE_LOW  = 60001;
    static const int PORT_RANGE_HIGH = 60999;

    static const unsigned int SERVER_ASSOCIATION_TIMEOUT = 2000000;
    static const unsigned int PORT_HOP_INTERVAL          = 2000000;

    static bool try_bind( int socket, uint32_t addr, int port );

    int sock;
    bool has_remote_addr;
    struct sockaddr_in remote_addr;

    bool server;

    int MTU;

    Base64Key key;
    Session session;

    void setup( void );

    Direction direction;
    uint64_t next_seq;
    uint16_t saved_timestamp;
    uint64_t saved_timestamp_received_at;
    uint64_t expected_receiver_seq;

    uint64_t last_heard;
    uint64_t last_port_choice;
    uint64_t last_roundtrip_success; /* transport layer needs to tell us this */

    bool RTT_hit;
    double SRTT;
    double RTTVAR;

    /* Exception from send(), to be delivered if the frontend asks for it,
       without altering control flow. */
    bool have_send_exception;
    NetworkException send_exception;

    Packet new_packet( const string &s_payload, uint16_t time_to_next );

    void hop_port( void );

    /* Sprout state */
    Receiver forecastr;
    bool forecastr_initialized;

    SendQueue send_queue;

  public:
    Connection( const char *desired_ip, const char *desired_port ); /* server */
    Connection( const char *key_str, const char *ip, int port ); /* client */
    ~Connection();

    void send( const string & s, uint16_t time_to_next = 0 );
    string recv( void );

    void send_raw( string s );
    string recv_raw( void );

    int fd( void ) const { return sock; }
    int get_MTU( void ) const { return MTU; }

    int port( void ) const;
    string get_key( void ) const { return key.printable_key(); }
    bool get_has_remote_addr( void ) const { return has_remote_addr; }

    uint64_t timeout( void ) const;
    double get_SRTT( void ) const { return SRTT; }

    const struct in_addr & get_remote_ip( void ) const { return remote_addr.sin_addr; }

    const NetworkException *get_send_exception( void ) const
    {
      return have_send_exception ? &send_exception : NULL;
    }

    void set_last_roundtrip_success( uint64_t s_success ) { last_roundtrip_success = s_success; }

    Sprout::DeliveryForecast forecast( void ) { forecastr.advance_to( timestamp() ); return forecastr.forecast(); }

    uint64_t get_next_seq( void ) const { return next_seq; }
    int get_tick_length( void ) const { return forecastr.get_tick_length(); }
  };
}

#endif
