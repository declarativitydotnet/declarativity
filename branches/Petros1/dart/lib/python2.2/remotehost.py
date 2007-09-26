import os
import tempfile
import StringIO

class RemoteHost:
    def __init__(self, host, verbose=None):
        self.host = host
        if verbose:
            self.out = ""
        else:
            self.out = ">& /dev/null"

    def xfer_buf_to(self, buf, dst, verbose=None):
        tmp = tempfile.mktemp()
        open(tmp, "w").write(buf)
        self.xfer_to(tmp, dst)
        os.unlink(tmp)

    def xfer_to(self, src, dst):
        src = os.path.abspath(src)
        if os.path.isdir(src):
            cwd = os.getcwd()
            os.chdir(src)
            rv = os.system("rsync -rz -e ssh . %s:%s %s" % \
                           (self.host, dst, self.out))
            os.chdir(cwd)
        else:
            rv = os.system("rsync -z -e ssh %s %s:%s %s" % \
                           (src, self.host, dst, self.out))
        if rv:
            raise "Error transferring %s to %s:%s" % \
                  (src, self.host, dst)

    def xfer_from(self, src, dst):
        # Note that src uses rsync convention on trailing / (see manpage)
        rv = os.system("rsync -rz -e ssh %s:%s %s %s" % \
                       (self.host, src, dst, self.out))
        if rv:
            raise "Error transferring %s:%s to %s" % \
                  (self.host, src, dst)
        
    def rexec(self, cmdline):
        rv = os.system("ssh %s \"%s\" %s" % (self.host, cmdline, self.out))
        if rv:
            raise "Error remotely executing %s on %s" % \
                  (cmdline, self.host)

if __name__ == "__main___":
    h = RemoteHost("users.emulab.net")
    s = StringIO.StringIO()
    s.write("touch foo.txt\n")
    s.write("touch bar.txt\n")
    buf = s.getvalue()
    h.xfer_buf_to(buf, "/users/bnc/foo.sh")
    h.rexec("sh /users/bnc/foo.sh")
    h.rexec("ls -l")
