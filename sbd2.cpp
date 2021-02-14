#include "sbd2.h"

uint32_t generate_number(uint32_t min, uint32_t max) {
  uint32_t delta = max - min;
  return rand()%delta + min;
}

uint32_t generate_number(uint32_t max) {
  return generate_number (1, max);
}

char generate_char() {
  char c = (char)generate_number((int)'a', (int)'z');
  return c;
}

void generate_name(char *f_name, char *l_name) {
  int f_name_index, l_name_index;

  f_name_index = generate_number(0, FIRST_NAMES_SIZE);
  l_name_index = generate_number(0, LAST_NAMES_SIZE);

  memcpy (f_name, first_names[f_name_index], FIRST_NAME_LEN);
  memcpy (l_name, last_names[l_name_index], LAST_NAME_LEN);
}

void generate_random_file(char *file_name) {
  Record record;
  char *first_name = (char *)calloc(FIRST_NAME_LEN + 1, sizeof(char));
  char *last_name = (char *)calloc(LAST_NAME_LEN + 1, sizeof(char));
  int seed;
  time_t t;
  FILE *file;
  int number_of_names;
  
  printf ("How many records?\n");
  int result = scanf ("%d", &number_of_names);
  if (result == EOF) {
    return;
  }

  printf("Generating %d names\n", number_of_names);

  first_name[FIRST_NAME_LEN] = '\0';
  last_name[LAST_NAME_LEN] = '\0';

  file = fopen(file_name, "w");
  
  seed = time(&t);
  srand(seed);

  while (number_of_names > 0) {
    Record records[RECORDS_IN_PAGE];
    memset (&records, 0, sizeof (records));

    for (int i = 0; (i < number_of_names) && (i < RECORDS_IN_PAGE); i++) {
      generate_name(records[i].first_name, records[i].last_name);
      records[i].key = generate_number(MAX_NUM-1);
      records[i].overflow_ptr = MAX_NUM;
    }
    fwrite (&records, sizeof (struct Record), RECORDS_IN_PAGE, file);
    number_of_names -= RECORDS_IN_PAGE;
  }

  fclose(file);
  free(first_name);
  free(last_name);
}


int compare_records(Record *a, Record *b)
{
  if (a->key == 0 && b->key == 0) {
    return 0;
  }
  if (a->key == 0) {
    return 1;
  }
  if (b->key == 0) {
    return -1;
  }
  if (a->key < b->key) {
    return -1;
  } else if (a->key == b->key) {
    return 0;
  } else {
    return 1;
  }
}

void print_out_file(const char name[]) {
  FILE *file, *overflow;
  Record file_records[RECORDS_IN_PAGE];
  Record overflow_records[RECORDS_IN_PAGE];
  int page = 0;
  int records_loaded;
  int file_records_counter = 0;
  bool from_overflow = false;
  Record *record_ptr = NULL;
  int empty_spaces = RECORDS_IN_PAGE;

  file = fopen (name, "r");
  overflow = fopen (overflow_area, "r");

  fread (&overflow_records, sizeof (Record), RECORDS_IN_PAGE, overflow);
  fclose (overflow);

  records_loaded = fread (&file_records, sizeof (Record), RECORDS_IN_PAGE, file);
  record_ptr = &file_records[file_records_counter];
  file_records_counter++;

  printf ("id, first name, last name, page\n");

  if (record_ptr->key != 0) {
    printf ("%d, %s, %s, %d\n", record_ptr->key, record_ptr->first_name, record_ptr->last_name, page);
    empty_spaces--;
  }
  
  while (records_loaded) {
    
    if (record_ptr->overflow_ptr == MAX_NUM || record_ptr->key == 0) {
      record_ptr = &file_records[file_records_counter];
      from_overflow = false;
      file_records_counter++;
    } else {
      record_ptr = &overflow_records[record_ptr->overflow_ptr];
      from_overflow = true;
    }
    
    if (record_ptr->key != 0) {
      if (from_overflow) {
        printf ("%d, %s, %s, Ov\n", record_ptr->key, record_ptr->first_name, record_ptr->last_name);
      } else {
        printf ("%d, %s, %s, %d\n", record_ptr->key, record_ptr->first_name, record_ptr->last_name, page);
        empty_spaces--;
      }
    }

    if (file_records_counter == RECORDS_IN_PAGE) {
      printf ("Empty spaces in previous page: %d\n\n", empty_spaces);
      empty_spaces = RECORDS_IN_PAGE;
      records_loaded = fread (&file_records, sizeof (Record), RECORDS_IN_PAGE, file);
      page++;
      file_records_counter = 0;
      if (records_loaded != RECORDS_IN_PAGE) {
        break;
      }
    }

  }

  fclose (file);
}

