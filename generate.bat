@echo off
git submodule update --init --recursive
tools\premake5 %* vs2022