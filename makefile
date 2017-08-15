myhttpd: queue.c thread_pool.c myhttpserver.c queue.h thread_pool.h type.h 
	gcc queue.c thread_pool.c myhttpserver.c -o hellomake -pthread -I.