void print_records(Record *records, int num) {
  printf("\nfrom mem\n");
  for (int i = 0; i < num; i++) {
    printf("%d, %.12s, %.12s, %x\n", records[i].key, records[i].last_name, records[i].first_name, records[i].overflow_ptr);
  }
}

void swap_records(Record *rec1, Record *rec2) {
  Record tmp;
  memcpy(&tmp, rec1, sizeof(Record));
  memcpy(rec1, rec2, sizeof(Record));
  memcpy(rec2, &tmp, sizeof(Record));
}

void swap_records(Record_with_index *rec1, Record_with_index *rec2) {
  Record_with_index tmp;
  memcpy(&tmp, rec1, sizeof(Record_with_index));
  memcpy(rec1, rec2, sizeof(Record_with_index));
  memcpy(rec2, &tmp, sizeof(Record_with_index));
}

void heapify(Record *records, int size, int root_index) {
  int heap_top_index = root_index;
  int left_child_index = 2 * root_index + 1;
  int right_child_index = 2 * root_index + 2;

  if (left_child_index < size) {
    if (compare_records(&records[left_child_index], &records[heap_top_index]) > 0) {
      heap_top_index = left_child_index;
    }
  }

  if (right_child_index < size) {
    if (compare_records(&records[right_child_index], &records[heap_top_index]) > 0) {
      heap_top_index = right_child_index;
    }
  }

  if (heap_top_index != root_index) {
    swap_records(&records[root_index], &records[heap_top_index]);
    heapify(records, size, heap_top_index);
  }
}

void heap_sort_records(Record *records, int size) {
  for (int i = size / 2 - 1; i >= 0; i--) {
    heapify(records, size, i);
  }

  for (int i = size - 1; i > 0; i--) {
    swap_records (&records[0], &records[i]);
    heapify (records, i, 0);
  }
}

void heapify(Record_with_index *records, int size, int root_index) {
  int heap_top_index = root_index;
  int left_child_index = 2 * root_index + 1;
  int right_child_index = 2 * root_index + 2;

  if (left_child_index < size) {
    if (compare_records((&records[left_child_index])->record, (&records[heap_top_index])->record) < 0) {
      heap_top_index = left_child_index;
    }
  }

  if (right_child_index < size) {
    if (compare_records((&records[right_child_index])->record, (&records[heap_top_index])->record) < 0) {
      heap_top_index = right_child_index;
    }
  }

  if (heap_top_index != root_index) {
    swap_records(&records[root_index], &records[heap_top_index]);
    heapify(records, size, heap_top_index);
  }
}

void heap_insert(Record_with_index *records, int size, int index) {
  if (size == 1 || index == 0) {
    return;
  }
  int parent_index = (index - 1) / 2;

  if (compare_records((&records[index])->record, (&records[parent_index])->record) < 0) {
    swap_records(&records[parent_index], &records[index]);
    heapify(records, size, parent_index);
  }
}

void save_run(Record *run, int size, int n) {
  FILE *file;

  char name[] = "run000";
  name[3] = '0' + n / 100;
  name[4] = '0' + (n % 100) / 10;
  name[5] = '0' + n % 10;

  file = fopen(name, "w");

  fwrite(run, sizeof(Record), size, file);
  disk_saves += size / RECORDS_IN_PAGE;
  if (size % RECORDS_IN_PAGE) {
    disk_saves++;
  }

  fclose(file);
}

