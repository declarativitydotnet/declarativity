import re
import socket

def getdnsips():
    try:
        ips = []
        f = open("/etc/resolv.conf")
        lines = f.readlines()
        f.close()
        for line in lines:
            m = re.match("^\W*nameserver\W+([\w\.]+)", line)
            if m:
                ips.append(m.group(1))
        return ips
    except:
        return []

def getlocalip():
    """Try TCP connections to DNS servers; use gethostbyname as last resort"""
    for ip in getdnsips():
        try:
            s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            s.connect((ip, 53))
            localaddr, localport = s.getsockname()
            s.close()
            return localaddr
        except:
            continue
    host = socket.getfqdn(socket.gethostname())
    return socket.gethostbyname(host)
