#include <iostream>
#include "fsa.h"

using namespace std ;

DFA::DFA( state initstate )
{
	start_state.dest = initstate.dest ;
	start_state.src = initstate.src ;

	transitions.resize( 200000 ) ;
	defaults.resize( 200000 ) ;
}

DFA::DFA( unordered_set<Node> initstate )
{
	
}

template<template <typename...> class Hashmap, typename T, typename U>
vector<T> DFA::getkeys( const Hashmap<T,U> &hashmap )
{
	vector<T> keys ;

	for( const auto& p : hashmap ) keys.push_back( p .first) ;

	return keys ;
}

void DFA::add_transition( unordered_set<Node> src, string input, unordered_set<Node> dest )
{
  	internalmap &internal = transitions[ src ] ;
  	internal[ input ] = dest ;
}

// void DFA::add_transition( Node src, string input, Node dest )
// {
//   	internalmap &internal = transitions[ src ] ;
//   	internal[ input ] = dest ;
// }

void DFA::set_default_transition( unordered_set<Node> src, unordered_set<Node> dest )
{
	defaults[ src ] = dest ;
}

// void DFA::set_default_transition( Node src, Node dest )
// {
//   	defaults[ src ] = dest ;
// }

void DFA::add_final_state( unordered_set<Node> state )
{
  	final_states.insert( state ) ;
}

// void DFA::add_final_state( Node state )
// {
//   	final_states.insert( curr ) ;
// }

bool DFA::is_final_state( Node state )
{
  	unordered_set<Node> it = final_states.find( state ) ;
  	return ( it != final_states.end() ) ;
}

Node DFA::next_state( Node src, string input )
{
  	Node empty { UINT64_MAX, UINT64_MAX } ;
  	bool flag = false ;

  	statemapiterator it = transitions.find( src ) ;					// Find the src in the transition table
  	if( it != transitions.end() )
  	{
    	internalmap state_transition = it->second ;
    	internalmapiterator sit = state_transition.find( input ) ;
    	if( sit != state_transition.end() )
      		return sit->second ;
    	else
      		flag = true ;
  	}
  	else
    	flag = true ;

  	if( flag ) ;													// else, look in defaults, otherwise return empty
  	{
    	unordered_map<Node,Node>::iterator dit = defaults.find( src ) ;
    	if( dit != defaults.end() )
      		return dit->second
    	else
    	  	return empty ;
  	}
}

string DFA::find_next_edge( Node s, string x )		// Gives the position of the lexicographically lower edge
{
	string none("") ;
	bool checkdefaults = false ;
	internalmap state_transition ;

	if( x.empty() )
		x = '\0'
	else
	{
		char c = x[ 0 ] ;
		++c ;
		x = c ;
	}

	statemapiterator it = transitions.find( s ) ;
	if( it != transitions.end() )
	{
		state_transition = it->second ;
		internalmapiterator sit = state_transition.find( x ) ;
		if( sit != state_transition.end() )
			return x ;
		else
			checkdefaults = true ;
	}
	else
		checkdefaults = true ;
	
	if( checkdefaults )
	{
		unordered_map<Node, Node>::iterator dit = defaults.find( s ) ;
		if( dit != defaults.end() )
			return x ;
	}

	vector<Node> labels = getkeys( state_transition ) ;
	std::sort( labels.begin(), labels.end() ) ;
	vector<Node>::iterator vit = std::lower_bound( labels.begin(), labels.end(), x ) ;
	uint64_t pos = vit - labels.begin() ;
	
	if( pos < labels.size() )
		return labels[ pos ] ;

	return none
}

