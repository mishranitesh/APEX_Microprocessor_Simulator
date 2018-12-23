all: simulator_compile_run_clean

simulator_compile_run_clean:
	gcc -o simulator_exec apex_simulator_implementation.c -lm
	./simulator_exec ./Sample_File.txt
	rm simulator_exec
