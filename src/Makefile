##  these are compile time constructs, that affect the running of the game
## See farther down for explaination of these and more! msw
##
##CPPFLAGS = -DFAST_TRACK -DBLOCK_WRITE
##
## -DCLEAN_AT_BOOT -DCHECK_RENT_INACTIVE
## -DSTRANGE_WACK
##
## The compiler your gonna use to compile this with...
##
#CC=g++-2.95
CC=g++-4.1
#CC=g++-3.3
#CC=g++
##
LD=$(CC)
##
##  these are architecture dependent flags
TITANFLAGS = -g -43 -vector_c -safe=ptrs -DTITAN
DEBUGFLAGS =-g # -O2 -fomit-frame-pointer -fno-strength-reduce -m486 -malign-loops=2 -malign-functions=2 -malign-jumps=2
MYFLAGS = $(DEBUGFLAGS) -pipe -Wall -DNETBSD -DLINUX -DCLEAN_AT_BOOT -DNEW_EXP -DNEW_GAIN -traditional
## -mno-fp-ret-in-387
CFLAGS = $(MYFLAGS) -DLOG_DEBUG -DEMANUELE
##
## **********************************************************
##  COMPILE TIME DEFINES, CPPFLAGS!
##
##  For lower exp loss at lower levels -DNEWEXP
##
##  For level loss for really low exp -DLEVEL_LOSS
##
##  To make some optimizations, you need to add things to the line above.
##  to use hash tables for rooms  (More cpu, less storage), add
##  -DHASH    
##
##  To get rid of tracking (lower cpu)
##  -DNOTRACK
##
##  To limit items above a specific rent cost
##  -DLIMITED_ITEMS
##
##  To force new players to be authorized
##  -DPLAYER_AUTH
##
##  To add some new debugging stuff (mostly limited to malloc_debug)
##  -DDEBUG
##  To add some commands for locking out specific hosts
##  -DSITELOCK
##
## To lock out oak.grove.iup.edu from boot time.
## -DLOCKGROVE
##
##
##  To modify some commands to make item duplication near impossible:
##  -DNODUPLICATES
##
##  To have each 58+ god restricted to one site, or set of sites
##  -DIMPL_SECURITY
##
##  To use BSD memory allocation/deallocation routines instead of the
##  native memory routines:
##  -DKLUDGE_MEM
##
##  If your system doesn't have built in functions of strdup() and
##  strstr(), then:
##  -DKLUDGE_STRING
##
##  If you want to use a faster mana/hit/move regen system.
##  -DNEWGAIN
##
##  If you want to save every room and mob items use this.
##  -DSAVEWORLD
##
##  If you want to use the Quest to Gain system use this. Not all classes
##  -DQUEST_GAIN
##
## If you want to use a lower gold ration for your game use this
## -DLOW_GOLD
##
## - DUSE_EGOS (check ego on give and gets )
##
##
## -DLOG_MOB
##    LOG_MOB logs all mob command sent to command_interpreter
##
##  -DLOG_DEBUG
##    This will add a bit more info to the logs, I use it to try
##    and track down impossible bugs.
##
## -DCLEAN_AT_BOOT
##    Forces the mud to remove outdated or inactive players and
##    demote inactive gods a boot up. Slows boot process down a
##    bit.
##
## -DSTRANGE_WACK
##  When used in combination with CLEAN_AT_BOOT it will clear out corrupted
##  user files. Do this once or twice a month if needed, otherwise do not
##  use it.
##
##
## -DZONE_COMM_ONLY
##    Enabling this will force all communications to be limited to
##    the zone that person is in. Making gossip/auction/tell NOT
##    GLOBAL. Tell is limited to people in the zone you are currently
##    standing in. This does NOT include SHOUT.
##
## -DLAG_MOBILES    
##    This define should make mobiles lag with commands as
##    pc's do. I.e. bashed pause, etc...
##
## -DPREVENT_PKILL
##    This define will add in a few checks in fight.c to keep
##    pc's from killing each other, willing or Un-willing.
##
## -DCHECK_RENT_INACTIVE
##    Check user inactivity against RENT_INACTIVE, if greater, wack
##    the rent file. RENT_INACTIVE located in db.c
##
## -DPERSONAL_PERM_LOCKOUTS
##    My personal annoying people I lock out perminatly. You can add
##    whomever is annoying in there.
##
## -DNEW_RENT
##    Charge 100 coins per day of rent, regardless of item types.
##
##

HEADERFILES = structs.h area.h comm.h db.h handler.h hash.h heap.h \
              interpreter.h limits.h poly.h race.h script.h spells.h trap.h \
              utils.h vt100c.h wizlist.h parser.h mail.h
##
OBJS = comm.o act.comm.o act.info.o act.move.o act.obj1.o act.obj2.o \
  act.off.o act.other.o act.social.o act.wizard.o handler.o \
  db.o interpreter.o utility.o spec_assign.o shop.o limits.o mobact.o \
  fight.o modify.o weather.o spells1.o spells2.o spell_parser.o \
  reception.o constants.o spec_procs.o signals.o board.o magic.o \
  magic2.o skills.o Opinion.o Trap.o magicutils.o multiclass.o hash.o \
  Sound.o Heap.o spec_procs2.o magic3.o security.o spec_procs3.o \
  create.o bsd.o parser.o ansi_parser.o mail.o mindskills1.o \
  mind_use1.o create.mob.o create.obj.o rhyodin.o gilde.o carceri.o \
  charlist.o lucertole.o

