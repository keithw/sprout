#ifndef FLOOD_HPP
#define FLOOD_HPP

#include <string>
#include <stdio.h>

class Flood
{
private:
    
public:
  Flood() {}
  
  /* interface for Network::Transport */
  void subtract( const Flood * ) {}
  std::string diff_from( const Flood &, const size_t len ) const { if ( len ) fprintf( stderr, "diff(%d)\n", (int) len ); return std::string( len, 'x' ); }
  void apply_string( std::string ) {}
  bool operator==( const Flood & ) const { return true; }

  bool compare( const Flood & ) const { return false; }
};

#endif
