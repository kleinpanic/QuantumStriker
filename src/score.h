#ifndef SCORE_H
#define SCORE_H

int load_highscore_for_username(const char *username);
void save_highscore_for_username(const char *username, int score);
char* load_username();
void save_username(const char *username);

#endif

