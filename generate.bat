@echo off
git submodule update --init --recursive
premake5 %* vs2022
