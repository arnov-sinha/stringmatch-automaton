#include <unordered_set>
#include <unordered_map>
#include <string>
#include <stdint.h>
#include <vector>

typedef struct Node
{
  uint64_t src ;
  uint64_t dest ;
} Node ;

// Data structure
typedef std::unordered_map<std::string, Node> internalmap ;
typedef std::unordered_map<Node, internalmap> statemap ;

// Iterators
typedef std::unordered_map<Node, internalmap>::iterator statemapiterator ;
typedef std::unordered_map<std::string, Node>::iterator internalmapiterator ;

class NFA
{

} ;

class DFA
{
private:
  statemap transitions ;
  std::unordered_map<Node, Node> defaults ;
  std::unordered_set<Node> final_states ;
  Node start_state ;

public:
  DFA() ;
  DFA( Node initstate ) ;
  void add_transition( Node src, uint64_t input, Node dest ) ;
  void set_default_transition( Node src, Node dest ) ;
  ~DFA() ;
} ;
