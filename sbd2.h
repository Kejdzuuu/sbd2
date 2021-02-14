#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cstdint>
#include <time.h>

#define MAX_NUM 0xFFFF //0xFFFFFFFF
#define FIRST_NAME_LEN 12
#define LAST_NAME_LEN FIRST_NAME_LEN
#define MAX_FILE_NAME_LEN 128
#define PAGE_SIZE 4096
#define RECORDS_IN_PAGE 4 //128
#define BUFFERS_AVAILABLE 16
#define MAX_RUN (RECORDS_IN_PAGE * BUFFERS_AVAILABLE)


char index_file_name[] = "index.file";
char overflow_area[] = "overflow.file";

char first_names[][FIRST_NAME_LEN] = {
  "james", "mary", "john", "patricia", "robert", "jennifer",
  "michael", "linda", "william", "elizabeth", "david", "barbara", 
  "maciej", "antoni", "jan", "jakub", "aleksander", "franciszek",
  "julia", "zuzanna", "zofia", "hanna", "maja", "lena", "alicja"
};

char last_names[][LAST_NAME_LEN] = {
  "smith", "johnson", "williams", "jones", "brown", "davis",
  "miller", "wilson", "moore", "taylor", "anderson", "thomas",
  "modrzejewsk", "nowak", "wojcik", "kowalczyk", "wozniak",
  "kowalski", "wisniewski", "kaminski", "lewandowski", "zielinski"
};

#define NELEMS(x)  (sizeof(x) / sizeof((x)[0]))
#define FIRST_NAMES_SIZE NELEMS(first_names)
#define LAST_NAMES_SIZE NELEMS(last_names)

struct Index_Record {
  uint32_t key;
  uint32_t page;
};

struct Record {
  uint32_t key;
  char last_name[LAST_NAME_LEN];
  char first_name[FIRST_NAME_LEN];
  uint32_t overflow_ptr;
};

struct Record_with_index
{
  Record *record;
  int index;
};


void sort_file(const char name[]);

void print_out_file(const char name[]);

int merge_runs(int runs, const char name[]);

char get_user_input();

int get_page_num_from_index(int key);

void create_overlow_area ();

void generate_index_file (char *name);

int disk_reads;
int disk_saves;
int records_read;
int records_saved;
int records_in_overflow_area;