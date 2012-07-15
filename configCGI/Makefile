#
# Copyright (C) 2006-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=confc
PKG_VERSION:=0.1
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/confc
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=configCGI - CGI Script
endef

define Package/confc/description
CGI Script
endef

TARGET_CFLAGS += \
	-lcgi

TARGET_LDFLAGS += \
	-Wl,-rpath-link=$(STAGING_DIR)/usr/lib

define Build/Prepare
	cp src/* $(PKG_BUILD_DIR)
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LFLAGS="$(TARGET_LDFLAGS)"
endef

define Package/confc/install
	$(INSTALL_DIR) $(1)/www/cgi-bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/confc.cgi $(1)/www/cgi-bin
	mkdir -p $(1)/www/cgi-bin/cfg
endef

$(eval $(call BuildPackage,confc))
