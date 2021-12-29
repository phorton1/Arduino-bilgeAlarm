# Bilge Alarm readme.md

## Internet Visibility

There are two approaches (thus far) to providing Internet Visibility of myIOT devices like the bilgeAlarm.

Both involve setting up piVPN on an rPi to encrypt LAN traffic before putting it on the net.

## Method 1 - public openVPN port 1194

This method is currently implemented and tested.

THX38, at current public IP address 190.140.69.189 is configured on THX38 to have a Dynamic DNS
entry from my noIP.com account to **myiot.dynns.com**

Please see **/zip/_rPi/_setup/prh-README-rPi** MyIOTSetup.docx for details on how to install piVPN, as well
as **/zip/WIFI_SETUP.docx** for how the LAN is configured so that piVPN has a fixed IP address 10.237.37.110
and forwards it's port 1194 thru THX37 and THX38 to the internet.

The bilgeAlarm has a fixed address of 10.237.37.120 for ease of use locally and via the VPN.

One *can* forward the ports 80 and 81 from the bilgeAlarm as 7800 and 7801 thru THX37 and THX38
as well for testing in unprotected HTTP mode.

The VPN only accepts routes to 10.237.37.*, so the bilgeAlarm can be hit from THX36 when
openVPN is running there at it's "local" address of 10.237.37.120.

Note that this ONLY works with THX38 which has an actual public IP address, and
THX38 and THX37 configured correctly to forward port 1194 in and out.


## Method 2 - SSH forwarding OpenVPN port 1194 from rPi to mbeSystems.net

This method of tunneling would not require Dynamic DNS or a public IP address.

It has not been implemented or tested.

This *theoretically* has the advantages that (a) it could be used on THX36 and in other
configurations that don't have public facing IP addresses, behind arbitrarily complex
LAN configurations, (b) it does not, per se, require a fixed IP address for the rPI,
and (c) does not require a paying service like noIP.com for Dynamic DNS services.


## General Problem that all Client Traffic is going through piVPN

- I was not able to easily get a good log of all VPN traffic on the rPi.
- I am surmising traffic usage from the OpenVPN Connect graphic, FAST.COM, and "my ip" websites

### rPi Server configuration

Commented much of rPi /etc/openvpn/server.conf out,

```
# push "dhcp-option DNS 9.9.9.9"
# push "dhcp-option 10.8.0.0 255.255.255.0"
# push "block-outside-dns"
# push "redirect-gateway def12"
# client-to-client
```

And turned on the duplicate-cn option to allow multiple
clients to use the same client certs(myiot.opvn) opvn file.

```
# PRH uncommented this line
duplicate-cn
```

and after any changes to server.conf, you must then run

```
$> sudo systemctl restart openvpn
```

### Client OPVN configuration

Added the following to /zip/_rpi/_setup/myiot.opvn (which
must be re-stuck in the openVPN clients on Win and the iPad
any time it changes, which furthermore requirs the use of
FileBrowser::openDAV server on the ipad:

```
# don't pull any routes from the VPN server
no-pull
# provide an explicit route to the THX37 (myIOT) subnet
route 10.237.37.0 255.255.255.0
```

And it appears to be somewhat working, allowing fast internet,
yet access to the VPN via THX36, however, at this time, I am
only connect one device via THX36 to the bilgeAlarm via the
VPN at a time.  Something to do with websockets.

So ... trying to make a separate myios.opvn for the iPad
with same password and manual edits.


## NOTES

Bsrely able to get an OTA to work from THX36 yet.

There are still lots of issues with the slowness of webSockets,
and openVPN disconnects that occur sporadically.

-----------

Eventually, I suppose, the rPi could implement SSDP search to "find" myIOTDevices.
You can "see" the bilgeAlarm and click on it in the Win10 Explorer Network tab.

The VPN is also providing SSL security, otherwise I could forward ports direc

I might want to forward other ports, like NODE-RED or MQTT (Mosquito Server).
Using the VPN does not require me to make each service SSL aware,
