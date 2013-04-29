import pygtk
import gtk
import sys
import math
import cairo

from collections import deque

import re

class Plot (gtk.DrawingArea):
    def __init__ (self):
        gtk.DrawingArea.__init__(self)
        self.connect ("expose_event", self.expose)
        self.connect ("size-allocate", self.size_allocate)
        self._surface = None
        self._options = None
        self.data = ("[3, 2, 1, 0] [4, 1, 0] [5,  0]", "[4,3]")
    def set_options (self, options):
        """Set plot's options"""
        self._options = options
    def expose (self, widget, event):
        context = widget.window.cairo_create ()
        context.clip ()
        self.draw (context)
        return False
    def draw (self, context):
        rect = self.get_allocation()
        self._surface = cairo.ImageSurface(cairo.FORMAT_ARGB32, \
                rect.width, rect.height)
        self.plot (context)
        context.paint ()
    def size_allocate (self, widget, requisition):
        self.queue_draw ()
    def set_data (self, data):
        self.data = data
        self.queue_draw ()
    def plot (self, context):
        """Initializes chart (if needed), set data and plots."""
        chains = re.findall("\[(.*?)\]", self.data[0])
        chains = [[y.strip() for y in x.split(",")] for x in chains] 
        maxdepth = max([len(c) for c in chains])
        maxwidth = len(chains)
        levels = {}
        successor = {}
        for chain in chains:
            for i in xrange(len(chain)-1, -1, -1):
                levels[chain[i]] = len(chain)-i
                if i!=len(chain)-1:
                    successor[chain[i]] = chain[i + 1]
        gc = self.style.fg_gc[gtk.STATE_NORMAL]
        context.set_source_rgb(0.5, .7, .3)
        positions = {"0": (10, 10)}
        # collect nodes by levels
        leveltasks = {}
        for k, l in levels.items():
            if l not in leveltasks:
                leveltasks[l] = []
            leveltasks[l].append(k)
        for l, _lt in sorted(leveltasks.items(), reverse=True):
            xx = 10
            def my_key(x):
                if x in successor:
                    return my_key(successor[x]) + (x, )
                return (x, )
            lt = sorted(_lt, key=my_key)
            for k in lt:
                print "marked: " + str([x.strip() for x in self.data[1].strip()[1:-1].split(",")])
                if(k.strip() in [x.strip() for x in self.data[1].strip()[1:-1].split(",")]):
                    color = self.get_colormap().alloc(0xFFFF, 0x0000, 0x0000)
                    gc.set_foreground(color)
                else:
                    color = self.get_colormap().alloc(0x0000, 0x0000, 0x0000)
                    gc.set_foreground(color)
                self.window.draw_arc(gc,True,xx-5,(l)*20-5,10,10,0,100000)
                positions[k] = (xx, l*20)
                xx = xx + 30
                color = self.get_colormap().alloc(0x0000, 0x0000, 0x0000)
                gc.set_foreground(color)
            print l, lt
        for ss,tt in successor.items():
            x1, y1 = positions[ss]
            x2, y2 = positions[tt]
            self.window.draw_line(gc, x1, y1, x2, y2)
        queue = deque(["0"])
        self.set_size_request(200, len(leveltasks)*30)

class Snapshot_Dag_Viewer(object):
    def on_row_activated(self, tv, model, *ignore):
        sel = tv.get_selection()
        mod, it = sel.get_selected()
        self.plot.set_data((mod[it][1], mod[it][2]))
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
            while(parent!=None):
                adj_rt = 1./(1+model[parent][2].count(","))
                first_sibling = model.iter_children(parent)
                while(first_sibling!=None):
                    current = model[first_sibling]
                    adj_rt = adj_rt + float(current[5]) * float(current[6])
                    first_sibling = model.iter_next(first_sibling)
                model[parent][6] = adj_rt
                parent = model.iter_parent(parent)
    def __init__(self, path):
        self.window = gtk.Window(gtk.WINDOW_TOPLEVEL)
        self.window.set_size_request(700,400)
        self.window.connect("delete_event", self.delete_event)
        self.layout = gtk.VBox()
        self.window.add(self.layout)
        self.ts = gtk.TreeStore(bool, str, str, str, str, str, str)
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
        # "adjusted runtime column"
        self.tvcol = gtk.TreeViewColumn("Adj. RT")
        self.cell = gtk.CellRendererText()
        self.tvcol.pack_start(self.cell)
        self.tvcol.add_attribute(self.cell, "text", 6)
        self.tv.append_column(self.tvcol)
        # done columns
        self.scrw = gtk.ScrolledWindow()
        self.layout.add(self.scrw)
        self.scrw.add(self.tv)
        self.tv.connect("cursor-changed", self.on_row_activated, self.ts, self.tv)
        # drawing area
        self.plot = Plot()
        self.plot.set_size_request(200,100)
        self.layout.pack_start(self.plot, False, False, 0)
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
            return [True, a.group(1), a.group(2), a.group(3), a.group(4), a.group(4), a.group(3)]
        cur = None
        lastdepth = 0
        all_lines = f.readlines()
        lcount = 0
        perc = 0.
        for _l in all_lines:
            lcount = lcount + 1
            if (float(lcount)/len(all_lines)) > perc + 0.005:
                print perc
                perc = perc + 0.005
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
