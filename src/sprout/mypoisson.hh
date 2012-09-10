#ifndef MYPOISSON_HH
#define MYPOISSON_HH

#include <boost/math/distributions/poisson.hpp>

double poissonpdf( const double rate, int counts )
{
  if ( rate == 0 ) {
    return ( counts == 0 );
  } else {
    return boost::math::pdf( boost::math::poisson( rate ), counts );
  }
}

#endif
