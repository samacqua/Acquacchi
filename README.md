# Acquacchi
A chess engine written in C during Spring break to help get through Coronavirus quarantine. I've realized I'm not great at chess, so I've tried to pretend I am through this engine (also to practice C). The name comes from a mash of Acqua--meaning water in Italian (tangentially related to the custom of naming chess engines after fish and also the beginning of my last name)--and Scacchi--meaning chess in Italian.

## Playing Strength
Acquacchi is much better than me (1200ish) and seems to consistently beat Fairy Max (1940) on 1+1 time control. So there isn't any rigorous controlled testing, but I would guess it is around 2000 ELO.

## Engine Features:
* UCI compatible
* Search
    * Alpha-Beta Search
    * Quiescence Search
    * Transposition Table
    * Null-move pruning
    * Iterative Deepening
    * Killer Heuristic, History Heuristic
* Board Structure
    * 120-square based
    * Limited use of bitboards
* Evaluation
    * Piece Square Tables
    * Evaluates open files, isolated, and passed pawns
    
## Features For Future Versions
* XBoard Compatible (current XBoard compatability is broken)
* Search
    * Parallel Search (Lazy SMP)
    * Faster sorting algorithm for move ordering
    * Handle PV clearance between moves better
    * Principal Variation Search
    * Improve Hash Table
    * Improve Quiescence Search
    * Search Extensions
    * Root Move Ordering
* Move Generation
    * Bitboard Move Generation
* Evaluation
    * Understand Double Pawns
    * Better Endgame Evaluation (only endgame evaluation now is King)
* More Efficient Time Management
* Opening Book and Endgame Tablebase
* When Parsing FEN, Use Half-Move Clock and Full-Move Counter
* Fix malloc Errors
    
    
## Best Resources:
* [Chess Programming Wiki](https://www.chessprogramming.org/Main_Page)
* [Tom's Simple Chess Program (In C)](http://www.tckerrigan.com/Chess/TSCP/)
* [Chess Programming by by François Dominic Laramée](http://archive.gamedev.net/archive/reference/articles/article1014.html)
* [VICE](https://www.youtube.com/playlist?list=PLZ1QII7yudbc-Ky058TEaOstZHVbT-2hg)
* [Bruce Moreland's Programming Topics](https://web.archive.org/web/20071026090003/http://www.brucemo.com/compchess/programming/index.htm)
* [Stockfish](https://github.com/official-stockfish/Stockfish)
* [Writing a Chess Program in 99 Steps](http://web.archive.org/web/20120315032415/http://www.sluijten.com/forum/)
