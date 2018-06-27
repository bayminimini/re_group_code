#100bao
^\x01\x01\x05\x0a
#aim
^(\*[\x01\x02].*\x03\x0b|\*\x01.?.?.?.?\x01)|flapon|toc_signon.*0x
#aimwebcontent
user-agent:aim/
#applejuice
^ajprot\x0d\x0a
#ares(ATTENTION: CANNOT BE IDENTIFIED FOR EMPTY RANGE)
#^\x03[]Z].?.?\x05$
#armagetron
YCLC_E|CYEL
#battlefield1942
^\x01\x11\x10\|\xf8\x02\x10\x40\x06
#battlefield2(ATTENTION: CANNOT BE IDENTIFIED FOR EMPTY RANGE) 
#^(\x11\x20\x01...?\x11|\xfe\xfd.?.?.?.?.?.?(\x14\x01\x06|\xff\xff\xff))|[]\x01].?battlefield2
#battlefield2142
^(\x11\x20\x01\x90\x50\x64\x10|\xfe\xfd.?.?.?\x18|[\x01\\].?battlefield2)
#bgp
^\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff..?\x01[\x03\x04]
#biff
^[a-z][a-z0-9]+@[1-9][0-9]+
#bittorrent	
^(\x13bittorrent protocol|azver\x01|get /scrape\?info_hash=get /announce\?info_hash=|get /client/bitcomet/|GET /data\?fid=)|d1:ad2:id20:|\x08'7P\)[RP]
#chikka
^CTPv1\.[123] Kamusta.*\x0d\x0a
#cimd
\x02[0-4][0-9]:[0-9]+.*\x03
#ciscovpn
^\x01\xf4\x01\xf4
#citrix
\x32\x26\x85\x92\x58
#counterstrike-source
^\xff\xff\xff\xff.*cstrikeCounter-Strike
#cvs
^BEGIN (AUTH|VERIFICATION|GSSAPI) REQUEST\x0a
#dayofdefeat-source
^\xff\xff\xff\xff.*dodDay of Defeat
#dazhihui
^(longaccoun|qsver2auth|\x35[57]\x30|\+\x10\*)
#dhcp
^[\x01\x02][\x01- ]\x06.*c\x82sc
#directconnect
^(\$mynick |\$lock |\$key )
#dns
^.?.?.?.?[\x01\x02].?.?.?.?.?.?[\x01-?][a-z0-9][\x01-?a-z]*[\x02-\x06][a-z][a-z][fglmoprstuvz]?[aeop]?(um)?[\x01-\x10\x1c][\x01\x03\x04\xFF]
#doom3
^\xff\xffchallenge
#edonkey
^[\xc5\xd4\xe3-\xe5].?.?.?.?([\x01\x02\x05\x14\x15\x16\x18\x19\x1a\x1b\x1c\x20\x21\x32\x33\x34\x35\x36\x38\x40\x41\x42\x43\x46\x47\x48\x49\x4a\x4b\x4c\x4d\x4e\x4f\x50\x51\x52\x53\x54\x55\x56\x57\x58[\x60\x81\x82\x90\x91\x93\x96\x97\x98\x99\x9a\x9b\x9c\x9e\xa0\xa1\xa2\xa3\xa4]|\x59................?[ -~]|\x96....$)
#fasttrack
^get (/.download/[ -~]*|/.supernode[ -~]|/.status[ -~]|/.network[ -~]*|/.files|/.hash=[0-9a-f]*/[ -~]*) http/1.1|user-agent: kazaa|x-kazaa(-username|-network|-ip|-supernodeip|-xferid|-xferuid|tag)|^give [0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9]?[0-9]?[0-9]?
#finger
^[a-z][a-z0-9\-_]+|login: [\x09-\x0d -~]* name: [\x09-\x0d -~]* Directory: 
#freenet
^\x01[\x08\x09][\x03\x04]
#ftp
^220[\x09-\x0d -~]*ftp
#gkrellm
^gkrellm [23].[0-9].[0-9]\x0a
#gnucleuslan
gnuclear connect/[\x09-\x0d -~]*user-agent: gnucleus [\x09-\x0d -~]*lan:
#gnutella
^(gnd[\x01\x02]?.?.?\x01|gnutella connect/[012]\.[0-9]\x0d\x0a|get /uri-res/n2r\?urn:sha1:|get /.*user-agent: (gtk-gnutella|bearshare|mactella|gnucleus|gnotella|limewire|imesh)|get /.*content-type: application/x-gnutella-packets|giv [0-9]*:[0-9a-f]*/|queue [0-9a-f]* [1-9][0-9]?[0-9]?\.[1-9][0-9]?[0-9]?\.[1-9][0-9]?[0-9]?\.[1-9][0-9]?[0-9]?:[1-9][0-9]?[0-9]?[0-9]?|gnutella.*content-type: application/x-gnutella|...................?lime)
#goboogy
<peerplat>|^get /getfilebyhash\.cgi\?|^get /queue_register\.cgi\?|^get /getupdowninfo\.cgi\?
#gopher
^[\x09-\x0d]*[1-9,+tgi][\x09-\x0d -~]*\x09[\x09-\x0d -~]*\x09[a-z0-9.]*\.[a-z][a-z].?.?\x09[1-9]
#guildwars
^[\x04\x05]\x0c.i\x01
#h323
^\x03..?\x08...?.?.?.?.?.?.?.?.?.?.?.?.?.?.?\x05
#halflife2-deathmatch
^\xff\xff\xff\xff.*hl2mpDeathmatch
#hddtemp
^\|/dev/[a-z][a-z][a-z]\|[0-9a-z]*\|[0-9][0-9]\|[cfk]\|
#hotline
^....................TRTPHOTL\x01\x02
#http
http/(0\.9|1\.0|1\.1) [1-5][0-9][0-9] [\x09-\x0d -~]*(connection:|content-type:|content-length:|date:)|post [\x09-\x0d -~]* http/[01]\.[019]
#http-rtsp
^(get[\x09-\x0d -~]* Accept: application/x-rtsp-tunnelled|http/(0\.9|1\.0|1\.1) [1-5][0-9][0-9] [\x09-\x0d -~]*a=control:rtsp://)
#ident
^[1-9][0-9]?[0-9]?[0-9]?[0-9]?[\x09-\x0d]*,[\x09-\x0d]*[1-9][0-9]?[0-9]?[0-9]?[0-9]?(\x0d\x0a|[\x0d\x0a])?
#imap
^(\* ok|a[0-9]+ noop)
#imesh
^(post[\x09-\x0d -~]*<PasswordHash>................................</PasswordHash><ClientVer>|\x34\x80?\x0d?\xfc\xff\x04|get[\x09-\x0d -~]*Host: imsh\.download-prod\.musicnet\.com|\x02[\x01\x02]\x83.*\x02[\x01\x02]\x83)
#ipp
ipp://
#irc
^(nick[\x09-\x0d -~]*user[\x09-\x0d -~]*:|user[\x09-\x0d -~]*:[\x02-\x0d -~]*nick[\x09-\x0d -~]*\x0d\x0a)
#jabber
<stream:stream[\x09-\x0d ][ -~]*[\x09-\x0d ]xmlns=['"]jabber
#kugoo
^(\x64.....\x70....\x50\x37|\x65.+)
#live365
membername.*session.*player
#liveforspeed
^..\x05\x58\x0a\x1d\x03
#lpd
^(\x01[!-~]+|\x02[!-~]+\x0a.[\x01\x02\x03][\x01-\x0a -~]*|[\x03\x04][!-~]+[\x09-\x0d]+[a-z][\x09-\x0d -~]*|\x05[!-~]+[\x09-\x0d]+([a-z][!-~]*[\x09-\x0d]+[1-9][0-9]?[0-9]?|root[\x09-\x0d]+[!-~]+).*)\x0a
#mohaa
^\xff\xff\xff\xffgetstatus\x0a
#msn-filetransfer
^(ver [ -~]*msnftp\x0d\x0aver msnftp\x0d\x0ausr|method msnmsgr:)
#msnmessenger
ver [0-9]+ msnp[1-9][0-9]? [\x09-\x0d -~]*cvr0\x0d\x0a$|usr 1 [!-~]+ [0-9. ]+\x0d\x0a$|ans 1 [!-~]+ [0-9. ]+\x0d\x0a
#mute
^(Public|AES)Key: [0-9a-f]*\x0aEnd(Public|AES)Key\x0a
#napster
^(.[\x02\x06][!-~]+ [!-~]+ [0-9][0-9]?[0-9]?[0-9]?[0-9]? "[\x09-\x0d -~]+" ([0-9]|10)|1(send|get)[!-~]+ "[\x09-\x0d -~]+")
#nbns
\x01\x10\x01|\)\x10\x01\x01|0\x10\x01
#ncp
^(dmdt.*\x01.*(""|\x11\x11|uu)|tncp.*33)
#netbios
\x81.?.?.[A-P][A-P][A-P][A-P][A-P][A-P][A-P][A-P][A-P][A-P][A-P][A-P][A-P][A-P][A-P][A-P][A-P][A-P][A-P][A-P][A-P][A-P][A-P][A-P][A-P][A-P][A-P][A-P][A-P][A-P][A-P][A-P][A-P][A-P][A-P][A-P][A-P][A-P][A-P][A-P]
#nntp
^(20[01][\x09-\x0d -~]*AUTHINFO USER|20[01][\x09-\x0d -~]*news)
#ntp
^([\x13\x1b\x23\xd3\xdb\xe3]|[\x14\x1c$].......?.?.?.?.?.?.?.?.?[\xc6-\xff])
#openft
x-openftalias: [-)(0-9a-z ~.]
#pcanywhere
^(nq|st)
#poco
^\x80\x94\x0a\x01....\x1f\x9e
#pop3
^(\+ok |-err )
#pplive
\x01...\xd3.+\x0c.
#qq
^.?.?\x02.+\x03
#quake1
^\x80\x0c\x01quake\x03
#quake-halflife
^\xff\xff\xff\xffget(info|challenge)
#radmin
^\x01\x01(\x08\x08|\x1b\x1b)
#rdp
rdpdr.*cliprdr.*rdpsnd
#replaytv-ivs
^(get /ivs-IVSGetFileChunk|http/(0\.9|1\.0|1\.1) [1-5][0-9][0-9] [\x09-\x0d -~]*\x23\x23\x23\x23\x23REPLAY_CHUNK_START\x23\x23\x23\x23\x23)
#rlogin
^[a-z][a-z0-9][a-z0-9]+/[1-9][0-9]?[0-9]?[0-9]?00
#rtp
^\x80[\x01-"`-\x7f\x80-\xa2\xe0-\xff]?..........*\x80
#rtsp
rtsp/1.0 200 ok
#runesofmagic
^\x10\x03...........\x0a\x02.....\x0e
#shoutcast
^get /.*icy-metadata:1|icy [1-5][0-9][0-9] [\x09-\x0d -~]*(content-type:audio|icy-)
#sip
^(invite|register|cancel|message|subscribe|notify) sip[\x09-\x0d -~]*sip/[0-2]\.[0-9]
#skypeout (ATTENTION: THIS PATTERN CAUSE SEGMENT FAULT IN BECCHI'S TOOL)
#skypetoskype
^..\x02.............
#smb
\xffsmb[\x72\x25]
#smtp
^220[\x09-\x0d -~]* (e?smtp|simple mail)
#snmp
^\x02\x01\x04.+([\xa0-\xa3]\x02[\x01-\x04].?.?.?.?\x02\x01.?\x02\x01.?\x30|\xa4\x06.+\x40\x04.?.?.?.?\x02\x01.?\x02\x01.?\x43)
#socks
\x05[\x01-\x08]*\x05[\x01-\x08]?.*\x05[\x01-\x03][\x01\x03].*\x05[\x01-\x08]?[\x01\x03]
#soribad
^GETMP3\x0d\x0aFilename|^\x01.?.?.?(\x51\x3a\+|\x51\x32\x3a)|^\x10[\x14-\x16]\x10[\x15-\x17].?.?.?.?
#soulseek
^(\x05..?|.\x01.[ -~]+\x01F..?.?.?.?.?.?.?)
#ssdp
^notify[\x09-\x0d ]\*[\x09-\x0d ]http/1\.1[\x09-\x0d -~]*ssdp:(alive|byebye)|^m-search[\x09-\x0d ]\*[\x09-\x0d ]http/1\.1[\x09-\x0d -~]*ssdp:discover
#ssh
^ssh-[12]\.[0-9]
#(from where?????????????????????????????????)
#aes256-cbc|rijndael-cbc@lysator.liu.sefaes128-cbc|3des-cbc|blowfish-cbc|cast128-cbc|arcfour|aes192-cbc|aes256-cbc|rijndael-cbc@lysator.liu.seuhmac-md5|hmac-sha1|hmac-ripemd160)+
#ssl
^(.?.?\x16\x03.*\x16\x03|.?.?\x01\x03\x01?.*\x0b)
#stun
^[\x01\x02]................?
#subspace
^\x01....\x11\x10........\x01
#subversion
^\( success \( 1 2 \(
#teamfortress2
^\xff\xff\xff\xff.....*tfTeam Fortress
#teamspeak
^\xf4\xbe\x03.*teamspeak
#telnet
^\xff[\xfb-\xfe].\xff[\xfb-\xfe].\xff[\xfb-\xfe]
#tesla
\x03\x9a\x89\x22\x31\x31\x31\.\x30\x30\x20\x42\x65\x74\x61\x20|\xe2\x3c\x69\x1e\x1c\xe9
#tftp
^(\x01|\x02)[ -~]*(netascii|octet|mail)
#thecircle
^t\x03ni.?[\x01-\x06]?t[\x01-\x05]s[\x0a\x0b](glob|who are you$|query data)
#tonghuashun
^(GET /docookie\.php\?uname=|\xfd\xfd\xfd\xfd\x30\x30\x30\x30\x30)
#tor
TOR1.*<identity>
#tsp
^[\x01-\x13\x16-$]\x01.?.?.?.?.?.?.?.?.?.?[ -~]+
#(from where?????????????????????????????????)
#.
#(from where?????????????????????????????????)
#.
#uucp
^\x10here=
#validcertssl
^(.?.?\x16\x03.*\x16\x03|.?.?\x01\x03\x01?.*\x0b).*(thawte|equifax secure|rsa data security, inc|verisign, inc|gte cybertrust root|entrust\.net limited)
#ventrilo
^..?v\$\xcf
#vnc
^rfb 00[1-9]\.00[0-9]\x0a
#whois
^[ !-~]+\x0d\x0a
#worldofwarcraft
^\x06\xec\x01
#x11
^[lb].?\x0b
#xboxlive
^\x58\x80........\xf3|^\x06\x58\x4e
#xunlei
^([()]|get)(...?.?.?(reg|get|query)|.+User-Agent: (Mozilla/4\.0 \(compatible; (MSIE 6\.0; Windows NT 5\.1;? ?\)|MSIE 5\.00; Windows 98\))))|Keep-Alive\x0d\x0a\x0d\x0a[26]
#yahoo
^(ymsg|ypns|yhoo).?.?.?.?.?.?.?[lwt].*\xc0\x80
#zmaap
^\x1b\xd7\x3b\x48[\x01\x02]\x01?\x01
