sample taken from https://github.com/openwebos/luna-init

== results ==
[==========] Running 4 tests from 1 test case.
[----------] Global test environment set-up.
[----------] 4 tests from performance
[ RUN      ] performance.measure_locale_count
[       OK ] performance.measure_locale_count (43 ms)
[ RUN      ] performance.measure_locale_pjson
[       OK ] performance.measure_locale_pjson (703 ms)
[ RUN      ] performance.measure_locale_yajl_dummy
[       OK ] performance.measure_locale_yajl_dummy (1192 ms)
[ RUN      ] performance.measure_locale_yajl
[       OK ] performance.measure_locale_yajl (1152 ms)
[----------] 4 tests from performance (3090 ms total)

pjson is ~39% faster than yajl
