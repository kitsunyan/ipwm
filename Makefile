CC=gcc
DESTDIR=

BINDIR=/usr/local/bin
LIBDIR=/usr/local/lib
MODPROBEDIR=/etc/modprobe.d
SYSCONFDIR=/etc
MKINITCPIODIR=

.PHONY: all clean install

define sedall
	sed \
	-e "s,@BINDIR@,${BINDIR},g" \
	-e "s,@SYSCONFDIR@,${SYSCONFDIR},g" \
	-e "s,@PKGLIBDIR@,${LIBDIR}/ipwm,g" \
	< $1 > $2
endef

all: \
	igpureg \
	ipwm \
	modprobe.conf \
	mkinitcpio.hook

igpureg: igpureg.c
	$(CC) $(CFLAGS) \
	$(LDFLAGS) $< -o $@

ipwm: ipwm.in
	$(call sedall,$<,$@)

modprobe.conf: modprobe.conf.in
	$(call sedall,$<,$@)

mkinitcpio.hook: mkinitcpio.hook.in
	$(call sedall,$<,$@)

clean:
	-rm -rf igpureg
	-rm -rf ipwm
	-rm -rf modprobe.conf
	-rm -rf mkinitcpio.hook

install: all
	install -Dm755 'igpureg' "${DESTDIR}${LIBDIR}/ipwm/igpureg"
	install -Dm755 'ipwm' "${DESTDIR}${BINDIR}/ipwm"
	install -Dm644 'modprobe.conf' "${DESTDIR}${MODPROBEDIR}/ipwm.conf"
	install -Dm644 'ipwm.conf' "${DESTDIR}${SYSCONFDIR}/ipwm.conf"
	[ -z "${MKINITCPIODIR}" ] || \
	install -Dm644 'mkinitcpio.hook' "${DESTDIR}${MKINITCPIODIR}/install/ipwm"
