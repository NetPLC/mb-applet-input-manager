#include "mbinputmgr.h"

static void 
fork_exec(MBInpmgrState *inpmgr, char *cmd)
{
  switch (inpmgr->PidCurrent = fork())
    {
    case 0: // Child process
      mb_exec(cmd); 		/* XXX handle for gnome */
      fprintf(stderr, "Can't exec \"%s\"\n", cmd);
      exit(1);
    case -1:
      fprintf(stderr, "Couldn't fork\n");
      /* XXX msg bubble error */
      break;
    }
}

static void 
selected_method_save(MBInpmgrState *inpmgr)
{
  FILE *fp;
  char *sessionfile = alloca(sizeof(char)*255);

  if (inpmgr->MethodSelected == NULL) 
    return;

  if (getenv("HOME") == NULL)
    {
      fprintf(stderr, "mbinputmgr: unable to get home directory, is HOME set?\n");
      return;
    }

  snprintf(sessionfile, 255, "%s/.mbinputmgr", getenv("HOME"));   
  
  if ((fp = fopen(sessionfile, "w")) == NULL)
    { 
      fprintf(stderr,"mbinputmgr: Unable to create Session file ( %s )\n", 
	      sessionfile); 
      return; 
    }

  fprintf(fp, "%s", inpmgr->MethodSelected->exec);  

  fclose(fp);
}

static void 
selected_method_load(MBInpmgrState *inpmgr)
{
  FILE *fp;
  char *sessionfile = alloca(sizeof(char)*255);
  char *selected_exec_str = alloca(sizeof(char)*255);

  int i = 0;

  if (getenv("HOME") == NULL)
    {
      fprintf(stderr, 
	      "mbinputmgr: unable to get home directory, is HOME set?\n");
      return;
    }

  snprintf(sessionfile, 255, "%s/.mbinputmgr", getenv("HOME"));   
  
  if ((fp = fopen(sessionfile, "r")) == NULL)
    {
      fprintf(stderr, "mbinputmgr: no session data found.");
      return;
    }
  
  if (fgets(selected_exec_str, 255, fp) != NULL)
    {
      for (i = 0; i < inpmgr->NMethods; i++)
	if (!strcmp(selected_exec_str, inpmgr->Methods[i].exec))
	  {
	    inpmgr->MethodSelected = &inpmgr->Methods[i];
	    break;
	  }
    }
 
  fclose(fp);
  return;
}

#define N_SEARCH_DIRS 2

MBInpmgrState*
mbinpmgr_init(void)
{
  MBInpmgrState *inpmgr = NULL;

  char app_paths[N_SEARCH_DIRS][256];
  int cur_entry_count = 0;
  DIR *dp;
  struct dirent *dir_entry;
  struct stat stat_info;
  char orig_wd[256];
  int i;

  inpmgr = malloc(sizeof(MBInpmgrState));
  memset(inpmgr,0,sizeof(MBInpmgrState));

  inpmgr->MethodSelected = NULL;
  inpmgr->NMethods       = 0;
  inpmgr->PidCurrent     = -1;

  if (getcwd(orig_wd, 255) == (char *)NULL)
    {
      fprintf(stderr, "mbinputmgr: cant get current directory\n");
      exit(0);
    }
  
  snprintf(app_paths[0], 256, DATADIR "/applications/inputmethods");
  snprintf(app_paths[1], 256, "%s/.applications/inputmethods", getenv("HOME"));

  for (i = 0; i < N_SEARCH_DIRS; i++)
  {
    if ((dp = opendir(app_paths[i])) == NULL)
      {
	fprintf(stderr, "mbinputmgr: failed to open %s\n", app_paths[i]);
	continue;
      }
  
    chdir(app_paths[i]);
    while((dir_entry = readdir(dp)) != NULL)
      {
	lstat(dir_entry->d_name, &stat_info);
	if (!(S_ISDIR(stat_info.st_mode)))
	  {

	    /* 
	       XXX should use libgnome/gnome-dentry.[hc]
	           for gnome .desktop parsing - use ifdefs or wrapper? 
	    */

	    MBDotDesktop *dd;
	    int flags = 0;
	    dd = mb_dotdesktop_new_from_file(dir_entry->d_name);
	    if (dd /* && mb_dotdesktop_get(dd, "X-MB-INPUT-MECHANSIM") */)   
	      {
		if (mb_dotdesktop_get(dd, "Icon")
		    && mb_dotdesktop_get(dd, "Name")
		    && mb_dotdesktop_get(dd, "Exec")
		    && cur_entry_count < MAX_METHODS)
		  {
		    MBMenuItem      *new_item = NULL;
		    
		    MBPixbufImage *img_tmp;
		    
		    inpmgr->Methods[cur_entry_count].name 
		      = strdup(mb_dotdesktop_get(dd,"Name"));
		    inpmgr->Methods[cur_entry_count].icon_name 
		      = strdup(mb_dotdesktop_get(dd,"Icon"));
		    inpmgr->Methods[cur_entry_count].exec 
		      = strdup(mb_dotdesktop_get(dd,"Exec"));

		    /* XXX Should be done elsewhere		    
		    load_icon(&inpmgr->Methods[cur_entry_count]);

		    inpmgr->Methods[cur_entry_count].item 
		      = mb_menu_new_item(PopupMenu, 
					 mb_menu_get_root_menu(PopupMenu), 
					 Methods[cur_entry_count].name, 
					 menu_item_activated_callback,
					 (void *)&Methods[cur_entry_count],
					 0 );
		    
		    mb_menu_item_icon_set (PopupMenu, 
					   Methods[cur_entry_count].item, 
					   Methods[cur_entry_count].icon );
		    */

		    if (inpmgr->MethodSelected == NULL) 
		      inpmgr->MethodSelected = &inpmgr->Methods[cur_entry_count];
		    
		    cur_entry_count++;
		  }
		mb_dotdesktop_free(dd);
	      }
#ifdef DEBUG
	    else
	      fprintf(stderr, "mbinputmgr: failed to parse %s :( \n", 
		      dir_entry->d_name);
#endif
	  }
      }
    
    closedir(dp);
  }

  chdir(orig_wd);

  if (cur_entry_count == 0)
    {
      /* XXX Msg bubble error */
      fprintf(stderr, "mbinputmgr: Failed to find any input mechanisms\n");
      exit(1);
    }

  inpmgr->NMethods = cur_entry_count;

  selected_method_load(inpmgr);
  
  return inpmgr;
}

void
mbinpmgr_change_selected_method (MBInpmgrState *inpmgr, 
				 InputMethod   *new_method)
{
  if (inpmgr->MethodSelected != new_method)
    {
      inpmgr->MethodSelected = new_method;

      /* If the old method executing, kill it and start a new one */
      if (inpmgr->PidCurrent != -1) 
	{
	  kill(inpmgr->PidCurrent, 15);
	  inpmgr->PidCurrent = -1;
	  fork_exec(inpmgr, inpmgr->MethodSelected->exec);
	}

      selected_method_save (inpmgr);
    }
}

void
mbinputmgr_toggle_selected_method (MBInpmgrState *inpmgr)
{
  /* Start/stop current sleected method */

  if ( (inpmgr->PidCurrent != -1) /* Something running */
       &&  (kill(inpmgr->PidCurrent, 0) != -1) )
    {
      kill(inpmgr->PidCurrent, 15); /* kill it */
      inpmgr->PidCurrent = -1;
    }
  else fork_exec(inpmgr, inpmgr->MethodSelected->exec);
}

int
mbinputmgr_method_active (MBInpmgrState *inpmgr)
{
  return (inpmgr->PidCurrent != -1);
}
