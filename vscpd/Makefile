#
# Copyright (C) 2006-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=vscpd
PKG_VERSION:=0.1
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/vscpd
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=VSCPd - Very Simple Control Protocol
endef

define Package/vscpd/description
VSCP Daemon
endef

TARGET_CFLAGS += \
	-lpthread

define Build/Prepare
	cp -r src/* $(PKG_BUILD_DIR)
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		all
endef

define Package/vscpd/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/vscpd $(1)/usr/bin/
endef

$(eval $(call BuildPackage,vscpd))
