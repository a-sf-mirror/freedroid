#ifndef _LANG_H
#define _LANG_H

#if ENABLE_NLS

#include <locale.h>
#include <libintl.h>

#define _(String)  (String[0]!='\0'?gettext(String):"")
#define N_(String) (String)
#define D_(String) (String[0]!='\0'?dgettext("freedroidrpg-data", String):"")
#define L_(String) (String[0]!='\0'?dgettext("freedroidrpg-dialogs", String):"")

#else

#define _(String)  (String)
#define N_(String) (String)
#define D_(String) (String)
#define L_(String) (String)

#endif

#endif
