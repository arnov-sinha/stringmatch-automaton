#include <iostream>
#include "fsa.h"

using namespace std ;

std::unordered_map<size_t , std::unordered_set<Node> > def ;

DFA::DFA( unordered_set<Node> initstate )
{
	start_state = initstate ;
	transitions.reserve( 200000 ) ;
	defaults.reserve( 200000 ) ;
}

vector<string> DFA::getkeys( const unordered_map<string, unordered_set<Node> > &hashmap )
{
	vector<string> keys ;
	keys.reserve( hashmap.size() ) ;

	unordered_map<string, unordered_set<Node> >::const_iterator it = hashmap.begin() ;
	for( ; it != hashmap.end() ; ++it )
		keys.push_back( it->first ) ;

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
	def[ key ] = src ;
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

  	smapiterator it = transitions.find( key ) ;					// Find the src in the transition table
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

  	if( flag ) 													// else, look in defaults, otherwise return empty
  	{
    	unordered_map<size_t , unordered_set<Node> >::iterator dit = defaults.find( key ) ;
    	if( dit != defaults.end() )
      	return dit->second ;
    }
		return empty ;
}

string DFA::find_next_edge( const unordered_set<Node> &s, string x )		// Gives the position of the lexicographically lower edge
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

	smapiterator it = transitions.find( key ) ;
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
  	int64_t inputsize = input.size() ;
  	uint64_t i = 0 ;
  	unordered_set<Node> state = start_state ;
  	
  	// The below vectors can be replaced by vector<std::variant<type1,type2,type3>
  	// for c++17 onwards.
  	vector<string> stack1 ;
  	vector<unordered_set<Node> > stack2 ;
  	vector<string> stack3 ;

	for( int64_t x = 0 ; x < inputsize ; ++x, ++i )
  	{
    	stack1.push_back( input.substr(0,i) ) ;
    	stack2.push_back( state ) ;
    	stack3.push_back( string(1,input[ x ]) ) ;

    	state = next_state( state, string(1,input[ x ]) ) ;

    	if( state.size() == 0 )			// No where to go
      		break ;
  	}

  	// If there is no other state create a final state and add it to the stack
    stack1.push_back( input.substr( 0, i + 1 ) ) ;
	stack2.push_back( state ) ;
	stack3.push_back( none ) ;

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

