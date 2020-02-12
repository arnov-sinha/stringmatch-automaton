# stringmatch-automaton


Finite State Automaton based string matching. It uses Levenshtein distances to find the approximately close match. Only Insertion, Deletion and Substitution used. Can be augmented for Transposition. 

Arguments to make it run:

1: File location
2: Pattern to match
3: K edit distance ( Ideally 1..4 )
4: Result to be displayed (1: Yes / 0:No)

./fsa /use/share/dict/words wood 2 0
