
#
# SYNOPSIS
#
#   AX_SAGA_CHECK_SSH([MINIMUM-VERSION])
#
# DESCRIPTION
#
#   Test for the ssh command line tools.
#
#   This macro calls:
#
#     AC_SUBST(HAVE_GSISSH)
#     AC_SUBST(SAGA_GSISSH)
#     AC_SUBST(HAVE_SSH)
#     AC_SUBST(SAGA_SSH)
#     AC_SUBST(HAVE_SCP)
#     AC_SUBST(SAGA_SCP)
#     AC_SUBST(HAVE_SSHFS)
#     AC_SUBST(SAGA_SSHFS)
#
# LAST MODIFICATION
#
#   2008-10-14
#
# COPYLEFT
#
#   Copyright (c) 2008 Andre Merzky      <andre@merzky.net>
#
#   Copying and distribution of this file, with or without
#   modification, are permitted in any medium without royalty 
#   provided the copyright notice and this notice are preserved.


AC_DEFUN([AX_SAGA_CHECK_GSISSH],
[
  AC_ARG_WITH([gsissh],
              [AS_HELP_STRING([--with-gsissh],
              [path to gsissh binary @<:@default=check@:>@])],
              [],
              [with_gsissh=check])

  if test "x$with_gsissh" = "xcheck"; then
  
    AC_PATH_PROG(SAGA_GSISSH, gsissh)

    HAVE_GSISSH=no

    if ! test "x$SAGA_GSISSH" = "x"; then
      HAVE_GSISSH=yes
    fi
  
  else

    if test -x "$with_gsissh"; then
      HAVE_GSISSH=yes
      SAGA_GSISSH=$with_gsissh
    else
      HAVE_GSISSH=no
    fi

  fi


  AC_SUBST(HAVE_GSISSH)
  AC_SUBST(SAGA_GSISSH)
])


AC_DEFUN([AX_SAGA_CHECK_SSH],
[
  AC_ARG_WITH([ssh],
              [AS_HELP_STRING([--with-ssh],
              [path to ssh binary @<:@default=check@:>@])],
              [],
              [with_ssh=check])

  if test "x$with_ssh" = "xcheck"; then
  
    AC_PATH_PROG(SAGA_SSH, ssh)

    HAVE_SSH=no

    if ! test "x$SAGA_SSH" = "x"; then
      HAVE_SSH=yes
    fi
  
  else

    if test -x "$with_ssh"; then
      HAVE_SSH=yes
      SAGA_SSH=$with_ssh
    else
      HAVE_SSH=no
    fi

  fi


  AC_SUBST(HAVE_SSH)
  AC_SUBST(SAGA_SSH)
])


AC_DEFUN([AX_SAGA_CHECK_SCP],
[
  AC_ARG_WITH([scp],
              [AS_HELP_STRING([--with-scp],
              [path to scp binary @<:@default=check@:>@])],
              [],
              [with_scp=check])

  if test "x$with_scp" = "xcheck"; then
  
    AC_PATH_PROG(SAGA_SCP, scp)

    HAVE_SCP=no

    if ! test "x$SAGA_SCP" = "x"; then
      HAVE_SCP=yes
    fi
  
  else

    if test -x "$with_scp"; then
      HAVE_SCP=yes
      SAGA_SCP=$with_scp
    else
      HAVE_SCP=no
    fi

  fi


  AC_SUBST(HAVE_SCP)
  AC_SUBST(SAGA_SCP)
])


AC_DEFUN([AX_SAGA_CHECK_SCP],
[
  AC_ARG_WITH([scp],
              [AS_HELP_STRING([--with-scp],
              [path to scp binary @<:@default=check@:>@])],
              [],
              [with_scp=check])

  if test "x$with_scp" = "xcheck"; then
  
    AC_PATH_PROG(SAGA_SCP, scp)

    HAVE_SCP=no

    if ! test "x$SAGA_SCP" = "x"; then
      HAVE_SCP=yes
    fi
  
  else

    if test -x "$with_scp"; then
      HAVE_SCP=yes
      SAGA_SCP=$with_scp
    else
      HAVE_SCP=no
    fi

  fi


  AC_SUBST(HAVE_SCP)
  AC_SUBST(SAGA_SCP)
])


AC_DEFUN([AX_SAGA_CHECK_SSHFS],
[
  AC_ARG_WITH([sshfs],
              [AS_HELP_STRING([--with-sshfs],
              [path to sshfs binary @<:@default=check@:>@])],
              [],
              [with_sshfs=check])

  if test "x$with_sshfs" = "xcheck"; then
  
    AC_PATH_PROG(SAGA_SSHFS, sshfs)

    HAVE_SSHFS=no

    if ! test "x$SAGA_SSHFS" = "x"; then
      HAVE_SSHFS=yes
    fi
  
  else

    if test -x "$with_sshfs"; then
      HAVE_SSHFS=yes
      SAGA_SSHFS=$with_sshfs
    else
      HAVE_SSHFS=no
    fi

  fi


  AC_SUBST(HAVE_SSHFS)
  AC_SUBST(SAGA_SSHFS)
])

