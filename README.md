# Introduction

I live in an area where reliable DSL is not available, therefore I access the
Internet at home using 3G mobile broadband and short wave radio. However, my 3G
mobile broadband contract is limited to 15GB of bandwidth per month, if I exceed
that limit then I start incurring additional charges which can mount up very
quickly.

`nullserv` is a minimal webserver. It's only purpose is to serve up null content.
`nullserv` was inspired by [pixelserv](http://proxytunnel.sourceforge.net/pixelserv.php)
which serves a 1x1 pixel transparent `.gif`. `nullserv` expands on this idea by
serving up null `.html`, `.js`, `.css`, `.php`, `.cgi`, `.pl`, `.asp`, `.aspx`,
`.txt`, `.gif`, `.png`, `.jpeg` and `.swf` files.

Using some creative firewalling (netfilter/iptables) rules or local DNS you can
redirect some requests (for adverts for example) to `nullserv`, thus saving
bandwidth :-)

`nullserv` is heavily based on `in.www` which is part of the excellent
[inetdextra](http://inetdxtra.sourceforge.net/) package. The null `.swf`
is compiled from `swfdec_test_initialize.as` which is part of
[swfdec](http://swfdec.freedesktop.org) 0.9.2.

I developed `nullserv` on [Ubuntu](http://www.ubuntu.com) Lucid 10.04 LTS and
deployed it on a NSLU2 running [Debian](http://www.debian.org) Lenny.

# Install

    make
    sudo make install

# Requirements

  * inetd (or similar).

# Usage

Currently, I'm using `dnsmasq` on a [Gargoyle](http://www.gargoyle-router.com/)
router to redirect ad-server IP addresses to `nullserv` running on an embedded Debian
Squeeze box. Gargoyle is based on OpenWRT, so this technique should also work with
OpenWRT.

## Debian Squeeze

Install `git` and the compiler toolchain.

    sudo apt-get install build-essential git-core

Compile and install `nullserv`

    git clone https://github.com/flexiondotorg/nullserv.git
    cd nullserv
    make
    sudo make install

Install OpenBSD inetd.

    sudo apt-get install openbsd-inetd

Add the following to `/etc/inetd.conf`

    www  stream  tcp  nowait  nobody  /usr/sbin/tcpd  /usr/bin/nullserv

Restart `inetd.conf`.

    sudo /etc/init.d/openbsd-inetd restart

Some ads are served via https, we can use `stunnel` to forward https 
connections to `nullserv`.

Install `stunnel`

    sudo apt-get install stunnel

Create SSL certificate and a key.

    sudo openssl req -new -nodes -x509 -out /etc/ssl/certs/stunnel.pem -keyout /etc/ssl/certs/stunnel.pem

Enable the `[https]` section in `/etc/stunnel.conf`, I also disable the `[pop3s]`,
`[imaps]` and `[ssmtp]` as I don't require them.

    [https]
    accept  = 443
    connect = 80
    TIMEOUTclose = 0

Enable `stunnel` in `/etc/default/stunnel4`. Find `ENABLED` and set it to `1`.

    ENABLED=1

Start `stunnel`.

    sudo /etc/init.d/stunnel4 start

Open a web browser and request anything you like from http://192.168.2.1 or
whatever the IP address is the host where `nullserv` is installed. If you
request a file type that is not recognised by `nullserv` it with send back a 0
byte response of `Content-Type: text/plain`.

Run `contrib/adaway.sh` and then `scp` the generated `adaway.txt` to `/etc/` 
on your Gargoyle/OpenWRT router.

  cd contrib
  ./adaway.sh
  scp adaway.txt root@192.168.2.1:/etc/  

## Gargoyle/OpenWRT

Add the following to `/etc/config/dhcp` under the `config dnsmasq` section.

   list addnhosts '/etc/adaway.txt'
   
Restart `dnsmasq`

    /etc/init.d/dnsmasq restart

You should now find that ads are blocked and replaced with null content.
You can do lots more with dnasmasq on Gargoyle/OpenWRT, see the Wiki:

  * <http://wiki.openwrt.org/doc/uci/dhcp>

# Changes

## v0.3 2013, 29th May.

  * Merged fixes and improvements contributed by Ben Shadwick.
  * Updated documentation.

## v0.2 2010, 16th September.

  * Added optional logging.

## v0.1 2010, 15th September.

  * Initial release

# Known Limitations

  * I am not a C programmer ;-)

# To Do

  * Document `dnsmasq` and `in.dns` configurations to compliment `nullserv`.

# Source Code

You can grab the source from Launchpad. Contributions are welcome :-)

  * https://github.com/flexiondotorg/nullserv

# License

A minimal webserver for serving up null content.
Copyright (c) 2013 Flexion.Org, http://flexion.org/

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
