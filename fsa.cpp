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
	// cout<<"\n\t\tnext_state->Input-> "<<input<<endl ;
  	unordered_set<Node> empty ;
  	bool flag = false ;

  	size_t key = hashonset( src ) ;

  	smapiterator it = transitions.find( key ) ;					// Find the src in the transition table
  	if( it != transitions.end() )
  	{

    	internalmap state_transition = it->second ;
    	internalmapiterator sit = state_transition.find( input ) ;
    	if( sit != state_transition.end() )
    	{
    		// cout<<"\t\tnext_state->Return1-> " ;
    		for( auto &e : sit->second )
				cout<<"("<<(e>>32)<<","<<(e&0xffff)<<") \t" ;
			cout<<endl;
      		return sit->second ;
    	}
    	else
      		flag = true ;
  	}
  	else
    	flag = true ;

  	if( flag ) 													// else, look in defaults, otherwise return empty
  	{
    	unordered_map<size_t , unordered_set<Node> >::iterator dit = defaults.find( key ) ;
    	if( dit != defaults.end() )
    	{
    		// cout<<"\t\tnext_state->Return2-> " ;
    // 		for( auto &e : dit->second )
				// 	cout<<"("<<(e>>32)<<","<<(e&0xffff)<<") \t" ;
				// cout<<endl;
      	return dit->second ;
    	}
    }
    // cout<<"\t\tnext_state->Return-> None"<<endl ;
		return empty ;
}

string DFA::find_next_edge( const unordered_set<Node> &s, string x )		// Gives the position of the lexicographically lower edge
{
	string empty("") ;
	bool checkdefaults = false ;
	internalmap state_transition ;
	size_t key = hashonset( s ) ;

	cout<<"find_next_edge::Before x.size("<<x.size()<<") => "<<x<<endl ;

	if( x.empty() )				// if no next edge add, a edge with '\0', to denote end
	{
		cout<<"find_next_edge::x => '0'"<<endl ;
		x = '\0' ;
	}
	else
	{
		cout<<"find_next_edge::x => + 1"<<endl ;
		char c = x[ 0 ] ;		// change x to the next character for the next edge
		++c ;
		x = c ;
	}

	cout<<"find_next_edge::After x => "<<x<<endl ;

	cout<<"\n\t-----------------DEFAULTS-----------------"<<endl ;

	for( auto &d : def )
	{
		cout<<"\tKey:"<<endl ;
		for( auto &e : d.second )
				cout<<"\t("<<src(e)<<","<<dest(e)<<")" ;

		size_t key = d.first ;

		unordered_map<size_t, unordered_set<Node> >::iterator xit = defaults.find( key ) ;
		if( xit != defaults.end() )
		{
			cout<<"\n\tValue:"<<endl ;
			for( auto &e : xit->second )
				cout<<"\t("<<src(e)<<","<<dest(e)<<")" ;				
		}
		cout<<endl<<endl ;
	}

	cout<<"\n\t-----------------DEFAULTS-----------------"<<endl ;

	smapiterator it = transitions.find( key ) ;
	if( it != transitions.end() )
	{
		state_transition = it->second ;
		internalmapiterator sit = state_transition.find( x ) ;
		if( sit != state_transition.end() )
		{
			cout<<"find_next_edge::Returning from internalmap"<<endl ;
			return x ;
		}
		else
			checkdefaults = true ;
	}
	else
		checkdefaults = true ;

	if( checkdefaults )
	{
		unordered_map<size_t, unordered_set<Node> >::iterator dit = defaults.find( key ) ;
		if( dit != defaults.end() )
		{
			cout<<"find_next_edge::Returning from defaults"<<endl ;
			if( x == string(1,'\0') )
  				cout<<"find_next_edge::x == 0 "<<endl ;
			return x ;
		}
	}

	vector<string> labels = getkeys( state_transition ) ;
	std::sort( labels.begin(), labels.end() ) ;
	vector<string>::iterator vit = std::lower_bound( labels.begin(), labels.end(), x ) ;
	uint64_t pos = vit - labels.begin() ;

	if( pos < labels.size() )
	{
		// cout<<"find_next_edge::Returning labels[pos]=> "<<labels[pos]<<endl ;
		return labels[ pos ] ;
	}


	// cout<<"FIND_NEXT_EDGE::Returning empty"<<endl ;
	return empty ;
}

