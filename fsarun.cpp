#include <iostream>
#include <string>
#include <vector>
#include <stdint.h>
#include <algorithm>
#include <assert.h>
#include "fsa.h"
#include "fastFileLoader.h"

using namespace std ;

std::string defaultcleaningtool( const std::string &s )
{
    return s ;
}

class Matcher
{
private:
	std::vector<char*> str ;
	uint32_t probes ;
public:
	Matcher( std::vector<char*> s ) : str(s), probes(0){}
	~Matcher(){}

	uint32_t getprobes(){ return probes ; }

	std::string nextinput( const std::string &s )
	{
		std::string empty ;
		++probes ;
		std::vector<char*>::iterator it = std::lower_bound( str.begin(), str.end(), s ) ;	// Should be already sorted
		uint32_t pos = it - str.begin() ;

		if( pos < str.size() )
			return str[ pos ] ;
		else
			return empty ;
	}
} ;

int main()
{
	vector<char*> corpus ;
	vector<string> results ;
	string filename = "worddict.txt" ;
	FileLoader *file ;
	file = new FileLoader( filename.c_str(), defaultcleaningtool, corpus ) ;
	std::sort( corpus.begin(), corpus.end() ) ;
	Matcher m( corpus ) ;

	results = find_all_matches( "food", 1, &m.nextinput ) ;

	//cout<<std::boolalpha<<assert( results.size() == 21 )<<endl ;

	return 0 ;
}
