TEXT	:=	0x926200000
DATA	:=	0x926300000

CC		:=	clang
AS		:=	clang
OBJCOPY	:=	objcopy

RDIR	:=	bin
ODIR	:=	build
SDIR	:=	source

ifndef LIBPS4
ifndef libps4
$(error Neither LIBPS4 or libps4 set)
else
LIBPS4	:= $(libps4)
endif
endif

IDIRS	:=	-I$(LIBPS4)/include -I. -Iinclude
LDIRS	:=	-L$(LIBPS4)/lib -L. -Llib

CFLAGS	:=	$(IDIRS) -D__PS4__ -fPIC -target x86_64-scei-ps4-elf -O2 -Wall -pedantic -std=c11 -Wl,--build-id=none -m64 -mcmodel=large -ffreestanding -nostdlib -nostdinc -fno-builtin
SFLAGS	:=	-target x86_64-scei-ps4-elf -m64
LFLAGS	:=	$(LDIRS) -Ttext $(TEXT) -Tdata $(DATA) -nostartfiles

CFILES	:=	$(wildcard $(SDIR)/*.c)
SFILES	:=	$(wildcard $(SDIR)/*.s)
OBJS	:=	$(patsubst $(SDIR)/%.c, build/%.o, $(CFILES)) $(patsubst $(SDIR)/%.s, build/%.o, $(SFILES))

LIBS	:= -lps4

TARGET = $(RDIR)/$(shell basename $(CURDIR))

$(TARGET): $(ODIR) $(OBJS)
	@mkdir -p $@
	@rmdir $@
	$(CC) $(LIBPS4)/crt0.s $(ODIR)/*.o -o $(TARGET).elf $(CFLAGS) $(LFLAGS) $(LIBS)
	@$(OBJCOPY) -O binary $(TARGET).elf $(TARGET).bin
#@rm -f $(TARGET).elf

$(ODIR)/%.o: $(SDIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/%.o: $(SDIR)/%.s
	$(AS) -c -o $@ $< $(SFLAGS)

$(ODIR):
	@mkdir $@

.PHONY: clean

clean:
	rm -fR $(RDIR) $(ODIR)/*.o
