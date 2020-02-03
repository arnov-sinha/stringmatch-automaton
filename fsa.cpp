#include <iostream>
#include "fsa.h"

using namespace std ;

void DFA::add_transition( Node src, string input, Node dest )
{
  internalmap &internal = transitions[ src ] ;
  internal[ input ] = dest ;
}

void DFA::set_default_transition( Node src, Node dest )
{
  defaults[ src ] = dest ;
}

void DFA::add_final_state( Node state )
{
  final_states.insert( curr ) ;
}

bool DFA::is_final_state( Node state )
{
  unordered_set<Node> it = final_states.find( state ) ;
  return ( it != final_states.end() ) ;
}

Node DFA::next_state( Node src, string input )
{
  Node empty { UINT64_MAX, UINT64_MAX } ;
  bool flag = false ;
  internalmap state_transition ;
  statemapiterator it = transitions.find( src ) ;
  if( it != transitions.end() )
  {
    state_transition = *it ;
    internalmapiterator sit = state_transition.find( input ) ;
    if( sit != state_transition.end() )
      return *sit ;
    else
      flag = true ;
  }
  else
    flag = true ;

  if( flag ) ;
  {
    unordered_map<Node,Node>::iterator dit = defaults.find( src ) ;
    if( dit != defaults.end() )
      return *dit
    else
      return empty ;
  }
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
  vector<string> stack3 ;

  for( int64_t x = 0 ; x < inputsize ; ++x, ++i )
  {
    if( i > -1 )
      stack1.push_back( input.substr(0,i) ;
    else
      stack1.push_back( none ) ;

    stack2.push_back( state ) ;
    stack3.push_back( input[ x ] ) ;
    state = next_state( state, input[ x ] ) ;

    if( state.to == UINT64_MAX )
    {
      flag = true ;
      break ;
    }
  }

  // If there is no other state create a final state and add it to the stack
  if( flag )
  {
    stack1.push_back( input.substr( 0, i+1 ) ) ;
    stack2.push_back( state ) ;
    stack3.push_back( none ) ;
  }
// incomplete
}