void sort_file(const char name[]) {
  FILE *file;
  Record records[MAX_RUN];
  int runs = 0;
  int sorts = 0;
  char c;

  disk_reads = 0;
  disk_saves = 0;

  printf ("Print file? [Y/n] ");
  c = get_user_input ();
  if (c == 'Y' || c == 'y') {
    print_out_file (name);
    printf ("Press any key...\n");
    get_user_input ();
  }

  file = fopen(name, "r");

  while (1) {
    int read_records;

    read_records = fread(records, sizeof(Record), MAX_RUN, file);
    if (!read_records) {
      break;
    }
    disk_reads += read_records / RECORDS_IN_PAGE;
    if (read_records % RECORDS_IN_PAGE) {
      disk_reads++;
    }

    heap_sort_records(records, read_records);
    save_run(records, read_records, runs);
    runs++;
    if(runs == 999) {
      printf ("Too many runs!\n");
      printf ("Press any key...\n");
      get_user_input ();
      return;
    }
  }
  
  fclose(file);

  do {
    runs = merge_runs(runs, name);
    sorts++;
    if (runs) {
      printf("%d sort cycles done. Print tapes? [Y/n]", sorts);
      c = get_user_input ();
      if (c == 'Y' || c == 'y') {
        for (int i = 0; i < runs; i++) {
          char run_name[] = "run000";
          run_name[3] = '0' + i / 100;
          run_name[4] = '0' + (i % 100) / 10;
          run_name[5] = '0' + i % 10;
          print_out_file (run_name);
        }
        printf ("Press any key...\n");
        get_user_input ();
      }
    }
  } while (runs);

  printf("Print sorted file? [Y/n]");
  c = get_user_input ();
  if (c == 'Y' || c == 'y') {
    print_out_file (name);
  }

  printf("Sort cycles: %d\n", sorts);
  printf("Pages read: %d\n", disk_reads);
  printf("Pages saved: %d\n", disk_saves);
  printf ("Press any key...\n");
  get_user_input ();
}

