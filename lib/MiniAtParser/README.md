# MiniAtParser

Parser library for a simplified version of the AT protocol.

## Notes & Limitations

* Passthrough-data data has to fit in the buffer
* Handlers can only be synchronous
* String constants cannot use the `\`-mechanism from V.250[^1] section 5.4.2.2
* Delmiter is `\r\n`
* Only one command per line. Multiple commands separated by `;` are not supported.

[^1]: https://www.itu.int/rec/T-REC-V.250-200307-I/en
