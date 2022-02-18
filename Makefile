name=gpsave
dest=/usr/bin
cc=gcc
etc=/etc/$(name)
mketc=if [ ! -d $(etc) ];then mkdir $(etc);fi
scr=gpsave.c
config=gpsave.config


$(name): $(scr)
	$(cc) -O3 -o $@ $^

install: update config
update: $(name)
	cp $(name) $(dest)/$(name)
config: $(config)
	$(mketc)
	cp $(config) $(etc)/$(config)
clean:
	if [ -f $(name) ];then rm $(name);fi
remove: clean
	if [ -d $(etc) ];then rm -r $(etc);fi
	if [ -f $(dest)/$(name) ];then rm $(dest)/$(name);fi
