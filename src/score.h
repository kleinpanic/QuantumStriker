#ifndef SCORE_H
#define SCORE_H

#include "blockchain.h"  // for ScoreBlock

int load_highscore_for_username(const char *username);
void save_highscore_for_username(const char *username, int score);
char* load_username();
void save_username(const char *username);
int get_last_block_for_user(const char *username, ScoreBlock *lastBlock);

#endif

