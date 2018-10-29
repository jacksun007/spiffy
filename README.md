Spiffy
======

For our FAST 2018 paper, please visit:

https://www.usenix.org/conference/fast18/presentation/sun

INSTALL
=======

You will need the following packages to install the following:

1. python -- usually comes with any reasonable distribution

2. pip -- required to install python packages

3. ply (python lex-yacc). You can easily do this with pip.

4. jinja2 -- code templating library. 

5. astyle -- used to format the generated output code

6. g++4.9 -- required for C++11 features

You can enter the following two commands in order to install everything:

`sudo apt-get install python-dev python-pip astyle`

`sudo pip install ply jinja2`

The above commands works assuming you are using Ubuntu 14.04 or later.

For g++ 4.9 on Ubuntu, you might need to install from Ubuntu Toolchain PPA:

```
sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt-get update
sudo apt-get install g++-4.9
```

HOW-TO
======

To build everything, type `make` in the root folder

