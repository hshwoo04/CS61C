#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <sys/stat.h>

#include "beargit.h"
#include "util.h"

/* Implementation Notes:
 *
 * - Functions return 0 if successful, 1 if there is an error.
 * - All error conditions in the function description need to be implemented
 *   and written to stderr. We catch some additional errors for you in main.c.
 * - Output to stdout needs to be exactly as specified in the function description.
 * - Only edit this file (beargit.c)
 * - You are given the following helper functions:
 *   * fs_mkdir(dirname): create directory <dirname>
 *   * fs_rm(filename): delete file <filename>
 *   * fs_mv(src,dst): move file <src> to <dst>, overwriting <dst> if it exists
 *   * fs_cp(src,dst): copy file <src> to <dst>, overwriting <dst> if it exists
 *   * write_string_to_file(filename,str): write <str> to filename (overwriting contents)
 *   * read_string_from_file(filename,str,size): read a string of at most <size> (incl.
 *     NULL character) from file <filename> and store it into <str>. Note that <str>
 *     needs to be large enough to hold that string.
 *  - You NEED to test your code. The autograder we provide does not contain the
 *    full set of tests that we will run on your code. See "Step 5" in the homework spec.
 */

/* beargit init
 *
 * - Create .beargit directory
 * - Create empty .beargit/.index file
 * - Create .beargit/.prev file containing 0..0 commit id
 *
 * Output (to stdout):
 * - None if successful
 */

int beargit_init(void) {
  fs_mkdir(".beargit");

  FILE* findex = fopen(".beargit/.index", "w");
  fclose(findex);
  
  write_string_to_file(".beargit/.prev", "0000000000000000000000000000000000000000");

  return 0;
}


/* beargit add <filename>
 * 
 * - Append filename to list in .beargit/.index if it isn't in there yet
 *
 * Possible errors (to stderr):
 * >> ERROR: File <filename> already added
 *
 * Output (to stdout):
 * - None if successful
 */

int beargit_add(const char* filename) {
  FILE *findex = fopen(".beargit/.index", "r");
  FILE *fnewindex = fopen(".beargit/.newindex", "w");

  char line[FILENAME_SIZE];
  while(fgets(line, sizeof(line), findex)) {
    strtok(line, "\n");
    if (strcmp(line, filename) == 0) {
      fprintf(stderr, "ERROR: File %s already added\n", filename);
      fclose(findex);
      fclose(fnewindex);
      fs_rm(".beargit/.newindex");
      return 3;
    }

    fprintf(fnewindex, "%s\n", line);
  }

  fprintf(fnewindex, "%s\n", filename);
  fclose(findex);
  fclose(fnewindex);

  fs_mv(".beargit/.newindex", ".beargit/.index");

  return 0;
}


/* beargit rm <filename>
 * 
 * See "Step 2" in the homework 1 spec.
 *
 */

int beargit_rm(const char* filename) {
  FILE *findex = fopen(".beargit/.index", "r");
  FILE *fnewindex = fopen(".beargit/.newindex", "w");
  int exists = 0;

  char line[FILENAME_SIZE];
  while(fgets(line, sizeof(line), findex)) {
    strtok(line, "\n");
    if (strcmp(line, filename) == 0) {
      exists = 1;
    } else {
      fprintf(fnewindex, "%s\n", line);
    }
  }
  if (exists == 1) {
    fs_mv(".beargit/.newindex", ".beargit/.index");
    return 0;
  } else {
    fprintf(stderr, "ERROR: File %s not tracked\n", filename);
    fclose(findex);
    fclose(fnewindex);
    return 1;
  }
}

/* beargit commit -m <msg>
 *
 * See "Step 3" in the homework 1 spec.
 *
 */

const char* go_bears = "GO BEARS!";