string DFA::next_valid_string( string input )
{
  	string none("") ;
  	bool flag = false ;
  	uint64_t inputsize = input.size() ;
  	int64_t i = -1 ;
  	Node state = start_state ;
  	
  	// The below vectors can be replaced by vector<std::variant<type1,type2,type3>
  	// for c++17 onwards.
  	vector<string> stack1 ;
  	vector<Node> stack2 ;
  	vector<string> stack3 ;				// Maybe make it a char?

	for( int64_t x = 0 ; x < inputsize ; ++x, ++i )
  	{
    	if( i > -1 )
      		stack1.push_back( input.substr( 0, i ) ) ;
    	else
      		stack1.push_back( none ) ;

    	stack2.push_back( state ) ;
    	stack3.push_back( input[ x ] ) ;
    	state = next_state( state, input[ x ] ) ;

    	if( state.dest == UINT64_MAX )			// No where to go
    	{
      		flag = true ;
      		break ;
    	}
  	}

  	// If there is no other state create a final state and add it to the stack
  	if( flag )
  	{
	    stack1.push_back( input.substr( 0, i + 1 ) ) ;
    	stack2.push_back( state ) ;
    	stack3.push_back( none ) ;
  	}

  	if( is_final_state( state ) )		// Input word is already valid
  		return input ;

  	while( !stack1.empty() )			// Iterate through till you find a lexicographically smallest accepting state
  	{
  		string path = stack1.back() ;
  		stack1.pop_back() ;

  		Node lstate = stack2.back() ;
  		stack2.pop_back() ;

		string x = stack3.back() ;
  		stack3.pop_back() ;

  		x = find_next_edge( lstate, x ) ;

  		if( !x.empty() )
  		{
  			path += x ;
  			lstate = next_state( lstate, x ) ;
  			if( is_final_state( lstate ) )			// TODO: does this correctly validate a empty node? src,dest := UINT64_MAX
  				return path ;

  			stack1.push_back( path ) ;
  			stack2.push_back( lstate ) ;
  			stack3.push_back( none ) ;
  		}
  	}

  	return none ;
}

//--------------------------------------------------------------NFA----------------------------------------------------------------------------

NFA::NFA( Node initstate )
{
	transitions.reserve( 20000 ) ;
	final_states.reserve( 20000 ) ;
	start_state.src = initstate.src ;
	start_state.dest = initstate.dest ;
}

unordered_set<Node> NFA::set_difference( const unordered_set<Node> &a, const unordered_set<Node> &b )
{ 
	vector<Node> vec ;
	// set difference -> A - B
	std::copy( a.begin(), a.end() , std::back_inserter( vec ), [&b] ( Node i ) { return b.find( i ) == b.end() ; } ) ;
	unordered_set<Node> result( vec.begin(), vec.end() ) ;
	return result ;
}

const unordered_set<Node> NFA::get_start_state( void )
{
	unordered_set<Node> currstate( {start_state} ) ;
	return expand( currstate ) ;
}

std::unordered_set<Node> NFA::expand( unordered_set<Node> currstate )
{
	unordered_set<Node> frontier( currstate ) ;
	unordered_set<Node>::iterator it = frontier.begin() ;
	unordered_set<Node> newstates ;
	Node state ;

	while( !frontier.empty() )
	{
		if( it != frontier.end() )
		{
			state = *it ;
			it = frontier.erase( it ) ;

			statemapiterator internal = transitions.find( state ) ;
			if( internal != transitions.end() )
			{
				internalstateiterator localset = internal.find( EPSILON ) ;
				if( localset != internal.end() )
					newstate = set_difference( localset, currstate ) ;
			}

			frontier.insert( newstate.begin(), newstate.end() ) ;
			currstate.insert( newstate.begin(), newstate.end() ) ;
		}
		else
			it = frontier.begin() ;
	}
	return currstate ;
}

void NFA::add_transitions( Node src, string input, Node dest )
{
	internalstate &internal = transitions[ src ] ;
	unordered_set<Node> &currset =  internal[ input ] ;
	currset.insert( dest ) ;
}

void NFA::add_final_state( Node state )
{
	final_states.insert( state ) ;
}

unordered_set<Node> NFA::is_final_state( unordered_set<Node> currstates )
{
	// unordered_set intersection
	unordered_set<Node> intersection ; 

	uint64_t size = final_states.size() ;
	for( uint64_t i = 0 ; i < size ; ++i )
	{
		Node elem = final_states[ i ] ;
		if( currstates.count( elem ) )
			intersection.insert( elem )
	}

	return intersection ;
}

