/** Copyright John Reid 2010
*/


#include <boost/itl/interval_set.hpp>
#include <iostream>



using namespace boost::itl;
using namespace std;



int main( int argc, char * argv[] ) {

	{
		const interval< double > empty_interval;
		cout << "empty interval cardinality is " << empty_interval.cardinality() << "\n";
		if( empty_interval.cardinality() == numeric_limits< interval< double >::size_type >::infinity() )
			cout << "empty interval cardinality is infinite\n";
	}

	{
		const interval< double > unit_interval( 0.0, 1.0 );
		cout << "unit interval cardinality is " << unit_interval.cardinality() << "\n";
		if( unit_interval.cardinality() == numeric_limits< interval< double >::size_type >::infinity() )
			cout << "unit interval cardinality is infinite\n";
	}

	{
		const interval< double > singleton_interval( 0.0 );
		cout << "singleton interval cardinality is " << singleton_interval.cardinality() << "\n";
		if( singleton_interval.cardinality() == numeric_limits< interval< double >::size_type >::infinity() )
			cout << "singleton interval cardinality is infinite\n";
	}

	return 0;
}
