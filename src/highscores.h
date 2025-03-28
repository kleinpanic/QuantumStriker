/* highscores.h */
#ifndef HIGHSCORES_H
#define HIGHSCORES_H

// Displays the high score table by reading and validating the blockchain.
// It groups entries by the base username (stripping any "DevAI" suffix)
// and then displays up to HIGHSCORE_FLAG_MAX_ENTRY_NUMBER entries.
void display_highscores(void);

#endif // HIGHSCORES_H

