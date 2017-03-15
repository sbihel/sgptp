# SGM - TP4 (Louis Béziaud & Simon Bihel)

Some Nachos related errors where fixed [[#3], [#4], [#5], [#6], [#9], [#10], [#11], [#12], [#13]].

We had troubles due to an error with our SGP base code [[#16]].

We note that debugging virtual memory errors is difficult, as standard tools (such as valgrind), do not help [[#14], [#18], [#19]].

We tested our code with combinations of two threaded *producer / consumer* (`test/prodcons.c`, `test/prodcons2.c`), a racing *producer / consumer* (`test/ab.c`) and `shell`. Launching tests inside multiple "layers" of `shell` was the main difficulty.

The code is now fully working.

[#3]: https://github.com/lbeziaud/sgmtp/issues/3
[#4]: https://github.com/lbeziaud/sgmtp/issues/4
[#5]: https://github.com/lbeziaud/sgmtp/issues/4
[#6]: https://github.com/lbeziaud/sgmtp/issues/6
[#9]: https://github.com/lbeziaud/sgmtp/issues/9
[#10]: https://github.com/lbeziaud/sgmtp/issues/10
[#11]: https://github.com/lbeziaud/sgmtp/issues/11
[#12]: https://github.com/lbeziaud/sgmtp/issues/12
[#13]: https://github.com/lbeziaud/sgmtp/issues/13
[#16]: https://github.com/lbeziaud/sgmtp/issues/16
[#14]: https://github.com/lbeziaud/sgmtp/issues/14
[#18]: https://github.com/lbeziaud/sgmtp/issues/18
[#19]: https://github.com/lbeziaud/sgmtp/issues/19
