License

nullserv is a minimal webserver. It's only purpose is serving up null content.
Copyright (c) 2010 Flexion.Org, http://flexion.org/

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

Introduction

I live in an area where reliable DSL is not available, therefore I access the 
Internet at home using 3G mobile broadband. However, my 3G mobile broadband 
contract is limited to 15GB of bandwidth per month, if I exceed that limit then 
I start incurring additional charges which can mount up very quickly.

nullserv is a minimal webserver. It's only purpose is serving up null content. 
nullserv was inspired by pixelserv which only serves a 1x1 pixel transparent gif 
file. nullserv expands on this idea by serving up null .html, .js, .css, .php, 
.cgi, .pl, .asp, .aspx, .txt, .gif, .png, .jpeg and .swf files. 

Using some creative firewalling (netfilter/iptables) rules or local DNS you can 
redirect some requests (for adverts for example) to nullserv, thus saving 
bandwidth :-)

nullserv is heavily based on in.www which is part of the excellent inetdextra
package. The null .swf is compiled from 'swfdec_test_initialize.as' which is 
part of swfdec 0.9.2.

I've developed nullserv on Ubuntu Lucid 10.04 LTS and deployed it on my 
NSLU2 running Debian Lenny.

Install

  make
  sudo make install
  
Usage

Add the following to /etc/inetd.conf

  www  stream  tcp  nowait  nobody  /usr/sbin/tcpd  /usr/local/bin/nullserv

Restart inetd.conf.

  sudo /etc/init.d/openbsd-inetd restart

Open a web browser and request anything you like from http://127.0.0.1 or 
whatever the IP address is of the host you install nullserv on. If you request 
a file type that is not recognised by nullserv it with send back a 0 byte 
response of Content-Type: text/plain.

Requirements

 - inetd (or similar).
   
Known Limitations

 - I am not a C programmer ;-)
 
To Do

 - Document dnsmasq and in.dns configuration to compliment nullserv.
 
Source Code

You can grab the source from Launchpad. Contributions are welcome :-)

 - https://code.launchpad.net/~flexiondotorg

References

 - http://proxytunnel.sourceforge.net/pixelserv.php
 - http://inetdxtra.sourceforge.net/
 - http://swfdec.freedesktop.org/

v0.2 2010, ??th September.
 - Added optional logging.

v0.1 2010, 16th September.
 - Initial release
