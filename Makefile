test_lockless_alloc: five_headed_monster.c
	gcc $< -o $@ -g -std=c99 -lpthread -D_GNU_SOURCE

clean:
	@rm test_lockless_alloc
	
