/* nodemenu.c -- Produce a menu of all visited nodes. */

/* This file is part of GNU Info, a program for reading online documentation
   stored in Info format.

   Copyright (C) 1993 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Written by Brian Fox (bfox@ai.mit.edu). */

#include "info.h"

/* Return a line describing the format of a node information line. */
static char *
nodemenu_format_info ()
{
  return ("\n\
* Menu:\n\
  (File)Node                        Lines   Size   Containing File\n\
  ----------                        -----   ----   ---------------");
}

/* Produce a formatted line of information about NODE.  Here is what we want
   the output listing to look like:

* Menu:
  (File)Node                        Lines   Size   Containing File
  ----------                        -----   ----   ---------------
* (emacs)Buffers::                  48      2230   /usr/gnu/info/emacs/emacs-1
* (autoconf)Writing configure.in::  123     58789  /usr/gnu/info/autoconf/autoconf-1
* (dir)Top::			    40      589    /usr/gnu/info/dir
*/
static char *
format_node_info (node)
     NODE *node;
{
  register int i, len;
  char *parent, *containing_file;
  static char *line_buffer = (char *)NULL;

  if (!line_buffer)
    line_buffer = (char *)xmalloc (1000);

  if (node->parent)
    {
      parent = filename_non_directory (node->parent);
      if (!parent)
	parent = node->parent;
    }
  else
    parent = (char *)NULL;

  containing_file = node->filename;

  if (!parent && !*containing_file)
    sprintf (line_buffer, "* %s::", node->nodename);
  else
    {
      char *file = (char *)NULL;

      if (parent)
	file = parent;
      else
	file = filename_non_directory (containing_file);

      if (!file)
	file = containing_file;

      if (!*file)
	file = "dir";

      sprintf (line_buffer, "* (%s)%s::", file, node->nodename);
    }

  len = pad_to (36, line_buffer);

  {
    int lines = 1;

    for (i = 0; i < node->nodelen; i++)
      if (node->contents[i] == '\n')
	lines++;

    sprintf (line_buffer + len, "%d", lines);
  }

  len = pad_to (44, line_buffer);
  sprintf (line_buffer + len, "%d", node->nodelen);

  if (node->filename && *(node->filename))
    {
      len = pad_to (51, line_buffer);
      sprintf (line_buffer + len, node->filename);
    }

  return (savestring (line_buffer));
}

/* Little string comparison routine for qsort (). */
static int
compare_strings (string1, string2)
     char **string1, **string2;
{
  return (stricmp (*string1, *string2));
}

/* The name of the nodemenu node. */
static char *nodemenu_nodename = "*Node Menu*";

/* Produce an informative listing of all the visited nodes, and return it
   in a node.  If FILTER_FUNC is non-null, it is a function which filters
   which nodes will appear in the listing.  FILTER_FUNC takes an argument
   of NODE, and returns non-zero if the node should appear in the listing. */
