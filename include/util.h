#ifndef CHALLENGE_UTIL_H
#define CHALLENGE_UTIL_H

/* funcão utilidade para adicionar users
* atualmente users são estáticos e não mudam de sessão a sessão 
* e não podem ser adicionados
* em runtime
*/
void add_user(struct user *p_user, char *username);
int check_hello(char* buffer);


#endif