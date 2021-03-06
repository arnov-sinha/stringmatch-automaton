#include <iostream>
#include <stdint.h>
#include <sstream>
#include <unordered_set>
#include <unordered_map>
#include <functional>
#include <algorithm>
#include <vector>
#include <string>
#include <bitset>
#include "fastFileLoader.h"

// Node for the Graph and supported functions

typedef uint64_t Node ;

inline uint64_t node( uint32_t src, uint32_t dest )
{
	return (( (uint64_t)src ) << 32 | dest ) ;
}

inline uint32_t src( uint64_t node )
{
	return(uint32_t)( node >> 32) ;
}

inline uint32_t dest( uint64_t node )
{
	return(uint32_t)( node & 0xffff ) ;
}

std::bitset<64> uintToBitSet( size_t value )
{
  std::bitset<64> bits ;
  uint64_t index = 0;
  while( value != 0 )
  {
    if (value % 2 != 0)
        bits.set( index );

    ++index;
    value = value >> 1;
  }
  return bits;
}

template<typename T>
size_t hashonset( const std::unordered_set<T> &uset )
{
	size_t buf = 0 ;
	std::hash<std::bitset<64> > s_hash ;
	for( auto &e : uset )
		buf += s_hash( uintToBitSet( e ) ) ;
	size_t key = s_hash(buf) ;
	return key ;
}


// Timer functions
static inline double compute_elapsed( const struct timespec &starttime)
	{
		struct timespec endtime;
		clock_gettime( CLOCK_REALTIME, &endtime );
		double elapsed = (( endtime.tv_sec +
		                    endtime.tv_nsec / ( double ) 1000000000 ) -
		                  ( starttime.tv_sec +
		                    starttime.tv_nsec / ( double ) 1000000000 )) ;
		return elapsed;
	}


static inline std::string makemytime( void )
  {
  struct timeval tv ;
  struct tm brokendowntime ;
  std::stringstream ss ;
  long partialsec ;

  gettimeofday( &tv, NULL ) ;
  gmtime_r( &tv.tv_sec, &brokendowntime ) ;
  partialsec = 1000 * tv.tv_usec * 0.000001 ;

  ss << std::setfill( '0' ) << std::setw( 4 ) << brokendowntime.tm_year + 1900 << "-" <<
        std::setfill( '0' ) << std::setw( 2 ) << brokendowntime.tm_mon + 1     << "-" <<
        std::setfill( '0' ) << std::setw( 2 ) << brokendowntime.tm_mday        << "T" <<
        std::setfill( '0' ) << std::setw( 2 ) << brokendowntime.tm_hour        << ":" <<
        std::setfill( '0' ) << std::setw( 2 ) << brokendowntime.tm_min         << ":" <<
        std::setfill( '0' ) << std::setw( 2 ) << brokendowntime.tm_sec         << "." <<
        std::setfill( '0' ) << std::setw( 3 ) << partialsec                    << "Z" ;

  return( ss.str() ) ;
  }

static inline std::string makemytimebracketed(){ return( std::string("[") + makemytime() + "] " ) ;	}

// DFA:

// Data structure
typedef std::unordered_map<std::string, std::unordered_set<Node> > internalmap ;
typedef std::unordered_map<size_t, internalmap> smap ;

// Iterators
typedef std::unordered_map<size_t, internalmap>::const_iterator smapiterator ;
typedef std::unordered_map<std::string, std::unordered_set<Node> >::const_iterator internalmapiterator ;

class DFA
{
public:
  	smap transitions ;
  	std::unordered_map<size_t , std::unordered_set<Node> > defaults ;
  	std::unordered_set<size_t> final_states ;
  	std::unordered_set<Node> start_state ;

public:
  	DFA(){} ;
  	DFA( std::unordered_set<Node> initstate ) ;
  	~DFA(){} ;
  	void add_transition( std::unordered_set<Node> src, std::string input, std::unordered_set<Node> dest ) ;
  	void set_default_transition( std::unordered_set<Node> src, std::unordered_set<Node> dest ) ;
  	void add_final_state( std::unordered_set<Node> state ) ;
  	bool is_final_state( std::unordered_set<Node> state ) ;
  	std::unordered_set<Node> next_state( std::unordered_set<Node> src, std::string input ) ;
  	std::string find_next_edge( const std::unordered_set<Node> &s, std::string x ) ;
  	std::string next_valid_string( std::string input ) ;

  	// Support functions
  	std::vector<std::string> getkeys( const std::unordered_map<std::string, std::unordered_set<Node> > &hashmap ) ;
} ;


// NFA :

// Data Structures
typedef std::unordered_map<std::string, std::unordered_set<Node> > internalstate ;
typedef std::unordered_map<Node, internalstate> statemap ;

// Iterators
typedef internalstate::const_iterator internalstateiterator ;
typedef statemap::const_iterator statemapiterator ;

class NFA
{
private:
	Node start_state ;

public:
	static const std::string EPSILON ;
	static const std::string ANY ;
	statemap transitions ;
	std::unordered_set<Node> final_states ;

public:
	NFA() ;
	~NFA(){} ;
	NFA( Node initstate ) ;

	const std::unordered_set<Node> get_start_state() ;
	void add_transition( Node src, std::string input, Node dest ) ;
	std::unordered_set<Node> expand( std::unordered_set<Node> currstate ) ;
	void add_final_state( Node state ) ;
	bool is_final_state( std::unordered_set<Node> currstates ) ;
	const std::unordered_set<Node> next_state( const std::unordered_set<Node> &currstates, const std::string &input ) ;
	std::unordered_set<std::string> get_inputs( const std::unordered_set<Node> &states ) ;
	bool isPresent( const std::unordered_set<size_t> &seen, const std::unordered_set<Node> &set ) ;
	DFA to_dfa() ;

	// Support functions
	std::unordered_set<Node> set_difference( const std::unordered_set<Node> &a, const std::unordered_set<Node> &b ) ;
} ;

class Matcher
{
private:
	std::vector<std::string> str ;
	uint32_t probes ;

public:
	Matcher( const std::string &filename ) ;
	~Matcher(){} ;

	inline uint32_t getprobes(){ return probes ; }
	std::string nextinput( const std::string &s ) ;
} ;