string DFA::next_valid_string( string input )
{
	cout<<"next_valid_string::Input -> "<<input<<endl ;
  	string none("") ;
  	bool flag = false ;
  	int64_t inputsize = input.size() ;
  	int64_t i = -1 ;
  	unordered_set<Node> state = start_state ;
  	
  	cout<<"next_valid_string::start_state-> " ;
  	for( auto &e : state )
  		cout<<"("<<src(e)<<","<<dest(e)<<") \t" ;
  	cout<<endl ;

  	// The below vectors can be replaced by vector<std::variant<type1,type2,type3>
  	// for c++17 onwards.
  	vector<string> stack1 ;
  	vector<unordered_set<Node> > stack2 ;
  	vector<string> stack3 ;				// Maybe make it a char?

	for( int64_t x = 0 ; x < inputsize ; ++x, ++i )
  	{
  		cout<<"next_valid_string::i-> "<<i<<endl ;
    	cout<<"next_valid_string::x->"<<input[ x ]<<endl ;
    	
    	if( i > -1 )
      		stack1.push_back( input.substr( 0, i ) ) ;
    	else
      		stack1.push_back( none ) ;

    	stack2.push_back( state ) ;
    	stack3.push_back( string(1,input[ x ]) ) ;

    	cout<<"next_valid_string::Appended to Stack1-> "<<stack1.back()<<endl ;
    	cout<<"next_valid_string::Appended to Stack2-> " ;
    	for( auto &e : stack2.back() )
    		cout<<"("<<src(e)<<","<<dest(e)<<") \t" ;
    	cout<<endl ;
    	cout<<"next_valid_string::Appended to Stack3-> "<<stack3.back()<<endl ;

    	state = next_state( state, string(1,input[ x ]) ) ;

    	cout<<"next_valid_string::next_state-> size =>"<<state.size()<<"  " ;
			for( auto &e : state )
				cout<<"("<<src(e)<<","<<dest(e)<<") \t" ;
			cout<<endl;    	

    	if( state.size() != 0 )			// No where to go
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

		cout<<"next_valid_string::Final append to Stack1-> "<<stack1.back()<<endl ;
    	cout<<"next_valid_string::Final append to Stack2-> " ;
    	for( auto &e : stack2.back() )
    		cout<<"("<<src(e)<<","<<dest(e)<<") \t" ;
    	cout<<endl ;
    	cout<<"next_valid_string::Final append to Stack3-> "<<stack3.back()<<endl ;
  	}

  	cout<<"\nnext_valid_string::Stack after Step 1: "<<endl ;
	  	
  	cout<<"Stack1-> " ;
		for( auto e : stack1 )
			cout<<e<<"\t" ;
		cout<<endl;

		cout<<"Stack2 ( size "<<stack2.size()<<" ) -> " ;
		for( auto k : stack2 )
		{
			for( auto &e : k )
				cout<<"("<<src(e)<<","<<dest(e)<<") \t" ;
			cout<<" | " ;
		}
		cout<<endl;

		cout<<"Stack3-> " ;
		for( auto e : stack1 )
			cout<<e<<"\t" ;
		cout<<endl;

  	if( is_final_state( state ) )		// Input word is already valid as a final state is available
  	{
  		cout<<"next_valid_string::is_final::Return-> "<<input<<endl ;
  		return input ;
  	}

  	while( !stack1.empty() )			// Iterate through till you find a lexicographically smallest accepting state
  	{
  		cout<<"\n--------------------While( Stack size: "<<stack1.size()<<" ----------------------"<<endl ;
  		cout<<"Before::Stack1 -> " ;
  		for( auto e : stack1 )
  			cout<<e<<" " ;
  		cout<<endl ;
  		cout<<"Before::Stack2 -> " ;
  		for( auto k : stack2 )
			{	
				for( auto &e : k )
					cout<<"("<<src(e)<<","<<dest(e)<<") \t" ;
				cout<<" | " ;
			}
			cout<<endl ;
  		cout<<"Before::Stack3 -> " ;
  		for( auto e : stack3 )
  			cout<<e<<" " ;
  		cout<<endl ;

  		string path = stack1.back() ;
  		stack1.pop_back() ;

  		unordered_set<Node> lstate = stack2.back() ;
  		stack2.pop_back() ;

			string x = stack3.back() ;
  		stack3.pop_back() ;

  		cout<<"\nPopped::Stack1 -> "<<path<<endl ;
  		cout<<"Popped::Stack2 -> " ;
  		for( auto &e : lstate )
    			cout<<"("<<src(e)<<","<<dest(e)<<") \t" ;
    	cout<<endl ;
  		cout<<"Popped::Stack3 -> "<<x<<endl ;
  		cout<<endl ;

  		x = find_next_edge( lstate, x ) ;

  		cout<<"next_valid_string::findnextedge -> "<<x<<endl ;

  		cout<<"next_valid_string::x.size() -> "<<x.size()<<" -> " ;
  		if( x == string(1,'\0') )
  			cout<<"x == 0 "<<endl ;
  		else
  			cout<<"x != 0 "<<endl ;

  		if( !x.empty() )
  		{
  			path += x ;
  			cout<<"\tPath-> "<<path ;
  			lstate = next_state( lstate, x ) ;
  			cout<<"\tlstate -> " ;
  			for( auto &e : lstate )
    			cout<<"("<<src(e)<<","<<dest(e)<<") \t" ;
    		cout<<endl ;

  			if( is_final_state( lstate ) )			// TODO: does this correctly validate a empty node? src,dest := UINT64_MAX
  			{
  				cout<<"\t\tReturn-> "<<path<<endl ;
  				return path ;
  			}

  			stack1.push_back( path ) ;
  			stack2.push_back( lstate ) ;
  			stack3.push_back( none ) ;

  			cout<<"\tFinal Stack1-> " ;
  			for( auto e : stack1 )
  				cout<<e<<"\t" ;
  			cout<<endl;

  			cout<<"\tFinal Stack2-> " ;
  			for( auto k : stack2 )
				{		
					for( auto &e : k )
						cout<<"("<<src(e)<<","<<dest(e)<<") \t" ;
					cout<<" | " ;
				}
				cout<<endl ;
  			cout<<"\tFinal Stack3-> None"<<endl ;
  		}
  	}
  	cout<<"Returning None"<<endl ;
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
	// cout<<"\tset_difference:"<<endl ;
	// for( auto e : vec )
	// 	cout<<"\t\t("<<src(e)<<","<<dest(e)<<")"<<endl ;
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
	// cout<<"------------------------------------------------EXPAND----------------------------------------------------"<<endl ;
    unordered_set<Node> frontier(currstate) ;
	while( !frontier.empty() )
	{
		unordered_set<Node> newstates ;
		unordered_set<Node>::iterator it = frontier.begin() ;
		Node state = *it ;
		// cout<<"\n\tState popped -> ("<<src(state)<<","<<dest(state)<<")"<<endl ;
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
	// cout<<"---------------------------------------------------------------------------------------------------------"<<endl ;
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

	// cout<<"NEXT_STATE-> BEFORE EXPAND -> " ;
	// for( auto &e : dest_states  )
	// 	cout<<"("<<src(e)<<","<<dest(e)<<") " ;
	// cout<<endl ;

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

	// std::vector<unordered_set<Node> >::iterator vit = frontier.begin() ; 

	while( !frontier.empty() )
	{
		// cout<<"-----------------------------Frontier-----------------------------"<<endl ;
		// for( auto &f : frontier  )
		// {	
		// 	for( auto &e : f )
		// 		cout<<"("<<src(e)<<","<<dest(e)<<") " ;
		// 	cout<<endl ;
		// }

		// vit = frontier.begin() ;
		// unordered_set<Node> current = *vit ;
		// frontier.erase(vit) ;

		unordered_set<Node> current = frontier.back() ;
		frontier.pop_back() ;
		// cout<<"POPPED=> " ;
		// for( auto &e : current )
		// 	cout<<"("<<src(e)<<","<<dest(e)<<") " ;
		// cout<<"\n------------------------------------------------------------------"<<endl ;
		
		unordered_set<string> inputs( get_inputs( current ) ) ;
		
		// cout<<"\n-----------------------------" ;
		// for( auto &e : inputs )
		// 	cout<<e<<"  " ;
		// cout<<"-----------------------------------";

		unordered_set<string>::const_iterator it ;
		for( it = inputs.begin() ; it != inputs.end() ; ++it )
		{
			const string &input = *it ;

			// cout<<"\n\nEVALUATING-> \""<<input<<"\""<<endl ;

			if( input == EPSILON )
				continue ;

			unordered_set<Node> new_state( next_state( current, input ) ) ;

			// cout<<"\t\tnew_state-> ";
			// for( auto &e : new_state )
			// 	cout<<"("<<src(e)<<","<<dest(e)<<") " ;
			// cout<<endl ;

			// cout<<"\t\tseen-> ";
			// for( auto &s : shash )
			// {
			// 	cout<<"\n\t\t\tKey: "<<s.first<<"\t Value: " ;
			// 	for( auto &e : s.second )
			// 		cout<<"("<<src(e)<<","<<dest(e)<<") " ;
			// 	cout<<endl ;
			// }
			// cout<<endl ;

			// cout<<"\t\tNot in Seen -> "<<std::boolalpha<<(!isPresent( seen, new_state ))<<endl ;

			if( !isPresent( seen, new_state ) )
			{
				// cout<<"Appending FRONTIER with new_state above "<<endl ;
				frontier.push_back( new_state ) ;
				size_t key = hashonset(new_state) ;
				seen.insert( key ) ;
				shash[ key ] = new_state ;

				if( is_final_state( new_state ) )
					dfa.add_final_state( new_state ) ;
			}

			if( input == ANY )
			{
				// cout<<"\t\tAdding Default transitions from " ;
				
				// for( auto &e : current )
				// 	cout<<"("<<src(e)<<","<<dest(e)<<") " ;
				// cout<<" -> " ;

				// for( auto &e : new_state )
				// 	cout<<"("<<src(e)<<","<<dest(e)<<") " ;
				// cout<<endl ;

				dfa.set_default_transition( current, new_state ) ;
			}
			else
			{
				// cout<<"\t\tAdding transitions from " ;
				
				// for( auto &e : current )
				// 	cout<<"("<<src(e)<<","<<dest(e)<<") " ;
				// cout<<" with \""<<input<<"\" -> " ;

				// for( auto &e : new_state )
				// 	cout<<"("<<src(e)<<","<<dest(e)<<") " ;
				// cout<<endl ;

				dfa.add_transition( current, input, new_state ) ;
			}
		}
	}

#ifdef _DBGDFA
	cout<<"-----------------------------DFA-----------------------------------"<<endl;
	smapiterator t = dfa.transitions.begin() ;
	for( ; t != dfa.transitions.end() ; ++t )
	{
		unordered_set<Node> tset = rhash[ t->first ] ;
		cout<<"\nKey :" ;
		for( auto &e : tset )
			cout<<"("<<src(e)<<","<<dest(e)<<") " ;
		cout<<" Value:- \n"<<endl ;

		internalmap i = t->second ;
		internalmapiterator it = i.begin() ;
		for( ; it != i.end() ; ++it )
		{
			cout<<"\t"<<it->first<<" -> " ;
		
		for( auto &e : it->second )
			cout<<"("<<src(e)<<","<<dest(e)<<") " ;
		cout<<endl ;
		}
		cout<<endl ;
	}
	cout<<endl ;
#endif

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

	// function pointer: A single argument function that returns the first word in the database that is greater than or equal to the input argument
	// Returns: Every matched word from edit distance 3 from the database
	vector<string> find_all_matches( string word, uint32_t k )
	{
		struct timespec buildlevenshteintime ;
		vector<string> matches ;
		Matcher m ;
		cout<<makemytimebracketed()<<"Building Levenshtein Automaton..."<<endl ;	
		DFA lev = levenshtein_automata( word, k ).to_dfa() ;
		clock_gettime( CLOCK_REALTIME, &buildlevenshteintime ) ;
		cout<<makemytimebracketed()<<"...Complete("<<compute_elapsed( buildlevenshteintime )<<" seconds)"<<endl<<endl ;
		string match = lev.next_valid_string(string(1,'\0')) ; 			// Can start with ^ for regex
		cout<<"DEBUG:find_all_matches::Found next valid string -> "<<match<<endl ;
		// while( match.size() > 0 )
		// {
		// 	cout<<"INSIDE"<<endl;
			string next = m.nextinput( match ) ;
			cout<<"find_all_matches::NEXT-> "<<next<<endl ;
			if( next.empty() )
			{
				//for debug only!!!!!!!!!!!!!!!!!!
				cout<<"DEBUG exit" <<endl ;
				exit(0) ;
				// break ;
			}
			if( match == next )
			{
				matches.push_back( match ) ;
				next += '\0' ;
			}
			match = lev.next_valid_string( next ) ;
			cout<<"DEBUG:find_all_matches::Found next valid string -> "<<match<<endl ;
		// }
		// cout<<"PROBES: "<<m.getprobes()<<endl ;
		return matches ;
	}
// }

std::string defaultcleaningtool( const std::string &s )
{
    return s ;
}

Matcher::Matcher()
{
	probes = 0 ;
	struct timespec sorttime ;
	struct timespec loadtime ;

	cout<<"\n"<<makemytimebracketed()<<"Loading the corpus..."<<endl ;
	clock_gettime( CLOCK_REALTIME, &loadtime ) ;

	if (FILE *fp = fopen("d2.txt", "r"))
	{
		std::vector<char> v;
		char buf[1024];
		while (size_t len = fread(buf, 1, sizeof(buf), fp))
			v.insert(v.end(), buf, buf + len);
		fclose(fp);

		str.reserve( 300000 ) ;

		string s ;
		uint32_t vsize = v.size() ;
		for( uint32_t i = 0 ; i < vsize ; ++i )
		{
			char ch = v[i] ;
			if( ch != 10 )
				s += ch ;
			else
			{
				str.push_back(s) ;
				s.clear() ;
			}
		}
	}

	if( str.size() == 0 )
	{
		cout<<"ERROR: File loading error"<<endl ;
		exit(0) ;
	}
	cout<<makemytimebracketed()<<"...Complete("<<compute_elapsed( loadtime )<<" seconds)"<<endl<<endl ;

	cout<<"\n"<<makemytimebracketed()<<"Sorting the corpus..."<<endl ;
	clock_gettime( CLOCK_REALTIME, &sorttime ) ;
	std::sort( str.begin(), str.end() ) ;
	cout<<makemytimebracketed()<<"...Complete("<<compute_elapsed( sorttime )<<" seconds)"<<endl<<endl ;
}

std::string Matcher::nextinput( const std::string &s )
{
	std::string empty ;
	++probes ;
	cout<<"nextinput => "<<s<<endl ;
	std::vector<std::string>::const_iterator it = std::lower_bound( str.begin(), str.end(), s ) ;	// Should be already sorted
	uint32_t pos = it - str.begin() - 1;
	cout<<"Iterator == end ?"<<std::boolalpha<<( it == str.end() )<<endl ;
	cout<<"Iterator =>"<<*it<<endl ;
	cout<<"Position =>"<<str[pos]<<endl ;

	cout<<"POS-> "<<pos<<endl ;

	cout<<"Corpus:"<<endl ;
	for( auto e : str )
		cout<<e<<endl ;

	if( pos < str.size() )
	{
		cout<<"nextinput::Returning -> "<<str[ pos ]<<endl<<endl ;
		return str[ pos ] ;
	}
	else
	{
		cout<<"nextinput::Returning -> empty"<<endl<<endl ;
		return empty ;
	}
}

int main()
{
	vector<string> results ;
	results = find_all_matches( "food", 1 ) ;

	cout<<"RESULTS: "<<results.size()<<endl ;
	for( auto e : results )
		cout<<e<<endl ;

	// cout<<std::boolalpha<<assert( results.size() == 21 )<<endl ;

	return 0 ;
}
