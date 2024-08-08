#include "utils.h"

#include <cstdio>
#include <cstdlib>
#include <getopt.h>

static char *log_file = nullptr;
static char *img_file = nullptr;

static bool batch_mode = false;

static void parse_args(int argc, char *argv[]) {
  const struct option table[] = {
    {"batch"  , no_argument      , nullptr, 'b'},
    {"log"    , required_argument, nullptr, 'l'},
    {"help"   , no_argument      , nullptr, 'h'},
    {nullptr  , 0                , nullptr,  0 },
  };
  int o;
  while ((o = getopt_long(argc, argv, "-bhl:", table, nullptr)) != -1) {
    switch (o) {
      case 'l': log_file = optarg; break;
      case 'b': batch_mode = true; break;
      case 1: img_file = optarg; return;
      default:
        printf("Usage: %s [OPTION...] IMAGE [args]\n\n", argv[0]);
        printf("\t-b,--batch              run with batch mode\n");
        printf("\t-l,--log=FILE           output log to FILE\n");
        printf("\n");
        exit(0);
    }
  }
}

void init_log(const char *filename);
void init_soc();
void load_img(const char *filename);

int main(int argc, char *argv[]) {
  parse_args(argc, argv);

  init_log(log_file);
  init_soc();

  load_img(img_file);

  Log("Hello, rvemu!");
  return 0;
}
