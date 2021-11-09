// Captive Portal
//
//    The captive portal works by virtue of the fact that the various OS's (and browsers)
//    will make requests upon a new connection to an Access Points.  Some of these are
//    specifically intended, and acted upon, by the OS, to popup a browser/html viewer
//    and display the results of the requests.  Others are just "junk".
//
//    We intercept any DNS requests and map them to the SoftAP ip address.
//       ** weirdness with setting softAP ip (order of operations, maybe limitations on ip addresses)
//
//    Then, basically, for any files not found, we return an HTML page that has a javascript timed
//      redirection through "window.location=" for any "not found" requests.
//
//    - Hard 302 redirects work on Windows/Firefox but not on IOS
//    - You can even spoof a domain name (i.e. bilgAlarm.local) on Win10, but not on IOS
//    - IOS and Android bring the page up in a "HTML Viewer" which is not the browser (chrome),
//      so it is not clear if you can actually USE those pages. You'd have to "remember the ip"
//      although most domains (i.e. aaa.bbb) will "work" ... but not google.com on android which
//      is "built into" the chrome web browser.
//    - Everyone seems to be expecting some kind of Authentication to occur.
//    - The design of Captive Portals seems to be specifically linked to Authentication
//      and remembering it, somehow, for use in "general" browser accesses.
//
// Captive Portal DNS/HTTP requests
//
//    Windows:
//          msftconnecttest.com/connecttest.txt
//          msftconnecttest.com/redirect
//          msftconnecttest.com/success.txt
//       Firefox:
//          detectportal.firefox.com/canonical.html
//    Android:
//          connectivitycheck.gstatic.com/generate_204
//    IOS:
//          captive.apple.com/hotspot-detect.html
//    In fact, a lot of stuff happens on starting a new AP connection.
//    Note all the DNS lookups on
//      Win10:
//          mbeSystems.net!! (my cmServer apparently)
//          example.org
//          ipv4only.arpa
//        Firefox:
//          push.services.mozilla.com
//      IOS:
//          gsp85-ssl.ls.apple.com
//      Android:
//          mtalk.google.com/ (http request)
//          resolver.msg.global.xiaomi.net/gslb/ (http request)
//          app.chat.global.xiaomi.net
//          alt8-mtalk.google.com
//          connect.rom.miui.com
//          userlocation.googleapis.com
//          alt6-mtalk.google.com
//          alt5-mtalk.google.com
//          resolver.msg.global.xiaomi.net
//          play.googleapis.com
//          android.googleapis.com
//          alt7-mtalk.google.com
//          path1.xtracloud.net
//          path2.xtracloud.net
//          path3.xtracloud.net
//          app.chat.global.xiaomi.net
//          connect.rom.miui.com
//          alt4-mtalk.google.com
//          people-pa.googleapis.com
//          app.chat.global.xiaomi.net
//          accounts.google.com
//          metok-ccc.intl.xiaomi.com
//  (domains determined by putting debugging in
//      C:\Users\Patrick\AppData\Local\Arduino15\packages\esp32\hardware\esp32\1.0.6\libraries\DNSServer\src)
//      DNSServer.cpp as there is no virtual API or history ..
