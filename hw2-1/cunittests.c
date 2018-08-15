#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <CUnit/Basic.h>
#include "beargit.h"
#include "util.h"

/* printf/fprintf calls in this tester will NOT go to file. */

#undef printf
#undef fprintf

/* The suite initialization function.
 * You'll probably want to delete any leftover files in .beargit from previous
 * tests, along with the .beargit directory itself.
 *
 * You'll most likely be able to share this across suites.
 */
int init_suite(void)
{
    // preps to run tests by deleting the .beargit directory if it exists
    fs_force_rm_beargit_dir();
    unlink("TEST_STDOUT");
    unlink("TEST_STDERR");
    return 0;
}

/* You can also delete leftover files after a test suite runs, but there's
 * no need to duplicate code between this and init_suite 
 */
int clean_suite(void)
{
    return 0;
}

/* Simple test of fread().
 * Reads the data previously written by testFPRINTF()
 * and checks whether the expected characters are present.
 * Must be run after testFPRINTF().
 */
void simple_sample_test(void)
{
    // This is a very basic test. Your tests should likely do more than this.
    // We suggest checking the outputs of printfs/fprintfs to both stdout
    // and stderr. To make this convenient for you, the tester replaces
    // printf and fprintf with copies that write data to a file for you
    // to access. To access all output written to stdout, you can read 
    // from the "TEST_STDOUT" file. To access all output written to stderr,
    // you can read from the "TEST_STDERR" file.
    int retval;
    retval = beargit_init();
    CU_ASSERT(0==retval);
    retval = beargit_add("asdf.txt");
    CU_ASSERT(0==retval);
}

struct commit {
  char msg[MSG_SIZE];
  struct commit* next;
};


void free_commit_list(struct commit** commit_list) {
  if (*commit_list) {
    free_commit_list(&((*commit_list)->next));
    free(*commit_list);
  }

  *commit_list = NULL;
}

void run_commit(struct commit** commit_list, const char* msg) {
    int retval = beargit_commit(msg);
    CU_ASSERT(0==retval);

    struct commit* new_commit = (struct commit*)malloc(sizeof(struct commit));
    new_commit->next = *commit_list;
    strcpy(new_commit->msg, msg);
    *commit_list = new_commit;
}

void simple_log_test(void)
{
    struct commit* commit_list = NULL;
    int retval;
    retval = beargit_init();
    CU_ASSERT(0==retval);
    FILE* asdf = fopen("asdf.txt", "w");
    fclose(asdf);
    retval = beargit_add("asdf.txt");
    CU_ASSERT(0==retval);
    run_commit(&commit_list, "GO BEARS!1");
    run_commit(&commit_list, "GO BEARS!2");
    run_commit(&commit_list, "GO BEARS!3");

    retval = beargit_log();
    CU_ASSERT(0==retval);

    struct commit* cur_commit = commit_list;

    const int LINE_SIZE = 512;
    char line[LINE_SIZE];

    FILE* fstdout = fopen("TEST_STDOUT", "r");
    CU_ASSERT_PTR_NOT_NULL(fstdout);

    while (cur_commit != NULL) {
      char refline[LINE_SIZE];

      // First line is empty
      CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
      CU_ASSERT(!strcmp(line,"\n"));

      // Second line is commit -- don't check the ID.
      CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
      CU_ASSERT(!strncmp(line,"commit", strlen("commit")));

      // Third line is msg
      sprintf(refline, "    %s\n", cur_commit->msg);
      CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
      CU_ASSERT_STRING_EQUAL(line, refline);

      cur_commit = cur_commit->next;
    }

    // Last line is empty
    CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
    CU_ASSERT(!strcmp(line,"\n"));

    CU_ASSERT_PTR_NULL(fgets(line, LINE_SIZE, fstdout));

    // It's the end of output
    CU_ASSERT(feof(fstdout));
    fclose(fstdout);

    free_commit_list(&commit_list);
}