// K-way merge (min-heap)
int merge_runs(int run_count, const char file_name[]) {
  FILE *in_files[BUFFERS_AVAILABLE - 1];
  FILE *out_file;
  Record input_records[RECORDS_IN_PAGE * (BUFFERS_AVAILABLE - 1)];
  int records_left_in_run[BUFFERS_AVAILABLE - 1];
  int current_run_index[BUFFERS_AVAILABLE - 1];
  int runs_left = run_count;
  int runs_loaded = 0;
  int merge_count = 0;
  run_count = 0;
  Record inf_record;
  inf_record.last_name[0] = 0xff;

  // keep loading runs until none left
  while (runs_left) {
    char out_name[] = "mrg000";
    out_name[3] = '0' + run_count / 100;
    out_name[4] = '0' + (run_count % 100) / 10;
    out_name[5] = '0' + run_count % 10;

    run_count++;

    if (runs_left == 1) {
      char run_name[] = "run000";
      int run_num = merge_count * (BUFFERS_AVAILABLE - 1);
      run_name[3] = '0' + run_num / 100;
      run_name[4] = '0' + (run_num % 100) / 10;
      run_name[5] = '0' + run_num % 10;
      remove(out_name);
      rename(run_name, out_name);
      merge_count++;
      break;
    }

    out_file = fopen(out_name, "w");

    if (runs_left > (BUFFERS_AVAILABLE - 1)) {
      runs_loaded = BUFFERS_AVAILABLE - 1;
    }
    else {
      runs_loaded = runs_left;
    }

    for (int i = 0; i < (BUFFERS_AVAILABLE - 1), i < runs_loaded; i++) {
      int run_num = merge_count * (BUFFERS_AVAILABLE - 1) + i;
      char name[] = "run000";
      name[3] = '0' + run_num / 100;
      name[4] = '0' + (run_num % 100) / 10;
      name[5] = '0' + run_num % 10;
      in_files[i] = fopen(name, "r");
    }

    int records_loaded = 0;
    bool records_left;
    int heap_size = 0;

    Record_with_index output_records[BUFFERS_AVAILABLE - 1];
    
    records_left = false;
    // keep loading page of records from each run until no more left
    for (int i = 0; i < runs_loaded; i++) {

      records_loaded = fread(&input_records[i * RECORDS_IN_PAGE], sizeof(Record), RECORDS_IN_PAGE, in_files[i]);  // part of run loaded
      records_left_in_run[i] = records_loaded;  // records loaded from run
      current_run_index[i] = 0;
      if (records_loaded) {
        disk_reads++;
      }
    }

    for (int i = 0; i < runs_loaded; i++) {
      output_records[i].record = &input_records[i * RECORDS_IN_PAGE];
      output_records[i].index = i;
      heap_size++;
      heap_insert(output_records, heap_size, i);
    }

    Record ordered_records[RECORDS_IN_PAGE];
    int ordered_records_index = 0;

    while (heap_size) {

      memcpy(&ordered_records[ordered_records_index], output_records->record, sizeof(Record));
      ordered_records_index++;

      current_run_index[output_records->index]++;
      records_left_in_run[output_records->index]--;
      if (records_left_in_run[output_records->index] > 0) {
        output_records[0].record++;
        heapify(output_records, heap_size, 0);
      } else {
        // run out of records in run, load more
        int i = output_records->index;
        records_loaded = fread(&input_records[i * RECORDS_IN_PAGE], sizeof(Record), RECORDS_IN_PAGE, in_files[i]);  // part of run loaded
        records_left_in_run[i] = records_loaded;  // records loaded from run
        current_run_index[i] = 0;
        if (records_loaded) {
          disk_reads++;
        }

        if (records_left_in_run[output_records->index] > 0) {
          output_records[0].record = &input_records[i * RECORDS_IN_PAGE];
          heapify(output_records, heap_size, 0);
        } else {
          heap_size--;
          swap_records(&output_records[0], &output_records[heap_size]);
          heapify(output_records, heap_size, 0);
        }
      }

      if (ordered_records_index == RECORDS_IN_PAGE || heap_size == 0) {
        fwrite(ordered_records, sizeof(Record), ordered_records_index, out_file);
        ordered_records_index = 0;
        disk_saves++;
      }
    }

    for (int i = 0; i < runs_loaded; i++) {
      fclose(in_files[i]);
      int run_num = merge_count * (BUFFERS_AVAILABLE - 1) + i;
      char name[] = "run000";
      name[3] = '0' + run_num / 100;
      name[4] = '0' + (run_num % 100) / 10;
      name[5] = '0' + run_num % 10;
      remove(name);
    }

    fclose(out_file);
    runs_left = runs_left - runs_loaded;
    merge_count++;
  }

  if (merge_count > 1) {
    for (int i = 0; i < merge_count; i++) {
      char merge_name[] = "mrg000";
      merge_name[3] = '0' + i / 100;
      merge_name[4] = '0' + (i % 100) / 10;
      merge_name[5] = '0' + i % 10;
      char run_name[] = "run000";
      run_name[3] = '0' + i / 100;
      run_name[4] = '0' + (i % 100) / 10;
      run_name[5] = '0' + i % 10;

      remove(run_name);
      rename(merge_name, run_name);
    }
    return merge_count;
  }
  else {
    for(int i = 0; i < runs_loaded; i++) {
      char run_name[] = "run000";
      run_name[3] = '0' + i / 100;
      run_name[4] = '0' + (i % 100) / 10;
      run_name[5] = '0' + i % 10;

      remove(run_name);
    }
    remove(file_name);
    rename("mrg000", file_name);
    return 0;
  }
}

char get_user_input() {
  char c = getchar ();
  if (c == '\n')
    return getchar();
  return c;
}

void take_user_input (Record *record) {

  memset (&record->last_name, 0, LAST_NAME_LEN);
  memset (&record->first_name, 0, FIRST_NAME_LEN);
  printf ("Key: ");
  scanf ("%d", &record->key);
  printf ("Last name: ");
  scanf("%s", &record->last_name);
  printf ("First name: ");
  scanf("%s", &record->first_name);
  record->overflow_ptr = MAX_NUM;
}

void generate_index_file (char *file_name) {
  FILE *file, *index_file;
  Record record;
  Index_Record index_record;
  int status;
  int page_num=0;
  long file_size;
  int reads = 0;
  int writes = 0;

  file = fopen (file_name, "r");
  index_file = fopen (index_file_name, "w");
  fseek (file, 0, SEEK_END);
  file_size = ftell (file);

  while (1) {
    int page_offset = sizeof(Record) * RECORDS_IN_PAGE * page_num;
    if (page_offset >= file_size) {
      break;
    }
    fseek (file, page_offset, 0);
    fread (&record, sizeof(Record), 1, file);
    reads++;
    index_record.key = record.key;
    index_record.page = page_num;
    fwrite (&index_record, sizeof (Index_Record), 1, index_file);
    writes++;
    page_num++;
  }
  
  fclose (file);
  fclose (index_file);
  printf ("Generate index\n");
  printf ("Disk reads: %d\n", reads);
  printf ("Disk writes: %d\n", writes);
}

