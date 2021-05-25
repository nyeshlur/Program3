#!/bin/sh
g++ driver.cpp -o driver
g++ eval.cpp -o eval

chmod 700 eval
chmod 700 driver