#ifndef __HAVE_MBINPUTMGR_H__
#define __HAVE_MBINPUTMGR_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <libmb/mb.h>

#define MAX_METHODS 10


extern int errno;

typedef struct InputMethods 
{
  char           *name;
  char           *exec;
  char           *icon_name;
  void           *data;
} InputMethod;

typedef struct MBInpmgrState
{
  InputMethod Methods[MAX_METHODS];
  InputMethod *MethodSelected; 
  int         NMethods;
  pid_t        PidCurrent
;
} MBInpmgrState;

MBInpmgrState* mbinpmgr_init(void);

void
mbinpmgr_change_selected_method (MBInpmgrState *inpmgr, 
				 InputMethod   *new_method);

void
mbinputmgr_toggle_selected_method (MBInpmgrState *inpmgr);

int
mbinputmgr_method_active (MBInpmgrState *inpmgr);

#endif