void print_index_file (char *name) {
  FILE *file;
  Index_Record record;

  printf ("Printing %s:\n", name);

  file = fopen(name, "r");

  while (1) {
    int read_bytes;

    read_bytes = fread (&record, sizeof(Index_Record), 1, file);
    if (!read_bytes) {
      break;
    }

    printf("Key: %d, Page: %d\n", record.key, record.page);
  }

  fclose (file);
  printf ("\n");
}

uint32_t save_record_to_overflow (Record record, uint32_t overflow_ptr) {
  FILE *overflow;
  Record records[RECORDS_IN_PAGE];
  int reads = 0;
  int writes = 0;

  overflow = fopen (overflow_area, "r");
  fread (&records, sizeof (Record), RECORDS_IN_PAGE, overflow);
  reads++;
  memcpy (&records[records_in_overflow_area], &record, sizeof (Record));
  fclose (overflow);

  if (overflow_ptr == MAX_NUM) {
    overflow_ptr = records_in_overflow_area;
  } else {
    Record *prev_rec = NULL;
    Record *next_rec = &records[overflow_ptr];
    do {
      if (next_rec->key > records[records_in_overflow_area].key) {
        if (prev_rec == NULL) {
          records[records_in_overflow_area].overflow_ptr = overflow_ptr;
          overflow_ptr = records_in_overflow_area;
          break;
        } else {
          records[records_in_overflow_area].overflow_ptr = prev_rec->overflow_ptr;
          prev_rec->overflow_ptr = records_in_overflow_area;
          break;
        }
      }
      prev_rec = next_rec;
      if (next_rec->overflow_ptr == MAX_NUM) {
        next_rec = NULL;
      } else {
        next_rec = &records[next_rec->overflow_ptr];
      }
    } while (prev_rec->overflow_ptr != MAX_NUM);
    if (next_rec == NULL) {
      prev_rec->overflow_ptr = records_in_overflow_area;
    }
  }
  overflow = fopen (overflow_area, "w");
  fwrite (&records, sizeof (Record), RECORDS_IN_PAGE, overflow);
  writes++;
  fclose (overflow);
  records_in_overflow_area++;
  printf ("Insert record to overflow\n");
  printf ("Disk reads: %d\n", reads);
  printf ("Disk writes: %d\n", writes);
  return overflow_ptr;
}

void reorganize (char *name) {
  FILE *file, *overflow, *new_file;
  Record file_records[RECORDS_IN_PAGE];
  Record overflow_records[RECORDS_IN_PAGE];
  Record new_records[RECORDS_IN_PAGE];
  int records_loaded;
  int file_records_counter = 0;
  int new_records_counter = 0;
  Record *record_ptr = NULL;
  Record last_record;
  int reads = 0;
  int writes = 0;

  file = fopen (name, "r");
  overflow = fopen (overflow_area, "r");
  new_file = fopen ("temp", "w");

  fread (&overflow_records, sizeof (Record), RECORDS_IN_PAGE, overflow);
  reads++;
  fclose (overflow);

  memset(&new_records, 0, sizeof(new_records));
  records_loaded = fread (&file_records, sizeof (Record), RECORDS_IN_PAGE, file);
  reads++;
  record_ptr = &file_records[file_records_counter];
  file_records_counter++;
  
  if (record_ptr->key != 0) {
    memcpy (&new_records[new_records_counter], record_ptr, sizeof (Record));
    new_records[new_records_counter].overflow_ptr = MAX_NUM;
    new_records_counter++;
  }

  while (records_loaded) {
    // load page
    // compare records from file to records from overflow
    // keep saving to new buffer till full, with overwritten overflow_ptr

    if (record_ptr->overflow_ptr == MAX_NUM || record_ptr->key == 0) {
      record_ptr = &file_records[file_records_counter];
      file_records_counter++;
    } else {
      record_ptr = &overflow_records[record_ptr->overflow_ptr];
    }

    if (record_ptr->key != 0) {
      memcpy (&new_records[new_records_counter], record_ptr, sizeof (Record));
      new_records[new_records_counter].overflow_ptr = MAX_NUM;
      new_records_counter++;
    }

    if (new_records_counter == RECORDS_IN_PAGE) {
      fwrite (&new_records, sizeof (Record), RECORDS_IN_PAGE, new_file);
      writes++;
      memset (&new_records, 0, sizeof (new_records));
      new_records_counter = 0;
    }

    if (file_records_counter == RECORDS_IN_PAGE) {
      memcpy (&last_record, record_ptr, sizeof (Record));
      record_ptr = &last_record;
      records_loaded = fread (&file_records, sizeof (Record), RECORDS_IN_PAGE, file);
      file_records_counter = 0;
      if (records_loaded != RECORDS_IN_PAGE) {
        break;
      }
      reads++;
    }

  }

  if (new_records_counter) {
    fwrite (&new_records, sizeof (Record), RECORDS_IN_PAGE, new_file);
    writes++;
  }

  fclose (file);
  fclose (new_file);

  remove (name);
  rename ("temp", name);

  printf ("Reorganize\n");
  printf ("Disk reads: %d\n", reads);
  printf ("Disk writes: %d\n", writes);

  generate_index_file (name);
  create_overlow_area ();
}


