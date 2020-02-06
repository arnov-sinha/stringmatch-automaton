#include <iostream>
#include "fsa.h"

using namespace std ;

template<typename T>
size_t DFA::hashonset( unordered_set<T> uset )
{
	vector<T> vec( uset.begin(), uset.end() ) ;
	unsigned int size = sizeof(T) ;
	string buf ;
	unsigned int vecsize = vec.size() ;
	buf.resize( vecsize * size ) ;
	std::sort( vec.begin(), vec.end() ) ;
	for( unsigned ii = 0 ; ii < vecsize ; ++ii )
	{
		T elem = ii ;
		for( unsigned int i = 0 ; i < size ; ++i )
			buf[ size + i ] = ((char*) &elem)[ i ] ;
	}
	std::hash<string> str_hash ;
	return str_hash( buf ) ;
}

DFA::DFA( unordered_set<Node> initstate )
{
	start_state = initstate ;
	transitions.reserve( 200000 ) ;
	defaults.reserve( 200000 ) ;	
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
	size_t key = hashonset(src) ;
  	internalmap &internal = transitions[ key ] ;
  	internal[ input ] = dest ;
}

void DFA::set_default_transition( unordered_set<Node> src, unordered_set<Node> dest )
{
	size_t key = hashonset(src) ;
	defaults[ key ] = dest ;
}

void DFA::add_final_state( unordered_set<Node> state )
{
  	final_states.insert( hashonset(state) ) ;
}

bool DFA::is_final_state( unordered_set<Node> state )
{
  	unordered_set<size_t>::const_iterator it = final_states.find( hashonset( state ) ) ;
  	return ( it != final_states.end() ) ;
}

unordered_set<Node> DFA::next_state( unordered_set<Node> src, string input )
{
  	unordered_set<Node> empty ;
  	bool flag = false ;

  	size_t key = hashonset( src ) ;
 
  	statemapiterator it = transitions.find( key ) ;					// Find the src in the transition table
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
    	unordered_map<size_t , unordered_set<Node> >::iterator dit = defaults.find( key ) ;
    	if( dit != defaults.end() )
      		return dit->second ;
    	else
    	  	return empty ;
  	}
}

string DFA::find_next_edge( unordered_set<Node> s, string x )		// Gives the position of the lexicographically lower edge
{
	string empty("") ;
	bool checkdefaults = false ;
	internalmap state_transition ;
	size_t key = hashonset( s ) ;

	if( x.empty() )				// if no next edge add, a edge with '\0', to denote end
		x = '\0' ;
	else
	{
		char c = x[ 0 ] ;		// change x to the next character for the next edge
		++c ;
		x = c ;
	}

	statemapiterator it = transitions.find( key ) ;
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
		unordered_map<size_t, unordered_set<Node> >::iterator dit = defaults.find( key ) ;
		if( dit != defaults.end() )
			return x ;
	}

	vector<string> labels = getkeys( state_transition ) ;
	std::sort( labels.begin(), labels.end() ) ;
	vector<string>::iterator vit = std::lower_bound( labels.begin(), labels.end(), x ) ;
	uint64_t pos = vit - labels.begin() ;
	
	if( pos < labels.size() )
		return labels[ pos ] ;

	return empty ;
}

string DFA::next_valid_string( string input )
{
  	string none("") ;
  	bool flag = false ;
  	int64_t inputsize = input.size() ;
  	int64_t i = -1 ;
  	unordered_set<Node> state = start_state ;
  	
  	// The below vectors can be replaced by vector<std::variant<type1,type2,type3>
  	// for c++17 onwards.
  	vector<string> stack1 ;
  	vector<unordered_set<Node> > stack2 ;
  	vector<string> stack3 ;				// Maybe make it a char?

	for( int64_t x = 0 ; x < inputsize ; ++x, ++i )
  	{
    	if( i > -1 )
      		stack1.push_back( input.substr( 0, i ) ) ;
    	else
      		stack1.push_back( none ) ;

    	stack2.push_back( state ) ;
    	stack3.push_back( string(1,input[ x ]) ) ;
    	state = next_state( state, string(1,input[ x ]) ) ;

    	if( state.size() == 0 )			// No where to go
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

  	if( is_final_state( state ) )		// Input word is already valid as a final state is available
  		return input ;

  	while( !stack1.empty() )			// Iterate through till you find a lexicographically smallest accepting state
  	{
  		string path = stack1.back() ;
  		stack1.pop_back() ;

  		unordered_set<Node> lstate = stack2.back() ;
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

NFA::NFA()
{
	transitions.reserve( 20000 ) ;
	final_states.reserve( 20000 ) ;
	start_state = 0 ;
}

NFA::NFA( Node initstate )
{
	transitions.reserve( 20000 ) ;
	final_states.reserve( 20000 ) ;
	start_state = initstate ;
}

unordered_set<Node> NFA::set_difference( const unordered_set<Node> &a, const unordered_set<Node> &b )
{ 
	vector<Node> vec ;
	// set difference -> A - B
	std::copy_if( a.begin(), a.end() , std::back_inserter( vec ), [&b] ( Node i ) { return b.find( i ) == b.end() ; } ) ;
	unordered_set<Node> result( vec.begin(), vec.end() ) ;
	return result ;
}

const unordered_set<Node> NFA::get_start_state( void )
{
	unordered_set<Node> currstate( {start_state} ) ;
	return expand( currstate ) ;
}

unordered_set<Node> NFA::expand( unordered_set<Node> currstate )
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
				internalstateiterator localset = internal->second.find( string(1,EPSILON) ) ;
				if( localset != internal->second.end() )
					newstates = set_difference( localset->second, currstate ) ;
			}

			frontier.insert( newstates.begin(), newstates.end() ) ;
			currstate.insert( newstates.begin(), newstates.end() ) ;
		}
		else
			it = frontier.begin() ;
	}
	return currstate ;
}

