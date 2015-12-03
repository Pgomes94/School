all: myshell tsp_p tsp_t tspsort

myshell: src/myshell.c
	@gcc -g -o $@ $^

tsp_p: src/tsp_p.c
	@gcc -g -o $@ $^

tsp_t: src/tsp_t.c
	@gcc -g -o $@ $^

tspsort: src/tspsort.c
	@gcc -g -o $@ $^

clean:
	@rm myshell tsp_p tsp_t tspsort
