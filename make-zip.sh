#!/bin/bash

typst compile doc/main.typ
ln -sf doc/main.pdf documentazione.pdf
# ln -sf . mugnaioli-677231
rm mugnaioli-progetto.zip
zip -vr9 mugnaioli-progetto.zip mugnaioli-677231/Makefile mugnaioli-677231/documentazione.pdf mugnaioli-677231/src/