const std::string NFA::EPSILON = "&" ;
const std::string NFA::ANY = "#" ;

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
    unordered_set<Node> frontier(currstate) ;
	while( !frontier.empty() )
	{
		unordered_set<Node> newstates ;
		unordered_set<Node>::iterator it = frontier.begin() ;
		Node state = *it ;
		frontier.erase( it ) ;
		statemapiterator tit = transitions.find( state ) ;
		if( tit != transitions.end() )
		{
			internalstateiterator iit = tit->second.find( EPSILON ) ;
			if( iit != tit->second.end() )
				newstates = set_difference( iit->second, currstate ) ;
		}
		frontier.insert( newstates.begin(), newstates.end() ) ;
		currstate.insert( newstates.begin(), newstates.end() ) ;
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

const unordered_set<Node> NFA::next_state( const unordered_set<Node> &currstates, const string &input )
{
	unordered_set<Node> dest_states ;
	unordered_set<Node>::const_iterator it = currstates.begin() ;

	for( ; it != currstates.end() ; ++it )
	{
		internalstate internal ;
		statemapiterator sit = transitions.find( *it ) ;
		if( sit != transitions.end() )
		{
			internal = sit->second ;
			internalstateiterator iit = internal.find( input ) ;
			if( iit != internal.end() )
				dest_states.insert( iit->second.begin(), iit->second.end() ) ;

			internalstateiterator iiit = internal.find( ANY ) ;
			if( iiit != internal.end() )
				dest_states.insert( iiit->second.begin(), iiit->second.end() ) ;
		}
	}
	return expand( dest_states ) ;
}

unordered_set<string> NFA::get_inputs( const unordered_set<Node> &states )
{
	unordered_set<string> inputs ;
	unordered_set<Node>::const_iterator state ;

	for( state = states.begin() ; state != states.end() ; ++state )
	{
		statemapiterator it = transitions.find( *state ) ;
		if( it != transitions.end() )
			for( auto& e : it->second )
				inputs.insert( e.first ) ;
	}
	return inputs ;
}

bool NFA::isPresent( const unordered_set<size_t> &seen, const unordered_set<Node> &set )
{
	size_t key = hashonset(set) ;
	unordered_set<size_t>::const_iterator it = seen.find( key ) ;
	return( it != seen.end() ) ;
}

DFA NFA::to_dfa()			// Write a sample code to check the return type, typedef of class maybe needed
{
	unordered_map<size_t, unordered_set<Node> > shash ;
	unordered_set<Node> n ( get_start_state() ) ;
	vector<unordered_set<Node> > frontier ;
	unordered_set<size_t> seen ;

	DFA dfa( n ) ;
	frontier.push_back( n ) ;

	while( !frontier.empty() )
	{
		unordered_set<Node> current = frontier.back() ;
		frontier.pop_back() ;
		
		unordered_set<string> inputs( get_inputs( current ) ) ;
		
		unordered_set<string>::const_iterator it ;
		for( it = inputs.begin() ; it != inputs.end() ; ++it )
		{
			const string &input = *it ;

			if( input == EPSILON )
				continue ;

			unordered_set<Node> new_state( next_state( current, input ) ) ;
			if( !isPresent( seen, new_state ) )
			{
				frontier.push_back( new_state ) ;
				size_t key = hashonset(new_state) ;
				seen.insert( key ) ;
				shash[ key ] = new_state ;

				if( is_final_state( new_state ) )
					dfa.add_final_state( new_state ) ;
			}

			if( input == ANY )
				dfa.set_default_transition( current, new_state ) ;
			else
				dfa.add_transition( current, input, new_state ) ;
		}
	}
	return dfa ;
}

//--------------------------------------------------------------AUTOMATA---------------------------------------------------------------------


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
				nfa.add_transition( src, NFA::ANY, del ) ;

				// Insertion
				Node ins = node( (i + 1), (e + 1) ) ;
				nfa.add_transition( src, NFA::EPSILON, ins ) ;

				// Substitution
				nfa.add_transition( src, NFA::ANY, ins ) ;
			}
		}
	}

	for( uint32_t e = 0 ; e <= k ; ++e )
	{
		if( e < k )
		{
			Node src = node( termsize, e ) ;
			Node dest = node( termsize , (e + 1) ) ;
			nfa.add_transition( src, NFA::ANY, dest ) ;
		}
		Node final = node( termsize, e ) ;
		nfa.add_final_state( final ) ;
	}
	return nfa ;
}

vector<string> find_all_matches( const string &filename, const string &word, const uint32_t &k )
{
	struct timespec buildlevenshteintime ;
	struct timespec findmatchestime ;
	struct timespec probetime ;
	double totalprobetime = 0 ;
	vector<string> matches ;
	Matcher m( filename ) ;

	cout<<makemytimebracketed()<<"Building Levenshtein Automaton..."<<endl ;	
	clock_gettime( CLOCK_REALTIME, &buildlevenshteintime ) ;
	DFA lev = levenshtein_automata( word, k ).to_dfa() ;
	cout<<makemytimebracketed()<<"...Complete("<<compute_elapsed( buildlevenshteintime )<<" seconds)"<<endl<<endl ;
	
	clock_gettime( CLOCK_REALTIME, &findmatchestime ) ;
	clock_gettime( CLOCK_REALTIME, &probetime ) ;
	string match = lev.next_valid_string(string(1,'\0')) ; 			// Can start with ^ for regex, hmm not sure
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
		totalprobetime += compute_elapsed( probetime ) ;
		clock_gettime( CLOCK_REALTIME, &probetime ) ;
		match = lev.next_valid_string( next ) ;
	}
	cout<<"Time to find potential matches: "<<compute_elapsed( findmatchestime )<<" seconds)"<<endl ;
	cout<<"No of probes: "<<m.getprobes()<<endl ;
	cout<<"Total probe time: "<<totalprobetime <<endl ;
	cout<<"Average probe time: "<<( totalprobetime / m.getprobes() )<<endl ;
	return matches ;
}