const unordered_set<Node> NFA::next_state( unordered_set<Node> currstates, string input )
{
	unordered_set<Node> dest_states ;
	unordered_set<Node>::iterator it ;
	
	for( it = currstates.begin() ; it != currstates.end() ; ++it )
	{
		internalstate internal ;
		statemapiterator sit = transitions.find( *it ) ;
		if( sit != transitions.end() )
		{
			internal = *sit ;
			internalstateiterator iit = internal.find( input ) ;
			if( iit != internal.end() )
				dest_states.insert( iit->second.begin(), iit->second.end() ) ;

			iit = internal.find( ANY ) ;
			if( iit != internal.end() )
				dest_states.insert( iit->second.begin(), iit->second.end() ) ;
		}
	}
	return expand( dest_states ) ;
}

unordered_set<string> NFA::get_inputs( unordered_set<Node> states )
{
	unordered_set<string> inputs ;
	unordered_set<Node>::iterator state ;
	statemapiterator it ; 

	for( state = states.begin() ; state != states.end() ; ++state )
	{
		it = transitions.find( state ) ;
		if( it != transitions.end() )
			for( auto& e : it->second )
				inputs.insert( e.first ) ;
	}
	return inputs ;
}

bool NFA::isPresent( vector<unordered_set<Node> > seen, unordered_set<Node> set )
{
	uint64_t vecsize = seen.size() ;
	for( uint64_t i = 0 ; i < vecsize ; ++i )
	{
		if( seen[ i ] == set )
			return true ;
	}
	return false ;
}

DFA NFA::to_dfa()			// Write a sample code to check the return type, typedef of class maybe needed
{
	unordered_set<Node> n ( get_start_state() ) ;
	vector<unordered_set<Node> > frontier ;
	vector<unordered_set<Node> > seen ;

	DFA dfa( n ) ;
	frontier.push_back( n ) ;

	while( !frontier.empty() )
	{
		unordered_set<Node> current = frontier.back() ;
		frontier.pop_back() ;

		unordered_set<string> inputs( get_inputs( current ) ) ;
		for( uint64_t i = 0 ; i < inputs.size() ; ++i )
		{
			string input = inputs[ i ] ;
			if( input.compare( EPSILON ) == 0 )
				continue ;

			unordered_set<Node> new_state( next_state( current, input ) ) ;
			if( !isPresent( seen, new_state ) )
			{
				frontier.push_back( new_state ) ;
				seen.push_back( new_state ) ;

				if( is_final_state( new_state ) )
					dfa.add_final_state( new_state ) ;
			}

			if( input.compare( ANY ) )
				dfa.set_default_transition( current, new_state ) ;
			else
				dfa.add_transitions( current, input, new_state ) ;
		}
	}
	return dfa ;
}

NFA levenshtein_automata( string term, uint64_t k  ) 		// Term to evaluate upto k errors
{
	NFA nfa ; 		// Default constructor with initial state ( 0, 0 )
	uint64_t termsize = term.size() ;
	
	for( uint64_t i = 0 ; i < termsize ; ++i )
	{
		for( uint64_t e = 0 ; e <= k ; ++e )
		{
			// Correct Character
			Node src { i, e } ;
			Node dest { i + 1, e } ;

			nfa.add_transitions( src, term[ i ], dest ) ;

			if( e < k )
			{
				// Deletion
				Node del { i, e + 1 } ;
				nfa.add_transitions( src, NFA.ANY, del ) ;

				// Insertion
				Node ins { i + 1, e + 1 } ;
				nfa.add_transitions( src, NFA.EPSILON, ins ) ;

				// Substitution
				nfa.add_transitions( src, NFA.ANY, ins ) ;
			}
		}
	}

	for( uint64_t e = 0 ; e <= k ; ++e )
	{
		if( e < k )
		{
			Node src { termsize, e } ;
			Node dest { termsize , e + 1 } ;
			nfa.add_transitions( src, NFA.ANY, dest ) ; 
		}
		Node final {termsize. e } ;
		nfa.add_transitions( final ) ;
	}

	return nfa ;
}