// Tests the output of the status function.
void status_test(void) {
    int val;
    val = beargit_init();
    CU_ASSERT(0==val);
    FILE* file1 = fopen("one.txt", "w");
    fclose(file1);
    FILE* file2 = fopen("two.txt", "w");
    fclose(file2);
    FILE* file3 = fopen("three.txt", "w");
    fclose(file3);
    FILE* file4 = fopen("four.txt", "w");
    fclose(file4);
    val = beargit_add("one.txt");
    CU_ASSERT(0==val);
    val = beargit_add("two.txt");
    CU_ASSERT(0==val);
    val = beargit_add("three.txt");
    CU_ASSERT(0==val);
    val = beargit_add("four.txt");
    CU_ASSERT(0==val);

    val = beargit_status();
    CU_ASSERT(0==val);

    const int LINE_SIZE = 512;
    char line[LINE_SIZE];

    FILE* fstdout = fopen("TEST_STDOUT", "r");
    CU_ASSERT_PTR_NOT_NULL(fstdout);

    int num_files = 4;

    // First line has "Tracked files:"
    CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
    CU_ASSERT(!strcmp(line,"Tracked files:\n"));

    // Second line is empty.
    CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
    CU_ASSERT(!strcmp(line,"\n"));

    // Third line is one.txt
    CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
    CU_ASSERT(!strcmp(line,"  one.txt\n"));

    // Fourth line is two.txt
    CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
    CU_ASSERT(!strcmp(line,"  two.txt\n"));

    // Fifth line is three.txt
    CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
    CU_ASSERT(!strcmp(line,"  three.txt\n"));

    // Sixth line is four.txt
    CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
    CU_ASSERT(!strcmp(line,"  four.txt\n"));

    // Seventh line is empty
    CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
    CU_ASSERT(!strcmp(line,"\n"));

    // Eighth line is has the number of files.
    CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
    CU_ASSERT(!strcmp(line,"4 files total\n"));

    CU_ASSERT_PTR_NULL(fgets(line, LINE_SIZE, fstdout));

    CU_ASSERT(feof(fstdout));
    fclose(fstdout);

}

// Tests special cases when commiting using different messages
void message_test(void) {

    struct commit* commit_list = NULL;
    int retval;
    retval = beargit_init();
    CU_ASSERT(0==retval);
    FILE* asdf = fopen("asdf.txt", "w");
    fclose(asdf);
    retval = beargit_add("asdf.txt");
    CU_ASSERT(0==retval);
    // Should only have one commit, the last one.

    retval = beargit_commit_hw1("GO BRUINS!");
    CU_ASSERT(1==retval);
    retval = beargit_commit_hw1("GOBEARS!!");
    CU_ASSERT(1==retval);
    retval = beargit_commit_hw1("GO BEARS");
    CU_ASSERT(1==retval);
    retval = beargit_commit_hw1("GO BEA!");
    CU_ASSERT(1==retval);
    retval = beargit_commit_hw1("");
    CU_ASSERT(1==retval);
    retval = beargit_commit_hw1("G");
    CU_ASSERT(1==retval);
    retval = beargit_commit_hw1("aaaaGO BEARS!aaaa");
    CU_ASSERT(0==retval);

    run_commit(&commit_list, "aaaaGO BEARS!aaaa");

    retval = beargit_log();
    CU_ASSERT(0==retval);

    struct commit* cur_commit = commit_list;

    const int LINE_SIZE = 512;
    char line[LINE_SIZE];

    FILE* fstdout = fopen("TEST_STDOUT", "r");
    CU_ASSERT_PTR_NOT_NULL(fstdout);

    char refline[LINE_SIZE];

    // First line is empty
    CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
    CU_ASSERT(!strcmp(line,"\n"));

    // Second line is commit -- don't check the ID.
    CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
    CU_ASSERT(!strncmp(line,"commit", strlen("commit")));

    // Third line is msg
    sprintf(refline, "    %s\n", cur_commit->msg);
    CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
    CU_ASSERT_STRING_EQUAL(line, refline);

    // Last line is empty
    CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
    CU_ASSERT(!strcmp(line,"\n"));

    fclose(fstdout);

    free_commit_list(&commit_list);

}