int is_commit_msg_ok(const char* msg) {
  while (*msg != '\0') {
    if (*msg == 'G') {
      char* msg1 = msg;
      char* go_bears1 = go_bears;
      while (*msg1 != '\0' && *msg1 == *go_bears1 && *go_bears1 != '\0') {
        msg1++;
        go_bears1++;
      }
      if (*go_bears1 == '\0') {
        return 1;
      }
    } 
    msg++;
  }
  return 0;
}

void next_commit_id(char* commit_id) {
  for (int j = 0; j < strlen(commit_id); j++) {
    if (commit_id[j]!= '6' && commit_id[j] != '1' && commit_id[j] != 'c') {
      commit_id[j] = '6';
    }
  }
  for (int i = 0; i< strlen(commit_id); i++) {
    switch (commit_id[i]) {
      case '6' :
        commit_id[i] = '1';
        break;
      case '1' : 
        commit_id[i] = 'c';
        break;
      case 'c' :
        commit_id[i] = '6';
        break;
    }
    if (commit_id[i] != '6') {
      break;
    }
  }
}

int beargit_commit(const char* msg) {
  if (!is_commit_msg_ok(msg)) {
    fprintf(stderr, "ERROR: Message must contain \"%s\"\n", go_bears);
    return 1;
  }
  char commit_id[COMMIT_ID_SIZE];
  read_string_from_file(".beargit/.prev", commit_id, COMMIT_ID_SIZE);
  next_commit_id(commit_id);

  char str[100] = ".beargit/";
  strcat(str, commit_id); 
  fs_mkdir(str);

  char str1[100] = ".beargit/";
  strcat(str1, commit_id);
  strcat(str1, "/.index");
  fs_cp(".beargit/.index", str1);

  char str2[100] = ".beargit/";
  strcat(str2, commit_id); 
  strcat(str2, "/.prev");
  fs_cp(".beargit/.prev", str2);

  FILE *findex = fopen(".beargit/.index", "r");
  char line[FILENAME_SIZE];
  while(fgets(line, sizeof(line), findex)) {
    strtok(line, "\n");
    char str4[100] = ".beargit/";
    strcat(str4, commit_id);
    strcat(str4, line);
    fs_cp(line, str4);
  }
  char str5[100] = ".beargit/";
  strcat(str5, commit_id);
  strcat(str5, "/.msg");
  write_string_to_file(str5, msg);
  write_string_to_file(".beargit/.prev", commit_id);

  return 0;
}

/* beargit status
 *
 * See "Step 1" in the homework 1 spec.
 *
 */

int beargit_status() {
  /* COMPLETE THE REST */
  FILE *findex = fopen(".beargit/.index", "r");
  int num = 0;
  fprintf(stdout, "Tracked files:\n\n");
  char line[FILENAME_SIZE];
  while(fgets(line, sizeof(line), findex)) {
    strtok(line, "\n");
    fprintf(stdout, "  %s\n", line);
    num = num + 1;
  }
  fprintf(stdout, "\n%d files total\n", num);
  return 0;
}

/* beargit log
 *
 * See "Step 4" in the homework 1 spec.
 *
 */

int beargit_log() {
  /* COMPLETE THE REST */
  char temp_id[COMMIT_ID_SIZE];
  read_string_from_file(".beargit/.prev", temp_id, COMMIT_ID_SIZE); 
  if (temp_id[0] == '0') {
    fprintf(stderr, "ERROR: There are no commits!\n");
    return 1;
  }
  while (temp_id[0] != '0') {
    fprintf(stdout, "\n");
    fprintf(stdout, "commit %s\n", temp_id);
    char str1[100] = ".beargit/";
    strcat(str1, temp_id);
    strcat(str1, "/.msg");
    char str2[100];
    read_string_from_file(str1, str2, 100);
    fprintf(stdout, "    %s\n", str2);
    char str[100] =  ".beargit/";
    strcat(str, temp_id);
    strcat(str, "/.prev");
    read_string_from_file(str, temp_id, COMMIT_ID_SIZE);
  }
  fprintf(stdout, "\n");
  return 0;
}
