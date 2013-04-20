import pygtk
import gtk
import sys

import re

class Snapshot_Dag_Viewer(object):
    def __init__(self, path):
        self.window = gtk.Window(gtk.WINDOW_TOPLEVEL)
        self.window.connect("delete_event", self.delete_event)
        self.layout = gtk.VBox()
        self.window.add(self.layout)
        self.ts = gtk.TreeStore(str, str)
        self.tv = gtk.TreeView(self.ts)
        # "intree column"
        self.tvcol = gtk.TreeViewColumn("Snapshot")
        self.cell = gtk.CellRendererText()
        self.tvcol.pack_start(self.cell)
        self.tvcol.add_attribute(self.cell, "text", 0)
        self.tv.append_column(self.tvcol)
        # "marked column"
        self.tvcol = gtk.TreeViewColumn("Marked")
        self.cell = gtk.CellRendererText()
        self.tvcol.pack_start(self.cell)
        self.tvcol.add_attribute(self.cell, "text", 1)
        self.tv.append_column(self.tvcol)
        # done columns
        self.scrw = gtk.ScrolledWindow()
        self.layout.add(self.scrw)
        self.scrw.add(self.tv)
        # status bar
        self.sb = gtk.Statusbar()
        self.layout.pack_end(self.sb, False, True, 0)
        cid = self.sb.get_context_id("Hallo.")
        self.window.show_all()
        numitems = self.load_file(path)
        self.sb.push(cid, "%d snapshots"%(numitems))
    def load_file(self,path):
        count = 0
        f = open(path, "r")
        def create_append_list(l):
            a = re.match("<(.+)\|(.+)>", l)
            return [a.group(1), a.group(2)]
        cur = None
        lastdepth = 0
        for _l in f:
            count = count + 1
            if _l.strip() == "":
                continue
            l = _l.strip("\n")
            if not l.startswith(" "):
                cur = self.ts.append(None, create_append_list(l.strip()))
                lastdepth = 0
            else:
                i = 0
                while(l[i]==" "):
                    i = i + 1
                if (lastdepth > i):
                    for _ in xrange(lastdepth - i):
                        cur = self.ts.iter_parent(cur)
                cur = self.ts.append(cur, create_append_list(l.strip()))
                lastdepth = i+1
        f.close()
        return count
    def delete_event(self, w, e, d=None):
        gtk.main_quit()
        return False

def main():
    gtk.main()

if __name__ == "__main__":
    dv = Snapshot_Dag_Viewer(sys.argv[1])
    main()
