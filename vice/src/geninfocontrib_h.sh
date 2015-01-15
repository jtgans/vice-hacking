#!/bin/sh
#
# geninfocontrib_h.sh - infocontrib.h generator script
#
# written by Marco van den Heuvel <blackystardust68@yahoo.com>

# extract years and name from input
extractnames()
{
   shift
   shift
   years=$1
   shift
   if test x"$years" = "x1993-1994,"; then
     years="$years $1"
     shift
   fi
   name="$*"
}

extractitem()
{
  item=`echo $* | sed -e "s/@b{//" -e "s/}//"`
}

extractlang()
{
  language=$3
}

# use system echo if possible, as it supports backslash expansion
if test -f /bin/echo; then
  ECHO=/bin/echo
else
  if test -f /usr/bin/echo; then
    ECHO=/usr/bin/echo
  else
    ECHO=echo
  fi
fi

rm -f try.tmp
$ECHO "\\\\n" >try.tmp
n1=`cat	try.tmp	| wc -c`
n2=`expr $n1 + 0`

if test x"$n2" = "x3"; then
    linefeed="\\\\n"
else
    linefeed="\\n"
fi
rm -f try.tmp

$ECHO "/*"
$ECHO " * infocontrib.h - Text of contributors to VICE, as used in info.c"
$ECHO " *"
$ECHO " * Autogenerated by geninfocontrib_h.sh, DO NOT EDIT !!!"
$ECHO " *"
$ECHO " * Written by"
$ECHO " *  Marco van den Heuvel <blackystardust68@yahoo.com>"
$ECHO " *"
$ECHO " * This file is part of VICE, the Versatile Commodore Emulator."
$ECHO " * See README for copyright notice."
$ECHO " *"
$ECHO " *  This program is free software; you can redistribute it and/or modify"
$ECHO " *  it under the terms of the GNU General Public License as published by"
$ECHO " *  the Free Software Foundation; either version 2 of the License, or"
$ECHO " *  (at your option) any later version."
$ECHO " *"
$ECHO " *  This program is distributed in the hope that it will be useful,"
$ECHO " *  but WITHOUT ANY WARRANTY; without even the implied warranty of"
$ECHO " *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the"
$ECHO " *  GNU General Public License for more details."
$ECHO " *"
$ECHO " *  You should have received a copy of the GNU General Public License"
$ECHO " *  along with this program; if not, write to the Free Software"
$ECHO " *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA"
$ECHO " *  02111-1307  USA."
$ECHO " *"
$ECHO " */"
$ECHO ""
$ECHO "#ifndef VICE_INFOCONTRIB_H"
$ECHO "#define VICE_INFOCONTRIB_H"
$ECHO ""
$ECHO "const char info_contrib_text[] ="

checkoutput()
{
  dooutput=yes
  case "$data" in
      @c*|"@itemize @bullet"|@item|"@end itemize") dooutput=no ;;
  esac
}

outputok=no
coreteamsection=no
exteamsection=no
transteamsection=no
docteamsection=no

rm -f coreteam.tmp exteam.tmp transteam.tmp docteam.tmp

while read data
do
  if test x"$data" = "x@c ---vice-core-team-end---"; then
    coreteamsection=no
  fi

  if test x"$data" = "x@c ---ex-team-end---"; then
    exteamsection=no
  fi

  if test x"$data" = "x@c ---translation-team-end---"; then
    transteamsection=no
  fi

  if test x"$data" = "x@c ---documentation-team-end---"; then
    docteamsection=no
  fi

  if test x"$coreteamsection" = "xyes"; then
    extractnames $data
    $ECHO >>coreteam.tmp "    { \"$years\", \"$name\" },"
  fi

  if test x"$exteamsection" = "xyes"; then
    extractnames $data
    $ECHO >>exteam.tmp "    { \"$years\", \"$name\" },"
  fi

  if test x"$transteamsection" = "xyes"; then
    extractitem $data
    read data
    extractlang $data
    read data
    $ECHO >>transteam.tmp "    { \"$item\", \"$language\" },"
  fi

  if test x"$docteamsection" = "xyes"; then
    extractitem $data
    read data
    $ECHO >>docteam.tmp "    \"$item\","
  fi

  if test x"$data" = "x@c ---vice-core-team---"; then
    coreteamsection=yes
  fi

  if test x"$data" = "x@c ---ex-team---"; then
    exteamsection=yes
  fi

  if test x"$data" = "x@c ---translation-team---"; then
    transteamsection=yes
  fi

  if test x"$data" = "x@c ---documentation-team---"; then
    docteamsection=yes
  fi

  if test x"$data" = "x@node Copyright, Contacts, Acknowledgments, Top"; then
    $ECHO "\"$linefeed\";"
    outputok=no
  fi
  if test x"$outputok" = "xyes"; then
    checkoutput
    if test x"$dooutput" = "xyes"; then
      if test x"$data" = "x"; then
        $ECHO "\"$linefeed\""
      else
        $ECHO "\"  $data$linefeed\""
      fi
    fi
  fi
  if test x"$data" = "x@chapter Acknowledgments"; then
    outputok=yes
  fi
done

$ECHO ""
$ECHO "vice_team_t core_team[] = {"
cat coreteam.tmp
rm -f coreteam.tmp
$ECHO "    { NULL, NULL }"
$ECHO "};"
$ECHO ""
$ECHO "vice_team_t ex_team[] = {"
cat exteam.tmp
rm -f exteam.tmp
$ECHO "    { NULL, NULL }"
$ECHO "};"
$ECHO ""
$ECHO "char *doc_team[] = {"
cat docteam.tmp
rm -f docteam.tmp
$ECHO "    NULL"
$ECHO "};"
$ECHO ""
$ECHO "vice_trans_t trans_team[] = {"
cat transteam.tmp
rm -f transteam.tmp
$ECHO "    { NULL, NULL }"
$ECHO "};"
$ECHO "#endif"
