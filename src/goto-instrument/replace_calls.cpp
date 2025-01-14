/*******************************************************************\

Module: Replace calls

Author: Daniel Poetzl

\*******************************************************************/

/// \file
/// Replace calls
/// Replaces calls to given functions with calls to other given functions. Needs
/// to be run after removing function pointer calls and before removing returns.

#include "replace_calls.h"

#include <util/base_type.h>
#include <util/exception_utils.h>
#include <util/invariant.h>
#include <util/namespace.h>
#include <util/string_utils.h>

#include <goto-programs/goto_model.h>
#include <goto-programs/remove_returns.h>

/// Replace function calls with calls to other functions
/// \param goto_model: goto model to modify
/// \param replacement_list: list of strings, with each string f:g denoting a
///   mapping between functions names; a mapping f -> g indicates that calls to
///   f should be replaced by calls to g
void replace_callst::operator()(
  goto_modelt &goto_model,
  const replacement_listt &replacement_list) const
{
  replacement_mapt replacement_map = parse_replacement_list(replacement_list);
  (*this)(goto_model, replacement_map);
}

/// Replace function calls with calls to other functions
/// \param goto_model: goto model to modify
/// \param replacement_map: mapping between function names; a mapping f -> g
///   indicates that calls to f should be replaced by calls to g
void replace_callst::operator()(
  goto_modelt &goto_model,
  const replacement_mapt &replacement_map) const
{
  const namespacet ns(goto_model.symbol_table);
  goto_functionst &goto_functions = goto_model.goto_functions;

  check_replacement_map(replacement_map, goto_functions, ns);

  for(auto &p : goto_functions.function_map)
  {
    goto_functionst::goto_functiont &goto_function = p.second;
    goto_programt &goto_program = goto_function.body;

    (*this)(goto_program, goto_functions, ns, replacement_map);
  }
}

void replace_callst::operator()(
  goto_programt &goto_program,
  const goto_functionst &goto_functions,
  const namespacet &ns,
  const replacement_mapt &replacement_map) const
{
  Forall_goto_program_instructions(it, goto_program)
  {
    goto_programt::instructiont &ins = *it;

    if(!ins.is_function_call())
      continue;

    const exprt &function = ins.call_function();

    PRECONDITION(function.id() == ID_symbol);

    const symbol_exprt &se = to_symbol_expr(function);
    const irep_idt &id = se.get_identifier();

    auto f_it1 = goto_functions.function_map.find(id);

    DATA_INVARIANT(
      f_it1 != goto_functions.function_map.end(),
      "Called functions need to be present");

    replacement_mapt::const_iterator r_it = replacement_map.find(id);

    if(r_it == replacement_map.end())
      continue;

    const irep_idt &new_id = r_it->second;

    auto f_it2 = goto_functions.function_map.find(new_id);
    PRECONDITION(f_it2 != goto_functions.function_map.end());

    // check that returns have not been removed
    if(to_code_type(function.type()).return_type().id() != ID_empty)
    {
      goto_programt::const_targett next_it = std::next(it);
      if(next_it != goto_program.instructions.end() && next_it->is_assign())
      {
        const exprt &rhs = next_it->assign_rhs();

        INVARIANT(
          rhs != return_value_symbol(id, ns),
          "returns must not be removed before replacing calls");
      }
    }

    // Finally modify the call
    ins.call_function().type() = ns.lookup(f_it2->first).type;
    to_symbol_expr(ins.call_function()).set_identifier(new_id);
  }
}

replace_callst::replacement_mapt replace_callst::parse_replacement_list(
  const replacement_listt &replacement_list) const
{
  replacement_mapt replacement_map;

  for(const std::string &s : replacement_list)
  {
    std::string original;
    std::string replacement;

    split_string(s, ':', original, replacement, true);

    const auto r =
      replacement_map.insert(std::make_pair(original, replacement));

    if(!r.second)
    {
      throw invalid_command_line_argument_exceptiont(
        "conflicting replacement for function " + original, "--replace-calls");
    }
  }

  return replacement_map;
}

void replace_callst::check_replacement_map(
  const replacement_mapt &replacement_map,
  const goto_functionst &goto_functions,
  const namespacet &ns) const
{
  for(const auto &p : replacement_map)
  {
    if(replacement_map.find(p.second) != replacement_map.end())
      throw invalid_command_line_argument_exceptiont(
        "function " + id2string(p.second) +
          " cannot both be replaced and be a replacement",
        "--replace-calls");

    auto it2 = goto_functions.function_map.find(p.second);

    if(it2 == goto_functions.function_map.end())
      throw invalid_command_line_argument_exceptiont(
        "replacement function " + id2string(p.second) + " needs to be present",
        "--replace-calls");

    auto it1 = goto_functions.function_map.find(p.first);
    if(it1 != goto_functions.function_map.end())
    {
      if(!base_type_eq(
           ns.lookup(it1->first).type, ns.lookup(it2->first).type, ns))
        throw invalid_command_line_argument_exceptiont(
          "functions " + id2string(p.first) + " and " + id2string(p.second) +
            " are not type-compatible",
          "--replace-calls");
    }
  }
}
