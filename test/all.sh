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
tst test4.cells
tst test5.cells
tst test6.cells
tst test7.cells
