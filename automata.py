import bisect

class NFA(object):
  EPSILON = object()
  ANY = object()
  
  def __init__(self, start_state):
    self.transitions = {}
    self.final_states = set()
    self._start_state = start_state
  
  @property
  def start_state(self):
    return frozenset(self._expand(set([self._start_state])))
  
  def add_transition(self, src, input, dest):
    self.transitions.setdefault(src, {}).setdefault(input, set()).add(dest)

  def add_final_state(self, state):
    self.final_states.add(state)
  
  def is_final(self, states):
    return self.final_states.intersection(states)
  
  def _expand(self, states):
    # print("---------------------------------EXPAND--------------------------------------")
    # print( "states ->", type(states) )
    # print( states )
    frontier = set(states)
    # print( "frontier ->", type(frontier) )
    # print(frontier)
    while frontier:
      state = frontier.pop()
      # print("State popped ->", state )
      new_states = self.transitions.get(state, {}).get(NFA.EPSILON, set()).difference(states)
      # print( "Set_difference ->\n" )
      # print("\t\t",new_states)
      frontier.update(new_states)
      states.update(new_states)
      # print("-----------------------------------------------------------------------------")
    return states
  
  def next_state(self, states, input):
    dest_states = set()
    for state in states:
      state_transitions = self.transitions.get(state, {})
      dest_states.update(state_transitions.get(input, []))
      dest_states.update(state_transitions.get(NFA.ANY, []))
    return frozenset(self._expand(dest_states))
  
  def get_inputs(self, states):
    inputs = set()
    for state in states:
      inputs.update(self.transitions.get(state, {}).keys())
    return inputs
  
  def to_dfa(self):
    dfa = DFA(self.start_state)
    frontier = [self.start_state]
    seen = set()
    while frontier:
      # print "\n-----------------------------Frontier-----------------------------"
      # print frontier
      current = frontier.pop()
      # print "POPPED=> ",current
      # print "------------------------------------------------------------------"
      inputs = self.get_inputs(current)
      # print "\n-----------------------------",inputs,"-----------------------------------"
      for input in inputs:
        # print "\n\nEVALUATING-> \"",input,"\""
        if input == NFA.EPSILON: continue
        new_state = self.next_state(current, input)
        # print "\t\tnext_state-> ",new_state
        if new_state not in seen:
          # print "\t\tNot in seen -> true"
          # print "Appending FRONTIER with new_state above "
          frontier.append(new_state)
          seen.add(new_state)
          if self.is_final(new_state):
            dfa.add_final_state(new_state)
        if input == NFA.ANY:
          # print "\t\tAdding Default transitions from ",current," -> ",new_state
          dfa.set_default_transition(current, new_state)
        else:
          # print "\t\tAdding transitions from ",current," with \"",input,"\" -> ",new_state
          dfa.add_transition(current, input, new_state)
    # print "-----------------------------DFA-----------------------------------"
    # for k,v in dfa.transitions.items():
    #   print k, " -> "
    #   for key,value in v.items():
    #     print "\t\t\t",key," -> ",value
    return dfa


