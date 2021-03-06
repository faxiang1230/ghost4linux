#!/bin/bash
#
# Quick interface check for G4L
#
# dan.s@hostdime.com
# Fri Jun  6 16:53:01 EDT 2008
#
# Modified Jul 17 2008 to use dialog instead of echo for messages.
got_ip()
{
speed=""
duplex=""
  if [ -n "$device" ] ; then
    speed=`ethtool $device | grep Speed | cut -f2 -d:`
    duplex=`ethtool $device | grep Duplex | cut -f2 -d:`
  fi
}

backtitle="Nic Interface checker for G4L by Daniel Selans dan.s@hostdime.com"
IF_COUNT=`ifconfig -a | grep eth | wc -l`
if [ $IF_COUNT -eq 0 ]; then
  dialog --backtitle "$backtitle" --sleep 5 --infobox "No ethernet device found! Skipping dhcp..." 5 65
  elif [ $IF_COUNT -eq 1 ]; then
  FOUND_IF=`ifconfig -a | grep eth | awk {'print $1'}`
  ifconfig $FOUND_IF up 2>/dev/null; ## sleep 5
  dialog --backtitle "$backtitle" --sleep 5 --infobox "Checking status $FOUND_IF for an active link." 5 65
  LINK_CHK=`ethtool $FOUND_IF | grep "Link detected" | awk {'print $3'}`
  if [ "$LINK_CHK" = "yes" ]; then
    dialog --backtitle "$backtitle" --sleep 5 --infobox "Found $FOUND_IF with active link, running dhcp client..." 5 65
    IP_ADDRESS=`udhcpc -n -t 8 -T 5 -i $FOUND_IF -s /udhcpc.sh 2>&1 | grep lease | cut -d\  -f4 |  tr -d "\n"`
    if [ -z "$IP_ADDRESS" ]; then
#     dialog --backtitle "$backtitle" --sleep 5 --infobox "Couldn't acquire IP via DHCP..." 5 65
      dialog --backtitle "$backtitle" --sleep 5 --infobox "Couldn't acquire IP via DHCP...\n\nCan use setdhcpd to set eth0 to 192.168.0.1,\nand start local dhpcd server" 7 65
      CURR_INT=eth0
    else
      device=$FOUND_IF
      got_ip
      CURR_INT=$FOUND_IF
    fi
  else
    dialog --backtitle "$backtitle" --sleep 5 --infobox "Found $FOUND_IF but no link..." 5 65
    CURR_INT=eth0
  fi
  elif [ $IF_COUNT -gt 1 ]; then
  dialog --backtitle "$backtitle" --sleep 5 --infobox "Found multiple interfaces, determining one with an active link..." 5 65
  IF_ARRAY=`ifconfig -a | grep eth | awk {'print $1'}`
  FOUND_ACTIVE=0
  for CURR_INT in $IF_ARRAY
  do
    ifconfig $CURR_INT up; ## sleep 5
    dialog --backtitle "$backtitle" --sleep 5 --infobox "Checking status $CURR_INT for an active link." 5 65
    LINK_CHK=`ethtool $CURR_INT | grep "Link detected" | awk {'print $3'}`
    if [ "$LINK_CHK" = "yes" ]; then
      FOUND_ACTIVE=1
      dialog --backtitle "$backtitle" --sleep 5 --infobox "Found $CURR_INT with active link, running dhcp client..." 5 65
## Added code to check for dhcp address, since link without ip doesn't work.
      IP_ADDRESS=`udhcpc -n -t 8 -T 5 -i $CURR_INT -s /udhcpc.sh 2>&1 | grep lease | cut -d\  -f4 | tr -d "\n"`
      if [ -z "$IP_ADDRESS" ]; then
#       dialog --backtitle "$backtitle" --sleep 5 --infobox "Couldn't acquire IP via DHCP..." 5 65
        dialog --backtitle "$backtitle" --sleep 5 --infobox "Couldn't acquire IP via DHCP...\n\nCan use setdhcpd to set eth0 to 192.168.0.1,\nand start local dhpcd server" 7 65
        FOUND_ACTIVE=0
      else
        device=$CURR_INT
        got_ip
        break
      fi
##
    else
      FOUND_ACTIVE=0
    fi
  done
  if [ "$FOUND_ACTIVE" -eq "0" ]; then
    dialog --backtitle "$backtitle" --sleep 5 --infobox "Did not find any interfaces with an active link..." 5 65
    CURR_INT=eth0
    elif [ "$FOUND_ACTIVE" -eq "1" ]; then
# Run dhcp client
    IP_ADDRESS=`udhcpc -n -t 8 -T 5 -i $CURR_INT -s /udhcpc.sh 2>&1 | grep lease | cut -d\  -f4 | tr -d "\n"`
    if [ -z "$IP_ADDRESS" ]; then
#     dialog --backtitle "$backtitle" --sleep 5 --infobox "Couldn't acquire IP via DHCP..." 5 65
      dialog --backtitle "$backtitle" --sleep 5 --infobox "Couldn't acquire IP via DHCP...\n\nCan use setdhcpd to set eth0 to 192.168.0.1,\nand start local dhpcd server" 7 65
    else
      device=$CURR_INT
      got_ip
    fi
  else
    dialog --backtitle "$backtitle" --sleep 5 --infobox "Something went wrong during active link check..." 5 65
  fi
fi
device=$CURR_INT
got_ip
echo $CURR_INT >/tmp/device
dialog --backtitle "$backtitle" --sleep 5 --infobox "IP ADRESS $IP_ADDRESS ..." 5 65
