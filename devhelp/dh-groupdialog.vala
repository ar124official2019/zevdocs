using Gtk;

[GtkTemplate (ui="/org/gnome/devhelp/dh-groupdialog.ui")]
public class DhGroupDialog : Dialog {
    [GtkChild]
    private FlowBox icons_flow_box;

    [GtkChild]
    private ScrolledWindow icons_scrolled_window;

    private Button name_button;

    private string current_text;
    private string current_letter;
    private string group_id;
    private string docset_id;

    public DhGroupDialog (string docset_id) {
        foreach (string s in IconTheme.get_default().list_icons("Categories")) {
            Image image = new Image.from_icon_name(s, IconSize.LARGE_TOOLBAR);
            this.icons_flow_box.add(image);
            image.show();
        }

        CssProvider css = new CssProvider();
        css.load_from_data("* { padding: 2pt 2pt; }");
        Box bbox = new Box(Orientation.HORIZONTAL, 1);
        this.name_button = new Button();
        bbox.add(this.name_button);
        bbox.set_halign(Align.CENTER);
        this.name_button.set_label("A");
        this.name_button.clicked.connect(this.name_button_clicked);
        bbox.show_all();
        this.name_button.get_style_context().add_provider(css, STYLE_PROVIDER_PRIORITY_APPLICATION);
        this.icons_flow_box.insert(bbox, 0);
        this.current_text = "";
        this.current_letter = "A";
        this.docset_id = docset_id;
    }

    private void name_button_clicked () {
        this.icons_flow_box.select_child(
            this.icons_flow_box.get_child_at_index(0)
        );
    }

    [GtkCallback]
    private void text_changed (Editable widget) {
        this.current_text = widget.get_chars();
        if (current_text != "") {
            current_letter = current_text.slice(0, 1);
            this.name_button.set_label(current_letter);
        }
    }

    [GtkCallback]
    private void save_clicked () {
        Soup.Session session = new Soup.Session();
        Soup.Message msg = new Soup.Message("POST", "http://localhost:12340/group");
        Json.Object object = new Json.Object();
        object.set_string_member("Icon", this.get_current_icon());
        object.set_string_member("Name", this.get_current_text());
        Json.Node node = new Json.Node(Json.NodeType.OBJECT);
        node.set_object(object);
        msg.set_request(
            "application/json",
            Soup.MemoryUse.COPY,
            (uint8[])Json.to_string(node, false).to_utf8()
        );
        GLib.InputStream result = session.send(msg, null);
        GLib.DataInputStream result_data = new GLib.DataInputStream(result);
        group_id = result_data.read_line();

        msg = new Soup.Message("POST", "http://localhost:12340/group/" + group_id + "/doc");
        msg.set_request(
            "text/plain",
            Soup.MemoryUse.COPY,
            (uint8[])this.docset_id.to_utf8()
        );
        session.send(msg);

        this.response(ResponseType.OK);
    }

    [GtkCallback]
    private void cancel_clicked () {
        this.response(ResponseType.CANCEL);
    }

    public string get_current_text() {
        return current_text;
    }

    public string get_group_id() {
        return group_id;
    }

    public string get_current_icon() {
        FlowBoxChild fb_child = this.icons_flow_box.get_selected_children().first().data;
        if (fb_child.get_index() == 0) {
            return current_letter;
        }
        Image image = (Image)(fb_child.get_child());
        IconSize size;
        string icon;
        image.get_icon_name(out icon, out size);
        return icon;
    }
}