void NFA::add_transition( Node src, string input, Node dest )
{
	internalstate &internal = transitions[ src ] ;
	unordered_set<Node> &currset =  internal[ input ] ;
	currset.insert( dest ) ;
}

void NFA::add_final_state( Node state )
{
	final_states.insert( state ) ;
}

bool NFA::is_final_state( unordered_set<Node> currstates )
{
	// unordered_set intersection
	unordered_set<Node> intersection ; 
	unordered_set<Node>::const_iterator it ;

	for( it = final_states.begin() ; it != final_states.end() ; ++it )
		if( currstates.count( *it ) )
			intersection.insert( *it ) ;

	return (intersection.size() != 0 ) ;
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
			internal = sit->second ;
			internalstateiterator iit = internal.find( input ) ;
			if( iit != internal.end() )
				dest_states.insert( iit->second.begin(), iit->second.end() ) ;

			iit = internal.find( string(1,ANY) ) ;
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
		it = transitions.find( *state ) ;
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
		unordered_set<string>::iterator it ; 
		for( it = inputs.begin() ; it != inputs.end() ; ++it )
		{
			string input = *it ;
			if( input.compare( string( 1, EPSILON ) ) == 0 )
				continue ;

			unordered_set<Node> new_state( next_state( current, input ) ) ;
			if( !isPresent( seen, new_state ) )
			{
				frontier.push_back( new_state ) ;
				seen.push_back( new_state ) ;

				if( is_final_state( new_state ) )
					dfa.add_final_state( new_state ) ;
			}

			if( input.compare( string( 1, ANY ) ) )
				dfa.set_default_transition( current, new_state ) ;
			else
				dfa.add_transition( current, input, new_state ) ;
		}
	}
	return dfa ;
}

//--------------------------------------------------------------AUTOMATA---------------------------------------------------------------------

// namespace fsa
// {

	NFA levenshtein_automata( string term, uint32_t k  ) 		// Term to evaluate upto k errors
	{
		NFA nfa ; 		// Default constructor with initial state ( 0, 0 )
		uint32_t termsize = term.size() ;
		
		for( uint32_t i = 0 ; i < termsize ; ++i )
		{
			for( uint32_t e = 0 ; e <= k ; ++e )
			{
				// Correct Character
				Node src = node( i,e ) ;
				Node dest = node( i + 1, e ) ;

				nfa.add_transition( src, string(1,term[ i ]), dest ) ;

				if( e < k )
				{
					// Deletion
					Node del = node( i, (e + 1) ) ;
					nfa.add_transition( src, string(1,NFA::ANY), del ) ;

					// Insertion
					Node ins = ( (i + 1), (e + 1) ) ;
					nfa.add_transition( src, string(1,NFA::EPSILON), ins ) ;

					// Substitution
					nfa.add_transition( src, string(1,NFA::ANY), ins ) ;
				}
			}
		}

		for( uint32_t e = 0 ; e <= k ; ++e )
		{
			if( e < k )
			{
				Node src = node( termsize, e ) ;
				Node dest = node( termsize , (e + 1) ) ;
				nfa.add_transition( src, string(1,NFA::ANY), dest ) ; 
			}
			Node final = node( termsize, e ) ;
			nfa.add_final_state( final ) ;
		}

		return nfa ;
	}

	// function pointer: A single argument function that returns the first word in the database that is greater than or equal to the input argument
	// Returns: Every matched word from edit distance 3 from the database
	vector<string> find_all_matches( string word, uint32_t k, auto &m/*std::string( *nextinput ) ( const std::string& inputstring )*/ )
	{
		vector<string> matches ;
		DFA lev = levenshtein_automata( word, k ).to_dfa() ;
		string match = lev.next_valid_string('\0') ; 			// Can start with ^ for regex
		while( match.size() > 0 )
		{
			string next = m.nextinput( match ) ;
			if( next.empty() )
				break ;
			if( match == next )
			{
				matches.push_back( match ) ;
				next += '\0' ; 
			}
			match = lev.next_valid_string( next ) ;
		}
		return matches ;
	}
// }
