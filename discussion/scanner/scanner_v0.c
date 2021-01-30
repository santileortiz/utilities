/*
 * Copyright (C) 2019 Santiago León O.
 */

////////////////
// API V0
//
// This was my very first attempt at writing reusable functions that were useful
// when doing things related to parsing. My idea here was to get the simplest
// possible thing that I needed at the moment. This later got expanded into V1.
// The main advantage of this is the little state and documentation overhead.
// What a function does can be guessed at a first glance, and we don't require
// the user to declare any state besides the pointer that they probably already
// have from somewhere.
//
// At the beginning this looked so simple to use that I thought it could
// generalize into a VERY easy to use, stateless parsing API. This was not the
// case.
//
// Pros:
//  - Completely stateless, we take a pointer from the user, and return a
//    pointer to the same string.
//  - Pretty much self documenting because the code is so small and simple.
//
// Cons:
//  - Will never be a 'const correct' API, without becoming complicated and
//    adding mental overhead to the user. For a lengthy discussion see below.

// LESSON: Const correctness of this idiom
// ---------------------------------------
//
// This API revolved around the idiom of passing a pointer to a string and
// getting a pointer back that points to the same string, this way we never need
// to keep any state and there is zero mental overhead for the user. When I
// tried to think about this being used more generally, maybe by people who care
// about const correctness I hit a wall. Turns out it's impossible to program it
// in a way that allows to be used by people that want const AND non-const
// strings.
//
// If people are using a const string we must declare the function as:
//
//      const char* consume_line (const char *c);
//
// This will keep the constness of the original string. But, if people are using
// a non const string even though it will be casted to const without problem, we
// will still return a const pointer that will be most likely assigned to the
// same non const variable sent in the first place, here the const qualifier
// will be discarded and the compiler will complain. There is no way to say "if
// we are sent const return const, if it's non-const return non-const". Maybe in
// C++ with templates this can be done.
//
// Because we want to cause the least ammount of friction when using this
// library, making the user think about constdness defeats this purpose, so
// better just not use this.
//
// We could try to declare functions as:
//
//      bool consume_line (const char **c);
//
// But in C, char **c won't cast implicitly to const char **c. Also this has the
// problem that now it will unconditionally update c, wheras before the user had
// the choice either to update it immediately, or store it in a variable and
// update it afterwards, if necessary.
//
// Maybe what's needed is a proper scanner API that stores state inside a struct
// with more clear semantics. I'm still experimenting with different
// alternatives.
//
// Conclusion:
//
// I really don't care about people wanting to be const correct. I will try hard
// to make it easy to understand when a passed char will be modified. Which as
// far as a scanner API is concerned, will be never. People with const strings
// can just cast their variables when calling us. If it's too much to ask, they
// can create their const wrappers. I don't really want to have twice as many
// functions in the API just to have const versions of them.

static inline
char* consume_line (char *c)
{
    while (*c && *c != '\n') {
           c++;
    }

    if (*c) {
        c++;
    }

    return c;
}

static inline
char* consume_spaces (char *c)
{
    while (is_space(c)) {
           c++;
    }
    return c;
}

