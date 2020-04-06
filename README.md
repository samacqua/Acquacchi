# SECS - Simple Engine for Chess by Sam
A chess engine written in C during Spring break to help get through Corona Virus quarantine. I've realized I'm not great at chess, so I've tried to pretend I am through this engine (also to learn C).

## Playing Strength
The engine is much better than me (1200ish) and seems to consistently beat Fairy Max (1940) on 1+1 time control. So who knows, but it's solid.

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
    
## Best Resources:
* [Chess Programming Wiki](https://www.chessprogramming.org/Main_Page)
* [Tom's Simple Chess Program (In C)](http://www.tckerrigan.com/Chess/TSCP/)
* [Chess Programming by by François Dominic Laramée](http://archive.gamedev.net/archive/reference/articles/article1014.html)
* [VICE](https://www.youtube.com/playlist?list=PLZ1QII7yudbc-Ky058TEaOstZHVbT-2hg)
* [Bruce Moreland's Programming Topics](https://web.archive.org/web/20071026090003/http://www.brucemo.com/compchess/programming/index.htm)
* [Stockfish](https://github.com/official-stockfish/Stockfish)
* [Writing a Chess Program in 99 Steps](http://web.archive.org/web/20120315032415/http://www.sluijten.com/forum/)
