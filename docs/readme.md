# Bilge Alarm readme.md

## Internet Visibility

There are two approaches (thus far) to providing Internet Visibility of myIOT devices like the bilgeAlarm.

Both involve setting up piVPN on an rPi to encrypt LAN traffic before putting it on the net.

Please see /zip/_rPi/_setup/prh-README-rPi MyIOTSetup.docx for details on how to install piVPN, as well
as /zip/WIFI_SETUP.docx for how the LAN is configured so that piVPN has a fixed IP address (for method 1),
and how the bilgeAlarm has a fixed address for ease of use locally and via VPN (though the rPi could implement
SSDP to "find" myIOTDevices AND you can "see" the bilgeAlarm and click on it in the Win10 Explorer Network tab,
I am not letting SSDP out over the internet and it doesn't seem to find it on the VPN)

## Method 1 = Exposing piVPN Port 1194 to the internet

**Requires that the rPi has a fixed IP address** which we might want anyways for things like NODE-RED
or it's MQTT (Mosquito Server).

Basically this amounts to giving the rPi a fixed IP address on THX37 and Forwarding piVPBN port 1194
(in the THX38 configuration) to that fixed IP:1194.

One *can* forward the ports 80 and 81 from the bilgeAlarm at a fixed IP address as well, to say
7800 and 7801 for testing by placing unprotected HTTP on the internet, but that's a bad idea, eh?

The major variation (implemented) in this is to use a paid account at noIP.com with THX38 routere
configuration to use the Dynamic DNS service, thus making the VPN available at **myiot.dynns.com**

Note that this ONLY works with THX38 which has an actual public IP address.
This technique could not be used to expose myIOT device (or the piVPN) on THX36 (MarinaAdmin) because
THX36 is under another layer of routers and does not have a public IP address.

## Method 2 = tunnelling port 1194 through to one of the mbeSystems.net SSH ports

This *theoretically* has the advantages that (a) it could be used on THX36 and in other
configurations that don't have public facing IP addresses, behind arbitrarily complex
LAN configurations, (b) it does not, per se, require a fixed IP address for the rPI,
and (c) does not require a paying service like noIP.com for Dynamic DNS services.

# General Problem that all Client Traffic is going through piVPN

- I was not able to easily get a good log of all VPN traffic on the rPi.
- I am surmising traffic usage from the OpenVPN Connect graphic, FAST.COM, and "my ip" websites

Commented much of rPi /etc/openvpn/server.conf out,

```
# push "dhcp-option DNS 9.9.9.9"
# push "dhcp-option 10.8.0.0 255.255.255.0"
# push "block-outside-dns"
# push "redirect-gateway def12"
# client-to-client
```

and added an explicit routing to my IOT device LAN

```
push "route 10.237.37.0 255.255.255.0"
```

```
$> sudo systemctl restart openvpn
```


And it appears to have worked.


## NOTES

There are still lots of issues with the slowness of webSockets,
and haven't been able to get an OTA to work from THX36 yet.

It is generally slow.
