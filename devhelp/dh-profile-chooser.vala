using Gdk;
using Gtk;

int _dh_util_surface_scale(int scale)
{
        if (scale == 1)
                return 1;
        else
                return (int)(2.0 * (2.0 / (double)scale));
}

public class DhProfileChooser : Box {

    ToggleButton drag_button;
    ToggleButton default_button;
    string cur_docset_id;
    private string[] group_ids;
    private string[] group_lists;
    private ToggleButton[] buttons;
    private Box toolbar;
    bool handling_toggle;
    CssProvider css;
    public signal void group_selected(string id, string comma_separated_docs);

    public DhProfileChooser() {
        css = new CssProvider();
        css.load_from_data("* { padding: 2pt 2pt; }");
        toolbar = new Box(Orientation.HORIZONTAL, 0);
        toolbar.pack_end(new Label("Drag to group..."));
        this.set_hexpand(true);
        toolbar.set_hexpand(true);
        toolbar.drag_motion.connect(this.on_drag_motion);
        toolbar.drag_data_received.connect(this.on_drag_data_received);
        toolbar.drag_drop.connect(this.on_drag_drop);
        toolbar.drag_leave.connect(this.on_drag_leave);
        TargetEntry list_targets[] = {TargetEntry(){
            target="zevdocs-docs-with-b64-icon",
            flags=TargetFlags.SAME_APP,
            info=1
        }};
        drag_dest_set(
            toolbar,
            DestDefaults.HIGHLIGHT,
            list_targets,
            DragAction.LINK
        );
        this.pack_end(toolbar);
        this.show_all();
        load_groups();
    }

    void bind_toggle_handler(ToggleButton btn, int i) {
        btn.toggled.connect(() => {
            if (handling_toggle) return;
            handling_toggle = true;
            for (int j = 0; j < buttons.length; ++j) {
                if (i == j) {
                    if (buttons[i].get_active()) {
                        this.group_selected(group_ids[i], group_lists[i]);
                    } else {
                        this.group_selected("*", "*");
                    }
                } else {
                    buttons[j].set_active(false);
                }
            }
            handling_toggle = false;
        });
    }

    void load_groups() {
        Soup.Session session = new Soup.Session();
        Soup.Request req = session.request("http://localhost:12340/group");
        GLib.InputStream stream = req.send();
        GLib.DataInputStream data_stream = new GLib.DataInputStream(stream);
        Json.Node line_node = Json.from_string(data_stream.read_line()); 
        Json.Array array = line_node.get_array();
        if (array.get_length() == 0) {
            default_button = new ToggleButton();
            default_button.set_label(_("drag&drop here to group"));
            this.add(default_button);
            default_button.set_sensitive(false);
        } else {
            if (default_button != null) {
                this.remove(default_button);
                default_button.destroy();
                default_button = null;
            }
        }
        for (int i = 0; i < buttons.length; ++i) {
            this.remove(buttons[i]);
            buttons[i].destroy();
        }
        buttons = new ToggleButton[0];
        group_ids = new string[0];
        group_lists = new string[0];
        for (int i = 0; i < array.get_length(); ++i) {
            Json.Object obj = array.get_element(i).get_object();
            string icon = obj.get_string_member("Icon");
            string name = obj.get_string_member("Name");
            ToggleButton btn = new ToggleButton();
            btn.set_relief(ReliefStyle.NONE);
            buttons += btn;
            group_ids += obj.get_string_member("Id");
            group_lists += obj.get_string_member("DocsList");
            if (icon.length == 1) {
                btn.set_label(icon);
            } else {
                btn.add(new Image.from_icon_name(icon, IconSize.LARGE_TOOLBAR));
            }
            bind_toggle_handler(btn, i);

            this.make_btn_on_drag_motion(btn);
            this.make_btn_on_drag_data_received(btn);
            this.make_btn_on_drag_drop(btn);
            this.make_btn_on_drag_leave(btn);
            TargetEntry list_targets[] = {TargetEntry(){
                target="zevdocs-docs-with-b64-icon",
                flags=TargetFlags.SAME_APP,
                info=1
            }};
            drag_dest_set(
                btn,
                DestDefaults.HIGHLIGHT,
                list_targets,
                DragAction.LINK
            );
            btn.get_style_context().add_provider(css, STYLE_PROVIDER_PRIORITY_APPLICATION);            this.add(btn);
            btn.show_all();
        }
    }

    bool on_drag_motion(DragContext context, int x, int y, uint time) {
        drag_get_data(toolbar, context, Atom.intern("zevdocs-docs-with-b64-icon", false), time);
        return true;
    }

    void on_drag_data_received(DragContext context, int x, int y, SelectionData data, uint info, uint time) {
        if (data.get_data() == null)
            return;
        drag_status(context, DragAction.LINK, time);
        if (drag_button == null) {
            string data_string = (string) data.get_data();
            string[] splitted = data_string.split(";", 2);
            cur_docset_id = splitted[0];
            uchar[] decoded = Base64.decode(splitted[1]);
            MemoryInputStream istream = new MemoryInputStream.from_data(decoded);
            Pixbuf pixbuf = new Pixbuf.from_stream(istream);
            Cairo.Surface surface = cairo_surface_create_from_pixbuf(
                pixbuf, _dh_util_surface_scale(this.get_scale_factor()), null
            );
            Image *image = new Image.from_surface(surface);
            drag_button = new ToggleButton();
            drag_button.add(image);
            drag_button.set_relief(ReliefStyle.NONE);
            toolbar.add(drag_button);
            drag_button.show_all();
            if (default_button != null)
                default_button.hide();
            drag_highlight(toolbar);
        }
    }

    bool on_drag_drop(DragContext context, int x, int y, uint time) {
        DhGroupDialog dialog = new DhGroupDialog(cur_docset_id);
        Gtk.Window parent_window = (Gtk.Window) this.get_toplevel();
        dialog.set_transient_for(parent_window);
        if (dialog.run() == ResponseType.OK) {
                string current_text = dialog.get_current_text();
                string current_icon = dialog.get_current_icon();
                load_groups();
        }
        dialog.destroy();
        cur_docset_id = null;
        return true;
    }

    void on_drag_leave(DragContext context, uint time) {
        drag_unhighlight(toolbar);
        if (drag_button != null) {
            drag_button.destroy();
        }
        drag_button = null;
    }


    void make_btn_on_drag_motion(ToggleButton btn) {
        btn.drag_motion.connect((context, x, y, time) => {
            drag_get_data(btn, context, Atom.intern("zevdocs-docs-with-b64-icon", false), time);
            return true;
        });
    }

    void make_btn_on_drag_data_received(ToggleButton btn) {
        btn.drag_data_received.connect((context, x, y, data, info, time) => {
            if (data.get_data() == null)
                return;
            drag_status(context, DragAction.LINK, time);
            string data_string = (string) data.get_data();
            string[] splitted = data_string.split(";", 2);
            cur_docset_id = splitted[0];

            drag_highlight(btn);
        });
    }

    void make_btn_on_drag_drop(ToggleButton btn) {
        btn.drag_drop.connect((context, x, y, time) => {
            print("Add %s\n", cur_docset_id);
            return true;
        });
    }

    void make_btn_on_drag_leave(ToggleButton btn) {
        btn.drag_leave.connect((context, time) => {
            drag_unhighlight(btn);
        });
    }
}