pjson (poll/pull json)
======================

Yep, that's another library for parsing json. But without callbacks.
This idea of borrowed from jsmn [ http://zserge.com/jsmn.html ].

Features
--------
- Queue of tokens instead of firing callbacks right away. I believe this should
  improve CPU instructions localization and page hits.
- Strings expansion.
- Unicode support. I.e. escaped UTF-16 sequences expanded into UTF-8.
- No malloc/free.
- Simple allocator with relocation for passed in supplementary buffer.

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

Any kind of documentation?
--------------------------
At this moment library is in active development and the only documentation is
header file inc/pjson.h and sequence diagrams in doc.
