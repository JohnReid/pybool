/** Copyright John Reid 2010
*/


#include <boost/itl/interval_map.hpp>

using namespace boost::itl;
using namespace std;

int main( int argc, char * argv[] ) {

	interval_map< int, float > map_1;
	interval_map< int, float > map_2;

	//map_1 & map_2;
	map_1 &= map_2;
	//map_1.add_intersection( map_1, map_2 );

	return 0;
}
