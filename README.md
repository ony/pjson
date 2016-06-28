pjson (poll/pull json)
======================

Yep, that's another library for parsing json. But without callbacks.
This idea is borrowed from [jsmn](http://zserge.com/jsmn.html).

Features
--------
- Queue of tokens instead of firing callbacks right away. I believe this should
  improve CPU instructions localization and page hits.
- Strings expansion.
- Unicode support. I.e. escaped UTF-16 sequences expanded into UTF-8.
- No `malloc()`/`free()`.
- Use passed in supplementary buffer for strings with simple allocator.
  Notification about overflow and possibility to re-alloc are included.

Why queue, but not callbacks?
-----------------------------
Parsers with callbacks use table of pointers and thus requires indirect calls.
That puts some stress on indirect jumps prediction. Moreover the fact that CPU
hops over different snippets of code on almost every character probably results
in forgetting statistics for prev jumps. Switching between different code that
access to different memory pages may decrease probability of page hits.

Putting a queue between library and client code should allow pjson to work
longer over input chunk without distracting on semantic analysis of each token.

Of course that approach have drawbacks. That's additional queue of tokens and
as result delay of processing. I.e. you'll know about semantic error earlier
than if you'd use callbacks.

Approaches to keep it fast
--------------------------
- Heavily rely on [tail call](https://en.wikipedia.org/wiki/Tail_call)
  elimination by changing state through tail-calling other functions. Keep an
  eye on arguments order.
- Minimize amount of updates in `parser` structure. Consider it for saving
  state before "suspend" (giving control back to client of library).

Example
-------
```c
/* Supplementary buffer for strings with escapes */
size_t obuf_size = 256;
void *obuf = malloc(obuf_size);

/* Declare and init state of parser */
pj_parser parser;
pj_init(&parser, obuf, obuf_size);

/* Feed next chunk of input */
ssize_t s = read(fd, ibuf, sizeof(ibuf));
if (s == 0) pj_feed_end(&parser);
else  pj_feed(&parser, ibuf, s);

/* Pull next chunk of tokens */
pj_token tokens[32];
pj_poll(&parser, tokens, ARRAY_SIZE(tokens));

/* Process tokens */
for (pj_token *token = tokens; token < tokens+ARRAY_SIZE(tokens); ++tokens)
{
    switch (token->token_type)
    {
    case PJ_OVERFLOW: /* supplementary buffer overflow */
        if (obuf_size >= token->len)
            /* We can handle it. Just restart poll so older tokens will be dropped out from obuf. */
        obuf_size = token->len;
        obuf = realloc(obuf, obuf_size);
        pj_realloc(&parser, obuf, obuf_size);
        break;
    case PJ_STARVING:
        /* Feed another chunk of input */
    case PJ_ERR:
        /* Detected that JSON is invalid (might happen in the middle of document) */
    case PJ_END:
        /* End of JSON document */

    /* Normal JSON tokens */
    case PJ_TOK_STR:
        printf("String: %.*s", (int)token->len, token->str);
        break;
    /* ... */
    case PJ_TOK_KEY: /* represents delimiter ':' */
    /* ... */
    }
}
```

Any kind of documentation?
--------------------------
At this moment library is in active development and the only documentation is
header file inc/pjson.h and sequence diagrams in doc.
