#!/usr/bin/python3
from mkpy.utility import *

assert sys.version_info >= (3,2)

ensure_dir ("bin")

def default():
    target = pers ('last_target', 'keyboard_layout_editor')
    call_user_function(target)

def tests ():
    ex ('gcc -g -o bin/tests tests.c -lm')

if __name__ == "__main__":
    # Everything above this line will be executed for each TAB press.
    # If --get_completions is set, handle_tab_complete() calls exit().
    handle_tab_complete ()

    pymk_default()

