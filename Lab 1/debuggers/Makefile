STUDENT_ID=2863545

all: bugs valgrind_test

bugs: bugs.c
	gcc -g $^ -o $@

valgrind_test: valgrind_test.c
	gcc -g $^ -o $@

test: bugs
	-valgrind --leak-check=full --track-origins=yes ./bugs > bugs_output.txt 2> valgrind_output.txt
	@sed -i 's/^==[0-9]*==/==#==/' valgrind_output.txt
	@echo ""
	@echo "======================================="
	@echo "VALGRIND DIFF"
	-@diff -a valgrind_desired.txt valgrind_output.txt
	@echo "---------------------------------------"
	@echo ""
	@echo "======================================="
	@echo "PROGRAM DIFF"
	-@diff -a bugs_desired.txt bugs_output.txt

clean:
	-rm bugs valgrind_test bugs_output.txt valgrind_output.txt

tar: clean
	mkdir $(STUDENT_ID)_eecs678_lab1
	cp bugs.c Bugs.txt Makefile valgrind_test.c bugs_desired.txt valgrind_desired.txt $(STUDENT_ID)_eecs678_lab1
	tar cfzv $(STUDENT_ID)_eecs678_lab1.tar.gz $(STUDENT_ID)_eecs678_lab1
	-rm -rf $(STUDENT_ID)_eecs678_lab1

.PHONY: clean test
