#!/usr/bin/env bash

COUNT=1
NULLSERV_IP=192\.168\.2\.3

for URL in "http://winhelp2002.mvps.org/hosts.txt" \
    "http://hosts-file.net/ad_servers.txt" \
    "http://pgl.yoyo.org/adservers/serverlist.php?hostformat=hosts&showintro=0&mimetype=plaintext" \
    "http://sysctl.org/cameleon/hosts" \
    "http://adaway.sufficientlysecure.org/hosts.txt"
do
    echo "Downloading ${URL}"
    wget -cq --timeout=3 --tries=2 ${URL} -O hosts.${COUNT}
    if [ $? -ne 0 ]; then
        echo "WARNING! wget failed. Skipping hosts from ${URL}"
        rm -f hosts.${COUNT} 2>/dev/null
    else
        dos2unix hosts.${COUNT}
    fi
    COUNT=$((${COUNT} + 1))
done

# Merge, clean and sort the hosts.
# Replace 127.0.0.1 with the nullserv IP address.
echo "Merging hosts"
grep -Ev --no-filename "localhost|google-analytics|www.googleadservices.com" hosts.* | tr '\t' ' ' | sed -e '/^#/d' -e 's/#.*$//' -e 's/  / /g' -e 's/ $//' -e "s/127\.0\.0\.1/${NULLSERV_IP}/" | sort -u > adaway.txt
wc -l adaway.txt

# Clean up
rm -f hosts.* 2>/dev/null

echo "All done!"
