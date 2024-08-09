#ifndef __UTILS_H__
#define __UTILS_H__

#include <cstdio>
#include <cassert>

// -------------------- Log --------------------

#define ANSI_FG_BLACK   "\33[1;30m"
#define ANSI_FG_RED     "\33[1;31m"
#define ANSI_FG_GREEN   "\33[1;32m"
#define ANSI_FG_YELLOW  "\33[1;33m"
#define ANSI_FG_BLUE    "\33[1;34m"
#define ANSI_FG_MAGENTA "\33[1;35m"
#define ANSI_FG_CYAN    "\33[1;36m"
#define ANSI_FG_WHITE   "\33[1;37m"
#define ANSI_BG_BLACK   "\33[1;40m"
#define ANSI_BG_RED     "\33[1;41m"
#define ANSI_BG_GREEN   "\33[1;42m"
#define ANSI_BG_YELLOW  "\33[1;43m"
#define ANSI_BG_BLUE    "\33[1;44m"
#define ANSI_BG_MAGENTA "\33[1;35m"
#define ANSI_BG_CYAN    "\33[1;46m"
#define ANSI_BG_WHITE   "\33[1;47m"
#define ANSI_NONE       "\33[0m"

#define ANSI_FMT(str, fmt) fmt str ANSI_NONE

// 输出log到log文件，原样输出
#define Log_write(...) \
  do { \
    extern FILE *log_fp; \
    if (log_fp) { \
      fprintf(log_fp, __VA_ARGS__); \
      fflush(log_fp); \
    } \
  } while (0)

// 输出log到stdout和log文件，原样输出
#define _Log(...) \
  do { \
    printf(__VA_ARGS__); \
    Log_write(__VA_ARGS__); \
  } while (0)

// 输出log到stdout和log文件，带有位置指示和颜色
#define Log(format, ...) \
  _Log(ANSI_FMT("[%s:%d %s] " format, ANSI_FG_BLUE) "\n", \
  __FILE__, __LINE__, __func__, ## __VA_ARGS__)

#define Assert(cond, format, ...) \
  do { \
    if (!(cond)) { \
      fflush(stdout); \
      fprintf(stderr, ANSI_FMT(format, ANSI_FG_RED) "\n", ##  __VA_ARGS__); \
      extern FILE *log_fp; \
      if (log_fp) fflush(log_fp); \
      assert(cond); \
    } \
  } while (0)

// -------------------- state --------------------

class GlobalState {
  private:
    int state;
    int halt_ret;

  public:
    enum {
      ST_RUNNING,
      ST_STOP,
      ST_END,
      ST_ABORT,
      ST_QUIT
    };

    void set_state(int state);
    void set_ret(int ret);
    int get_state();
    int get_ret();
    int exit_code();
};

extern GlobalState emu_state;

#endif