void insert_record (char *name, Record record) {
  int page;
  int records_read;
  int page_offset;
  FILE *file;
  Record records[RECORDS_IN_PAGE];
  bool index_needs_updating = false;
  int reads = 0;
  int writes = 0;

  if (record.key == 0) {
    printf ("Insert record\n");
    printf ("Disk reads: %d\n", reads);
    printf ("Disk writes: %d\n", writes);
    return;
  }

  page = get_page_num_from_index (record.key);
  page_offset = sizeof (Record) * RECORDS_IN_PAGE * page;

  memset (&records, 0, sizeof (records));
  file = fopen (name, "r+");
  if (file == NULL) {
    file = fopen (name, "w");
    memcpy(&records[0], &record, sizeof(Record));
    fwrite(&records, sizeof(Record), RECORDS_IN_PAGE, file);
    writes++;
    fclose(file);
    printf ("Insert record\n");
    printf ("Disk reads: %d\n", reads);
    printf ("Disk writes: %d\n", writes);
    generate_index_file(name);
    return;
  }

  fseek (file, page_offset, 0);
  records_read = fread (records, sizeof (Record), RECORDS_IN_PAGE, file);
  reads++;

  if (records[0].key > record.key) {
    Record tmp;
    memcpy(&tmp, &records[0], sizeof(Record));
    memcpy(&records[0], &record, sizeof(Record));
    memcpy(&record, &tmp, sizeof(Record));
    records[0].overflow_ptr = save_record_to_overflow (record, records[0].overflow_ptr);
    index_needs_updating = true;
  } else {
    int i;
    for (i = 0; i < records_read; i++) {
      if (i == records_read - 1) {
        if (records[i].key != 0) {
          records[i].overflow_ptr = save_record_to_overflow (record, records[i].overflow_ptr);
        } else if (records[i].overflow_ptr == MAX_NUM) {
          memcpy (&records[i], &record, sizeof (Record));
        } else {
          records[i-1].overflow_ptr = save_record_to_overflow (record, records[i-1].overflow_ptr);
        }
        break;
      }
      // empty spot
      if (records[i].key == 0 && records[i].overflow_ptr == 0) {
        memcpy (&records[i], &record, sizeof (Record));
        break;
      }
      // deleted record; is next record key bigger?
      if (records[i].key == 0 && records[i].overflow_ptr == MAX_NUM) {
        int j;
        for (j = i; j < records_read; j++) {
          if (j == records_read - 1) {
            break;
          }
          if (records[j+1].key > record.key) {
            if (records[i-1].overflow_ptr == MAX_NUM) {
              memcpy (&records[j], &record, sizeof (Record));
            } else {
              records[i-1].overflow_ptr = save_record_to_overflow (record, records[i-1].overflow_ptr);
            }
            break;
          }
        }
        if (j == records_read - 1) {
          memcpy (&records[i], &record, sizeof (Record));
        }
        break;
      }
      // record should be between 2 records, save to overflow
      if (records[i+1].key > record.key && records[i+1].key != 0) {
        records[i].overflow_ptr = save_record_to_overflow (record, records[i].overflow_ptr);
        break;
      }
    }
    if (i == 0) {
      index_needs_updating = true;
    }
  }

  fseek(file, page_offset, 0);
  fwrite(&records, sizeof(Record), RECORDS_IN_PAGE, file);
  writes++;
  fclose (file);
  printf ("Insert record\n");
  printf ("Disk reads: %d\n", reads);
  printf ("Disk writes: %d\n", writes);

  if (records_in_overflow_area == RECORDS_IN_PAGE) {
    reorganize (name);
  } else if (index_needs_updating) {
    generate_index_file (name);
  }

}

