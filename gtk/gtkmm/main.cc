#include <gtkmm.h>

class MainWindow : public Gtk::Window {
    Gtk::Box container_;

    Gtk::Label label_;
    Gtk::Button button_;
    Gtk::Button move_button_;

    void on_close() { this->close(); }

    void move_window() {
        int x;
        int y;
        this->get_position(x, y);
        this->move(x, y + 10);
    }

public:
    MainWindow()
        : label_("Hello, world!"), button_("Click me!"),
          move_button_("Move this window") {
        set_title("Sample App");
        set_default_size(400, 300);

        set_gravity(Gdk::GRAVITY_NORTH);

        container_.set_orientation(Gtk::Orientation::ORIENTATION_VERTICAL);
        container_.set_margin_start(10);
        container_.set_margin_end(10);
        container_.add(label_);
        container_.add(button_);
        container_.add(move_button_);

        // Why don't we use C++ lambda instead?

        button_.signal_clicked().connect(
            sigc::mem_fun(this, &MainWindow::on_close));

        // On Wayland, this doesn't work.
        move_button_.signal_clicked().connect(
            sigc::mem_fun(this, &MainWindow::move_window));

        add(container_);
        show_all_children();
    }
};

auto main(int argc, char **argv) -> int {
    auto app = Gtk::Application::create("org.kofuk.s.gtkmm.Sample");

    MainWindow window;
    return app->run(window, argc, argv);
}