NODE *
get_visited_nodes (filter_func)
     Function *filter_func;
{
  register int i, iw_index;
  INFO_WINDOW *info_win;
  NODE *node;
  char **lines = (char **)NULL;
  int lines_index = 0, lines_slots = 0;

  if (!info_windows)
    return ((NODE *)NULL);

  for (iw_index = 0; info_win = info_windows[iw_index]; iw_index++)
    {
      for (i = 0; i < info_win->nodes_index; i++)
	{
	  node = info_win->nodes[i];

	  /* We skip mentioning "*Node Menu*" nodes. */
	  if (internal_info_node_p (node) &&
	      (strcmp (node->nodename, nodemenu_nodename) == 0))
	    continue;

	  if (node && (!filter_func || (*filter_func) (node)))
	    {
	      char *line;

	      line = format_node_info (node);
	      add_pointer_to_array
		(line, lines_index, lines, lines_slots, 20, char *);
	    }
	}
    }

  /* Sort the array of information lines. */
  qsort (lines, lines_index, sizeof (char *), compare_strings);

  /* Delete duplicates. */
  {
    register int j, newlen;
    char **temp;

    for (i = 0, newlen = 1; i < lines_index - 1; i++)
      {
	if (strcmp (lines[i], lines[i + 1]) == 0)
	  {
	    free (lines[i]);
	    lines[i] = (char *)NULL;
	  }
	else
	  newlen++;
      }

    /* We have free ()'d and marked all of the duplicate slots.  Copy the
       live slots rather than pruning the dead slots. */
    temp = (char **)xmalloc ((1 + newlen) * sizeof (char *));
    for (i = 0, j = 0; i < lines_index; i++)
      if (lines[i])
	temp[j++] = lines[i];

    temp[j] = (char *)NULL;
    free (lines);
    lines = temp;
    lines_index = newlen;
  }

  initialize_message_buffer ();
  printf_to_message_buffer
    ("Here is a menu of nodes you could select with info-history-node:\n");
  printf_to_message_buffer ("%s\n", nodemenu_format_info ());
  for (i = 0; i < lines_index; i++)
    {
      printf_to_message_buffer ("%s\n", lines[i]);
      free (lines[i]);
    }
  free (lines);

  node = message_buffer_to_node ();
  add_gcable_pointer (node->contents);
  return (node);
}

DECLARE_INFO_COMMAND (list_visited_nodes,
   "Make a window containing a menu of all of the currently visited nodes")
{
  WINDOW *new;
  NODE *node;

  set_remembered_pagetop_and_point (window);

  /* If a window is visible and showing the buffer list already, re-use it. */
  for (new = windows; new; new = new->next)
    {
      node = new->node;

      if (internal_info_node_p (node) &&
	  (strcmp (node->nodename, nodemenu_nodename) == 0))
	break;
    }

  /* If we couldn't find an existing window, try to use the next window
     in the chain. */
  if (!new && window->next)
    new = window->next;

  /* If we still don't have a window, make a new one to contain the list. */
  if (!new)
    {
      WINDOW *old_active;

      old_active = active_window;
      active_window = window;
      new = window_make_window ((NODE *)NULL);
      active_window = old_active;
    }

  /* If we couldn't make a new window, use this one. */
  if (!new)
    new = window;

  /* Lines do not wrap in this window. */
  new->flags |= W_NoWrap;
  node = get_visited_nodes ((Function *)NULL);
  name_internal_node (node, nodemenu_nodename);

  /* Even if this is an internal node, we don't want the window
     system to treat it specially.  So we turn off the internalness
     of it here. */
  node->flags &= ~N_IsInternal;

  /* If this window is already showing a node menu, reuse the existing node
     slot. */
  {
    int remember_me = 1;

#if defined (NOTDEF)
    if (internal_info_node_p (new->node) &&
	(strcmp (new->node->nodename, nodemenu_nodename) == 0))
      remember_me = 0;
#endif /* NOTDEF */

    window_set_node_of_window (new, node);

    if (remember_me)
      remember_window_and_node (new, node);
  }

  active_window = new;
}

DECLARE_INFO_COMMAND (select_visited_node,
      "Select a node which has been previously visited in a visible window")
{
  char *line;
  NODE *node;
  REFERENCE **menu;

  node = get_visited_nodes ((Function *)NULL);

  menu = info_menu_of_node (node);
  free (node);

  line =
    info_read_completing_in_echo_area (window, "Select visited node: ", menu);

  window = active_window;

  /* User aborts, just quit. */
  if (!line)
    {
      info_abort_key (window, 0, 0);
      info_free_references (menu);
      return;
    }

  if (*line)
    {
      REFERENCE *entry;

      /* Find the selected label in the references. */
      entry = info_get_labeled_reference (line, menu);

      if (!entry)
	info_error ("The reference disappeared! (%s).", line);
      else
	info_select_reference (window, entry);
    }

  free (line);
  info_free_references (menu);

  if (!info_error_was_printed)
    window_clear_echo_area ();
}
