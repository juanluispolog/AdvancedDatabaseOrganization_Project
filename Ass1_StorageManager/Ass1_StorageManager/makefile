defatult: test

test: test_assign1_1.o storage_mgr.o dberror.o
	gcc -Wall -g -o test test_assign1_1.o storage_mgr.o dberror.o

run_test:
	./test

test_assign1_1.o: test_assign1_1.c test_helper.h storage_mgr.h dberror.h  
	gcc -Wall -g -c test_assign1_1.c

storage_mgr.o: storage_mgr.c storage_mgr.h dberror.h
	gcc -Wall -g -c storage_mgr.c

dberror.o: dberror.c dberror.h
	gcc -Wall -g -c dberror.c

clean:
	rm test *.o *.log