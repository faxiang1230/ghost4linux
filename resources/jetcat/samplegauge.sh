#/bin/sh
size=102400
var=0

rm /tmp/file.out

dd if=file.img 2>/dev/null |  jetcat-mod -p $size 2>/tmp/file.out | dd of=test.img 2>/dev/null &

(
while test "$var" != "100"
do
var=`tail -n1 /tmp/file.out `
echo "XXX"
echo $var
echo "NEW TEXT"

echo "XXX"
done
) | dialog --gauge "TEXT" 0 0
