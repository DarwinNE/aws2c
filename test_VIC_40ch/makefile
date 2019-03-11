all: vic-40ch
    
vic-40ch: test40ch.c vic40col.s
	cl65 -t vic20 -C vic20-24k_exp_gr.cfg test40ch.c vic40col.s -o hello40.prg