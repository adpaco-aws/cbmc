/*******************************************************************\

Module: Show the goto functions

Author: Peter Schrammel

\*******************************************************************/

/// \file
/// Show the goto functions

#ifndef CPROVER_GOTO_PROGRAMS_SHOW_GOTO_FUNCTIONS_H
#define CPROVER_GOTO_PROGRAMS_SHOW_GOTO_FUNCTIONS_H

class namespacet;
class goto_modelt;
class goto_functionst;
class ui_message_handlert;

#define OPT_SHOW_GOTO_FUNCTIONS \
  "(show-goto-functions)" \
  "(list-goto-functions)"

#define HELP_SHOW_GOTO_FUNCTIONS                                               \
  help_entry("--show-goto-functions", "show loaded goto program")              \
    << help_entry("--list-goto-functions", "list loaded goto functions")

void show_goto_functions(
  const namespacet &ns,
  ui_message_handlert &ui_message_handler,
  const goto_functionst &goto_functions,
  bool list_only);

void show_goto_functions(
  const goto_modelt &,
  ui_message_handlert &ui_message_handler,
  bool list_only);

#endif // CPROVER_GOTO_PROGRAMS_SHOW_GOTO_FUNCTIONS_H
