@echo off

cl src\calc_sector_lookup_tables_h.c /Febin\calc_sector_lookup_tables_h.exe

bin\calc_sector_lookup_tables_h.exe > include\sector_lookup_tables.h

del /Q bin\calc_sector_lookup_tables_h.exe

cl -Iinclude src\bin2iso.c src\sector.c /Febin\bin2iso.exe