std::string defaultcleaningtool( const std::string &s )
{
    return s ;
}

Matcher::Matcher( const string &filename )
{
	probes = 0 ;
	struct timespec sorttime ;
	FileLoader *file = new FileLoader( filename.c_str(), defaultcleaningtool ) ;
	file->copycorpus( str ) ;
	if( str.size() == 0 )
	{
		cout<<"ERROR: File loading error"<<endl ;
		exit(0) ;
	}
	cout<<"\n"<<makemytimebracketed()<<"Sorting the corpus..."<<endl ;
	clock_gettime( CLOCK_REALTIME, &sorttime ) ;
	std::sort( str.begin(), str.end() ) ;
	cout<<makemytimebracketed()<<"...Complete("<<compute_elapsed( sorttime )<<" seconds)"<<endl<<endl ;

}

// Matcher::Matcher()
// {
// 	probes = 0 ;
// 	struct timespec sorttime ;
// 	struct timespec loadtime ;

// 	cout<<"\n"<<makemytimebracketed()<<"Loading the corpus..."<<endl ;
// 	clock_gettime( CLOCK_REALTIME, &loadtime ) ;

// 	if (FILE *fp = fopen("d2.txt", "r"))
// 	{
// 		std::vector<char> v;
// 		char buf[1024];
// 		while (size_t len = fread(buf, 1, sizeof(buf), fp))
// 			v.insert(v.end(), buf, buf + len);
// 		fclose(fp);

// 		str.reserve( 300000 ) ;

// 		string s ;
// 		uint32_t vsize = v.size() ;
// 		for( uint32_t i = 0 ; i < vsize ; ++i )
// 		{
// 			char ch = v[i] ;
// 			if( ch != 10 )
// 				s += ch ;
// 			else
// 			{
// 				str.push_back(s) ;
// 				s.clear() ;
// 			}
// 		}
// 	}

// 	if( str.size() == 0 )
// 	{
// 		cout<<"ERROR: File loading error"<<endl ;
// 		exit(0) ;
// 	}
// 	cout<<makemytimebracketed()<<"...Complete("<<compute_elapsed( loadtime )<<" seconds)"<<endl<<endl ;

// 	cout<<"\n"<<makemytimebracketed()<<"Sorting the corpus..."<<endl ;
// 	clock_gettime( CLOCK_REALTIME, &sorttime ) ;
// 	std::sort( str.begin(), str.end() ) ;
// 	cout<<makemytimebracketed()<<"...Complete("<<compute_elapsed( sorttime )<<" seconds)"<<endl<<endl ;
// }

std::string Matcher::nextinput( const std::string &s )
{
	std::string empty ;
	++probes ;
	std::vector<std::string>::const_iterator it = std::lower_bound( str.begin(), str.end(), s ) ;	// Should be already sorted
	uint32_t pos = it - str.begin() ;
	if( pos < str.size() )
		return str[ pos ] ;
	else
		return empty ;
}

int main( int argc, char** argv )
{
	vector<string> results ;
	string filename ;
	string pattern ;
	uint32_t k ;	// Edit distance, upto k errors
	bool showresult = false ; 

	if( argc > 2 )
	{
		filename = argv[ 1 ] ;
		pattern = argv[ 2 ] ;
		k = stoi(argv[ 3 ]) ;
		showresult = stoi(argv[ 4 ]) ;
	}

	results = find_all_matches( filename, pattern, k ) ;

	cout<<"No of Results:"<<results.size()<<endl ;

	if( showresult )
	{
		cout<<"RESULTS: "<<endl ;
		for( auto e : results )
			cout<<e<<endl ;
	}
	return 0 ;
}
