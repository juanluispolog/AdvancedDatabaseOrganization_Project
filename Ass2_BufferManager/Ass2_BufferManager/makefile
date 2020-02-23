default: test1 test2

test1: test_assign2_1.o storage_mgr.o dberror.o buffer_mgr_stat.o buffer_mgr.o
	gcc -Wall -g -o test1 test_assign2_1.o buffer_mgr.o storage_mgr.o dberror.o buffer_mgr_stat.o
 
run_test1:
	./test1



test2: test_assign2_2.o storage_mgr.o dberror.o buffer_mgr_stat.o buffer_mgr.o
	gcc -Wall -g -o test2 test_assign2_2.o buffer_mgr.o storage_mgr.o dberror.o buffer_mgr_stat.o
 
run_test2:
	./test2



test_assign2_1.o: test_assign2_1.c test_helper.h storage_mgr.h dberror.h buffer_mgr_stat.h buffer_mgr.h  
	gcc -Wall -g -c test_assign2_1.c

test_assign2_2.o: test_assign2_2.c test_helper.h storage_mgr.h dberror.h buffer_mgr_stat.h buffer_mgr.h  
	gcc -Wall -g -c test_assign2_2.c

buffer_mgr.o: buffer_mgr.c storage_mgr.h buffer_mgr.h
	gcc -Wall -g -c buffer_mgr.c

buffer_mgr_stat.o:  buffer_mgr_stat.c buffer_mgr_stat.h buffer_mgr.h
	gcc -Wall -g -c buffer_mgr_stat.c

storage_mgr.o: storage_mgr.c storage_mgr.h dberror.h
	gcc -Wall -g -c storage_mgr.c

dberror.o: dberror.c dberror.h
	gcc -Wall -g -c dberror.c

clean:
	rm test1 test2 *.o *.log