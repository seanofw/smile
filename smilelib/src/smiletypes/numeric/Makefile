
.PHONY: ints reals floats

all: ints reals floats

ints: smileinteger.template smileinteger_base.template
	perl makeinttypes.pl

floats: smilefloat.template smilefloat_base.template
	perl makefloattypes.pl

reals: smilereal.template smilereal_base.template
	perl makerealtypes.pl

clean:
	rm -f smile*.generated.c

