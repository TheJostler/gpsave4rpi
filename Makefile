name=gpsave
dest=/usr/bin
cc=gcc
etc=/etc/gpsave
config=gpsave.config

$(name): gpsave.c
	$(cc) -O3 -o $@ $^

install:
	cp $(name) $(dest)/$(name)

clean:
	rm $(name)
