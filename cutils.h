#ifndef CUTILS_H 
#define CUTILS_H 

int strstart(const char *str, const char *val, const char **ptr);
int stristart(const char *str, const char *val, const char **ptr);
void pstrcpy(char *buf, int buf_size, const char *str);
char *pstrcat(char *buf, int buf_size, const char *s);


#endif 

