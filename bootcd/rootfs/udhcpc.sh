#!/bin/sh

# udhcpc script edited by Tim Riker <Tim@Rikers.org> and Devin Bayer

[ -z "$1" ] && echo "Error: should be called from udhcpc" && exit 1

{ # redirect to logger
  
  RESOLV_CONF="/etc/resolv.conf"
  [ -n "$broadcast" ] && BROADCAST="broadcast $broadcast"
  [ -n "$subnet" ] && NETMASK="netmask $subnet"
  
  case "$1" in
    deconfig)
    /sbin/ifconfig $interface 0.0.0.0
    ;;
    
    renew|bound)
    /sbin/ifconfig $interface $ip $BROADCAST $NETMASK
    
    if [ -n "$router" ] ; then
      echo "deleting routers for interface $interface"
      while route -n del default gw 0.0.0.0 dev $interface 2> /dev/null; do
      true
    done
    
    if route -n | egrep "^0.0.0.0"; then
      echo "default route already in place"
    else
      echo "adding default router $router"
      route add default gw $router dev $interface
    fi
  fi
  
  if (! grep -q -v DISABLE_DHCP $RESOLV_CONF) && echo -n > $RESOLV_CONF; then
    [ -n "$domain" ] && echo search $domain >> $RESOLV_CONF
    for i in $dns ; do
    echo adding dns $i
    echo nameserver $i >> $RESOLV_CONF
  done
fi
;;
esac
} 2>&1 | ( logger -s -p daemon.info -t uhdcp || cat )

exit
