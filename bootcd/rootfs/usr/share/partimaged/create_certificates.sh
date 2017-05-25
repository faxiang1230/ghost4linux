#!/bin/sh

opensslpath=/usr/bin/openssl

$opensslpath req -new -x509 -nodes -config \
           /etc/partimaged/partimage-certs.cnf \
           -out /etc/partimaged/partimaged.cert -keyout \
           /etc/partimaged/partimaged.key

rm -f /etc/partimaged/partimaged.csr

chmod 600 /etc/partimaged/partimaged.key
chmod 600 /etc/partimaged/partimaged.cert
chown partimag:partimag /etc/partimaged/partimaged.key
chown partimag:partimag /etc/partimaged/partimaged.cert
