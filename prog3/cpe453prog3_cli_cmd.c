#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "cparser.h"
#include "cparser_token.h"
#include "cparser_tree.h"

#define PROMPT "> "

int main() {

  cparser_t parser;
  char *config_file = NULL;
  int ch, debug = 0, n;
  cparser_result_t rc;

  memset(&parser, 0, sizeof(parser));

  parser.cfg.root = &cparser_root;
  parser.cfg.ch_complete = '\t';
  /* 
   * Instead of making sure the terminal setting of the target and 
   * the host are the same. ch_erase and ch_del both are treated
   * as backspace.
   */
  parser.cfg.ch_erase = '\b';
  parser.cfg.ch_del = 127;
  parser.cfg.ch_help = '?';
  parser.cfg.flags = (debug ? CPARSER_FLAGS_DEBUG : 0);
  strcpy(parser.cfg.prompt, PROMPT);
  parser.cfg.fd = STDOUT_FILENO;
  cparser_io_config(&parser);

  if (CPARSER_OK != cparser_init(&parser.cfg, &parser)) {
      printf("Failed to initialize parser.\n");
      return -1;
  }
  
  // Start the parser
  cparser_run(&parser);
}

/** 
 * Below are the "Action Functions" which respond to the specific commands
 * defined in the .cli file
 */

// Note: CLI Parser 0.5 does not support non-nested optional arguments, so the
//       'add' command had to be broken up into 9 distinct Action Functions.
//       For more details, check the manual under CLI Files 5.2

// Also Note: Optional parameters will be set to NULL if the corresponding 
//            optional value is not supplied at the CLI.  For example, see
//            cparser_cmd_add__s__f_filename__i_interval(cparser_context_t *context,
//                char **filename_ptr,
//                    int32_t *interval_ptr)  <-- interval_ptr is optional, and
//                                                can be null!

// Add system monitor using defaults
cparser_result_t cparser_cmd_add__s(cparser_context_t *context) {
  printf("Add system monitor using defaults\n");
}

// Add system monitor using set interval (optional filename)
cparser_result_t cparser_cmd_add__s__i_interval__f_filename(cparser_context_t *context,
    int32_t *interval_ptr,
    char **filename_ptr) {
  printf("Add system monitor using set interval (optional filename)\n");
}

// Add system monitor using set filename (optional interval)
cparser_result_t cparser_cmd_add__s__f_filename__i_interval(cparser_context_t *context,
    char **filename_ptr,
    int32_t *interval_ptr) {
  printf("Add system monitor using set filename (optional interval)\n");
}

// Add process monitor to pid using defaults
cparser_result_t cparser_cmd_add__p_pid(cparser_context_t *context,
    int32_t *pid_ptr) {
  printf("Add process monitor to pid using defaults\n");
}

// Add process monitor to pid using set interval (optional filename)
cparser_result_t cparser_cmd_add__p_pid__i_interval__f_filename(cparser_context_t *context,
    int32_t *pid_ptr,
    int32_t *interval_ptr,
    char **filename_ptr) {
  printf("Add process monitor to pid using set interval (optional filename)\n");
}

// Add process monitor to pid using set filename (optional interval)
cparser_result_t cparser_cmd_add__p_pid__f_filename__i_interval(cparser_context_t *context,
    int32_t *pid_ptr,
    char **filename_ptr,
    int32_t *interval_ptr) {
  printf("Add process monitor to pid using set filename (optional interval)\n");
}

// Exec and add process monitor using defaults
cparser_result_t cparser_cmd_add__e_executable(cparser_context_t *context,
    char **executable_ptr) {
  printf("Exec and add process monitor using defaults\n");
}

// Exec and add process monitor using set interval (optional filename)
cparser_result_t cparser_cmd_add__e_executable__i_interval__f_filename(cparser_context_t *context,
    char **executable_ptr,
    int32_t *interval_ptr,
    char **filename_ptr) {
  printf("Exec and add process monitor using set interval (optional filename)\n");
}

// Exec and add process monitor using set filename (optional interval)
cparser_result_t cparser_cmd_add__e_executable__f_filename__i_interval(cparser_context_t *context,
    char **executable_ptr,
    char **filename_ptr,
    int32_t *interval_ptr) {
  printf("Exec and add process monitor using set filename (optional interval)\n");
}

// Set the default interval (in units of microseconds)
cparser_result_t cparser_cmd_set_interval_interval(cparser_context_t *context,
    int32_t *interval_ptr) {
  printf("Set the default interval (in units of microseconds)\n");
}

// Set the default log file
cparser_result_t cparser_cmd_set_logfile_logFileName(cparser_context_t *context,
    char **logFileName_ptr) {
  printf("Set the default log file\n");
}

// Show information about active monitors
cparser_result_t cparser_cmd_listactive(cparser_context_t *context) {
  printf("Show information about active monitors\n");
}

// Show monitors which have been stopped, or for which the target process has
// stopped
cparser_result_t cparser_cmd_listcompleted(cparser_context_t *context) {
  printf("Show monitors which have been stopped, or for which the target process has stopped\n");
}

// Remove the system monitor thread
cparser_result_t cparser_cmd_remove__s(cparser_context_t *context) {
  printf("Remove the system monitor thread\n");
}

// Remove a specific process monitor thread
cparser_result_t cparser_cmd_remove__t_threadID(cparser_context_t *context,
    int32_t *threadID_ptr) {
  printf("Remove a specific process monitor thread\n");
}

// Terminate all threads associated with the process processID, then terminate
// the process
cparser_result_t cparser_cmd_kill_processID(cparser_context_t *context,
    int32_t *processID_ptr) {
  printf("Terminate all threads associated with the process processID, then terminate the process\n");
}

// List all available commands
cparser_result_t cparser_cmd_help_filter (cparser_context_t *context, char **filter) {
  assert(context);
  return cparser_help_cmd(context->parser, filter ? *filter : NULL);
}

/**
 * Exit the parser test program.
 */
cparser_result_t cparser_cmd_exit (cparser_context_t *context) {
  assert(context);
  return cparser_quit(context->parser);
}
