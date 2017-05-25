#!/bin/sh
#
#
## Michael Setzer II - Program Modifications
projectname="GetKey"
AESPIPE=""
AESPIPEd=""
#AESPIPE=" aespipe -K /mykey1.gpg -P /passphrase -G / | "
#AESPIPEd="| aespipe -K /mykey1.gpg -P /passphrase -G / -d "

: ${DIALOG=dialog}
: ${DIALOG_OK=0}
: ${DIALOG_CANCEL=1}
: ${DIALOG_ESC=255}


createTemp()
{
  export tempfile${1}="/tmp/test${1}$$"
}

# local drive
createTemp localdrive
createTemp localname

netdevices=`cat /proc/net/dev |grep eth |cut -d: -f 1`
partitions=`cat /proc/partitions |grep .d |awk '{print $4}'`

### 2008/5/27 New partlist to have size and partitiion type.
partlist="";
fdisk -l 2>/dev/null | grep /dev | grep -v Disk | sed s/*/\ / | awk '{print $1,$6$7$8}' | cut -b6-30 >/tmp/partout
cat /proc/partitions |grep .d | awk '{print $4,$3}' >/tmp/sysinfo2
while read LINE
do
#    xxdev=`echo $LINE | cut -b1-4`;
#    xxsize=`echo $LINE | cut -b5-30`;
  xxdev=`echo $LINE | cut -f1 -d\  `;
  xxsize=`echo $LINE | cut -f2 -d\  `;
  xxsize2=`echo ${#xxsize}`;
#    dev2=`echo $xxdev | cut -b4-4`;
#    if [ "$xxdev2" = "" ]; then
  pad="";
#    else
#      pad="_";
#    fi
    a=$xxsize2;
    while [ $a -le 12 ]
#   for (a=xxsize2;a<=12;a++) Doesn't work on new bash?
#   for ((a=xxsize2;a<=12;a++))
  do
    pad=$pad\_;
  done
  xxtype="";
  xxtype=`cat /tmp/partout | grep $xxdev\  2>/dev/null |  cut -f2 -d\  `;
  if [ -z $xxtype ]; then
    xxtype="DISK";
  fi
  partlist="$partlist `printf "%s %s%s--%s off" $xxdev  $pad $xxsize $xxtype `";
done </tmp/sysinfo2

### End of addition

for j in $netdevices
do netlist="$netlist `echo $j detected...OK off`"
done

$DIALOG --backtitle "$backtitle" \
--title "PICK DRIVE" \
--radiolist "Choose drive to write to/read from.\n(Use Space Bar)\n\
The drive that contains or will contain the image\
\nSupported FS are:\n\
ext2 - Linux\n\
ext3 - Linux\n\
reiserfs - Linux\n\
fat32 - Windows95/98/NT/2k/XP\n\
ntfs - Windows NT/2k/XP\
\n\nSelect partition (example: /dev/hda1):" 0 0 0 \
$partlist 2> $tempfilelocaldrive


retvallocaldrive=$?

choicelocaldrive=`cat $tempfilelocaldrive`
case $retvallocaldrive in
$DIALOG_OK)

localdrive=$choicelocaldrive
###
umount /mnt/local 2>/dev/null
mount /dev/$localdrive /mnt/local >/tmp/mount 2>&1
retvalmount=$?
if [ $retvalmount != 0 ] ; then
  $DIALOG --backtitle "$backtitle" \
  --title "ERROR" \
  --textbox /tmp/mount 0 0
  localdrive=""
  umount /mnt/local
## July 25m 2008
## Obsolete with addition of mount.helpers.
##                else
##                  testntfs=`cat /etc/mtab | grep /dev/$localdrive | cut -f 3 -d\  `
##                  if [ "$testntfs" = "ntfs" ]; then
##                    umount /mnt/local 2>/dev/null
##                    ntfs-3g /dev/$localdrive /mnt/local >/tmp/mount 2>&1
##                  fi
fi
esac
files=`ls -1 -p /mnt/local/*.gpg `
###filelist="`echo Enter Filename on`"
for i in $files
do filelist="$filelist `echo $i file off`"
done

$DIALOG --backtitle "$backtitle" \
--title "BACKUP" \
--radiolist "Choose GPG Key File \
\n Select from following files:" 0 0 0 \
$filelist 2> $tempfilelocalname

retvallocalname=$?
localimagename=`cat $tempfilelocalname`
cp $localimagename /mykey1.gpg
files=""
filelist=""
files=`ls -1 -p /mnt/local/pass* `
###filelist="`echo Enter Filename on`"
for i in $files
do filelist="$filelist `echo $i file off`"
done

$DIALOG --backtitle "$backtitle" \
--title "BACKUP" \
--radiolist "Choose Passphrase File \
\n Select from following files:" 0 0 0 \
$filelist 2> $tempfilelocalname

retvallocalname=$?
localimagename=`cat $tempfilelocalname`
cp $localimagename /passphrase
umount /mnt/local

echo "TESTING KEY and Passphrase"
cd /
dd if=/dev/urandom of=./aes1.test bs=512 count=40
aespipe -K ./mykey1.gpg -P ./passphrase -G ./ <./aes1.test >./aes2.test
aespipe -K ./mykey1.gpg -P ./passphrase -G ./ -d <./aes2.test >./aes3.test
cmp -l ./aes1.test ./aes3.test

echo "cmp should result in no difference"
rm aes?.test -f