class DFA(object):
  def __init__(self, start_state):
    self.start_state = start_state
    self.transitions = {}
    self.defaults = {}
    self.final_states = set()
  
  def add_transition(self, src, input, dest):
    self.transitions.setdefault(src, {})[input] = dest
  
  def set_default_transition(self, src, dest):
    self.defaults[src] = dest
  
  def add_final_state(self, state):
    self.final_states.add(state)

  def is_final(self, state):
    return state in self.final_states
  
  def next_state(self, src, input):
    # print("    ->next_state->Input-> ",input)
    state_transitions = self.transitions.get(src, {})
    # print( "    ->next_state->Return->",state_transitions.get(input, self.defaults.get(src, None)) )
    return state_transitions.get(input, self.defaults.get(src, None))

  def next_valid_string(self, input):
    print "next_valid_string::Input -> ", input 
    state = self.start_state
    print "next_valid_string::start_state-> ", state
    stack = []
    # Evaluate the DFA as far as possible
    for i, x in enumerate(input):
      print "next_valid_string::i-> ",i
      print "next_valid_string::x-> ",x
      stack.append((input[:i], state, x))
      print "next_valid_string::Appended to Stack1-> ", input[:1]
      print "next_valid_string::Appended to Stack2-> ",state
      print "next_valid_string::Appended to Stack3-> ",x
      state = self.next_state(state, x)
      print "next_valid_string::next_state-> ",state
      if not state: break
    else:
      stack.append((input[:i+1], state, None))
      print "next_valid_string::Final appended to Stack1-> ", input[:i+1]
      print "next_valid_string::Final appended to Stack2-> ",state
      print "next_valid_string::Final appended to Stack3-> ",None

    print "\nnext_valid_string::Stack after Step 1: ",stack

    if self.is_final(state):
      # Input word is already valid
      print  "next_valid_string::is_final::Return-> ",input 
      return input
    
    print "\n"

    # Perform a 'wall following' search for the lexicographically smallest
    # accepting state.
    while stack:
      print "\n--------------------While( Stack size: ",len(stack)," ----------------------"
      print "\t\tBefore::Stack-> ",stack
      path, state, x = stack.pop()
      print "\t\tPopped::Stack1-> ",path
      print "\t\tPopped::Stack2-> ",state
      print "\t\tPopped::Stack3-> ",x

      x = self.find_next_edge(state, x)
      print "\t\tnext_valid_string::findnextedge-> ",x
      print "\t\tx ==",x
      if x:
        path += x
        print "\t\tPath-> ",path
        state = self.next_state(state, x)
        print "\t\tlstate -> ",state
        if self.is_final(state):
          print "\t\tReturn-> ",path
          return path
        stack.append((path, state, None))
        print "\t\tFinal Stack1-> ",path
        print "\t\tFinal Stack2-> ",state
        print "\t\tFinal Stack3-> None"
    print "\t\tReturning None"
    return None

  def find_next_edge(self, s, x):
    if x is None:
      x = u'\0'
    else:
      print "Before x => ",x
      x = unichr(ord(x) + 1)
      print "After x => ",x
    state_transitions = self.transitions.get(s, {})
    if x in state_transitions or s in self.defaults:
      print "find_next_edge::Return x -> ",x
      return x
    labels = sorted(state_transitions.keys())
    pos = bisect.bisect_left(labels, x)
    if pos < len(labels):
      print "find_next_edge::Return label[pos] -> ",labels[ pos ]
      return labels[pos]
    return None
    

def levenshtein_automata(term, k):
  nfa = NFA((0, 0))
  for i, c in enumerate(term):
    for e in range(k + 1):
      # Correct character
      nfa.add_transition((i, e), c, (i + 1, e))
      if e < k:
        # Deletion
        nfa.add_transition((i, e), NFA.ANY, (i, e + 1))
        # Insertion
        nfa.add_transition((i, e), NFA.EPSILON, (i + 1, e + 1))
        # Substitution
        nfa.add_transition((i, e), NFA.ANY, (i + 1, e + 1))
  for e in range(k + 1):
    if e < k:
      nfa.add_transition((len(term), e), NFA.ANY, (len(term), e + 1))
    nfa.add_final_state((len(term), e))
  return nfa


def find_all_matches(word, k, lookup_func):
  """Uses lookup_func to find all words within levenshtein distance k of word.
  
  Args:
    word: The word to look up
    k: Maximum edit distance
    lookup_func: A single argument function that returns the first word in the
      database that is greater than or equal to the input argument.
  Yields:
    Every matching word within levenshtein distance k from the database.
  """
  lev = levenshtein_automata(word, k).to_dfa()
  match = lev.next_valid_string(u'\0')
  print("DEBUG:find_all_matches::Found next valid string -> ",match)
  while match:
    next = lookup_func(match)
    if not next:
      return
    if match == next:
      yield match
      next = next + u'\0'
    match = lev.next_valid_string(next)
    print("DEBUG:find_all_matches::Found next valid string -> ",match)