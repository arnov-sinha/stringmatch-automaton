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

typedef std::unordered_map< uint64_t, std::unordered_set<Node> > internalmap ;
typedef std::unordered_map< Node, internalmap > statemap ;

class NFA
{

} ;

class DFA
{
private:
  statemap transitions ;
  statemap defaults ;
  std::unordered_set<Node> final_states ;
  Node start_state ;

public:
  DFA() ;
  DFA( Node initstate ) ;
  void add_transition( Node src, uint64_t input, Node dest ) ;
  ~DFA() ;
} ;
