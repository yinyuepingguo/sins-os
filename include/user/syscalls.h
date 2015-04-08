#ifndef _USER_SYSCALLS_H
#define _USER_SYSCALLS_H

#include <types.h>
#include <sins/signal.h>
#include <sins/fs.h>
#include <sins/time.h>

extern int open(const char * filename, int flags);
extern int creat(const char * pathname);
extern int close(unsigned int fd);
extern ssize_t read(unsigned int, byte  *, size_t);
extern ssize_t write(unsigned int, const byte  *, size_t);
extern result_t ioctl(unsigned int,
	unsigned long, unsigned long );
extern loff_t lseek(unsigned int, loff_t, int);

extern int exec(const char * filename,
	unsigned long argc, const char * argv[]);
extern void exit(result_t code);
extern result_t wait_pid(unsigned long pid);
extern result_t wait_chld(unsigned long *pid);
extern result_t chdir(const char *filename);
extern result_t chroot(const char *filename);
extern result_t getcwd(char *path, unsigned long len);
extern int dup(unsigned int old_fd);
extern int dup2(unsigned int old_fd, unsigned int new_fd);
extern ssize_t readdir(unsigned int fd, struct dir *buf, size_t size);
extern unsigned long sigreturn();
extern result_t sigaction(unsigned long signal, sig_handler_t s_handler);
extern result_t sigmask(unsigned long signal, int mask);
extern result_t kill(unsigned long pid, unsigned long signal);
extern unsigned long getpid();
extern unsigned long getppid();
extern time_t time(struct tm *tm);

#endif
