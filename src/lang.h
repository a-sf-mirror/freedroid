#ifndef _LANG_H
#define _LANG_H

#ifndef HAVE_LOCALE_H
#undef ENABLE_NLS
#endif

#if ENABLE_NLS

#include <locale.h>
#include <libintl.h>

#define LOCALE_DIR FD_DATADIR"/locale/"

#define _(String) (String[0]!='\0'?gettext(String):"")
#define N_(String) String

#define D_(String) (String[0]!='\0'?dgettext("freedroidrpg_data", String):"")

#define L_(String) (String[0]!='\0'?dgettext("freedroidrpg_dialogs", String):"")

#else

#define _(String) String
#define N_(String) String

#define D_(String) String

#define L_(String) String

#endif

#endif