void update_record (char *name, Record record) {
  int page;
  int records_read;
  int page_offset;
  FILE *file, *overflow;
  Record records[RECORDS_IN_PAGE];
  Record overflow_records[RECORDS_IN_PAGE];
  bool search_finished = false;
  int reads = 0;
  int writes = 0;

  page = get_page_num_from_index (record.key);
  page_offset = sizeof (Record) * RECORDS_IN_PAGE * page;

  file = fopen (name, "r+");
  fseek (file, page_offset, 0);
  records_read = fread (records, sizeof (Record), RECORDS_IN_PAGE, file);
  reads++;

  int i;
  if (records[0].key > record.key) {
    search_finished = true;
  } else {
    for (i = 0; i < records_read; i++) {
      if (records[i].key > record.key) {
        if (records[i-1].overflow_ptr == MAX_NUM) {
          search_finished = true;
        }
        break;
      }

      if (records[i].key == record.key) { // check overflow for next!
        search_finished = true;
        record.overflow_ptr = records[i].overflow_ptr;
        memcpy (&records[i], &record, sizeof (Record));
        break;
      }
    }
  }

  if (!search_finished) {
    Record *record_ptr = &records[i-1];
    Record overflow_records[RECORDS_IN_PAGE];
    FILE *overflow;
  
    overflow = fopen (overflow_area, "r");
    fread (&overflow_records, sizeof (Record), RECORDS_IN_PAGE, overflow);
    reads++;
    fclose (overflow);

    while (record_ptr->overflow_ptr != MAX_NUM) { // TODO: TEST
      if (record_ptr->key == record.key) {
        record.overflow_ptr = record_ptr->overflow_ptr;
        memcpy (record_ptr, &record, sizeof (Record));
        break;
      }
      record_ptr = &overflow_records[record_ptr->overflow_ptr];
    }

    overflow = fopen (overflow_area, "w");
    fwrite (&overflow_records, sizeof (Record), RECORDS_IN_PAGE, overflow);
    writes++;
    fclose (overflow);
  }

  fseek (file, page_offset, 0);
  fwrite (&records, sizeof (Record), RECORDS_IN_PAGE, file);
  writes++;
  fclose (file);
  
  printf ("Update record\n");
  printf ("Disk reads: %d\n", reads);
  printf ("Disk writes: %d\n", writes);
}

