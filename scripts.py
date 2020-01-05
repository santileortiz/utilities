class file_scanner:
    def __init__(self, string):
        self.string = string

        self.pos = 0
        self.len = len(self.string)
        self.is_eof = False

    def advance_char(self):
        if self.pos < self.len:
            self.pos += 1

        if self.pos == self.len:
            self.is_eof = True

    def char(self, c):
        if self.string[self.pos] == c:
            self.advance_char()
            return True

        return False

    def str (self, s):
        for c in s:
            if c == self.string[self.pos]:
                self.advance_char()
            else:
                return False

    def to_char (self, c):
        temp_pos = self.pos

        while not self.is_eof and self.string[self.pos] != c:
            self.advance_char()

        if self.string[self.pos] == c:
            self.advance_char()
            return True
        else:
            return False
    
    def consume_spaces (self):
        while not self.is_eof and self.string[self.pos].isspace():
            self.advance_char()


def scan_macro_declaration(scnr, macro):
    args = []
    scnr.str(macro)
    scnr.consume_spaces()
    if scnr.char ('('):
        start = scnr.pos

        paren_count = 1;
        while not scnr.is_eof and paren_count > 0:
            if scnr.char ('('):
                paren_count += 1
            elif scnr.char (')'):
                paren_count -= 1
            else:
                scnr.advance_char()

        end = scnr.pos
        args = scnr.string[start:end-1].split(',')

    return [arg.strip() for arg in args]
