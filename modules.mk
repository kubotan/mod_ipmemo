mod_ipmemo.la: mod_ipmemo.slo
	$(SH_LINK) -rpath $(libexecdir) -module -avoid-version  mod_ipmemo.lo
DISTCLEAN_TARGETS = modules.mk
shared =  mod_ipmemo.la
