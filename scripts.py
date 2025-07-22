def scan_macro_declaration(scnr, macro):
    args = []
    scnr.str(macro)
    scnr.consume_spaces()
    if scnr.char ('('):
        arg_start = scnr.pos
        paren_count = 1;
        while not scnr.is_eof and paren_count > 0:
            if scnr.char ('('):
                paren_count += 1
            elif scnr.char (')'):
                paren_count -= 1
            elif paren_count == 1 and scnr.char (','):
                args.append(scnr.string[arg_start:scnr.pos-1])
                arg_start = scnr.pos
            else:
                scnr.advance_char()

        args.append(scnr.string[arg_start:scnr.pos-1])

    return [arg.strip() for arg in args]
