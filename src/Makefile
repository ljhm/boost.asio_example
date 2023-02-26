SUBDIRS = main # foo

all : $(SUBDIRS)
# main : foo
install : $(SUBDIRS)

define versioning # version soversion
	$(MAKE) -C $@ soversion=lib$@.so.$(2)
	@ cp $@/$@ $@/lib$@.so.$(1)
	@ cd $@; ln -f -s lib$@.so.$(1) lib$@.so.$(2); cd ..
	@ cd $@; ln -f -s lib$@.so.$(1) lib$@.so; cd ..
endef

# foo :
#	$(call versioning,1.2.3,1.2) #,version,soversion

main :
	$(MAKE) -C $@

# make -C src/ DESTDIR=~/foo install
install :
	install -d "$(DESTDIR)/usr/local/bin"
#	install -d "$(DESTDIR)/usr/local/lib"
	install -m0755 main/main "$(DESTDIR)/usr/local/bin"
#	install -m0755 foo/*.so foo/*.so.* "$(DESTDIR)/usr/local/lib"

clean :
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir $@; \
	done
.PHONY : $(SUBDIRS) all clean install
