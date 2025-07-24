#!/usr/bin/python3
from mkpy.utility import *
import scripts
import re
import sys
sys.path.append('python')
from translation import set_default_language

assert sys.version_info >= (3,2)

ensure_dir ("bin")

def default ():
    target = store_get ('last_snip', 'tests')
    call_user_function(target)

def tests ():
    ex ('gcc -Wall -g -o bin/tests tests.c -lm -lrt')

def run_tests ():
    tests()
    print(ecma_bold('== C Tests =='), flush=True)
    ex('./bin/tests')

    print(ecma_bold('\n== Python Tests =='), flush=True)
    ex('python3 python/execute_tests.py')

def linear_solver_usage ():
    ex ('gcc -Wall -g -o bin/linear_solver linear_solver_usage.c -lm -lrt')

def expand_macro ():
    """
    This is like a preprocessor but we preserve indentation and don't output
    line information. It's useful during the development of big macros.
    """

    # TODO: Support #include?
    args = get_cli_no_opt ()
    if not args or len(args) != 2:
        print ('Usage:')
        print ('pymk.py expand_macro <file path> <macro call>')
        return

    fpath = args[0]
    macro = args[1]
    macro_name = macro.split('(')[0].strip()

    f = open (fpath)
    #code = f.read()
    #f.close()

    parsing_comment = False
    start = False
    macro_code = ''
    for line in f:
        if line.startswith('#define'):
            if line.split()[1].startswith(macro_name + '('):
                start = True

        if start:
            macro_code += line

            # Macros only allow multiline comments in them. In that case lines
            # won't end with \\ so we need to detect this case separately.
            # FIXME: This will break if the code contains a string with "/*" or
            # "*/" in it. We should be more clever about it.
            # FIXME: This will also detect a line with "/*/" as the start and
            # end of a multiline comment. Lines like this are only starts, not
            # ends.
            if "/*" in line:
                parsing_comment = True
            if "*/" in line:
                parsing_comment = False

            if line.endswith('\\\n') or parsing_comment:
                continue
            elif line.endswith('\n'):
                break

    if macro_code == '':
        print ('Macro {} not defined in {}.'.format(macro_name, fpath))
        return

    # Create a scanner for the macro definition and get the argument names
    scnr = scripts.file_scanner(macro_code)
    scnr.str('#define')
    scnr.consume_spaces()
    args = scripts.scan_macro_declaration(scnr, macro_name)

    # Create a scanner for the passed macro instance and get the argument values
    macro_inst_scnr = scripts.file_scanner(macro)
    values = scripts.scan_macro_declaration(macro_inst_scnr, macro_name)

    # Scan the macro definition, strip trailing spaces, substitute arguments
    # and remove ##.
    # NOTE: We don't support the # operator yet.
    while not scnr.is_eof:
        start = scnr.pos
        scnr.to_char ('\n')
        line = macro_code[start:scnr.pos]

        stripchars = -2
        if line[-2] != '\\':
            stripchars = -1;
        clean_line = line[:stripchars].rstrip()

        for i, arg in enumerate(args):
            clean_line = re.sub(r'\b'+arg+r'\b', values[i], clean_line)

        clean_line = re.sub(r'\s*##\s*', '', clean_line)
        print (clean_line)

def translate_set_default():
    """
    Sets the default language files in a directory from translated versions.
    Usage: ./pymk.py translate_set_default <directory> <language_code>
    """
    args = get_cli_no_opt()
    if not args or len(args) != 2:
        print('Usage:')
        print('./pymk.py translate_set_default <directory> <language_code>')
        print('Example: ./pymk.py translate_set_default ./translations en')
        return

    directory = args[0]
    language_code = args[1]

    try:
        set_default_language(directory, language_code)
        print(f'Set default language to "{language_code}" for files in "{directory}"')
    except ValueError as e:
        print(f'Error: {e}')

if __name__ == "__main__":
    # Everything above this line will be executed for each TAB press.
    # If --get_completions is set, handle_tab_complete() calls exit().
    handle_tab_complete ()

    pymk_default(skip_snip_cache=['expand_macro'])

