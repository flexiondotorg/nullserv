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
was initial compiled from `swfdec_test_initialize.as` which is part of
[swfdec](http://swfdec.freedesktop.org) 0.9.2.

I developed `nullserv` on [Ubuntu](http://www.ubuntu.com) Lucid 10.04 LTS and
deployed it on a NSLU2 running [Debian](http://www.debian.org) Lenny.

# Install

    make
    sudo make install

# Usage

Add the following to `/etc/inetd.conf`

    www  stream  tcp  nowait  nobody  /usr/sbin/tcpd  /usr/local/bin/nullserv

Restart `inetd.conf`.

    sudo /etc/init.d/openbsd-inetd restart

Open a web browser and request anything you like from http://127.0.0.1 or
whatever the IP address is of the host where `nullserv` is installed. If you
request a file type that is not recognised by `nullserv` it with send back a 0
byte response of `Content-Type: text/plain`.

# Requirements

  * inetd (or similar).

# Changes

## v0.3 2013, 29my May.

  * Merged fixes and improvements contributed by Ben Shadwick.
  * Updated documentation.

## v0.2 2010, ??th September.

  * Added optional logging.

## v0.1 2010, 16th September.

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
