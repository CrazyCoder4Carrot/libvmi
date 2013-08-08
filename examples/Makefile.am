## Source directory

SUBDIRS = 

AM_CPPFLAGS = -I$(top_srcdir) $(GLIB_CFLAGS)
AM_LDFLAGS = -L$(top_srcdir)/libvmi/.libs/
LDADD = -lvmi -lm $(LIBS) $(GLIB_LIBS)

bin_PROGRAMS = module-list process-list map-symbol map-addr dump-memory win-guid event-example msr-event-example singlestep-event-example get_pid get_procname get_files get_systable
module_list_SOURCES = module-list.c
process_list_SOURCES = process-list.c
map_symbol_SOURCES = map-symbol.c
map_addr_SOURCES = map-addr.c
dump_memory_SOURCES = dump-memory.c
event_example_SOURCES = event-example.c
msr_event_example_SOURCES = msr-event-example.c
singlestep_event_example_SOURCES = singlestep-event-example.c
win_guid_SOURCES = win-guid.c
get_pid_SOURCES = get_pid.c
get_procname_SOURCES = get_procname.c
get_files_SOURCES = get_files.c
get_systable_SOURCES = get_systable.c

