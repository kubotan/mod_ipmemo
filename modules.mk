mod_iptag.la: mod_iptag.slo
	$(SH_LINK) -rpath $(libexecdir) -module -avoid-version  mod_iptag.lo
DISTCLEAN_TARGETS = modules.mk
shared =  mod_iptag.la
