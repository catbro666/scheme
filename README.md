# scheme
A scheme interpreter written in C

It basically supports the [r5rs](https://conservatory.scheme.org/schemers/Documents/Standards/R5RS/r5rs.pdf),
though the implementation is still incomplete, e.g. not all primitives are added, nor the gc.
In particular, it supports the hygienic macros without unnatural restrictions about the ellipses ([srfi-149](https://srfi.schemers.org/srfi-149/srfi-149.html)).

To play with this:

```bash
git clone https://github.com/catbro666/scheme.git
cd scheme
git module update --init
make # or `CC=gcc make` if you want to use gcc, by default it use clang
./scheme
```
