import pygtk
import gtk
import sys

class Snapshot_Dag_Viewer(object):
    def __init__(self, path):
        self.window = gtk.Window(gtk.WINDOW_TOPLEVEL)
        self.window.connect("delete_event", self.delete_event)
        self.ts = gtk.TreeStore(str)
        self.tv = gtk.TreeView(self.ts)
        self.tvcol = gtk.TreeViewColumn("Snapshot")
        self.cell = gtk.CellRendererText()
        self.tvcol.pack_start(self.cell)
        self.tvcol.add_attribute(self.cell, "text", 0)
        self.tv.append_column(self.tvcol)
        self.window.add(self.tv)
        self.window.show_all()
        self.load_file(path)
    def load_file(self,path):
        f = open(path, "r")
        cur = None
        lastdepth = 0
        for _l in f:
            if _l.strip() == "":
                continue
            l = _l.strip("\n")
            if not l.startswith(" "):
                cur = self.ts.append(None, [l.strip()])
                lastdepth = 0
            else:
                print l
                i = 0
                while(l[i]==" "):
                    i = i + 1
                print "%d spaces"%i
                if (lastdepth > i):
                    for _ in xrange(lastdepth - i):
                        cur = self.ts.iter_parent(cur)
                        print "parenting from " + str(cur)
                cur = self.ts.append(cur, [l.strip()])
                lastdepth = i+1
        f.close()
    def delete_event(self, w, e, d=None):
        gtk.main_quit()
        return False

def main():
    gtk.main()

if __name__ == "__main__":
    dv = Snapshot_Dag_Viewer(sys.argv[1])
    main()
