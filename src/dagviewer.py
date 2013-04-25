import pygtk
import gtk
import sys

import re

class Snapshot_Dag_Viewer(object):
    def on_toggle(self, cell, path, model, *ignore):
        if path is not None:
            it = model.get_iter(path)
            model[it][0] = not model[it][0]
            parent = model.iter_parent(it)
            # collect raw probabilities
            first_sibling = model.iter_children(parent)
            newprobs = []
            while(first_sibling!=None):
                current = model[first_sibling]
                if current[0]:
                    newprobs.append(float(current[4]))
                first_sibling = model.iter_next(first_sibling)
            print newprobs
            sss = sum(newprobs)
            newprobs = [x/sss for x in newprobs]
            # compute adjusted probabilities
            first_sibling = model.iter_children(parent)
            i = 0
            while(first_sibling!=None):
                current = model[first_sibling]
                if current[0]:
                    current[5] = newprobs[i]
                    i = i + 1
                else:
                    current[5] = 0
                first_sibling = model.iter_next(first_sibling)
    def __init__(self, path):
        self.window = gtk.Window(gtk.WINDOW_TOPLEVEL)
        self.window.set_size_request(600,400)
        self.window.connect("delete_event", self.delete_event)
        self.layout = gtk.VBox()
        self.window.add(self.layout)
        self.ts = gtk.TreeStore(bool, str, str, str, str, str)
        self.tv = gtk.TreeView(self.ts)
        # "valid column"
        self.cell = gtk.CellRendererToggle()
        self.tvcol = gtk.TreeViewColumn("U", self.cell, active=0)
        self.cell.connect("toggled", self.on_toggle, self.ts)
        self.tvcol.pack_start(self.cell)
        self.tv.append_column(self.tvcol)
        # "intree column"
        self.tvcol = gtk.TreeViewColumn("Snapshot")
        self.cell = gtk.CellRendererText()
        self.tvcol.pack_start(self.cell)
        self.tvcol.add_attribute(self.cell, "text", 1)
        self.tv.append_column(self.tvcol)
        # "marked column"
        self.tvcol = gtk.TreeViewColumn("Marked")
        self.cell = gtk.CellRendererText()
        self.tvcol.pack_start(self.cell)
        self.tvcol.add_attribute(self.cell, "text", 2)
        self.tv.append_column(self.tvcol)
        # "runtime column"
        self.tvcol = gtk.TreeViewColumn("Runtime")
        self.cell = gtk.CellRendererText()
        self.tvcol.pack_start(self.cell)
        self.tvcol.add_attribute(self.cell, "text", 3)
        self.tv.append_column(self.tvcol)
        # "probability column"
        self.tvcol = gtk.TreeViewColumn("Probability")
        self.cell = gtk.CellRendererText()
        self.tvcol.pack_start(self.cell)
        self.tvcol.add_attribute(self.cell, "text", 4)
        self.tv.append_column(self.tvcol)
        # "adjusted Probability column"
        self.tvcol = gtk.TreeViewColumn("Adj. Prob.")
        self.cell = gtk.CellRendererText()
        self.tvcol.pack_start(self.cell)
        self.tvcol.add_attribute(self.cell, "text", 5)
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
            a = re.match("<(.+)\|(.+)> (.*?) (.*?)$", l)
            return [True, a.group(1), a.group(2), a.group(3), a.group(4), a.group(4)]
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