SRCS = comm.c act.comm.c act.info.c act.move.c act.obj1.c act.obj2.c \
  act.off.c act.other.c act.social.c act.wizard.c handler.c \
  db.c interpreter.c utility.c spec_assign.c shop.c limits.c mobact.c \
  fight.c modify.c weather.c spells1.c spells2.c spell_parser.c \
  reception.c constants.c spec_procs.c signals.c board.c magic.c \
  magic2.c skills.c Opinion.c Trap.c magicutils.c multiclass.c hash.c \
  Sound.c Heap.c spec_procs2.c magic3.c security.c spec_procs3.c \
  create.c bsd.c parser.c ansi_parser.c mail.o mind_use1.c \
  mindskills1.c create.mob.c create.obj.c rhyodin.c gilde.c carceri.c \
  charlist.c lucertole.c

all: leu
	chmod g+rw *.o

##tags: ${SRCS}
##  ctags -vtrS *.[ch]

leu: ${OBJS}

## if you want to add debugging stuff, uncomment the following, and comment
##  out the line after that
##  $(LD) -o $@ -g ${OBJS} /local/lib/gnu/gcc-gnulib /usr/lib/debug/malloc.o
##  $(LD) -o $@ -g ${OBJS} /usr/lib
##	$(LD) -o $@  ${OBJS}
##	$(LD) -nostartfiles -o $@ /usr/lib/gcrt0.o -gdb3 ${OBJS} /usr/lib/libgmon.a
##	$(LD) $(DEBUGFLAGS) -o $@ ${OBJS}
	$(LD) $(DEBUGFLAGS) -o $@ ${OBJS} /usr/lib/libcrypt.a
##  $(LD) -o $@ -g ${OBJS} -L/usr/local/gnu/lib/gcc-lib/sun4-sunos4.1.1/2.3

depend :
	echo Making depends...
	for i in ${SRCS};do $(CC) $(CFLAGS) -M $$i;done > .tmpdepend
	mv .tmpdepend .depend
##	makedepend ${SRCS}



# DO NOT DELETE THIS LINE -- make depend depends on it.

comm.o: protos.h ${HEADERFILES} version.h status.h fight.h
act.comm.o: protos.h ${HEADERFILES}
act.move.o: protos.h ${HEADERFILES} cmdid.h fight.h status.h
act.off.o: protos.h ${HEADERFILES} fight.h
act.obj1.o: protos.h ${HEADERFILES} status.h
act.obj2.o: protos.h ${HEADERFILES}
act.info.o: protos.h ${HEADERFILES} cmdid.h version.h fight.h
act.other.o: protos.h ${HEADERFILES} fight.h
act.social.o: protos.h ${HEADERFILES}
act.wizard.o: protos.h ${HEADERFILES} cmdid.h fight.h
handler.o: protos.h ${HEADERFILES} fight.h
db.o: protos.h ${HEADERFILES} gilde.h status.h fight.h
interpreter.o: protos.h ${HEADERFILES} cmdid.h
utility.o: protos.h ${HEADERFILES} fight.h
spec_assign.o: protos.h ${HEADERFILES} rhyodin.h carceri.h lucertole.h
shop.o: protos.h ${HEADERFILES}
limits.o: protos.h ${HEADERFILES} fight.h
mobact.o: protos.h ${HEADERFILES} cmdid.h status.h fight.h
fight.o: protos.h ${HEADERFILES}
modify.o: protos.h ${HEADERFILES}
weather.o: protos.h ${HEADERFILES}
spells1.o: protos.h ${HEADERFILES}
spells2.o: protos.h ${HEADERFILES} fight.h
spell_parser.o: protos.h ${HEADERFILES} cmdid.h fight.h
reception.o: protos.h ${HEADERFILES} status.h
constants.o: protos.h ${HEADERFILES}
spec_procs.o: protos.h ${HEADERFILES} cmdid.h fight.h
signals.o: protos.h ${HEADERFILES} status.h
board.o: protos.h ${HEADERFILES}
magic.o: protos.h ${HEADERFILES} fight.h
magic2.o: protos.h ${HEADERFILES} fight.h
magic3.o: protos.h ${HEADERFILES} fight.h
magicutils.o: protos.h ${HEADERFILES} fight.h
multiclass.o: protos.h ${HEADERFILES}
skills.o: protos.h ${HEADERFILES} fight.h status.h
Opinion.o: protos.h ${HEADERFILES}
Trap.o: protos.h ${HEADERFILES} fight.h
hash.o: protos.h ${HEADERFILES}
Sound.o: protos.h ${HEADERFILES}
Heap.o: protos.h ${HEADERFILES}
spec_procs2.o: protos.h ${HEADERFILES} cmdid.h status.h fight.h
spec_procs3.o: protos.h ${HEADERFILES} cmdid.h fight.h
security.o:
create.o: protos.h ${HEADERFILES}
bsd.o:
parser.o: protos.h ${HEADERFILES}
ansi_parser.o: protos.h ${HEADERFILES}
mail.o:protos.h ${HEADERFILES}
mind_use1.o: protos.h ${HEADERFILES}
mindskills1.o:protos.h ${HEADERFILES} fight.h
create.mob.o:protos.h ${HEADERFILES}
create.obj.o:protos.h ${HEADERFILES}
rhyodin.o:protos.h ${HEADERFILES} rhyodin.h cmdid.h charlist.h fight.h
gilde.o:protos.h ${HEADERFILES} gilde.h cmdid.h charlist.h
carceri.o:protos.h ${HEADERFILES} carceri.h cmdid.h charlist.h fight.h
charlist.o: charlist.h
lucertole.o:protos.h ${HEADERFILES} lucertole.h cmdid.h fight.h
