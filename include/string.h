#ifndef _STRING_H_
#define _STRING_H_

/* We don't want strings.h stuff being user by user stuff by accident */

#include <types.h>	/* for size_t */
#include <stddef.h>	/* for NULL */
#include <stdarg.h>

extern char * ___strtok;
extern char * strpbrk(const char *,const char *);
extern char * strtok(char *,const char *);
extern char * strsep(char **,const char *);
extern size_t strspn(const char *,const char *);


extern char * strcpy(char *,const char *);
extern char * strncpy(char *,const char *, size_t);
extern char * strcat(char *, const char *);
extern char * strncat(char *, const char *, size_t);
extern int strcmp(const char *,const char *);
extern int strncmp(const char *,const char *, size_t);
extern int strnicmp(const char *, const char *, size_t);
extern char * strchr(const char *,int);
extern char * strrchr(const char *,int);
extern char * strstr(const char *,const char *);
extern size_t strlen(const char *);
extern size_t strnlen(const char *,size_t);
extern void * memset(void *,int,size_t);
extern void * memcpy(void *,const void *,size_t);
extern void * memmove(void *,const void *,size_t);
extern void * memscan(void *,int,size_t);
extern int memcmp(const void *,const void *,size_t);
extern void * memchr(const void *,int,size_t);

extern unsigned long simple_strtoul(const char *,char **,unsigned int);
extern long simple_strtol(const char *,char **,unsigned int);
extern unsigned long long simple_strtoull(const char *,char **,unsigned int);
extern long long simple_strtoll(const char *,char **,unsigned int);

extern int vsprintf(char *buf, const char *, va_list);
extern int sprintf(char *buf, const char *fmt, ...);

extern int vsscanf(const char *buf, const char *fmt, va_list);
extern int sscanf(const char *buf, const char *fmt, ...);

#endif /* _STRING_H_ */
