#sccsid	newvers.sh	1.2	84/08/28
if [ ! -r version ]; then echo 0 > version; fi
touch version
awk '	{	version = $1 + 1; }\
END	{	printf "char version[] = \"4.2 BSD UNIX #%d: ", version > "vers.c";\
		printf "%d\n", version > "version"; }' < version
echo `date`'\n    '$USER'@'`hostname`':'`pwd`'\n";' >> vers.c