//Tests the output of the branch function, and the branch parts of checkout_commit
void branch_test(void) {

  struct commit* commit_list = NULL;
  int retval;
  retval = beargit_init();
  CU_ASSERT(0==retval);
  FILE* asdf = fopen("asdf.txt", "w");
  fclose(asdf);
  retval = beargit_add("asdf.txt");
  CU_ASSERT(0==retval);
  run_commit(&commit_list, "GO BEARS!");
  
  // add one branch (newbranch) and made it the current branch
  // also making sure all the errors are working
  retval = beargit_checkout("newbranch", 1);
  CU_ASSERT(0==retval);

  retval = beargit_checkout("newbranch", 1);
  CU_ASSERT(1==retval);

  retval = beargit_checkout("oldbranch", 0);
  CU_ASSERT(1==retval);

  retval = beargit_checkout("newbranch", 0);
  CU_ASSERT(0==retval);

  retval = beargit_branch();
  CU_ASSERT(0==retval)

  const int LINE_SIZE = 512;
  char line[LINE_SIZE];

  FILE* fstdout = fopen("TEST_STDOUT", "r");
  CU_ASSERT_PTR_NOT_NULL(fstdout);

  char refline[LINE_SIZE];

  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  CU_ASSERT(!strcmp(line,"  master\n"));

  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  CU_ASSERT(!strcmp(line,"* newbranch\n"));

  CU_ASSERT_PTR_NULL(fgets(line, LINE_SIZE, fstdout));

  // It's the end of output
  CU_ASSERT(feof(fstdout));
  fclose(fstdout);

}

/* The main() function for setting up and running the tests.
 * Returns a CUE_SUCCESS on successful running, another
 * CUnit error code on failure.
 */
int cunittester()
{
   CU_pSuite pSuite = NULL;
   CU_pSuite pSuite2 = NULL;
   CU_pSuite pSuite3 = NULL;
   CU_pSuite pSuite4 = NULL;
   CU_pSuite pSuite5 = NULL;

   /* initialize the CUnit test registry */
   if (CUE_SUCCESS != CU_initialize_registry())
      return CU_get_error();

   /* add a suite to the registry */
   pSuite = CU_add_suite("Suite_1", init_suite, clean_suite);
   if (NULL == pSuite) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* Add tests to the Suite #1 */
   if (NULL == CU_add_test(pSuite, "Simple Test #1", simple_sample_test))
   {
      CU_cleanup_registry();
      return CU_get_error();
   }

   pSuite2 = CU_add_suite("Suite_2", init_suite, clean_suite);
   if (NULL == pSuite2) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* Add tests to the Suite #2 */
   if (NULL == CU_add_test(pSuite2, "Log output test", simple_log_test))
   {
      CU_cleanup_registry();
      return CU_get_error();
   }

   pSuite3 = CU_add_suite("Suite_3", init_suite, clean_suite);
   if (NULL == pSuite3) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* Add tests to the Suite #3 */
   if (NULL == CU_add_test(pSuite3, "Status Output test", status_test))
   {
      CU_cleanup_registry();
      return CU_get_error();
   }

   pSuite4 = CU_add_suite("Suite_4", init_suite, clean_suite);
   if (NULL == pSuite4) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* Add tests to the Suite #4 */
   if (NULL == CU_add_test(pSuite4, "Message Validity test", message_test))
   {
      CU_cleanup_registry();
      return CU_get_error();
   }

   pSuite5 = CU_add_suite("Suite_5", init_suite, clean_suite);
   if (NULL == pSuite5) {
      CU_cleanup_registry();
      return CU_get_error();
   }

  /*Add tests to the Suite #5 */
   if (NULL == CU_add_test(pSuite5, "Branch test", branch_test))
   {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* Run all tests using the CUnit Basic interface */
   CU_basic_set_mode(CU_BRM_VERBOSE);
   CU_basic_run_tests();
   CU_cleanup_registry();
   return CU_get_error();
}