void delete_record (char *name, uint32_t key) {
  int page;
  int records_read;
  int page_offset;
  FILE *file, *overflow;
  Record records[RECORDS_IN_PAGE];
  Record overflow_records[RECORDS_IN_PAGE];
  bool search_finished = false;
  bool index_needs_updating = false;
  int writes = 0;
  int reads = 0;

  page = get_page_num_from_index (key);
  page_offset = sizeof (Record) * RECORDS_IN_PAGE * page;

  file = fopen (name, "r+");
  fseek (file, page_offset, 0);
  records_read = fread (records, sizeof (Record), RECORDS_IN_PAGE, file);
  reads++;

  int i;
  for (i = 0; i < records_read; i++) {
    if (records[i].key > key) {
      if (records[i-1].overflow_ptr == MAX_NUM) {
        search_finished = true;
      }
      break;
    }

    if (records[i].key == key) { // check overflow for next!
      search_finished = true;
      if (i == 0) {
        index_needs_updating = true;
      }
      if (records[i].overflow_ptr != MAX_NUM) {
        Record overflow_records[RECORDS_IN_PAGE];
        FILE *overflow = fopen (overflow_area, "r");
        fread (&overflow_records, sizeof (Record), RECORDS_IN_PAGE, overflow);
        reads++;
        fclose (overflow);
        memcpy (&records[i], &overflow_records[records[i].overflow_ptr], sizeof (Record));
      } else if (i == 0) {
        int j;
        for (j = 1; j < records_read; j++) {
          if (records[j].key != 0) {
            memcpy (&records[i], &records[j], sizeof (Record));
            memset (&records[j], 0, sizeof (Record));
            records[j].overflow_ptr = MAX_NUM;
            break;
          }
        }
        if (j == records_read) {
          memset (&records[i], 0, sizeof (Record));
          records[i].overflow_ptr = MAX_NUM;
        }
      } else {
        memset (&records[i], 0, sizeof (Record));
        records[i].overflow_ptr = MAX_NUM;
      }
      break;
    }
  }

  if (!search_finished) { // record could be in overflow
    Record overflow_records[RECORDS_IN_PAGE];
    Record *prev_rec, *this_rec;
    FILE *overflow;
  
    overflow = fopen (overflow_area, "r");
    fread (&overflow_records, sizeof (Record), RECORDS_IN_PAGE, overflow);
    reads++;
    fclose (overflow);

    prev_rec = &records[i-1]; // last record that wasn't bigger than passed key
    this_rec = &overflow_records[prev_rec->overflow_ptr];

    while (1) {
      if (this_rec->key == key) {
        prev_rec->overflow_ptr = this_rec->overflow_ptr;
        break;
      }
      if (this_rec->overflow_ptr == MAX_NUM) {
        break;
      }
      prev_rec = this_rec;
      this_rec = &overflow_records[this_rec->overflow_ptr];
    }

    overflow = fopen (overflow_area, "w");
    fwrite (&overflow_records, sizeof (Record), RECORDS_IN_PAGE, overflow);
    writes++;
    fclose (overflow);
  }

  fseek (file, page_offset, 0);
  fwrite (&records, sizeof (Record), RECORDS_IN_PAGE, file);
  writes++;
  fclose (file);
  
  printf ("Delete record\n");
  printf ("Disk reads: %d\n", reads);
  printf ("Disk writes: %d\n", writes);

  if (index_needs_updating) {
    generate_index_file (name);
  }
}

int get_page_num_from_index (int key) { 
  int page;
  int index_records_read;
  FILE *index_file;
  Index_Record index[RECORDS_IN_PAGE];

  index_file = fopen (index_file_name, "r");
  index_records_read = fread (index, sizeof (Index_Record), RECORDS_IN_PAGE, index_file);
  fclose (index_file);

  for (page = 0; page < (index_records_read-1); page++) {
    if (index[page+1].key > key) {
      break;
    }
  }

  return page;
}

void create_overlow_area () {
  FILE *file;
  Record null_records[RECORDS_IN_PAGE];
  
  records_in_overflow_area = 0;
  memset (&null_records, 0, sizeof (null_records));
  file = fopen (overflow_area, "w");
  fwrite (&null_records, sizeof (Record), RECORDS_IN_PAGE, file);
  fclose (file);
}


int main()
{
  int id;
  int result;
  char name[MAX_FILE_NAME_LEN];
  Record record;
  char c;

  printf ("File name: ");
  scanf ("%s", name);
  create_overlow_area();

  printf ("Generate random file? [Y/n] ");
  c = get_user_input ();
  if (c == 'Y' || c == 'y') {
    generate_random_file (name);
    sort_file(name);
    generate_index_file (name);
    print_index_file(index_file_name);
  }

  while (1) {
    printf("[i]nsert\n[u]pdate\n[d]elete\n[p]rint\n[r]eorganize\n[q]uit\n");
    c = get_user_input ();
    if (c == 'i') {
      take_user_input (&record);
      insert_record(name, record);
    } else if (c == 'd') {
      printf("enter key to delete:\n");
      result = scanf ("%d", &id);
      delete_record(name, id);
    } else if (c == 'u') {
      take_user_input (&record);
      update_record(name, record);
    } else if (c == 'p') {
      print_out_file(name);
      print_index_file(index_file_name);
    } else if (c == 'r') {
      reorganize (name);
    } else {
      break;
    }
  }
  reorganize (name);
  print_out_file(name);

  return 0;
}
