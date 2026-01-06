all:
	@cc -std=c89 -Wpedantic -Wall -Wextra src/calc_sector_lookup_tables_h.c \
                 -o bin/calc_sector_lookup_tables_h
	@bin/calc_sector_lookup_tables_h > include/sector_lookup_tables.h
	@rm -f bin/calc_sector_lookup_tables_h
	@cc -std=c89 -Wpedantic -Wall -Wextra -Iinclude \
                 src/sector.c src/bin2iso.c -o bin/bin2iso

clean:
	@rm -f bin/calc_sector_lookup_tables_h
	@rm -f bin/bin2iso
	@rm -f include/sector_lookup_tables.h

style:
	@clang-format-21 -i -style=file:clang_format \
        src/calc_sector_lookup_tables_h.c
	@clang-format-21 -i -style=file:clang_format \
        include/sector.h
	@clang-format-21 -i -style=file:clang_format \
        src/sector.c
	@clang-format-21 -i -style=file:clang_format \
        src/bin2iso.c

lint:
	@echo Preparing...
	@cc -std=c89 -Wpedantic -Wall -Wextra src/calc_sector_lookup_tables_h.c \
                 -o bin/calc_sector_lookup_tables_h
	@bin/calc_sector_lookup_tables_h > include/sector_lookup_tables.h
	@rm -f bin/calc_sector_lookup_tables_h
	@echo Testing...
	@echo " gcc in C mode: src/calc_sector_lookup_tables_h.c:"
	@gcc -std=c89 -Wpedantic -Wall -Wextra src/calc_sector_lookup_tables_h.c \
         -o bin/calc_sector_lookup_tables_h
	@rm -f bin/calc_sector_lookup_tables_h
	
	@echo " gcc in C mode: src/sector.c src/bin2iso.c:"
	@gcc -std=c89 -Wpedantic -Wall -Wextra -Iinclude \
         src/sector.c src/bin2iso.c -o bin/bin2iso
	@rm -f bin/bin2iso
	
	@echo " clang in C mode: src/calc_sector_lookup_tables_h.c:"
	@clang -std=c89 -Wpedantic -Wall -Wextra src/calc_sector_lookup_tables_h.c \
         -o bin/calc_sector_lookup_tables_h
	@rm -f bin/calc_sector_lookup_tables_h
	
	@echo " clang in C mode: src/sector.c src/bin2iso.c:"
	@clang -std=c89 -Wpedantic -Wall -Wextra -Iinclude \
         src/sector.c src/bin2iso.c -o bin/bin2iso
	@rm -f bin/bin2iso
	
	@echo " gcc in C++ mode: src/calc_sector_lookup_tables_h.c:"
	@g++ -Wpedantic -Wall -Wextra src/calc_sector_lookup_tables_h.c \
         -o bin/calc_sector_lookup_tables_h
	@rm -f bin/calc_sector_lookup_tables_h
	
	@echo " gcc in C++ mode: src/sector.c src/bin2iso.c:"
	@g++ -Wpedantic -Wall -Wextra -Iinclude \
         src/sector.c src/bin2iso.c -o bin/bin2iso
	@rm -f bin/bin2iso

	@echo " clang in C++ mode: src/calc_sector_lookup_tables_h.c:"
	@clang++ -x c++ -Wpedantic -Wall -Wextra \
         src/calc_sector_lookup_tables_h.c -o bin/calc_sector_lookup_tables_h
	@rm -f bin/calc_sector_lookup_tables_h
	
	@echo " clang in C++ mode: src/sector.c src/bin2iso.c:"
	@clang++ -x c++ -Wpedantic -Wall -Wextra -Iinclude \
         src/sector.c src/bin2iso.c -o bin/bin2iso
	@rm -f bin/bin2iso

	@echo " cppcheck: "
	@cppcheck --enable=all --suppress=missingIncludeSystem \
	          --inconclusive --check-config --std=c89 \
              src/calc_sector_lookup_tables_h.c src/sector.c src/bin2iso.c \
              include/sector.h include/sector_lookup_tables.h
