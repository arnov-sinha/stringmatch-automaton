#include <stdint.h>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <functional>
#include <algorithm>
#include <vector>
#include <string>

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


// DFA:

// Data structure
typedef std::unordered_map<std::string, std::unordered_set<Node> > internalmap ;
typedef std::unordered_map<size_t, internalmap> statemap ;

// Iterators
typedef std::unordered_map<size_t, internalmap>::iterator statemapiterator ;
typedef std::unordered_map<std::string, std::unordered_set<Node> >::iterator internalmapiterator ;

class DFA
{
private:
  	statemap transitions ;
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
  	std::string find_next_edge( std::unordered_set<Node> s, std::string x ) ;
  	std::string next_valid_string( std::string input ) ;

  	// Support functions
  	template<typename T>
	size_t hashonset( std::unordered_set<T> uset ) ;
  	
  	template<template <typename...> class Hashmap, typename T, typename U>
  	std::vector<T> getkeys( const Hashmap<T,U> &hashmap ) ;
} ;


// NFA :

// Data Structures
typedef std::unordered_map<std::string, std::unordered_set<Node> > internalstate ;
typedef std::unordered_map<Node, internalstate> statemap ;

// Iterators
typedef internalstate::iterator internalstateiterator ;
typedef statemap::iterator statemapiterator ;

class NFA
{
private:
	Node start_state ;

public:
	static const char EPSILON = '&' ;
	static const char ANY = '#' ;
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
	const std::unordered_set<Node> next_state( std::unordered_set<Node> currstates, std::string input ) ;
	std::unordered_set<std::string> get_inputs( std::unordered_set<Node> states ) ;
	bool isPresent( std::vector<std::unordered_set<Node> > seen, std::unordered_set<Node> set ) ;
	DFA to_dfa() ;

	// Support functions
	std::unordered_set<Node> set_difference( const std::unordered_set<Node> &a, const std::unordered_set<Node> &b ) ;
} ;
