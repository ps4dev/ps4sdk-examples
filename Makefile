.DEFAULT_GOAL := all

Dirs := $(wildcard */*/)
Targets := all clean

DirTargets := $(foreach t,$(Targets),$(addsuffix $t,$(Dirs)))

.PHONY : $(Targets) $(DirTargets)

$(Targets) : % : $(addsuffix %,$(Dirs))

$(DirTargets) :
	$(MAKE) -C $(@D) $(@F:.%=%)
