#!/bin/sh

tst()
{
    echo $1
    ../src/hashlife read $1
    echo
}

tst glider.cells
tst test.cells
tst test2.cells
tst test3.cells
