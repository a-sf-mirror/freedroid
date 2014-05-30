# Proxy automake file for the Makefile provided by Lua.

LUA_MAKEFILE = $(top_srcdir)/lua/lua.mk

# Export the VPATH variable to the Makefile provided by Lua.
export VPATH

all-local:
	$(MAKE) -f $(LUA_MAKEFILE) $(AM_MAKEFLAGS) liblua.a

check-local:
	$(MAKE) -f $(LUA_MAKEFILE) $(AM_MAKEFLAGS) test

clean-local:
	$(MAKE) -f $(LUA_MAKEFILE) $(AM_MAKEFLAGS) clean

