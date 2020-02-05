#include <stdint.h>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <vector>
#include <string>

typedef struct Node
{
	uint64_t src ;
	uint64_t dest ;
} Node ;

// DFA:

// Data structure
typedef std::unordered_map<std::string, std::unordered_set<Node> > internalmap ;
typedef std::unordered_map<std::unordered_set<Node>, internalmap> statemap ;

// Iterators
typedef std::unordered_map<std::unordered_set<Node>, internalmap>::iterator statemapiterator ;
typedef std::unordered_map<std::string, std::unordered_set<Node> >::iterator internalmapiterator ;

class DFA
{
private:
  	statemap transitions ;
  	std::unordered_map<unordered_set<Node>, unordered_set<Node> > defaults ;
  	std::unordered_set<unordered_set<Node> > final_states ;
  	Node start_state ;

public:
  	DFA() ;
  	DFA( Node initstate ) ;
  	~DFA() ;
  	void add_transition( Node src, std::string input, Node dest ) ;
  	void set_default_transition( Node src, Node dest ) ;
  
  	template<template <typename...> class Hashmap, typename T, typename U>
  	vector<T> getkeys( const Hashmap<T,U> &hashmap ) ;
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
	static const char EPSILON = "&" ;
	static const char ANY = "#" ;
	statemap transitions ;
	std::unordered_set<Node> final_states ;					// Not sure if set or unordered_set

public:
	NFA() ;
	~NFA() ;
	NFA( Node initstate ) ;

	std::unordered_set<Node> set_difference( const std::unordered_set<Node> &a, const std::unordered_set<Node> &b ) ;
	const unordered_set<Node> get_start_state() ;
	void add_transitions( Node src, std::string input, Node dest ) ;
	std::unordered_set<Node> expand( unordered_set<Node> currstate ) ;
} ;
